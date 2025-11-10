#include "Laboratorio.h"
#include "personaje.h"

#include <QGuiApplication>
#include <QScreen>
#include <QVBoxLayout>
#include <QKeyEvent>
#include <QPainter>
#include <QLineF>
#include <algorithm>

// ===== Rutas =====
static const char* kLabBg =
    "C:/Users/Maria Gabriela/OneDrive/Documents/ProyectoEstru2MariaRodriguez/Assets/Laboratorio.jpg";
static const char* kDescartesPng =
    "C:/Users/Maria Gabriela/OneDrive/Documents/ProyectoEstru2MariaRodriguez/Assets/ReneDescartes.png";

// ---------------------------------------------------------

Laboratorio::Laboratorio(QWidget *parent)
    : QGraphicsView(parent)
{
    setWindowTitle("Laboratorio de Descartes");
    setRenderHint(QPainter::Antialiasing);
    setRenderHint(QPainter::SmoothPixmapTransform);
    setAlignment(Qt::AlignCenter);
    setBackgroundBrush(QColor(20,20,20));

    const QRect avail = QGuiApplication::primaryScreen()->availableGeometry();
    const double sx = (avail.width()*0.9)/BG_W;
    const double sy = (avail.height()*0.9)/BG_H;
    const double sc = std::min(1.0,std::min(sx,sy));
    resize(int(BG_W*sc), int(BG_H*sc));

    scene_ = new QGraphicsScene(this);
    setScene(scene_);
    scene_->setSceneRect(0,0,BG_W,BG_H);

    loadBackground();
    buildCharacters();
    buildMesas();
    buildQuestions();
    buildUI();

    timer_ = new QTimer(this);
    timer_->setInterval(80);
    connect(timer_, &QTimer::timeout, this, &Laboratorio::tick);
    timer_->start();

    prevSafePos_ = QPointF(BG_W/2.0 - 40, BG_H - 180);
    fitView();

    // 🔔 Estado inicial (0/5)
    emitLabProgress();
}

// --- Utilidades de imagen y vista ---
QPixmap Laboratorio::safeLoad(const QString& path, const QSize& fb)
{
    QPixmap pm;
    if (!pm.load(path)) {
        pm = QPixmap(fb);
        pm.fill(Qt::darkGray);
    }
    return pm;
}
void Laboratorio::loadBackground()
{
    QPixmap bg = safeLoad(QString::fromUtf8(kLabBg), QSize(BG_W,BG_H))
    .scaled(BG_W,BG_H,Qt::KeepAspectRatioByExpanding,Qt::SmoothTransformation);
    bgItem_ = scene_->addPixmap(bg);
    bgItem_->setZValue(-100);
}
void Laboratorio::fitView()
{
    resetTransform();
    fitInView(scene_->sceneRect(), Qt::KeepAspectRatio);
}

// --- Construcción ---
void Laboratorio::buildCharacters()
{
    QPixmap d = safeLoad(QString::fromUtf8(kDescartesPng), QSize(160,160))
    .scaled(150,150,Qt::KeepAspectRatio,Qt::SmoothTransformation);
    npcDescartes_ = scene_->addPixmap(d);
    npcDescartes_->setPos(480,140);
    npcDescartes_->setZValue(12);

    penguin_ = new Personaje();
    scene_->addItem(penguin_);
    penguin_->setPos(100, 350);
    penguin_->setZValue(20);      // delante del NPC
    penguin_->setScale(0.7);
}

void Laboratorio::buildMesas()
{
    auto addMesa = [&](const QRectF& r, const QVector<int>& qs){
        Mesa m;
        m.collider = scene_->addRect(r, QPen(Qt::NoPen), QBrush(Qt::NoBrush));
        m.qIndex = qs;
        mesas_.push_back(m);
    };

    // 2 + 2 + 1 preguntas
    addMesa(QRectF(325, 378, 64, 40), {0,1});  // mesa izquierda
    addMesa(QRectF(475, 470, 64, 40), {2,3});  // mesa centro
    addMesa(QRectF(210, 530, 64, 40), {4});    // mesa abajo-izq

    // bloque corto delante del sillón para no colarse
    Mesa bloqueNPC;
    bloqueNPC.collider = scene_->addRect(QRectF(0, 138, 800, 55),
                                         QPen(Qt::NoPen), QBrush(Qt::NoBrush));
    mesas_.push_back(bloqueNPC);
}

void Laboratorio::buildQuestions()
{
    questions_ = {
        "Son los pasos o razones de la duda metódica:",
        "Una de las siguientes es considerada de las cuatro reglas del método en Descartes:",
        "En relación con los datos experimentales que otorgan los sentidos, Descartes define:",
        "“Solo debemos aceptar como verdadero lo EVIDENTE, CLARO y DISTINTO” fue dicho por:",
        "En cuanto a la certeza del conocimiento, René Descartes afirma:"
    };
    optA_ = {"Los sentidos nos engañan","Hipótesis","Los sentidos son una fuente confiable para conocer","René Descartes","Es preciso dudar"};
    optB_ = {"La existencia del genio maligno","Deducción","Desconfianza de lo que los sentidos nos proporcionan","David Hume","Debemos confiar ciegamente"};
    optC_ = {"Imposibilidad de diferenciar vigilia del sueño","Evidencia","Los sentidos son complementarios","George Berkeley","Nada es importante"};
    optD_ = {"Todas son correctas","Inducción","Los sentidos son determinantes","Aristóteles","Todo es posible"};
    answers_ = {3,2,1,0,0}; // D, C, B, A, A
}

void Laboratorio::buildUI()
{
    // Hint
    hintLabel_ = new QLabel;
    hintLabel_->setStyleSheet(
        "background:rgba(248,229,151,220);"
        "border:2px solid #b58a3a;border-radius:10px;"
        "padding:6px 12px;color:#2b1d02;font:12pt 'Times New Roman';");
    hintProxy_ = scene_->addWidget(hintLabel_);
    hintProxy_->setZValue(1000);
    hintProxy_->setPos(16,12);
    hintProxy_->hide();

    // Panel pregunta
    qPanel_ = new QWidget();
    qPanel_->setStyleSheet("background:rgba(0,0,0,180);border:1px solid #d2b48c;");
    auto *v = new QVBoxLayout(qPanel_);
    v->setContentsMargins(14,12,14,12);
    v->setSpacing(10);

    qTitle_ = new QLabel("Pregunta");
    qTitle_->setStyleSheet("color:#ffd87a;font:bold 14pt 'Times New Roman';");
    qText_  = new QLabel("...");
    qText_->setStyleSheet("color:white;font:12pt 'Times New Roman';");
    qText_->setWordWrap(true);

    v->addWidget(qTitle_);
    v->addWidget(qText_);

    // Opciones en columna
    for (int i=0;i<4;i++) {
        auto *b = new QPushButton(QString("Opción %1").arg(i+1));
        b->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
        b->setStyleSheet(
            "QPushButton{background:#2d2d2d;color:#eee;border:1px solid #888;"
            "border-radius:6px;padding:8px;text-align:left;}"
            "QPushButton:hover{background:#3a3a3a;}");
        connect(b,&QPushButton::clicked,this,&Laboratorio::onOptionClicked);
        qOpts_.push_back(b);
        v->addWidget(b);
    }

    auto *proxy = scene_->addWidget(qPanel_);
    proxy->setZValue(1500);
    qPanel_->hide();

    reflowQuestionPanel();
}

// --- Hints / Toast ---
void Laboratorio::ensureToast()
{
    if (toastLabel_) return;
    toastLabel_ = new QLabel;
    toastLabel_->setAttribute(Qt::WA_TranslucentBackground);
    toastLabel_->setStyleSheet(
        "QLabel{background:rgba(0,0,0,180);color:#000000;border:1px solid #d2b48c;"
        "border-radius:10px;font:12pt 'Times New Roman';padding:8px;}");
    toastProxy_ = scene_->addWidget(toastLabel_);
    toastProxy_->setZValue(2000);
    toastProxy_->hide();
    toastTimer_ = new QTimer(this);
    toastTimer_->setSingleShot(true);
    connect(toastTimer_,&QTimer::timeout,[this](){toastProxy_->hide();});
}
void Laboratorio::showToast(const QString& text,int msec)
{
    ensureToast();
    toastLabel_->setText(text);
    toastLabel_->adjustSize();
    const QRectF R=scene_->sceneRect();
    toastProxy_->setPos(R.center().x()-toastLabel_->width()/2.0,
                        R.bottom()-toastLabel_->height()-20);
    toastProxy_->show();
    toastTimer_->start(msec);
}
void Laboratorio::setHintText(const QString& t){hintLabel_->setText(t);hintLabel_->adjustSize();hintProxy_->show();}
void Laboratorio::placeHintTopLeft(){hintProxy_->setPos(16,12);}

// --- Proximidad / colisiones ---
int Laboratorio::nearestMesa() const
{
    if (!penguin_) return -1;
    const QPointF hero = penguin_->sceneBoundingRect().center();
    int best=-1; qreal bestD=1e9;
    for (int i=0;i<mesas_.size();++i){
        const QPointF c = mesas_[i].collider->sceneBoundingRect().center();
        const qreal d = QLineF(hero,c).length();
        if (d<bestD){bestD=d;best=i;}
    }
    if (best>=0 && bestD<=MESA_RADIUS) return best;
    return -1;
}
QRectF Laboratorio::feetRectAt(const QPointF &topLeft) const
{
    Q_UNUSED(topLeft);
    QRectF full = penguin_->sceneBoundingRect();
    const qreal h = full.height();
    return full.adjusted(6, h*0.45, -10, -4);   // solo “pies”
}

bool Laboratorio::hitsAnyColliderAt(const QPointF &p) const
{
    const QPointF old = penguin_->pos();
    const_cast<Personaje*>(penguin_)->setPos(p);
    const QRectF feet = feetRectAt(p);
    bool hit = false;
    for (const Mesa& m : mesas_) {
        if (m.collider && feet.intersects(m.collider->sceneBoundingRect())) { hit = true; break; }
    }
    const_cast<Personaje*>(penguin_)->setPos(old);
    return hit;
}
bool Laboratorio::hitsAnyCollider() const
{
    const QRectF feet = feetRectAt(penguin_->pos());
    for (const Mesa& m : mesas_) {
        if (m.collider && feet.intersects(m.collider->sceneBoundingRect()))
            return true;
    }
    return false;
}

bool Laboratorio::nearNPC() const
{
    if (!penguin_ || !npcDescartes_) return false;
    QPointF a=penguin_->sceneBoundingRect().center();
    QPointF b=npcDescartes_->sceneBoundingRect().center();
    return (QLineF(a,b).length() <= NPC_RADIUS);
}

// --- Loop ---
void Laboratorio::tick()
{
    if (hitsAnyCollider()) { penguin_->setPos(prevSafePos_); penguin_->stopMove(); }
    else { prevSafePos_ = penguin_->pos(); }

    hintProxy_->hide();

    const int nm = nearestMesa();
    if (nm>=0){
        if (mesas_[nm].progress < mesas_[nm].qIndex.size()){
            setHintText("Pulsa E para resolver");
            placeHintTopLeft();
        }
        return;
    }
    if (nearNPC()){
        setHintText("Pulsa E para hablar");
        placeHintTopLeft();
        return;
    }
}

// --- Teclado ---
void Laboratorio::keyPressEvent(QKeyEvent *e)
{
    if (!penguin_) return;
    if (e->isAutoRepeat()){ QGraphicsView::keyPressEvent(e); return; }

    switch (e->key()){
    case Qt::Key_W: case Qt::Key_Up:    penguin_->startMove(Personaje::Direction::Up);    break;
    case Qt::Key_S: case Qt::Key_Down:  penguin_->startMove(Personaje::Direction::Down);  break;
    case Qt::Key_A: case Qt::Key_Left:  penguin_->startMove(Personaje::Direction::Left);  break;
    case Qt::Key_D: case Qt::Key_Right: penguin_->startMove(Personaje::Direction::Right); break;
    case Qt::Key_E:{
        const int nm = nearestMesa();
        if (nm>=0){ abrirPreguntaDeMesa(nm); return; }
        if (nearNPC()){ hablarConDescartes(); return; }
        break;
    }
    default: break;
    }
    QGraphicsView::keyPressEvent(e);
}
void Laboratorio::keyReleaseEvent(QKeyEvent *e)
{
    if (!penguin_ || e->isAutoRepeat()) return;
    switch (e->key()){
    case Qt::Key_W: case Qt::Key_Up:
    case Qt::Key_S: case Qt::Key_Down:
    case Qt::Key_A: case Qt::Key_Left:
    case Qt::Key_D: case Qt::Key_Right:
        penguin_->stopMove(); break;
    default: break;
    }
}
void Laboratorio::resizeEvent(QResizeEvent *e)
{
    QGraphicsView::resizeEvent(e);
    fitView();
}

// --- Preguntas ---
void Laboratorio::abrirPreguntaDeMesa(int mesaIdx)
{
    mesaActual_ = mesaIdx;
    Mesa &M = mesas_[mesaIdx];
    if (M.progress >= M.qIndex.size()) { showToast("Ya resolviste esta mesa.",1400); return; }

    const int qid = M.qIndex[M.progress];
    qTitle_->setText(QString("Mesa %1 — Pregunta").arg(mesaIdx+1));
    qText_->setText(questions_[qid]);
    qOpts_[0]->setText("A) " + optA_[qid]);
    qOpts_[1]->setText("B) " + optB_[qid]);
    qOpts_[2]->setText("C) " + optC_[qid]);
    qOpts_[3]->setText("D) " + optD_[qid]);

    reflowQuestionPanel();
    qPanel_->show();
}

bool Laboratorio::todasLasPreguntasResueltas() const
{
    return respondidas_ >= totalPreguntas_;  // simple y directo
}

void Laboratorio::onOptionClicked()
{
    if (mesaActual_<0 || mesaActual_>=mesas_.size()) return;
    Mesa &M = mesas_[mesaActual_];
    if (M.progress >= M.qIndex.size()) return;

    auto* btn = qobject_cast<QPushButton*>(sender());
    const int chosen = qOpts_.indexOf(btn);
    const int qid = M.qIndex[M.progress];
    const int correct = answers_[qid];

    if (chosen == correct){
        M.progress++;
        respondidas_++;                     //  suma global
        qPanel_->hide();

        // Progreso tras acertar
        emitLabProgress();

        if (todasLasPreguntasResueltas()){
            finalizado_ = true;
            showToast("¡Excelente! Demuestras buena capacidad de análisis.",2600);
        } else if (M.progress < M.qIndex.size()){
            showToast("¡Correcto! Continúa en esta mesa.",1600);
        } else {
            showToast("¡Correcto! Ve a otra mesa.",1600);
        }
    } else {
        qTitle_->setText("Incorrecto");
        qText_->setText("Inténtalo de nuevo.");
    }
}

void Laboratorio::hablarConDescartes()
{
    if (!finalizado_){
        showToast("Descartes: Resuelve las preguntas de las mesas para demostrar tu método.",2500);
    } else {
        showToast("Descartes: ¡Excelente! Has comprendido la duda metódica.",2500);
        QTimer::singleShot(1200, this, [this](){
            close();
        });
    }
}

void Laboratorio::reflowQuestionPanel()
{
    const QRectF R = scene_->sceneRect();
    const int margin = 18;
    const int panelW = int(R.width() * 0.86);

    qPanel_->setMinimumWidth(panelW);
    qPanel_->setMaximumWidth(panelW);
    qText_->setMaximumWidth(panelW - 40);
    qPanel_->adjustSize();

    const int x = int(R.center().x() - qPanel_->width()/2.0);
    const int y = margin;
    qPanel_->move(std::max(margin, x), y);
}

// ---------- Emisor de progreso ----------
void Laboratorio::emitLabProgress()
{
    const bool done = (respondidas_ >= totalPreguntas_);
    emit progresoLaboratorio(done, respondidas_, totalPreguntas_);
}
