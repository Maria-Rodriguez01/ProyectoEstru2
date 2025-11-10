#include "locksprite.h"
#include <QPropertyAnimation>
#include <QPixmap>
#include <QDebug>

LockSprite::LockSprite(const QString& spritesheetPath,
                       Layout layout,
                       QGraphicsItem* parent)
    : QObject(), QGraphicsPixmapItem(parent)
{
    QPixmap sheet(spritesheetPath);
    if (sheet.isNull()) {
        qWarning() << "⚠️ LockSprite: no cargó spritesheet:" << spritesheetPath;
        // Placeholder rojo (para detectar ruta mal puesta)
        QPixmap ph(24,24); ph.fill(QColor(200,30,30));
        setPixmap(ph);
        setZValue(10);
        return;
    }

    // Recorte con pequeño margen interno para evitar bordes sucios
    if (layout == Horizontal) {
        const int fw = sheet.width() / 2;
        const int fh = sheet.height();
        const int m  = 2;
        m_closed = sheet.copy(0 + m,     0 + m, fw - 2*m, fh - 2*m);
        m_open   = sheet.copy(fw + m,    0 + m, fw - 2*m, fh - 2*m);
    } else {
        const int fw = sheet.width();
        const int fh = sheet.height() / 2;
        const int m  = 2;
        m_closed = sheet.copy(0 + m,   0 + m,   fw - 2*m, fh - 2*m);
        m_open   = sheet.copy(0 + m,   fh + m,  fw - 2*m, fh - 2*m);
    }

    setPixmap(m_closed);
    setTransformationMode(Qt::SmoothTransformation);
    setZValue(10);
    setOpacity(0.98);

    setOffset(-m_closed.width()/2.0, -m_closed.height()/2.0);

}

void LockSprite::setSpriteScale(qreal s) { setScale(s); }

void LockSprite::unlock() {
    if (m_state == Unlocked) return;
    m_state = Unlocked;

    // Animación: sube y desvanece, luego cambia sprite y vuelve
    auto *animY = new QPropertyAnimation(this, "y");
    animY->setDuration(320);
    animY->setStartValue(y());
    animY->setEndValue(y() - 16);

    auto *fade = new QPropertyAnimation(this, "opacity");
    fade->setDuration(320);
    fade->setStartValue(1.0);
    fade->setEndValue(0.0);

    connect(fade, &QPropertyAnimation::finished, this, [this]() {
        setPixmap(m_open);
        setOpacity(1.0);
        setY(y() + 16);
    });

    animY->start(QAbstractAnimation::DeleteWhenStopped);
    fade->start(QAbstractAnimation::DeleteWhenStopped);
}

