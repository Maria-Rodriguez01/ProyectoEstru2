#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QSet>
#include <QMap>
#include <QPointF>
#include <QString>

class QGraphicsScene;
class QGraphicsPixmapItem;
class QGraphicsProxyWidget;
class QLabel;
class QPushButton;
class QTimer;
class QKeyEvent;
class QResizeEvent;

class Personaje;
class RouletteWidget;
class LockSprite;
class MinijuegoHistoria;
class MinigameArte;
class minijuegopolitica;
class minijuegociencia;
class HeartBar;

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

signals:
    void nivelCompletado(int n); // por si lo usas para volver al mapa

protected:
    void keyPressEvent(QKeyEvent *event) override;
    void keyReleaseEvent(QKeyEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;

private:
    Ui::MainWindow *ui = nullptr;

    // Escena y fondo
    QGraphicsScene *scene = nullptr;
    QGraphicsPixmapItem *backgroundItem = nullptr;

    // Personaje y UI ruleta
    Personaje *penguin = nullptr;
    RouletteWidget *rouletteWidget = nullptr;
    QLabel *rouletteResultLabel = nullptr;
    QPushButton *spinBtn = nullptr;

    // Candados
    QMap<QString, LockSprite*> locksByCategory;
    QSet<QString> unlockedCats;

    // Hint “Pulsa E…”
    QGraphicsProxyWidget *hintProxy = nullptr;
    QLabel *enterHintLabel = nullptr;
    QString doorNearNow;     // <- categoría más cercana calculada por proximityTick()

    // HUD
    HeartBar *heartBar = nullptr;

    // Ventanas de minijuegos
    MinijuegoHistoria *historiaWin = nullptr;
    MinigameArte *arteWin = nullptr;
    minijuegopolitica *politicaWin = nullptr;
    minijuegociencia *cienciaWin = nullptr;

    // Flags de entrada única
    bool historiaEnteredOnce = false;
    bool arteEnteredOnce = false;
    bool politicaEnteredOnce = false;
    bool cienciaEnteredOnce = false;
    void tryUnlockNivel3();

    // Timer de proximidad
    QTimer *proximityTimer = nullptr;

private:
    // Auxiliares
    void initLocks();
    void layoutLocks();
    void proximityTick();
    void unlockFromRoulette(const QString &category);
    QPointF anchorForCategory(const QString &cat) const;
    bool isNearDoor(const QString &cat, qreal radiusPx) const;
    bool containsCaseInsensitive(const QSet<QString>& set, const QString& s) const;
    void setupHUD();
    bool allRouletteUnlocked() const;
};

#endif // MAINWINDOW_H





