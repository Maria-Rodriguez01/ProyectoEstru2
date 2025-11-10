#ifndef HEARTBAR_H
#define HEARTBAR_H

#include <QGraphicsPixmapItem>
#include <QPainter>
#include <QPainterPath>
#include <QtMath>

class HeartBar : public QGraphicsPixmapItem {
public:
    explicit HeartBar(const QString& /*unusedSpritePath*/ = QString(),
                      int maxHearts = 4,
                      int heightPx  = 26,
                      QGraphicsItem *parent=nullptr)
        : QGraphicsPixmapItem(parent),
        max_(qBound(1, maxHearts, 4)),
        heightPx_(qMax(14, heightPx))
    {
        buildFrames();
        setHearts(0);
        setZValue(1000);
    }

    void setHearts(int h) {
        hearts_ = qBound(0, h, max_);
        if (!frames_.isEmpty())
            setPixmap(frames_[hearts_]);
    }

    void addHeart(int delta = 1) { setHearts(hearts_ + delta); }
    int hearts() const { return hearts_; }
    int maxHearts() const { return max_; }

    void setHeightPx(int h) {
        heightPx_ = qMax(14, h);
        buildFrames();
        setHearts(hearts_);
    }

private:
    int max_ = 4;
    int hearts_ = 0;
    int heightPx_ = 26;
    QVector<QPixmap> frames_;

    static QPainterPath heartPath(int w, int h) {
        QPainterPath p;
        const double x = w / 2.0;
        const double y = h * 0.62;
        p.moveTo(x, y + h * 0.28);
        p.cubicTo(x + w * 0.38, y + h * 0.08, x + w * 0.42, y - h * 0.32, x, y - h * 0.20);
        p.cubicTo(x - w * 0.42, y - h * 0.32, x - w * 0.38, y + h * 0.08, x, y + h * 0.28);
        p.closeSubpath();
        return p;
    }

    QPixmap drawBar(int filled, int total, int tile, int gap) {
        const int W = (tile + gap) * total - gap;
        const int H = tile;
        QPixmap bar(W, H);
        bar.fill(Qt::transparent);
        QPainter p(&bar);
        p.setRenderHint(QPainter::Antialiasing, true);

        QPen pen(Qt::white, qMax(2, tile / 10), Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin);
        QBrush red(Qt::red);

        for (int i = 0; i < total; ++i) {
            QRect r(i * (tile + gap), 0, tile, tile);
            p.save();
            p.translate(r.topLeft());
            p.setPen(pen);

            QPainterPath path = heartPath(tile, tile);
            if (i < filled) {
                p.setBrush(red);
            } else {
                p.setBrush(Qt::NoBrush);
            }
            p.drawPath(path);
            p.restore();
        }

        p.end();
        return bar;
    }

    void buildFrames() {
        frames_.clear();
        frames_.reserve(max_ + 1);
        const int tile = heightPx_;
        const int gap = qMax(3, tile / 6);

        for (int h = 0; h <= max_; ++h) {
            frames_.push_back(drawBar(h, max_, tile, gap));
        }
    }
};

#endif // HEARTBAR_H

