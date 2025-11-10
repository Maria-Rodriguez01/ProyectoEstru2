#ifndef AULA_H
#define AULA_H

#include <QGraphicsView>
#include <QGraphicsScene>
#include <QGraphicsPixmapItem>
#include <QGraphicsRectItem>
#include <QGraphicsProxyWidget>
#include <QLabel>
#include <QPushButton>
#include <QTimer>
#include <QVector>
#include <QSet>
#include <QLineEdit>
#include <QMap>

class Personaje;

class Aula : public QGraphicsView
{
    Q_OBJECT
public:
    explicit Aula(QWidget *parent = nullptr);
    ~Aula() override = default;

signals:
    void progresoAula(bool completado, int respondidas, int total);

protected:
    void keyPressEvent(QKeyEvent *e) override;
    void keyReleaseEvent(QKeyEvent *e) override;
    void resizeEvent(QResizeEvent *e) override;

private slots:
    void tick();
    void onOptionClicked();
    void submitAnswer();

private:
    // ---- vista / escena ----
    QGraphicsScene*        scene_   = nullptr;
    QGraphicsPixmapItem*   bgItem_  = nullptr;

    // ---- personajes ----
    Personaje*             penguin_ = nullptr;
    QGraphicsPixmapItem*   npcKant_ = nullptr;

    // ---- interacción ----
    QGraphicsProxyWidget*  hintProxy_ = nullptr;
    QLabel*                hintLabel_ = nullptr;

    // Libros (sensores) y estado
    QVector<QGraphicsRectItem*> libros_;
    QSet<int>                  librosResueltos_;

    // Toast
    QGraphicsProxyWidget *toastProxy_ = nullptr;
    QLabel *toastLabel_ = nullptr;
    QTimer *toastTimer_ = nullptr;

    // Panel “decir el libro”
    QWidget              *answerPanel_  = nullptr;
    QGraphicsProxyWidget *answerProxy_  = nullptr;
    QLabel               *answerTitle_  = nullptr;
    QLineEdit            *answerEdit_   = nullptr;
    QPushButton          *answerBtn_    = nullptr;

    void ensureToast();

    // Preguntas y respuestas
    QVector<QString>  questions_;
    QVector<QString>  optA_, optB_, optC_, optD_;
    QVector<int>      answers_;
    QMap<int,QString> pistas_;

    // Panel de pregunta
    QWidget*              qPanel_  = nullptr;
    QLabel*               qTitle_  = nullptr;
    QLabel*               qText_   = nullptr;
    QVector<QPushButton*> qOpts_;
    bool                  showingHint_ = false;
    int                   currentBook_ = -1;

    // Timer
    QTimer*               timer_   = nullptr;

    // Constantes
    static constexpr int  BG_W = 1125;
    static constexpr int  BG_H = 683;
    static constexpr int  BOOK_RADIUS = 40;
    static constexpr int  NPC_RADIUS  = 55;

    // Rutas
    QPixmap safeLoad(const QString& path, const QSize& fb = QSize(512,512));
    void    loadBackground();
    void    fitView();

    // construcción de escena
    void buildUI();
    void buildCharacters();
    void buildBooks();
    void buildQuestions();

    // helpers de proximidad
    int  nearestBook() const;
    bool nearNPC() const;

    // UI helpers
    void setHintText(const QString& t);
    void placeHintOver(const QRectF& target);
    void showToast(const QString& msg, int ms = 1400);

    // preguntas
    void abrirPregunta(int idx);
    bool panelAbierto_ = false;

    // respuesta final
    void showAnswerPanel();
    static QString normalize(const QString&);

    void emitAulaProgress();
};

#endif // AULA_H
