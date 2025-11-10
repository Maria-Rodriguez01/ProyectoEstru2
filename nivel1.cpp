#include "nivel1.h"
#include <QVBoxLayout>
#include <QUrl>
#include <QFileInfo>
#include <QDebug>

Nivel1::Nivel1(QWidget *parent)
    : QWidget(parent)
{
    setWindowTitle("Nivel 1");
    setMinimumSize(800, 600);

    // ---- Widget de video ----
    videoWidget = new QVideoWidget(this);

    // ---- Player y audio (Qt6) ----
    player = new QMediaPlayer(this);
#if QT_VERSION >= QT_VERSION_CHECK(6,0,0)
    audio = new QAudioOutput(this);
    player->setAudioOutput(audio);
#endif
    player->setVideoOutput(videoWidget);

    // ---- Ruta fija del video ----
    QString pathVideo = QStringLiteral(
        "C:/Users/Maria Gabriela/OneDrive/Documents/ProyectoEstru2MariaRodriguez/Assets/Nivel1Video.mp4"
        );

    // ---- Verificación de existencia ----
    qDebug() << "[Nivel1] Cargando video:" << pathVideo
             << "exists?" << QFileInfo(pathVideo).exists();

    QUrl url = QUrl::fromLocalFile(pathVideo);
#if QT_VERSION >= QT_VERSION_CHECK(6,0,0)
    player->setSource(url);
#else
    player->setMedia(url);
    player->setVolume(80);
#endif

    // ---- Botón salir ----
    btnSalir = new QPushButton("Salir", this);
    btnSalir->setStyleSheet(
        "QPushButton {background-color:#4CAF50; color:white; font-size:18px;"
        "padding:8px 20px; border-radius:10px;}"
        "QPushButton:hover { background-color:#45A049; }"
        );

    // ---- Layout ----
    auto *layout = new QVBoxLayout(this);
    layout->addWidget(videoWidget, 1);
    layout->addWidget(btnSalir, 0, Qt::AlignHCenter | Qt::AlignBottom);

    // ---- Conexiones ----
    connect(btnSalir, &QPushButton::clicked, this, &Nivel1::onSalir);
    connect(player, &QMediaPlayer::mediaStatusChanged,
            this, &Nivel1::onMediaStatusChanged);

    // ---- Reproduce automáticamente ----
    player->play();
}

void Nivel1::onSalir()
{
    emit regresarMapa();
    close();
}

void Nivel1::onMediaStatusChanged(QMediaPlayer::MediaStatus status)
{
    if (status == QMediaPlayer::EndOfMedia) {
        emit regresarMapa();
        close();
    }
}
