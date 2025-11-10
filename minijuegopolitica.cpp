#include "minijuegopolitica.h"

#include <QGraphicsView>
#include <QGraphicsScene>
#include <QGraphicsPixmapItem>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QButtonGroup>
#include <QRadioButton>
#include <QKeyEvent>
#include <QMessageBox>
#include <QTimer>
#include <QPixmap>
#include <QStringList>
#include <algorithm>

#include "MazeLoader.h"

// =================== RUTAS ===================
// Acepta QString y devuelve la ruta absoluta al asset
static QString ASSETS(const QString& rel)
{
    static const QString kBase =
        "C:/Users/Maria Gabriela/OneDrive/Documents/ProyectoEstru2MariaRodriguez/Assets/";
    return kBase + rel;
}

// =================== PREGUNTAS ===================
struct SimpleQ {
    QString text;
    QStringList opts;
    int correct = 0;
};

static const QVector<SimpleQ> kBank = {
    { "Durante el Renacimiento, el modelo de gobierno es uno de los siguientes:",
     { "A) Monarquía absoluta.", "B) Tiranía republicana.",
      "C) Democracia participativa.", "D) Liberalismo político." }, 0 },
    { "De los siguientes acontecimientos, seleccione el que inicia el período moderno:",
     { "A) Toma de Constantinopla.", "B) Tratado de paz de Westfalia.",
      "C) Toma de la Bastilla.", "D) La ruta de la seda." }, 1 },
    { "Durante el siglo XV, la sociedad se estratifica en tres estamentos definidos:",
     { "A) Clase media, baja y alta.", "B) Nobleza, clero y estado llano.",
      "C) Artesanos, guardianes y gobernantes.", "D) Todas las opciones." }, 1 },
    { "Aparece el realismo político, basado en un orden establecido y recomendaciones de cómo gobernar:",
     { "A) Tomás Moro.", "B) Jean Bodín.", "C) Nicolás Maquiavelo.", "D) Erasmo de Rotterdam." }, 2 },
    { "Terminada la Edad Media, en el contexto de la política resulta que:",
     { "A) La Iglesia resalta su poder.", "B) La Iglesia pierde el papel rector en la política.",
      "C) La Iglesia evangélica se posiciona.", "D) La política desaparece." }, 1 },
    };

// =================== AUX: carga/escala ===================
static QPixmap loadScaled(const QString& path, int px, const QColor fallback = QColor(60,60,60))
{
    QPixmap pm(path);
    if (pm.isNull()) { QPixmap ph(px, px); ph.fill(fallback); return ph; }
    return pm.scaled(px, px, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
}

// =================== IMPLEMENTACIÓN ===================
minijuegopolitica::minijuegopolitica(QWidget *parent)
    : QWidget(parent)
{
    setFocusPolicy(Qt::StrongFocus);

    auto *root = new QVBoxLayout(this);
    root->setContentsMargins(0,0,0,0);
    root->setSpacing(0);

    // Panel de preguntas
    quizPanel_ = new QWidget(this);
    quizPanel_->setObjectName("quizPanel");
    quizPanel_->setStyleSheet(
        "#quizPanel { background: rgba(10,10,10,200); border-bottom: 1px solid #777; } "
        "QLabel { color:#f0f0f0; font: 12pt 'Segoe UI'; } "
        "QRadioButton { color:#f0f0f0; font: 10pt 'Segoe UI'; } "
        "QPushButton { background:#2e5a7a; color:#fff; padding:6px 12px; border-radius:6px; } "
        "QPushButton:disabled { background:#555; } ");

    auto *pv = new QVBoxLayout(quizPanel_);
    pv->setContentsMargins(16,10,16,10);
    pv->setSpacing(6);

    quizLabel_ = new QLabel("Pregunta", quizPanel_);
    quizLabel_->setWordWrap(true);
    pv->addWidget(quizLabel_);

    quizGroup_ = new QButtonGroup(quizPanel_);
    for (int i=0;i<4;++i) {
        opt_[i] = new QRadioButton(QString("Opción %1").arg(i+1), quizPanel_);
        quizGroup_->addButton(opt_[i], i);
        pv->addWidget(opt_[i]);
    }
    auto *row = new QHBoxLayout();
    row->addStretch();
    answerBtn_ = new QPushButton("Responder", quizPanel_);
    row->addWidget(answerBtn_);
    pv->addLayout(row);

    connect(answerBtn_, &QPushButton::clicked, this, &minijuegopolitica::onAnswerClicked);

    quizPanel_->setFixedHeight(140);
    quizPanel_->hide();
    root->addWidget(quizPanel_);

    // View + Scene
    view_  = new QGraphicsView(this);
    scene_ = new QGraphicsScene(this);
    view_->setScene(scene_);
    view_->setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform);
    view_->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    view_->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    view_->setViewportUpdateMode(QGraphicsView::FullViewportUpdate);
    root->addWidget(view_, 1);

    connect(&animTimer_, &QTimer::timeout, this, &minijuegopolitica::tickAnim);
    animTimer_.start(100);

    // Spritesheet
    puffleSheet_.load(ASSETS("sprites/puffle_pink_walk_4x4_48.png"));

    setMinimumSize(1000, 700);
    resize(1200, 800);

    // -------- Aviso flotante (toast) transparente --------
    toast_ = new QLabel(this);
    toast_->setVisible(false);
    toast_->setAttribute(Qt::WA_TransparentForMouseEvents, true);
    toast_->setAlignment(Qt::AlignCenter);
    toast_->setStyleSheet("background: rgba(0,0,0,0); color: black; "
                          "font: 16pt 'Segoe UI'; font-weight: 700; padding: 8px;");

    loadCSV(ASSETS("mazes/maze_with_5_doors.csv"));
}

bool minijuegopolitica::loadCSV(const QString& absPath)
{
    try {
        mz_ = loadMazeCSV(absPath.toStdString());
    } catch (...) { return false; }

    p_r_ = mz_.sr; p_c_ = mz_.sc;
    animDir_ = 0; animFrame_ = 0;
    moving_ = false;
    visitedMask_ = 0;
    pendingDoorIndex_ = -1;

    // Calcula un tamaño de tile inicial razonable basado en el view
    if (view_->width() > 0 && view_->height() > 0) {
        int cand = std::min(view_->width()  / std::max(1, mz_.w),
                            view_->height() / std::max(1, mz_.h));
        tile = std::clamp(cand, 32, 96);
    } else {
        tile = 64;
    }

    buildScene(false);
    return true;
}

void minijuegopolitica::buildScene(bool /*fitViewNow*/)
{
    // Tiles
    pxFloor      = loadScaled(ASSETS("tiles/tile_floor.png"), tile);
    pxWall       = loadScaled(ASSETS("tiles/tile_wall.png"), tile);
    pxStart      = loadScaled(ASSETS("tiles/tile_start.png"), tile);
    pxExit       = loadScaled(ASSETS("tiles/tile_exit.png"), tile);
    pxExitLocked = loadScaled(ASSETS("tiles/tile_exit_locked.png"), tile);
    for (int i=0;i<5;++i)
        pxDoor[i] = loadScaled(ASSETS(QString("tiles/tile_door%1.png").arg(i+1)), tile);

    scene_->clear();

    // Dibuja el grid
    for (int r=0; r<mz_.h; ++r) {
        for (int c=0; c<mz_.w; ++c) {
            const Cell& cell = mz_.at(r,c);
            QPixmap pix;
            switch (cell.type) {
            case TileType::Wall:  pix = pxWall;  break;
            case TileType::Floor: pix = pxFloor; break;
            case TileType::Start: pix = pxStart; break;
            case TileType::Exit:
                pix = (visitedMask_ == kAllVisitedMask) ? pxExit : pxExitLocked; break;
            case TileType::Door:
                pix = pxDoor[ std::clamp(cell.doorIndex,0,4) ]; break;
            }
            auto *it = scene_->addPixmap(pix);
            it->setPos(c*tile, r*tile);
            it->setZValue(0);
        }
    }

    // Tras clear(), recreamos el puffle y lo ponemos arriba
    puffleItem_ = scene_->addPixmap(QPixmap(tile, tile));
    puffleItem_->setPos(p_c_*tile, p_r_*tile);
    puffleItem_->setZValue(10); // siempre por encima

    // Frame inicial
    if (!puffleSheet_.isNull()) {
        const int frameW = 48, frameH = 48;
        QPixmap frame = puffleSheet_.copy(0, 0, frameW, frameH)
                            .scaled(tile, tile, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
        puffleItem_->setPixmap(frame);
    } else {
        QPixmap ph(tile,tile); ph.fill(Qt::white);
        puffleItem_->setPixmap(ph);
    }

    scene_->setSceneRect(0, 0, mz_.w*tile, mz_.h*tile);
    view_->resetTransform();
    view_->centerOn(scene_->sceneRect().center());
}

void minijuegopolitica::resizeEvent(QResizeEvent *ev)
{
    QWidget::resizeEvent(ev);

    if (mz_.w <= 0 || mz_.h <= 0) {
        if (toast_) toast_->setGeometry(0, quizPanel_ ? quizPanel_->height() + 8 : 8, width(), 48);
        return;
    }

    int cand = std::min(view_->width()  / std::max(1, mz_.w),
                        view_->height() / std::max(1, mz_.h));
    cand = std::clamp(cand, 32, 96);

    if (cand != tile) {
        tile = cand;
        buildScene(false);
    } else {
        view_->resetTransform();
        view_->centerOn(scene_->sceneRect().center());
    }

    // Colocar el toast justo debajo del panel de preguntas
    if (toast_)
        toast_->setGeometry(0, quizPanel_ ? quizPanel_->height() + 8 : 8, width(), 48);
}

bool minijuegopolitica::canWalk(int nr, int nc) const
{
    if (nr<0 || nc<0 || nr>=mz_.h || nc>=mz_.w) return false;
    const Cell& cell = mz_.at(nr,nc);
    if (cell.type == TileType::Wall) return false;
    if (cell.type == TileType::Exit) return visitedMask_ == kAllVisitedMask;
    return true;
}

void minijuegopolitica::unlockDoorBit(int doorIndex)
{
    if (doorIndex>=0 && doorIndex<kDoorCount) visitedMask_ |= (1u << doorIndex);
}

void minijuegopolitica::onEnter(int r, int c)
{
    const Cell& cell = mz_.at(r,c);

    if (cell.type == TileType::Door && cell.doorIndex>=0) {
        if ((visitedMask_ & (1u<<cell.doorIndex)) == 0) {
            showDoorQuestion(cell.doorIndex);
            return;
        }
    }

    if (cell.type == TileType::Exit && visitedMask_ == kAllVisitedMask) {
        emit juegoCompletado(true);
        close();
    }
}

void minijuegopolitica::setDirection(int dirIndex)
{
    animDir_ = std::clamp(dirIndex, 0, 3);
}

void minijuegopolitica::keyPressEvent(QKeyEvent *e)
{
    if (quizPanel_ && quizPanel_->isVisible()) {
        QWidget::keyPressEvent(e);
        return;
    }

    int nr = p_r_, nc = p_c_;
    bool moved = false;

    switch (e->key()) {
    case Qt::Key_W:
    case Qt::Key_Up:    nr--; setDirection(3); moved = true; break;
    case Qt::Key_S:
    case Qt::Key_Down:  nr++; setDirection(0); moved = true; break;
    case Qt::Key_A:
    case Qt::Key_Left:  nc--; setDirection(1); moved = true; break;
    case Qt::Key_D:
    case Qt::Key_Right: nc++; setDirection(2); moved = true; break;
    default:
        QWidget::keyPressEvent(e);
        return;
    }

    if (moved && (nr!=p_r_ || nc!=p_c_) && canWalk(nr, nc)) {
        p_r_ = nr; p_c_ = nc;
        puffleItem_->setPos(p_c_*tile, p_r_*tile);
        onEnter(p_r_, p_c_);
        moving_ = true;
    } else {
        moving_ = false;
    }
}

void minijuegopolitica::tickAnim()
{
    if (puffleSheet_.isNull() || !puffleItem_) return;

    const int frameW = 48, frameH = 48;

    if (!moving_) {
        QPixmap frame = puffleSheet_.copy(0, animDir_*frameH, frameW, frameH)
        .scaled(tile, tile, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
        puffleItem_->setPixmap(frame);
        return;
    }

    animFrame_ = (animFrame_ + 1) % 4;
    QPixmap frame = puffleSheet_.copy(animFrame_*frameW, animDir_*frameH, frameW, frameH)
                        .scaled(tile, tile, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
    puffleItem_->setPixmap(frame);
    moving_ = false;
}

void minijuegopolitica::showDoorQuestion(int doorIndex)
{
    const SimpleQ &qq = kBank[doorIndex % kBank.size()];
    quizLabel_->setText(qq.text);
    for (int i=0;i<4;++i) {
        opt_[i]->setText(qq.opts.value(i));
        opt_[i]->setChecked(false);
    }
    pendingDoorIndex_ = doorIndex;
    quizPanel_->show();

    // Reubica el toast justo debajo del panel
    if (toast_) toast_->setGeometry(0, quizPanel_->height() + 8, width(), 48);
}

void minijuegopolitica::onAnswerClicked()
{
    if (pendingDoorIndex_ < 0) return;
    int sel = quizGroup_->checkedId();
    if (sel < 0) return;

    const SimpleQ &qq = kBank[pendingDoorIndex_ % kBank.size()];
    if (sel == qq.correct) {
        unlockDoorBit(pendingDoorIndex_);
        pendingDoorIndex_ = -1;
        quizPanel_->hide();
        buildScene(false);             // redibuja con puerta actualizada
        view_->resetTransform();       // asegura 1:1
        view_->centerOn(scene_->sceneRect().center());
    } else {
        // Mostrar notificación y cerrar después
        quizPanel_->hide();
        showToast("No lograste salvar al puffle", 1700);
        setEnabled(false); // bloquear interacción mientras se muestra el aviso
        QTimer::singleShot(1700, this, [this](){
            close();
        });
    }
}

// -------- Aviso flotante --------
void minijuegopolitica::showToast(const QString& msg, int ms)
{
    if (!toast_) return;
    toast_->setText(msg);
    toast_->setGeometry(0, quizPanel_ ? quizPanel_->height() + 8 : 8, width(), 48);
    toast_->raise();
    toast_->show();

    QTimer::singleShot(ms, this, [this](){
        if (toast_) toast_->hide();
    });
}

