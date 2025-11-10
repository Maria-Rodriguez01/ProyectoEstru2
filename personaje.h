#ifndef PERSONAJE_H
#define PERSONAJE_H

#include <QObject>
#include <QGraphicsPixmapItem>
#include <QPixmap>
#include <QTimer>
#include <map>

class Personaje : public QObject, public QGraphicsPixmapItem {
    Q_OBJECT

public:
    enum class Direction {
        Down  = 0,
        Up    = 1,
        Left  = 2,
        Right = 3,
        Idle  = 5,
        Up_Idle = 4
    };

    explicit Personaje(QGraphicsItem *parent = nullptr);
    void startMove(Direction dir);
    void stopMove();
    void setSceneBounds(const QRectF &bounds);

private slots:
    void advanceFrame();

private:
    // Dimensiones de cada frame en el spritesheet
    const int FRAME_WIDTH  = 98;
    const int FRAME_HEIGHT = 197;

    const int ANIMATION_INTERVAL_MS = 130;
    const float MOVEMENT_SPEED      = 11.0f;

    QPixmap spriteSheet;
    QTimer *animationTimer = nullptr;
    Direction currentDirection = Direction::Down;
    Direction activeMovement   = Direction::Idle;
    int currentFrameIndex = 0;

    QRectF sceneBounds;

    struct AnimationData {
        int startCol;    // columna inicial
        int frameCount;  // número de frames
        int row;         // fila en el spritesheet
    };

    const std::map<Direction, AnimationData> AnimationMap = {
        {Direction::Down,    {1, 3, 0}},
        {Direction::Up,      {1, 3, 1}},
        {Direction::Left,    {0, 2, 2}},
        {Direction::Right,   {2, 2, 2}},
        {Direction::Idle,    {1, 1, 0}},
        {Direction::Up_Idle, {2, 1, 1}}
    };

    void setInitialFrame();
    void updatePosition();

    // Crea un spritesheet de respaldo suficientemente grande
    QPixmap makeFallbackSheet() const;
};

#endif // PERSONAJE_H

