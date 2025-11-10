#ifndef NIVEL1_H
#define NIVEL1_H

#include <QWidget>
#include <QPushButton>
#include <QVideoWidget>
#include <QMediaPlayer>

#if QT_VERSION >= QT_VERSION_CHECK(6,0,0)
#include <QAudioOutput>
#endif

class Nivel1 : public QWidget
{
    Q_OBJECT
public:
    explicit Nivel1(QWidget *parent = nullptr);

signals:
    void regresarMapa();  // señal para volver al mapa

private slots:
    void onSalir();
    void onMediaStatusChanged(QMediaPlayer::MediaStatus status);

private:
    QPushButton *btnSalir = nullptr;
    QMediaPlayer *player = nullptr;
    QVideoWidget *videoWidget = nullptr;

#if QT_VERSION >= QT_VERSION_CHECK(6,0,0)
    QAudioOutput *audio = nullptr;
#endif
};

#endif // NIVEL1_H




