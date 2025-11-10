#ifndef MAPA_H
#define MAPA_H

#include <QWidget>
#include <QPixmap>
#include <QPushButton>
#include <QMap>
#include <QVector>
#include <QString>
#include <QSet>

struct NodoNivel {
    QString nombre;
    QPointF posRel;                   // posición relativa (0–1)
    bool desbloqueado = false;
    QVector<QString> conexiones;      // nombres de los niveles conectados
    QPushButton* boton = nullptr;     // botón asociado en la GUI
};

class Mapa : public QWidget
{
    Q_OBJECT
public:
    explicit Mapa(QWidget *parent = nullptr);

protected:
    void paintEvent(QPaintEvent *e) override;
    void resizeEvent(QResizeEvent *e) override;

private slots:
    void abrirNivel(const QString &nombreNivel);
    void abrirRanking();
    void volverLobby();

private:
    // helpers
    void cargarFondo();
    void inicializarGrafo();
    void colocarBotones();
    void actualizarBotones();
    void desbloquearVecinos(const QString &nivelActual);
    QRect drawRect() const;                 // rect donde se dibuja el fondo escalado
    bool allLevelsCompleted() const;        // ¿Nivel 1..4 completados?
    void actualizarRankingBtn();            // habilitar ranking según progreso

private:
    // fondo
    QPixmap bgOrig_;
    QPixmap bgScaled_;

    // grafo de niveles
    QMap<QString, NodoNivel> niveles_; // Niveles 1..4
    QString nivelActivo_;

    // progreso
    QSet<QString> completados_;        // niveles completados (al cerrar ventana)

    // botones extra
    QPushButton *btnRanking_ = nullptr;
    QPushButton *btnLobby_   = nullptr;

    // última corrida (ms) al cerrar Nivel 4
    qint64 lastRunMs_ = -1;
};

#endif // MAPA_H


