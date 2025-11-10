#ifndef ROULETTEWIDGET_H
#define ROULETTEWIDGET_H

#include <QWidget>
#include <QTimer>
#include <QStringList>
#include <QMap>
#include <QColor>

class RouletteWidget : public QWidget
{
    Q_OBJECT
public:
    explicit RouletteWidget(QWidget *parent = nullptr);
    void startSpin();
    void resetRoulette();

signals:
    void resultSelected(const QString &result);

protected:
    void paintEvent(QPaintEvent *event) override;

private:
    QTimer *cycleTimer = nullptr;
    QTimer *decelTimer = nullptr;
    QTimer *stopTimer  = nullptr;
    QTimer *glowTimer  = nullptr;

    bool spinning = false;
    int  cycleIntervalMs = 80;
    QString currentLabel;

    QStringList labels;
    QStringList originalLabels;
    QMap<QString, QColor> labelColors;

    bool glowActive = false;
    int glowElapsedMs = 0;
    int glowDurationMs = 900;
    bool exhausted = false;

    void pickRandomLabel();
    void stopSpinNow();
};

#endif // ROULETTEWIDGET_H
