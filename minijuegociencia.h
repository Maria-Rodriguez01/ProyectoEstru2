#ifndef MINIJUEGOCIENCIA_H
#define MINIJUEGOCIENCIA_H

#include <QWidget>
#include <QPushButton>
#include <QLabel>
#include <QTimer>
#include <QVector>
#include <QElapsedTimer>
#include <QPixmap>

class minijuegociencia : public QWidget
{
    Q_OBJECT
public:
    explicit minijuegociencia(QWidget *parent = nullptr);
    ~minijuegociencia();

signals:
    void juegoCompletado(bool exito, int got, int total);

protected:
    void paintEvent(QPaintEvent *e) override;
    void resizeEvent(QResizeEvent *e) override;

private slots:
    void tick();
    void onPuffleClicked();

private:
    // Helpers
    static QString ASSETS(const QString &rel);
    bool fileExists(const QString &absPath) const;
    QRect playArea() const;              // zona verde
    void placeRandom(QWidget *w);        // posición inicial
    void applyPuffleSkin(QPushButton *btn, const QString &letter);
    void cargarPreguntas();              // solo de ciencia
    void setPreguntaActual(int idx);

    struct Puf {
        QPushButton *btn = nullptr;
        QPointF vel;                     // velocidad px/s
        QString letra;                   // A/B/C/D
    };
    struct QItem {
        QString texto;
        QStringList opciones;
        int correcta = 0;
    };

    // UI
    QLabel *lblTitulo = nullptr;
    QWidget *hud = nullptr;
    QLabel *lblPregunta = nullptr;
    QLabel *lblOpciones = nullptr;

    // Animación y sprites
    QVector<Puf> puffles;
    QTimer *timer = nullptr;
    QElapsedTimer clock;
    QPixmap bg;

    // Preguntas
    QVector<QItem> banco;
    int qIndex = 0;
    int aciertos = 0;

    // Constantes
    static constexpr int   PUFFLE_SIZE = 96;
    static constexpr int   FPS_MS      = 16;   // ~60fps
    static constexpr int   HUD_H       = 140;
    static constexpr double GREEN_TOP_RATIO = 0.34;
    static constexpr double SPEED_MIN  = 200.0;
    static constexpr double SPEED_MAX  = 320.0;
};

#endif // MINIJUEGOCIENCIA_H


