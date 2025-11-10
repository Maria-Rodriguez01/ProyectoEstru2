#ifndef COMBATENIVEL3_H
#define COMBATENIVEL3_H

#include <QGraphicsView>
#include <QGraphicsScene>
#include <QGraphicsPixmapItem>
#include <QGraphicsProxyWidget>
#include <QLabel>
#include <QTimer>

class Personaje;
class Nivel3Combate;   // forward

class CombateNivel3 : public QGraphicsView
{
    Q_OBJECT
public:
    // heartsFromHud: corazones traídos del HUD de MainWindow (>=1)
    explicit CombateNivel3(int heartsFromHud, QWidget *parent = nullptr);
    explicit CombateNivel3(QWidget *parent) : CombateNivel3(1, parent) {}

    void setDebugTriggersVisible(bool visible);

protected:
    void showEvent(QShowEvent *e) override;
    void closeEvent(QCloseEvent *event) override;
    void focusOutEvent(QFocusEvent *event) override;
    void keyPressEvent(QKeyEvent *event) override;
    void keyReleaseEvent(QKeyEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;

private:
    void initOnce();
    void fitView();
    void placeRectsFromPercents();
    void safeStopTimer();
    void setBackgroundPath(const QString &path);
    void resizeToScreen();                 // ⬅️ ajusta a 90% de pantalla

    void tickGame();
    void updateHint();

    static bool hitTriggerFeet(const QRectF& trigger, const QRectF& penguinBounds, qreal margin = 18.0);

private:
    QGraphicsScene *scene_ = nullptr;
    QGraphicsPixmapItem *bgItem_ = nullptr;

    QGraphicsRectItem *walkRectItem_ = nullptr;
    QGraphicsRectItem *leftTrigger_  = nullptr; // EMPIRISTA
    QGraphicsRectItem *rightTrigger_ = nullptr; // RACIONALISTA

    Personaje *player_ = nullptr;
    QPointF    lastValidPos_;
    bool       inited_ = false;

    QGraphicsProxyWidget *hintProxy_ = nullptr;
    QLabel *hintLabel_ = nullptr;

    QTimer gameTimer_;
    bool debugVisible_ = false;

    int heartsFromHud_ = 1;

    QPixmap bgPixmap_;

    // Rectángulos normalizados (x,y,w,h) relativos al sceneRect:
    QRectF walkPrc_  = QRectF(0.05, 0.47, 0.90, 0.42); // suelo
    QRectF leftPrc_  = QRectF(0.25, 0.46, 0.10, 0.20); // puerta izquierda
    QRectF rightPrc_ = QRectF(0.70, 0.46, 0.10, 0.20); // puerta derecha
};

#endif // COMBATENIVEL3_H





