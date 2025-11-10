#ifndef ESCUELA_H
#define ESCUELA_H

#include <QGraphicsView>
#include <QGraphicsScene>
#include <QGraphicsPixmapItem>
#include <QGraphicsProxyWidget>
#include <QGraphicsRectItem>
#include <QLabel>
#include <QTimer>

class Personaje;
class Aula;
class Laboratorio;

class Escuela : public QGraphicsView
{
    Q_OBJECT
public:
    explicit Escuela(QWidget *parent = nullptr);

protected:
    void keyPressEvent(QKeyEvent *e) override;
    void keyReleaseEvent(QKeyEvent *e) override;
    void resizeEvent(QResizeEvent *e) override;
    void closeEvent(QCloseEvent *e) override;

private slots:
    void tick();

    // 🔔 Señales que llegan desde Aula y Laboratorio
    void onAulaProgreso(bool completado, int respondidas, int total);
    void onLaboratorioProgreso(bool completado, int respondidas, int total);

private:
    // --- escena / fondo ---
    QGraphicsScene*      scene_   = nullptr;
    QGraphicsPixmapItem* bgItem_  = nullptr;

    // --- personaje ---
    Personaje*           penguin_ = nullptr;

    // --- UI hint ---
    QGraphicsProxyWidget* hintProxy_ = nullptr;
    QLabel*               hintLabel_ = nullptr;

    // --- UI estado de subniveles ---
    QGraphicsProxyWidget* statusProxy_ = nullptr;
    QLabel*               statusLabel_ = nullptr;

    // --- triggers / colisiones ---
    QGraphicsRectItem*   triggerAula_        = nullptr;   // esquina izquierda
    QGraphicsRectItem*   triggerLab_         = nullptr;   // esquina derecha
    QGraphicsRectItem*   triggerPuertaArr_   = nullptr;   // puerta superior para salir

    // --- estado de progreso ---
    bool aulaCompletada_ = false;
    bool labCompletada_  = false;
    int  aulaResp_ = 0, aulaTotal_ = 0;
    int  labResp_  = 0, labTotal_  = 0;

    // --- referencias de ventanas hijas (opcional) ---
    Aula*         aulaWin_ = nullptr;
    Laboratorio*  labWin_  = nullptr;

    // --- timer ---
    QTimer*              timer_ = nullptr;

    // Constantes
    static constexpr int BG_W = 1125;
    static constexpr int BG_H = 683;

    // Rutas
    QPixmap safeLoad(const QString& path, const QSize& fb = QSize(512,512));
    void    loadBackground();
    void    fitView();

    // Construcción
    void buildCharacters();
    void buildUI();
    void buildTriggers();

    // Proximidad
    bool near(const QGraphicsRectItem* it, qreal radiusPx = 90.0) const;
    bool canExit() const { return aulaCompletada_ && labCompletada_; }

    // Acciones
    void abrirAula();
    void abrirLaboratorio();

    // UI helpers
    void setHint(const QString& t, const QPointF& viewPos=QPointF(16,12));

};

#endif // ESCUELA_H


