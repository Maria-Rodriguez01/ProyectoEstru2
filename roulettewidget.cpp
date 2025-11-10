#include "roulettewidget.h"
#include <QPainter>
#include <QRandomGenerator>
#include <QLinearGradient>
#include <QFontMetrics>
#include <QtMath>

RouletteWidget::RouletteWidget(QWidget *parent)
    : QWidget(parent)
{
    originalLabels = {"Politica", "Arte", "Ciencia", "Historia"};
    labels = originalLabels;

    labelColors["Politica"] = Qt::white;
    labelColors["Arte"]     = Qt::white;
    labelColors["Ciencia"]  = Qt::white;
    labelColors["Historia"] = Qt::white;

    cycleTimer = new QTimer(this);
    decelTimer = new QTimer(this);
    stopTimer  = new QTimer(this);
    glowTimer  = new QTimer(this);

    connect(cycleTimer, &QTimer::timeout, this, [this]() {
        pickRandomLabel();
        update();
    });

    connect(decelTimer, &QTimer::timeout, this, [this]() {
        cycleIntervalMs = qMin(cycleIntervalMs + 12, 260);
        cycleTimer->setInterval(cycleIntervalMs);
    });

    stopTimer->setSingleShot(true);
    connect(stopTimer, &QTimer::timeout, this, [this]() {
        stopSpinNow();
    });

    glowTimer->setInterval(30);
    connect(glowTimer, &QTimer::timeout, this, [this]() {
        glowElapsedMs += glowTimer->interval();
        if (glowElapsedMs >= glowDurationMs) {
            glowTimer->stop();
            glowActive = false;
        }
        update();
    });

    setAttribute(Qt::WA_OpaquePaintEvent, false);
}

void RouletteWidget::startSpin()
{
    if (exhausted || spinning) return;
    if (labels.isEmpty()) {
        exhausted = true;
        update();
        return;
    }

    spinning = true;
    glowActive = false;
    glowElapsedMs = 0;
    cycleIntervalMs = 80;

    if (labels.size() == 1) {
        currentLabel = labels.first();
        cycleTimer->start(cycleIntervalMs);
        decelTimer->start(200);
        stopTimer->start(900);
    } else {
        cycleTimer->start(cycleIntervalMs);
        decelTimer->start(200);
        const int totalMs = 2000 + QRandomGenerator::global()->bounded(1200);
        stopTimer->start(totalMs);
        pickRandomLabel();
    }

    update();
}

void RouletteWidget::resetRoulette()
{
    spinning = false;
    cycleTimer->stop();
    decelTimer->stop();
    stopTimer->stop();
    glowTimer->stop();
    labels = originalLabels;
    currentLabel.clear();
    exhausted = false;
    update();
}

void RouletteWidget::pickRandomLabel()
{
    if (labels.isEmpty()) return;
    if (labels.size() == 1) {
        currentLabel = labels.first();
        return;
    }
    currentLabel = labels.at(QRandomGenerator::global()->bounded(labels.size()));
}

void RouletteWidget::stopSpinNow()
{
    cycleTimer->stop();
    decelTimer->stop();
    stopTimer->stop();
    spinning = false;

    if (currentLabel.isEmpty() && !labels.isEmpty())
        currentLabel = labels.first();

    emit resultSelected(currentLabel);
    labels.removeOne(currentLabel);

    if (labels.isEmpty()) {
        exhausted = true;
        update();
        return;
    }

    glowActive = true;
    glowElapsedMs = 0;
    glowTimer->start();
    update();
}

void RouletteWidget::paintEvent(QPaintEvent *)
{
    if (exhausted) return;

    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);
    p.setRenderHint(QPainter::TextAntialiasing);

    const int w = width(), h = height();
    const int boxW = int(w * 0.7), boxH = int(h * 0.35);
    const int boxX = (w - boxW) / 2, boxY = (h - boxH) / 2;
    const QRect boxRect(boxX, boxY, boxW, boxH);
    const int rxy = 14;

    QLinearGradient bgGrad(0, boxY, 0, boxY + boxH);
    bgGrad.setColorAt(0.0, QColor(120, 80, 50));
    bgGrad.setColorAt(1.0, QColor(80, 50, 30));
    const QColor gold(201, 162, 58);

    p.setPen(QPen(QColor(75,46,46,160),1));
    p.setBrush(bgGrad);
    p.drawRoundedRect(boxRect.adjusted(1,1,-1,-1), rxy, rxy);
    p.setPen(QPen(gold,3));
    p.drawRoundedRect(boxRect, rxy, rxy);

    if (glowActive) {
        double t = qBound(0.0, double(glowElapsedMs) / glowDurationMs, 1.0);
        double pulse = std::sin(t * M_PI);
        QColor glow = gold; glow.setAlpha(int(160 * pulse));
        QPen gp(glow, int(6*pulse)+2);
        p.setPen(gp);
        p.drawRoundedRect(boxRect.adjusted(-2,-2,2,2), rxy+2, rxy+2);
    }

    QString text = currentLabel.isEmpty() ? (spinning ? "..." : "") : currentLabel;
    if (!text.isEmpty()) {
        QFont font("Times New Roman", qMax(10, boxH / 3), QFont::Bold);
        p.setFont(font);
        p.setPen(QColor(0,0,0,180));
        p.drawText(boxRect.adjusted(0,2,0,2), Qt::AlignCenter, text);
        p.setPen(labelColors.value(currentLabel, Qt::white));
        p.drawText(boxRect, Qt::AlignCenter, text);
    }
}




