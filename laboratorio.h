#ifndef LABORATORIO_H
#define LABORATORIO_H

#include <QGraphicsView>
#include <QGraphicsScene>
#include <QGraphicsPixmapItem>
#include <QGraphicsRectItem>
#include <QGraphicsProxyWidget>
#include <QLabel>
#include <QPushButton>
#include <QTimer>
#include <QVector>
#include <QMap>
#include <QPointF>

class Personaje;

class Laboratorio : public QGraphicsView
{
    Q_OBJECT
public:
    explicit Laboratorio(QWidget *parent = nullptr);
    ~Laboratorio() override = default;

signals:
    // 🔔 Progreso “real”: respondidas y total (5). completado si respondidas == total.
    void progresoLaboratorio(bool completado, int respondidas, int total);

protected:
    void keyPressEvent(QKeyEvent *e) override;
    void keyReleaseEvent(QKeyEvent *e) override;
    void resizeEvent(QResizeEvent *e) override;

private slots:
    void tick();
    void onOptionClicked();

private:
    // --- Escena ---
    QGraphicsScene* scene_ = nullptr;
    QGraphicsPixmapItem* bgItem_ = nullptr;
    void reflowQuestionPanel();

    // --- Personajes ---
    Personaje* penguin_ = nullptr;
    QGraphicsPixmapItem* npcDescartes_ = nullptr;

    // --- UI (hints/toast) ---
    QGraphicsProxyWidget* hintProxy_ = nullptr;
    QLabel* hintLabel_ = nullptr;
    QGraphicsProxyWidget* toastProxy_ = nullptr;
    QLabel* toastLabel_ = nullptr;
    QTimer* toastTimer_ = nullptr;

    // --- Panel de preguntas ---
    QWidget* qPanel_ = nullptr;
    QLabel*  qTitle_ = nullptr;
    QLabel*  qText_  = nullptr;
    QVector<QPushButton*> qOpts_;   // 4 botones

    // --- Preguntas ---
    QVector<QString> questions_;
    QVector<QString> optA_, optB_, optC_, optD_;
    QVector<int>     answers_;

    struct Mesa {
        QGraphicsRectItem* collider = nullptr; // solo el rectángulo de la mesa
        QVector<int> qIndex;                   // índices de preguntas para esa mesa
        int progress = 0;                      // cuántas acertó en esa mesa
    };
    QVector<Mesa> mesas_;
    int mesaActual_ = -1;

    // --- Estado ---
    int  totalPreguntas_  = 5;   // 2 + 2 + 1
    int  respondidas_     = 0;   // acumuladas
    bool finalizado_      = false;
    QPointF prevSafePos_;

    // --- Timer del loop ---
    QTimer* timer_ = nullptr;

    // --- Constantes ---
    static constexpr int BG_W = 1000;
    static constexpr int BG_H = 632;
    static constexpr int MESA_RADIUS = 70;
    static constexpr int NPC_RADIUS  = 120;

    // --- Utilidades ---
    QPixmap safeLoad(const QString& path, const QSize& fb = QSize(512,512));
    void loadBackground();
    void fitView();

    void buildCharacters();
    void buildMesas();
    void buildQuestions();
    void buildUI();

    void ensureToast();
    void showToast(const QString& text, int msec = 1800);
    void setHintText(const QString& t);
    void placeHintTopLeft();

    // --- colisiones ---
    QRectF feetRectAt(const QPointF &topLeft) const;
    bool   hitsAnyCollider() const;
    bool   hitsAnyColliderAt(const QPointF &p) const;

    int    nearestMesa() const;
    bool   nearNPC() const;

    void abrirPreguntaDeMesa(int mesaIdx);
    void hablarConDescartes();
    bool  todasLasPreguntasResueltas() const;

    // 🔸 Emisor centralizado del progreso
    void emitLabProgress();
};

#endif // LABORATORIO_H

