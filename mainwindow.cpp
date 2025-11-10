#include "mainwindow.h"
#include "./ui_mainwindow.h"

#include <QGraphicsView>
#include <QGraphicsScene>
#include <QGraphicsPixmapItem>
#include <QGraphicsProxyWidget>
#include <QVBoxLayout>
#include <QPainter>
#include <QDebug>
#include <QLabel>
#include <QPushButton>
#include <QKeyEvent>
#include <QResizeEvent>
#include <QLineF>
#include <algorithm>
#include <QApplication>
#include <QFrame>
#include <QTimer>
#include <QScreen>
#include <QGuiApplication>

#include "heartbar.h"
#include "personaje.h"
#include "locksprite.h"
#include "roulettewidget.h"
#include "minijuegohistoria.h"
#include "minigamearte.h"
#include "minijuegopolitica.h"
#include "minijuegociencia.h"

// Tamaño nativo del fondo
static const int BACKGROUND_WIDTH  = 1125;
static const int BACKGROUND_HEIGHT = 683;

// Rutas
static const QString BACKGROUND_PATH =
    "C:/Users/Maria Gabriela/OneDrive/Documents/ProyectoEstru2MariaRodriguez/Assets/background1.jpg";
static const QString LOCK_SHEET_PATH =
    "C:/Users/Maria Gabriela/OneDrive/Documents/ProyectoEstru2MariaRodriguez/Assets/lock_sheet.png";

// Puertas (coordenadas normalizadas)
struct Anchor { QString cat; QPointF norm; QPointF nudge; };
static const Anchor kAnchors[] = {
    {"Politica", QPointF(0.300, 0.500), QPointF(0,0)},
    {"Ciencia",  QPointF(0.530, 0.475), QPointF(0,0)},
    {"Historia", QPointF(0.696, 0.505), QPointF(0,0)},
    {"Arte",     QPointF(0.378, 0.178), QPointF(0,0)},
    {"Nivel3",   QPointF(0.880, 0.505), QPointF(0,0)} // opcional
};

bool MainWindow::containsCaseInsensitive(const QSet<QString>& set, const QString& s) const {
    for (const QString &v : set)
        if (v.compare(s, Qt::CaseInsensitive) == 0)
            return true;
    return false;
}

bool MainWindow::allRouletteUnlocked() const
{
    static const QStringList req = {"Arte","Historia","Politica","Ciencia"};
    for (const QString& r : req)
        if (!containsCaseInsensitive(unlockedCats, r)) return false;
    return true;
}

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    if (auto *lay = this->centralWidget()->layout()) {
        lay->setContentsMargins(0,0,0,0);
        lay->setSpacing(0);
    }
    this->centralWidget()->setContentsMargins(0,0,0,0);
    ui->graphicsView->setFrameShape(QFrame::NoFrame);

    // Escena
    scene = new QGraphicsScene(this);
    scene->setSceneRect(0, 0, BACKGROUND_WIDTH, BACKGROUND_HEIGHT);
    ui->graphicsView->setScene(scene);
    ui->graphicsView->setStyleSheet("border:none;");
    ui->graphicsView->setBackgroundBrush(Qt::black);
    ui->graphicsView->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    ui->graphicsView->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    ui->graphicsView->setRenderHint(QPainter::Antialiasing);
    ui->graphicsView->setRenderHint(QPainter::SmoothPixmapTransform);
    ui->graphicsView->setDragMode(QGraphicsView::NoDrag);

    // Ajustar ventana a 90% de pantalla, sin exceder el fondo
    const QRect avail = QGuiApplication::primaryScreen()->availableGeometry();
    const double sx = (avail.width()  * 0.90) / BACKGROUND_WIDTH;
    const double sy = (avail.height() * 0.90) / BACKGROUND_HEIGHT;
    const double scale = std::min(1.0, std::min(sx, sy));
    const int W = int(BACKGROUND_WIDTH  * scale);
    const int H = int(BACKGROUND_HEIGHT * scale);
    setMinimumSize(W, H);
    setMaximumSize(W, H);
    resize(W, H);

    // --- Centrar ventana en pantalla ---
    const QRect screenGeometry = QGuiApplication::primaryScreen()->availableGeometry();
    move(screenGeometry.center() - rect().center());

    // Fondo (MIEMBRO backgroundItem)
    QPixmap bg(BACKGROUND_PATH);
    if (!bg.isNull()) {
        QPixmap scaled = bg.scaled(BACKGROUND_WIDTH, BACKGROUND_HEIGHT,
                                   Qt::KeepAspectRatioByExpanding,
                                   Qt::SmoothTransformation);
        backgroundItem = new QGraphicsPixmapItem(scaled);
        backgroundItem->setZValue(-1);
        scene->addItem(backgroundItem);
    } else {
        scene->setBackgroundBrush(Qt::darkBlue);
    }
    ui->graphicsView->fitInView(scene->sceneRect(), Qt::KeepAspectRatio);

    // Personaje
    penguin = new Personaje();
    scene->addItem(penguin);
    penguin->setSceneBounds(scene->sceneRect());
    penguin->setPos(BACKGROUND_WIDTH/2.0 - 100, BACKGROUND_HEIGHT/2.0);

    // Ruleta
    rouletteWidget = new RouletteWidget();
    rouletteWidget->setFixedSize(200, 200);

    rouletteResultLabel = new QLabel("Resultado: ");
    rouletteResultLabel->setAlignment(Qt::AlignCenter);
    rouletteResultLabel->setStyleSheet(
        "color:#f5f2e8; border:1px solid #c0a060; padding:3px 6px;"
        "background:rgba(60,40,20,200); font:bold 12pt 'Times New Roman';");

    spinBtn = new QPushButton("Girar");
    spinBtn->setStyleSheet(
        "background:rgba(60,40,20,255); color:#c0a060;"
        "border:2px solid #c0a060; padding:5px 15px; border-radius:5px;");

    QWidget *rouletteContainer = new QWidget();
    QVBoxLayout *vbox = new QVBoxLayout(rouletteContainer);
    vbox->setAlignment(Qt::AlignCenter);
    vbox->addWidget(rouletteWidget);
    vbox->addWidget(rouletteResultLabel);
    vbox->addWidget(spinBtn);
    rouletteContainer->setStyleSheet("background:transparent; border:none; padding:0;");

    QGraphicsProxyWidget *rouletteProxy = scene->addWidget(rouletteContainer);
    rouletteProxy->setZValue(5);
    rouletteProxy->setPos(BACKGROUND_WIDTH - 250, BACKGROUND_HEIGHT - 300);

    connect(spinBtn, &QPushButton::clicked, rouletteWidget, &RouletteWidget::startSpin);
    connect(rouletteWidget, &RouletteWidget::resultSelected, this, &MainWindow::unlockFromRoulette);

    // Candados
    initLocks();

    // Hint “Pulsa E…” (MIEMBRO hintProxy)
    enterHintLabel = new QLabel(nullptr);
    enterHintLabel->setText("Pulsa E para entrar");
    enterHintLabel->setAlignment(Qt::AlignCenter);
    enterHintLabel->setStyleSheet(
        "color:#fff; background:rgba(0,0,0,120);"
        "border:1px solid #c0a060; border-radius:6px;"
        "padding:4px 8px; font:bold 10pt 'Times New Roman';");
    hintProxy = scene->addWidget(enterHintLabel);
    hintProxy->setZValue(1000);
    hintProxy->hide();

    // Proximidad
    proximityTimer = new QTimer(this);
    proximityTimer->setInterval(120);
    connect(proximityTimer, &QTimer::timeout, this, &MainWindow::proximityTick);
    proximityTimer->start();

    // HUD
    setupHUD();

    setFocusPolicy(Qt::StrongFocus);
    ui->graphicsView->setFocusPolicy(Qt::NoFocus);
    this->setFocus();
}

MainWindow::~MainWindow() { delete ui; }

// --------- Candados ----------
void MainWindow::initLocks()
{
    for (const Anchor &a : kAnchors) {
        auto *lock = new LockSprite(LOCK_SHEET_PATH, LockSprite::Horizontal);
        lock->setSpriteScale(1.15);
        lock->setZValue(10);
        scene->addItem(lock);
        locksByCategory[a.cat] = lock;
    }
    layoutLocks();
}

void MainWindow::layoutLocks()
{
    QRectF bg = backgroundItem ? backgroundItem->sceneBoundingRect() : scene->sceneRect();
    for (const Anchor &a : kAnchors) {
        auto *lock = locksByCategory.value(a.cat, nullptr);
        if (!lock) continue;
        QPointF p(bg.left() + a.norm.x()*bg.width(),
                  bg.top()  + a.norm.y()*bg.height());
        lock->setPos(p + a.nudge);
    }
}

// --------- Ruleta ----------
void MainWindow::tryUnlockNivel3()
{
    // Solo considerar Nivel 3 si todos los minijuegos están desbloqueados por la ruleta
    if (!allRouletteUnlocked()) return;

    // Se exige ≥1 corazón para habilitar Nivel 3
    const int h = heartBar ? heartBar->hearts() : 0;
    if (h >= 1) {
        if (locksByCategory.contains("Nivel3") && locksByCategory["Nivel3"]) {
            locksByCategory["Nivel3"]->unlock();
        }
    }
}

void MainWindow::unlockFromRoulette(const QString &category)
{
    if (rouletteResultLabel)
        rouletteResultLabel->setText("Resultado: " + category);

    if (locksByCategory.contains(category) && locksByCategory[category])
        locksByCategory[category]->unlock();

    unlockedCats.insert(category.trimmed());

    // No cerramos aquí. Solo re-evaluamos Nivel3 (requiere ≥1 corazón).
    tryUnlockNivel3();
}


// --------- Helpers de puerta ----------
QPointF MainWindow::anchorForCategory(const QString &cat) const
{
    QRectF bg = backgroundItem ? backgroundItem->sceneBoundingRect() : scene->sceneRect();
    for (const auto &a : kAnchors) {
        if (a.cat.compare(cat, Qt::CaseInsensitive) == 0) {
            return QPointF(bg.left() + a.norm.x()*bg.width(),
                           bg.top()  + a.norm.y()*bg.height());
        }
    }
    return bg.center();
}

bool MainWindow::isNearDoor(const QString &cat, qreal radiusPx) const
{
    if (!penguin) return false;
    QPointF door = anchorForCategory(cat);
    QPointF hero = penguin->sceneBoundingRect().center();
    return QLineF(hero, door).length() <= radiusPx;
}

// --------- Proximidad ----------
void MainWindow::proximityTick()
{
    struct Candidate { QString cat; qreal dist; };
    QList<Candidate> cands;
    const qreal detectRadius = 100.0;

    if (!penguin) {
        if (hintProxy) hintProxy->hide();
        doorNearNow.clear();
        return;
    }
    const QPointF hero = penguin->sceneBoundingRect().center();

    for (const QString &cat : {"Arte", "Historia", "Politica", "Ciencia"}) {
        if (!containsCaseInsensitive(unlockedCats, cat)) continue;
        const QPointF door = anchorForCategory(cat);
        const qreal d = QLineF(hero, door).length();
        if (d <= detectRadius) cands.push_back({cat, d});
    }

    if (allRouletteUnlocked()) {
        const QString cat = "Nivel3";
        const QPointF door = anchorForCategory(cat);
        const qreal d = QLineF(hero, door).length();
        if (d <= detectRadius) cands.push_back({cat, d});
    }

    if (cands.isEmpty()) {
        doorNearNow.clear();
        if (hintProxy) hintProxy->hide();
        return;
    }

    std::sort(cands.begin(), cands.end(),
              [](const Candidate &a, const Candidate &b){ return a.dist < b.dist; });
    doorNearNow = cands.first().cat;

    QString text;
    if (doorNearNow == "Nivel3" && allRouletteUnlocked()) {
        const int hearts = heartBar ? heartBar->hearts() : 0;
        if (hearts >= 1) text = "Pulsa E para volver al mapa";
        else             text = "Necesitas ≥1 corazón para desbloquear Nivel 3";
    } else {
        text = QString("Pulsa E para entrar a %1").arg(doorNearNow);
    }

    if (enterHintLabel && hintProxy) {
        enterHintLabel->setText(text);
        enterHintLabel->adjustSize();

        const QRectF hb = penguin->sceneBoundingRect();
        const QPointF pos(hb.center().x() - enterHintLabel->width() / 2.0,
                          hb.top() - 24.0);
        hintProxy->setPos(pos);
        hintProxy->show();
    }
}


// --------- Controles ----------
void MainWindow::keyPressEvent(QKeyEvent *event)
{
    if (event->isAutoRepeat()) { QMainWindow::keyPressEvent(event); return; }

    switch (event->key()) {
    case Qt::Key_W: case Qt::Key_Up:    penguin->startMove(Personaje::Direction::Up);    break;
    case Qt::Key_S: case Qt::Key_Down:  penguin->startMove(Personaje::Direction::Down);  break;
    case Qt::Key_A: case Qt::Key_Left:  penguin->startMove(Personaje::Direction::Left);  break;
    case Qt::Key_D: case Qt::Key_Right: penguin->startMove(Personaje::Direction::Right); break;

    case Qt::Key_E: {
        const qreal enterRadius = 110.0;
        if (doorNearNow.isEmpty()) break;

        // Historia
        if (doorNearNow.compare("Historia", Qt::CaseInsensitive) == 0) {
            if (containsCaseInsensitive(unlockedCats, "Historia") &&
                isNearDoor("Historia", enterRadius) && !historiaEnteredOnce)
            {
                historiaEnteredOnce = true;
                if (!historiaWin) {
                    historiaWin = new MinijuegoHistoria();
                    connect(historiaWin, &MinijuegoHistoria::quizFinished,
                            this, [this](bool perfect){
                                if (perfect && heartBar) heartBar->addHeart(1);
                            });
                    connect(historiaWin, &MinijuegoHistoria::destroyed, this, [this](){
                        historiaWin = nullptr;
                    });
                }
                historiaWin->show(); historiaWin->raise(); historiaWin->activateWindow();
                return;
            }
        }
        // Arte
        else if (doorNearNow.compare("Arte", Qt::CaseInsensitive) == 0) {
            if (containsCaseInsensitive(unlockedCats, "Arte") &&
                isNearDoor("Arte", enterRadius) && !arteEnteredOnce)
            {
                arteEnteredOnce = true;
                if (!arteWin) {
                    arteWin = new MinigameArte();

                    // ✅ Premia si terminó perfecto
                    connect(arteWin, &MinigameArte::quizFinished,
                            this, [this](bool perfect){
                                if (perfect && heartBar) heartBar->addHeart(1);
                            });

                    connect(arteWin, &QObject::destroyed, this, [this](){ arteWin = nullptr; });
                }
                arteWin->show(); arteWin->raise(); arteWin->activateWindow();
                return;
            }
        }
        // Política
        else if (doorNearNow.compare("Politica", Qt::CaseInsensitive) == 0) {
            if (containsCaseInsensitive(unlockedCats, "Politica") &&
                isNearDoor("Politica", enterRadius) && !politicaEnteredOnce)
            {
                politicaEnteredOnce = true;
                if (!politicaWin) {
                    politicaWin = new minijuegopolitica();

                    // ✅ Premia si el juego se completó con éxito
                    connect(politicaWin, &minijuegopolitica::juegoCompletado,
                            this, [this](bool exito){
                                if (exito && heartBar) heartBar->addHeart(1);
                            });

                    connect(politicaWin, &QObject::destroyed, this, [this](){ politicaWin = nullptr; });
                }
                politicaWin->show(); politicaWin->raise(); politicaWin->activateWindow();
                return;
            }
        }
        // Ciencia
        else if (doorNearNow.compare("Ciencia", Qt::CaseInsensitive) == 0) {
            if (containsCaseInsensitive(unlockedCats, "Ciencia") &&
                isNearDoor("Ciencia", enterRadius) && !cienciaEnteredOnce)
            {
                cienciaEnteredOnce = true;
                if (!cienciaWin) {
                    cienciaWin = new minijuegociencia();

                    // ✅ Premia solo si atrapó todos (got == total); ajusta si quieres premiar distinto
                    connect(cienciaWin, &minijuegociencia::juegoCompletado,
                            this, [this](bool exito, int got, int total){
                                if (exito && got == total && heartBar) heartBar->addHeart(1);
                            });

                    connect(cienciaWin, &QObject::destroyed, this, [this](){ cienciaWin = nullptr; });
                }

                cienciaWin->show(); cienciaWin->raise(); cienciaWin->activateWindow();
                return;
            }
        }
        // Nivel 3 (si decides abrir selección o combate desde aquí)
        if (doorNearNow.compare("Nivel3", Qt::CaseInsensitive) == 0) {
            if (allRouletteUnlocked() && isNearDoor("Nivel3", enterRadius)) {
                const int h = heartBar ? heartBar->hearts() : 0;
                if (h >= 1) {
                    this->close();   // Mapa reaparece por el connect(destroyed...) en Mapa::abrirNivel
                    return;
                }
            }
        }
        break;
    }

    default:
        QMainWindow::keyPressEvent(event);
    }
}

void MainWindow::keyReleaseEvent(QKeyEvent *event)
{
    if (event->isAutoRepeat()) { QMainWindow::keyReleaseEvent(event); return; }
    switch (event->key()) {
    case Qt::Key_W: case Qt::Key_S: case Qt::Key_A: case Qt::Key_D:
    case Qt::Key_Up: case Qt::Key_Down: case Qt::Key_Left: case Qt::Key_Right:
        penguin->stopMove(); break;
    default:
        QMainWindow::keyReleaseEvent(event);
    }
}

// --------- HUD ----------
void MainWindow::setupHUD()
{
    heartBar = new HeartBar("", 4, 26);
    scene->addItem(heartBar);
    heartBar->setPos(10, 8);
    heartBar->setHearts(0);
}

// --------- Resize ----------
void MainWindow::resizeEvent(QResizeEvent *event)
{
    QMainWindow::resizeEvent(event);
    if (ui->graphicsView && scene) {
        ui->graphicsView->fitInView(scene->sceneRect(), Qt::KeepAspectRatio);
        layoutLocks();
        if (heartBar) heartBar->setPos(10, 8);
    }
}
