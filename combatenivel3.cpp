#include "combatenivel3.h"
#include "personaje.h"
#include "nivel3combate.h"

#include <QKeyEvent>
#include <QFocusEvent>
#include <QResizeEvent>
#include <QShowEvent>
#include <QCloseEvent>
#include <QPainter>
#include <QDebug>
#include <QScreen>
#include <QGuiApplication>

// Ruta del fondo de selección de bandos
static const char* kBgPath =
    "C:/Users/Maria Gabriela/OneDrive/Documents/ProyectoEstru2MariaRodriguez/Assets/nivel3pt1.jpg";

// --- Helper: detección con “pies” del pingüino + margen de tolerancia ---
bool CombateNivel3::hitTriggerFeet(const QRectF& trigger, const QRectF& penguinBounds, qreal margin)
{
    QRectF t = trigger.adjusted(-margin, -margin, margin, margin);
    QPointF feet(penguinBounds.center().x(), penguinBounds.bottom());
    return t.contains(feet);
}

CombateNivel3::CombateNivel3(int heartsFromHud, QWidget *parent)
    : QGraphicsView(parent),
    heartsFromHud_(qMax(1, heartsFromHud))
{
    setWindowTitle("Nivel 3 — Elige tu bando");
    setRenderHint(QPainter::Antialiasing, false);
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setFrameShape(QFrame::NoFrame);
    setBackgroundBrush(QColor(15,15,15));
    setAutoFillBackground(false);
    setAttribute(Qt::WA_OpaquePaintEvent);
    setFocusPolicy(Qt::StrongFocus);

    scene_ = new QGraphicsScene(this);
    setScene(scene_);

    bgItem_ = new QGraphicsPixmapItem();
    bgItem_->setZValue(-100);
    scene_->addItem(bgItem_);

    walkRectItem_ = scene_->addRect(QRectF(), QPen(Qt::NoPen), QBrush(Qt::NoBrush));
    leftTrigger_  = scene_->addRect(QRectF(), QPen(Qt::NoPen), QBrush(Qt::NoBrush));
    rightTrigger_ = scene_->addRect(QRectF(), QPen(Qt::NoPen), QBrush(Qt::NoBrush));
    walkRectItem_->setZValue(-50);
    leftTrigger_->setZValue(5);
    rightTrigger_->setZValue(5);

    // Hint
    hintLabel_ = new QLabel(" ");
    hintLabel_->setAlignment(Qt::AlignCenter);
    hintLabel_->setStyleSheet(
        "color:#fff; background:rgba(0,0,0,140);"
        "border:1px solid #d2b48c; border-radius:6px; padding:3px 7px;"
        "font: bold 10pt 'Times New Roman';");
    hintProxy_ = scene_->addWidget(hintLabel_);
    hintProxy_->setZValue(1000);
    hintProxy_->hide();

    // Timer
    connect(&gameTimer_, &QTimer::timeout, this, &CombateNivel3::tickGame);
}

void CombateNivel3::showEvent(QShowEvent *e)
{
    QGraphicsView::showEvent(e);

    initOnce();

    const QRect a = QGuiApplication::primaryScreen()->availableGeometry();
    resize(int(a.width()*0.90), int(a.height()*0.90));
    fitInView(scene()->sceneRect(), Qt::KeepAspectRatio);

    move(a.center() - rect().center());
}



void CombateNivel3::resizeToScreen()
{
    const QRect a = QGuiApplication::primaryScreen()->availableGeometry();
    const int W = int(a.width()  * 0.90);
    const int H = int(a.height() * 0.90);
    setMinimumSize(W, H);
    resize(W, H);
    fitView();
}

void CombateNivel3::closeEvent(QCloseEvent *event)
{
    Q_UNUSED(event);
    safeStopTimer();
}

void CombateNivel3::focusOutEvent(QFocusEvent *event)
{
    Q_UNUSED(event);
    if (player_) player_->stopMove();
}

void CombateNivel3::safeStopTimer()
{
    if (gameTimer_.isActive()) gameTimer_.stop();
    disconnect(&gameTimer_, nullptr, this, nullptr);
}

void CombateNivel3::initOnce()
{
    if (inited_) return;
    inited_ = true;

    setBackgroundPath(kBgPath);
    fitView();

    if (!player_) {
        player_ = new Personaje();
        player_->setZValue(10);
        scene_->addItem(player_);
        player_->setSceneBounds(scene_->sceneRect());
        player_->setPos(scene_->width()*0.25, scene_->height()*0.62);
        lastValidPos_ = player_->pos();
    }
}

void CombateNivel3::setBackgroundPath(const QString &path)
{
    QPixmap bg;
    if (!bg.load(path)) {
        qWarning() << "" ;
        bg = QPixmap(1125, 683);
        bg.fill(QColor(40,40,40));
        QPainter p(&bg);
        p.setPen(Qt::white);
        p.drawText(bg.rect(), Qt::AlignCenter, "" );
        p.end();
    } else {
        qDebug() << "";
    }
    bgPixmap_ = bg;
    bgItem_->setPixmap(bgPixmap_);
    scene_->setSceneRect(QRectF(QPointF(0,0), bgPixmap_.size()));
    setViewportUpdateMode(QGraphicsView::SmartViewportUpdate);
    placeRectsFromPercents();
}

void CombateNivel3::fitView()
{
    if (!scene_) return;
    fitInView(scene_->sceneRect(), Qt::KeepAspectRatio);
}

void CombateNivel3::placeRectsFromPercents()
{
    const QRectF s = scene_->sceneRect();
    if (s.isEmpty()) return;

    walkRectItem_->setRect(QRectF(s.left() + walkPrc_.x()*s.width(),
                                  s.top()  + walkPrc_.y()*s.height(),
                                  walkPrc_.width()*s.width(),
                                  walkPrc_.height()*s.height()));
    leftTrigger_->setRect(QRectF(s.left() + leftPrc_.x()*s.width(),
                                 s.top()  + leftPrc_.y()*s.height(),
                                 leftPrc_.width()*s.width(),
                                 leftPrc_.height()*s.height()));
    rightTrigger_->setRect(QRectF(s.left() + rightPrc_.x()*s.width(),
                                  s.top()  + rightPrc_.y()*s.height(),
                                  rightPrc_.width()*s.width(),
                                  rightPrc_.height()*s.height()));

    if (debugVisible_) {
        walkRectItem_->setBrush(QColor(0,255,0,60));
        leftTrigger_->setBrush(QColor(255,0,0,80));
        rightTrigger_->setBrush(QColor(0,0,255,80));
    } else {
        walkRectItem_->setBrush(Qt::NoBrush);
        leftTrigger_->setBrush(Qt::NoBrush);
        rightTrigger_->setBrush(Qt::NoBrush);
    }
}

void CombateNivel3::tickGame()
{
    if (!player_ || !walkRectItem_) return;
    const QRectF walk = walkRectItem_->rect();
    if (walk.isNull()) return;

    const QRectF pb = player_->sceneBoundingRect();
    if (pb.intersects(walk)) lastValidPos_ = player_->pos();
    else                     player_->setPos(lastValidPos_);

    updateHint();
}

void CombateNivel3::updateHint()
{
    if (!hintProxy_ || !player_ || !leftTrigger_ || !rightTrigger_) return;

    const QRectF pb = player_->sceneBoundingRect();

    const bool nearLeft  = hitTriggerFeet(leftTrigger_->rect(),  pb);
    const bool nearRight = hitTriggerFeet(rightTrigger_->rect(), pb);

    if (nearLeft || nearRight) {
        const QString txt = nearLeft
                                ? "Presione E para elegir el bando Empirista"
                                : "Presione E para elegir el bando Racionalista";
        hintLabel_->setText(txt);
        hintLabel_->adjustSize();

        const QPointF pos(pb.center().x() - hintLabel_->width()/2, pb.top() - 25);
        hintProxy_->setPos(pos);
        hintProxy_->show();
    } else {
        hintProxy_->hide();
    }
}

void CombateNivel3::keyPressEvent(QKeyEvent *event)
{
    if (!player_) { QGraphicsView::keyPressEvent(event); return; }
    if (event->isAutoRepeat()) return;

    switch (event->key()) {
    case Qt::Key_W:
    case Qt::Key_Up:    player_->startMove(Personaje::Direction::Up);    break;
    case Qt::Key_S:
    case Qt::Key_Down:  player_->startMove(Personaje::Direction::Down);  break;
    case Qt::Key_A:
    case Qt::Key_Left:  player_->startMove(Personaje::Direction::Left);  break;
    case Qt::Key_D:
    case Qt::Key_Right: player_->startMove(Personaje::Direction::Right); break;

    case Qt::Key_E: {
        const QRectF pb = player_->sceneBoundingRect();

        auto startFight = [&](Nivel3Combate::Side side){
            auto *comb = new Nivel3Combate(side, heartsFromHud_, nullptr);
            comb->setAttribute(Qt::WA_DeleteOnClose);

            // Cuando termina el combate, cerramos esta selección para que el Mapa reaparezca.
            connect(comb, &Nivel3Combate::matchFinished, this, [this, comb](){
                comb->deleteLater();
                this->close();
            });

            this->hide();
            comb->show();
            comb->raise();
            comb->activateWindow();
        };

        if (hitTriggerFeet(leftTrigger_->rect(), pb)) {
            startFight(Nivel3Combate::Side::Empirista);
        } else if (hitTriggerFeet(rightTrigger_->rect(), pb)) {
            startFight(Nivel3Combate::Side::Racionalista);
        }
        break;
    }

    default:
        QGraphicsView::keyPressEvent(event);
    }
}

void CombateNivel3::keyReleaseEvent(QKeyEvent *event)
{
    if (!player_) { QGraphicsView::keyReleaseEvent(event); return; }
    if (event->isAutoRepeat()) return;

    switch (event->key()) {
    case Qt::Key_W:
    case Qt::Key_S:
    case Qt::Key_A:
    case Qt::Key_D:
    case Qt::Key_Up:
    case Qt::Key_Down:
    case Qt::Key_Left:
    case Qt::Key_Right:
        player_->stopMove();
        break;
    default:
        QGraphicsView::keyReleaseEvent(event);
    }
}

void CombateNivel3::resizeEvent(QResizeEvent *event)
{
    QGraphicsView::resizeEvent(event);
    fitView();
}
void CombateNivel3::setDebugTriggersVisible(bool visible)
{
    debugVisible_ = visible;
    placeRectsFromPercents();
}

