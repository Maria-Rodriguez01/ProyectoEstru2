#include "personaje.h"
#include <QDebug>
#include <QPainter>

Personaje::Personaje(QGraphicsItem *parent)
    : QGraphicsPixmapItem(parent),
    animationTimer(new QTimer(this))   // ← SIEMPRE se crea
{
    const QString SPRITESHEET_PATH =
        "C:/Users/Maria Gabriela/OneDrive/Documents/ProyectoEstru2MariaRodriguez/Assets/penguin_knight_spritesheet.png";

    // Intenta cargar el spritesheet real
    if (!spriteSheet.load(SPRITESHEET_PATH)) {
        qWarning() << "" << SPRITESHEET_PATH;
        // Crea un spritesheet de respaldo (placeholder)
        spriteSheet = makeFallbackSheet();
    } else {
        qDebug() << "";
    }

    setInitialFrame();

    // Conecta el timer SIEMPRE (ya existe)
    connect(animationTimer, &QTimer::timeout, this, &Personaje::advanceFrame);
}

void Personaje::setSceneBounds(const QRectF &bounds) {
    sceneBounds = bounds;
}

QPixmap Personaje::makeFallbackSheet() const
{
    // Según tu AnimationMap, lo máximo que se usa es:
    //  - filas: 3 (0,1,2)
    //  - columnas: para Down start=1 y frameCount=3 => columnas 1,2,3 → ancho mínimo = 4 columnas
    const int cols = 4;
    const int rows = 3;

    QPixmap sheet(cols * FRAME_WIDTH, rows * FRAME_HEIGHT);
    sheet.fill(Qt::transparent);

    QPainter p(&sheet);
    for (int r = 0; r < rows; ++r) {
        for (int c = 0; c < cols; ++c) {
            QRect cell(c * FRAME_WIDTH, r * FRAME_HEIGHT, FRAME_WIDTH, FRAME_HEIGHT);
            QColor base = (r == 0 ? QColor(255, 230, 150)
                                  : (r == 1 ? QColor(200, 220, 255)
                                            : QColor(210, 210, 210)));
            p.fillRect(cell, base);
            p.setPen(QPen(Qt::black, 2));
            p.drawRect(cell.adjusted(1,1,-1,-1));
            p.drawText(cell.adjusted(6,6,-6,-6),
                       Qt::AlignLeft | Qt::AlignTop,
                       QString("r%1 c%2").arg(r).arg(c));
        }
    }
    p.end();
    return sheet;
}

void Personaje::setInitialFrame() {
    if (spriteSheet.isNull()) {
        // Nunca debería ocurrir gracias al fallback, pero por si acaso:
        QPixmap ph(FRAME_WIDTH, FRAME_HEIGHT);
        ph.fill(QColor(250, 220, 120));
        setPixmap(ph);
    } else {
        const int DEFAULT_IDLE_COL = 1;
        const int DEFAULT_ROW      = 0;
        QRect rect(DEFAULT_IDLE_COL * FRAME_WIDTH, DEFAULT_ROW * FRAME_HEIGHT,
                   FRAME_WIDTH, FRAME_HEIGHT);
        setPixmap(spriteSheet.copy(rect));
    }

    setTransformOriginPoint(FRAME_WIDTH / 2.0f, FRAME_HEIGHT / 2.0f);
    setScale(0.6f);
    setShapeMode(QGraphicsPixmapItem::MaskShape);
}

void Personaje::startMove(Direction dir) {
    if (!animationTimer) return;                       // guard
    if (activeMovement == Direction::Idle) {
        animationTimer->start(ANIMATION_INTERVAL_MS);
    }
    currentFrameIndex = 0;
    activeMovement = dir;
    currentDirection = dir;
}

void Personaje::stopMove() {
    if (animationTimer) animationTimer->stop();        // guard
    activeMovement = Direction::Idle;

    Direction idlePoseDirection = currentDirection;
    int staticCol = 1;

    if (currentDirection == Direction::Up) {
        idlePoseDirection = Direction::Up_Idle;
    } else {
        // Ajuste de columna de reposo por dirección
        if (currentDirection == Direction::Right) staticCol = 2;
        else staticCol = 1;
    }

    AnimationData data = AnimationMap.at(idlePoseDirection);
    int frameY = data.row * FRAME_HEIGHT;
    int frameX = (idlePoseDirection == Direction::Up_Idle)
                     ? (data.startCol * FRAME_WIDTH)
                     : (staticCol * FRAME_WIDTH);

    QRect rect(frameX, frameY, FRAME_WIDTH, FRAME_HEIGHT);
    setPixmap(spriteSheet.copy(rect));
}

void Personaje::updatePosition() {
    QPointF newPos = pos();

    switch (activeMovement) {
    case Direction::Up:    newPos.setY(y() - MOVEMENT_SPEED); break;
    case Direction::Down:  newPos.setY(y() + MOVEMENT_SPEED); break;
    case Direction::Left:  newPos.setX(x() - MOVEMENT_SPEED); break;
    case Direction::Right: newPos.setX(x() + MOVEMENT_SPEED); break;
    default: break;
    }

    // Mantener dentro de la escena
    if (!sceneBounds.isNull()) {
        if (newPos.x() < sceneBounds.left()) newPos.setX(sceneBounds.left());
        if (newPos.y() < sceneBounds.top()) newPos.setY(sceneBounds.top());
        if (newPos.x() + boundingRect().width() * scale() > sceneBounds.right())
            newPos.setX(sceneBounds.right() - boundingRect().width() * scale());
        if (newPos.y() + boundingRect().height() * scale() > sceneBounds.bottom())
            newPos.setY(sceneBounds.bottom() - boundingRect().height() * scale());
    }

    setPos(newPos);
}

void Personaje::advanceFrame() {
    updatePosition();

    if (activeMovement == Direction::Idle) {
        if (animationTimer) animationTimer->stop();    // guard
        return;
    }

    AnimationData data = AnimationMap.at(activeMovement);
    int frameCount = data.frameCount;
    int startCol   = data.startCol;
    int row        = data.row;

    currentFrameIndex = (currentFrameIndex + 1) % frameCount;

    int frameY = row * FRAME_HEIGHT;
    int frameX = (startCol + currentFrameIndex) * FRAME_WIDTH;

    QRect rect(frameX, frameY, FRAME_WIDTH, FRAME_HEIGHT);
    setPixmap(spriteSheet.copy(rect));
}
