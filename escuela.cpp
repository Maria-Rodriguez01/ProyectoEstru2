#include "escuela.h"

#include <QGuiApplication>
#include <QScreen>
#include <QPainter>
#include <QKeyEvent>
#include <QCloseEvent>
#include <QLineF>
#include <QVBoxLayout>
#include <QFont>

#include "personaje.h"
#include "aula.h"
#include "laboratorio.h"

// ---- rutas de assets ----
static const char* kEscuelaBg =
    "C:/Users/Maria Gabriela/OneDrive/Documents/ProyectoEstru2MariaRodriguez/Assets/escuela.jpg";

Escuela::Escuela(QWidget *parent)
    : QGraphicsView(parent)
{
    setWindowTitle("Escuela — Nivel 4");
    setFrameShape(QFrame::NoFrame);
    setRenderHint(QPainter::Antialiasing, true);
    setRenderHint(QPainter::SmoothPixmapTransform, true);
    setAlignment(Qt::AlignCenter);
    setBackgroundBrush(QColor(20,20,20));

    // Ventana cómoda (90% pantalla)
    const QRect avail = QGuiApplication::primaryScreen()->availableGeometry();
    const double sx = (avail.width()  * 0.90) / BG_W;
    const double sy = (avail.height() * 0.90) / BG_H;
    const double scale = std::min(1.0, std::min(sx, sy));
    const int  W = int(BG_W * scale);
    const int  H = int(BG_H * scale);
    setMinimumSize(W, H);
    setMaximumSize(W, H);
    resize(W, H);

    // Escena
    scene_ = new QGraphicsScene(this);
    setScene(scene_);
    scene_->setSceneRect(0,0,BG_W,BG_H);

    loadBackground();
    buildCharacters();
    buildUI();
    buildTriggers();

    // Proximidad
    timer_ = new QTimer(this);
    timer_->setInterval(90);
    connect(timer_, &QTimer::timeout, this, &Escuela::tick);
    timer_->start();

    fitView();

}

QPixmap Escuela::safeLoad(const QString& path, const QSize& fb)
{
    QPixmap pm;
    if (!pm.load(path)) {
        pm = QPixmap(fb);
        pm.fill(Qt::darkGray);
        QPainter p(&pm);
        p.setPen(Qt::white);
        p.drawText(pm.rect(), Qt::AlignCenter, "No se pudo cargar:\n" + path);
    }
    return pm;
}

void Escuela::loadBackground()
{
    QPixmap bg = safeLoad(QString::fromUtf8(kEscuelaBg), QSize(BG_W, BG_H))
    .scaled(BG_W, BG_H, Qt::KeepAspectRatioByExpanding, Qt::SmoothTransformation);
    if (bgItem_) scene_->removeItem(bgItem_);
    bgItem_ = scene_->addPixmap(bg);
    bgItem_->setZValue(-100);
    scene_->setSceneRect(bg.rect());
}

void Escuela::fitView()
{
    if (!scene_) return;
    resetTransform();
    fitInView(scene_->sceneRect(), Qt::KeepAspectRatio);
}

void Escuela::buildCharacters()
{
    // Pingüino
    penguin_ = new Personaje();
    scene_->addItem(penguin_);
    penguin_->setSceneBounds(scene_->sceneRect());
    penguin_->setScale(1.0);
    // posición inicial abajo-centro
    penguin_->setPos(BG_W/2.0 - 40, BG_H - 140);
}

void Escuela::buildUI()
{
    // Hint flotante
    hintLabel_ = new QLabel(" ");
    hintLabel_->setStyleSheet(
        "background:rgba(248,229,151,220);"
        "border:2px solid #b58a3a; border-radius:10px;"
        "padding:6px 12px; color:#2b1d02; font: 12pt 'Times New Roman';");
    hintLabel_->setAttribute(Qt::WA_TransparentForMouseEvents);
    hintProxy_ = scene_->addWidget(hintLabel_);
    hintProxy_->setZValue(1000);
    hintProxy_->hide();

}

void Escuela::buildTriggers()
{
    // Triggers (ajusta a tu fondo real)
    triggerAula_ = scene_->addRect(QRectF(30, BG_H - 200, 160, 160), QPen(Qt::NoPen), QBrush(Qt::NoBrush));
    triggerLab_  = scene_->addRect(QRectF(BG_W - 190, BG_H - 200, 160, 160), QPen(Qt::NoPen), QBrush(Qt::NoBrush));
    triggerPuertaArr_ = scene_->addRect(QRectF(BG_W/2.0 - 80, 10, 160, 70), QPen(Qt::NoPen), QBrush(Qt::NoBrush));

    triggerAula_->setZValue(5);
    triggerLab_->setZValue(5);
    triggerPuertaArr_->setZValue(5);
}

bool Escuela::near(const QGraphicsRectItem* it, qreal radiusPx) const
{
    if (!it || !penguin_) return false;
    const QPointF hero = penguin_->sceneBoundingRect().center();
    const QPointF c    = it->sceneBoundingRect().center();  // ✅ coords de escena
    return (QLineF(hero, c).length() <= radiusPx);
}


void Escuela::setHint(const QString& t, const QPointF& viewPos)
{
    hintLabel_->setText(t);
    hintLabel_->adjustSize();
    hintProxy_->setPos(viewPos);
    hintProxy_->show();
}


void Escuela::tick()
{
    if (!penguin_) return;
    hintProxy_->hide();

    // Candidatos de interacción
    if (near(triggerAula_, 110)) {
        setHint("Pulsa E para entrar a Aula");
        return;
    }
    if (near(triggerLab_, 110)) {
        setHint("Pulsa E para entrar a Laboratorio");
        return;
    }
    if (near(triggerPuertaArr_, 120)) {
        if (canExit())
            setHint("Pulsa E para salir al Mapa");
        else
            setHint("Debes completar Aula y Laboratorio para salir");
        return;
    }
}

void Escuela::abrirAula()
{
    if (aulaWin_) { aulaWin_->raise(); aulaWin_->activateWindow(); return; }
    aulaWin_ = new Aula();
    aulaWin_->setAttribute(Qt::WA_DeleteOnClose);

    // Conectamos a señales de progreso/finalización (ver cambios en Aula más abajo)
    connect(aulaWin_, SIGNAL(progresoAula(bool,int,int)),
            this,     SLOT(onAulaProgreso(bool,int,int)));

    connect(aulaWin_, &QObject::destroyed, this, [this]() {
        aulaWin_ = nullptr;  // solo referencia
    });

    aulaWin_->show();
}

void Escuela::abrirLaboratorio()
{
    if (labWin_) { labWin_->raise(); labWin_->activateWindow(); return; }
    labWin_ = new Laboratorio();
    labWin_->setAttribute(Qt::WA_DeleteOnClose);

    // Conectamos a señales de progreso/finalización (ver cambios en Laboratorio más abajo)
    connect(labWin_, SIGNAL(progresoLaboratorio(bool,int,int)),
            this,    SLOT(onLaboratorioProgreso(bool,int,int)));

    connect(labWin_, &QObject::destroyed, this, [this]() {
        labWin_ = nullptr;  // solo referencia
    });

    labWin_->show();
}

void Escuela::onAulaProgreso(bool completado, int respondidas, int total)
{
    aulaCompletada_ = completado;
    aulaResp_ = respondidas;
    aulaTotal_ = total;
}

void Escuela::onLaboratorioProgreso(bool completado, int respondidas, int total)
{
    labCompletada_ = completado;
    labResp_ = respondidas;
    labTotal_ = total;
}

void Escuela::keyPressEvent(QKeyEvent *e)
{
    if (!penguin_) { QGraphicsView::keyPressEvent(e); return; }
    if (e->isAutoRepeat()) { QGraphicsView::keyPressEvent(e); return; }

    switch (e->key()) {
    case Qt::Key_W: case Qt::Key_Up:    penguin_->startMove(Personaje::Direction::Up);    break;
    case Qt::Key_S: case Qt::Key_Down:  penguin_->startMove(Personaje::Direction::Down);  break;
    case Qt::Key_A: case Qt::Key_Left:  penguin_->startMove(Personaje::Direction::Left);  break;
    case Qt::Key_D: case Qt::Key_Right: penguin_->startMove(Personaje::Direction::Right); break;

    case Qt::Key_E: {
        if (near(triggerAula_, 120)) {
            abrirAula();
            return;
        }
        if (near(triggerLab_, 120)) {
            abrirLaboratorio();
            return;
        }
        if (near(triggerPuertaArr_, 130)) {
            if (canExit()) close();
            else setHint("Aún no puedes salir: completa Aula y Laboratorio.");
            return;
        }
        break;
    }
    default: break;
    }

    QGraphicsView::keyPressEvent(e);
}

void Escuela::keyReleaseEvent(QKeyEvent *e)
{
    if (!penguin_ || e->isAutoRepeat()) { QGraphicsView::keyReleaseEvent(e); return; }
    switch (e->key()) {
    case Qt::Key_W: case Qt::Key_S: case Qt::Key_A: case Qt::Key_D:
    case Qt::Key_Up: case Qt::Key_Down: case Qt::Key_Left: case Qt::Key_Right:
        penguin_->stopMove(); break;
    default: break;
    }
    QGraphicsView::keyReleaseEvent(e);
}

void Escuela::resizeEvent(QResizeEvent *e)
{
    QGraphicsView::resizeEvent(e);
    fitView();
}

void Escuela::closeEvent(QCloseEvent *e)
{
    if (!canExit()) {
        e->ignore();
        setHint("No puedes salir todavía. Completa Aula y Laboratorio.");
        return;
    }
    e->accept();
}
