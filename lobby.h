#ifndef LOBBY_H
#define LOBBY_H

#include <QWidget>
#include <QPushButton>
#include <QPixmap>
#include <QPropertyAnimation>

class Lobby : public QWidget
{
    Q_OBJECT
public:
    explicit Lobby(QWidget *parent = nullptr);

protected:
    void paintEvent(QPaintEvent *event) override;
    void enterEvent(QEnterEvent *event) override;
    void leaveEvent(QEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;

private slots:
    void abrirMapa();

private:
    QPixmap background;
    QPushButton *btnJugar = nullptr;
    QPropertyAnimation *glowAnimation = nullptr;
};

#endif // LOBBY_H


