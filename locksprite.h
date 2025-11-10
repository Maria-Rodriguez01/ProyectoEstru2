#ifndef LOCKSPRITE_H
#define LOCKSPRITE_H

#include <QObject>
#include <QGraphicsPixmapItem>

class LockSprite : public QObject, public QGraphicsPixmapItem {
    Q_OBJECT
public:
    enum State  { Locked, Unlocked };
    enum Layout { Horizontal, Vertical };

    explicit LockSprite(const QString& spritesheetPath,
                        Layout layout = Horizontal,
                        QGraphicsItem* parent = nullptr);

    void setSpriteScale(qreal s);
    void unlock();
    State state() const { return m_state; }

private:
    QPixmap m_closed;
    QPixmap m_open;
    State   m_state = Locked;
};

#endif // LOCKSPRITE_H


