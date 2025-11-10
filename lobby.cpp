#include "lobby.h"
#include "nivel1.h"
#include "mapa.h"

#include <QPainter>
#include <QGraphicsDropShadowEffect>
#include <QEnterEvent>

Lobby::Lobby(QWidget *parent)
    : QWidget(parent)
{
    setWindowTitle("El camino al conocimiento");
    resize(900, 600);
    setMinimumSize(640, 480);

    // === Fondo ===
    background = QPixmap("C:/Users/Maria Gabriela/OneDrive/Documents/ProyectoEstru2MariaRodriguez/Assets/Lobby.png");

    // === Botón Jugar ===
    btnJugar = new QPushButton("Jugar", this);
    btnJugar->setGeometry(40, height() - 80, 120, 45);
    btnJugar->setStyleSheet(
        "QPushButton {"
        "background-color: #FFD700;"
        "color: black;"
        "font-weight: bold;"
        "border-radius: 12px;"
        "font-size: 20px;"
        "padding: 6px;"
        "}"
        );

    // === Efecto de brillo ===
    auto *shadow = new QGraphicsDropShadowEffect(btnJugar);
    shadow->setColor(Qt::yellow);
    shadow->setOffset(0, 0);
    shadow->setBlurRadius(0);
    btnJugar->setGraphicsEffect(shadow);

    glowAnimation = new QPropertyAnimation(shadow, "blurRadius", this);
    glowAnimation->setDuration(600);
    glowAnimation->setStartValue(0);
    glowAnimation->setEndValue(40);
    glowAnimation->setEasingCurve(QEasingCurve::InOutSine);

    connect(btnJugar, &QPushButton::clicked, this, &Lobby::abrirMapa);
}

void Lobby::paintEvent(QPaintEvent *event)
{
    QPainter painter(this);
    if (!background.isNull()) {
        // 👇 Escala proporcional al tamaño de la ventana
        QPixmap scaled = background.scaled(size(), Qt::KeepAspectRatioByExpanding, Qt::SmoothTransformation);
        painter.drawPixmap(0, 0, scaled);
    }
}

void Lobby::enterEvent(QEnterEvent *event)
{
    glowAnimation->setDirection(QAbstractAnimation::Forward);
    glowAnimation->start();
    QWidget::enterEvent(event);
}

void Lobby::leaveEvent(QEvent *event)
{
    glowAnimation->setDirection(QAbstractAnimation::Backward);
    glowAnimation->start();
    QWidget::leaveEvent(event);
}

void Lobby::resizeEvent(QResizeEvent *event)
{
    btnJugar->move(40, height() - 80);
    QWidget::resizeEvent(event);
}

void Lobby::abrirMapa()
{
    auto *ventanaMapa = new Mapa();
    ventanaMapa->show();
    this->close();
}
