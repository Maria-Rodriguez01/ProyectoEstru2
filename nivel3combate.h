#ifndef NIVEL3COMBATE_H
#define NIVEL3COMBATE_H

#include <QGraphicsView>
#include <QGraphicsScene>
#include <QGraphicsPixmapItem>
#include <QLabel>
#include <QPushButton>
#include <QGraphicsProxyWidget>
#include <QVariantAnimation>
#include <QMediaPlayer>
#include <QAudioOutput>
#include <QUrl>

class Nivel3Combate : public QGraphicsView
{
    Q_OBJECT
public:
    enum class Side { None, Empirista, Racionalista };

    // heartsPlayer = corazones iniciales del jugador
    explicit Nivel3Combate(Side playerSide, int heartsPlayer = 1, QWidget *parent = nullptr);

signals:
    void matchFinished();                     // ️ para volver al mapa al terminar

protected:
    void resizeEvent(QResizeEvent *e) override;
    void showEvent(QShowEvent *e) override;

private:
    // --- helpers de carga / layout ---
    QPixmap  safeLoad(const QString &path, const QSize &fallback = QSize(256,256));
    void     loadBackground();
    void     buildUI();
    void     placeFighters();
    qreal    scaleForSheetHeight(const QPixmap &sheet, int rows, qreal targetFrameHeightPx) const;
    QSize    frameSize(const QPixmap &sheet) const;
    void     fitView();
    void     layoutUI();
    void     updateQuestionLayout();
    void     resizeToScreen();

    //audio
    QMediaPlayer  *mediaPlayer_ = nullptr;
    QAudioOutput  *audioOut_    = nullptr;

    void playVictoryTheme(Side winner);

    // --- flujo de preguntas / combate ---
    void     askNextQuestion();
    void     onPressResponder();
    void     onAnswerChosen();
    void     evaluateAnswer(bool playerCorrect);
    void     launchFireball(bool fromPlayer);
    void     onFireballArrived();
    void     applyDamage(bool toCPU);
    void     updateHeartsUI();
    void     endMatch(bool playerWon);

    // --- sprites ---
    void     drawFrame(QGraphicsPixmapItem *item, const QPixmap &sheet, int col, int row, bool mirror);
    void     setIdle();

private:
    static constexpr int SHEET_COLS = 4;
    static constexpr int SHEET_ROWS = 3;
    static constexpr int IDLE_ROW   = 2;

    // Estado principal
    Side   playerSide_ = Side::None;
    int    playerHearts_ = 1;
    int    cpuHearts_    = 3;

    // Escena y fondo
    QGraphicsScene       *scene_  = nullptr;
    QGraphicsPixmapItem  *bgItem_ = nullptr;

    // Luchadores
    QGraphicsPixmapItem  *playerItem_ = nullptr;
    QGraphicsPixmapItem  *cpuItem_    = nullptr;
    QPixmap               playerSheet_;
    QPixmap               cpuSheet_;

    // Proyectil (bola de fuego)
    QGraphicsPixmapItem  *fireball_ = nullptr;
    QVariantAnimation    *fireAnim_ = nullptr;
    bool                  fireInFlight_ = false;

    // UI (pregunta, opciones y HUD)
    QLabel                 *questionLabel_ = nullptr;
    QGraphicsProxyWidget   *questionProxy_ = nullptr;

    QPushButton            *btnResponder_ = nullptr;
    QGraphicsProxyWidget   *respProxy_ = nullptr;

    QWidget                *optionsPanel_ = nullptr;
    QGraphicsProxyWidget   *optionsProxy_ = nullptr;
    QVector<QPushButton*>   optionBtns_;
    bool                    optionsVisible_ = false;

    QLabel                 *hudLabel_ = nullptr;
    QGraphicsProxyWidget   *hudProxy_ = nullptr;

    // Banco de preguntas y respuestas
    QVector<QPair<QString, QVector<QString>>> questions_;
    QVector<int>            answers_;
    int                     currentQuestion_ = -1;
    int                     correctIndex_    = 0;
};

#endif // NIVEL3COMBATE_H

