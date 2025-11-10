#ifndef MINIGAMEARTE_H
#define MINIGAMEARTE_H

#include <QWidget>
#include <QPixmap>
#include <QVector>
#include <QString>
#include <QPushButton>
#include <QLabel>

class MinigameArte : public QWidget
{
    Q_OBJECT
public:
    explicit MinigameArte(QWidget *parent = nullptr);

signals:
    //  Se emite al terminar. true si todas correctas
    void quizFinished(bool allCorrect);

protected:
    void paintEvent(QPaintEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;

private slots:
    void handleAnswerClicked();

private:
    struct QA {
        QString question;
        QStringList options; // 4
        int correctIndex;    // 0..3
    };

    // UI
    QPixmap background_;
    QLabel *titleLabel_ = nullptr;
    QLabel *questionLabel_ = nullptr;
    QPushButton *btn_[4] = {nullptr,nullptr,nullptr,nullptr};
    QLabel *progressLabel_ = nullptr;

    // Quiz
    QVector<QA> quiz_;
    int current_ = 0;
    int score_   = 0;
    bool locked_ = false;

    // Imágenes del lienzo (progreso)
    QVector<QPixmap> canvasImgs_;
    int canvasIndex_ = 0;

    // Helpers
    void buildUI();
    void layoutOnBoard();
    void loadQuestions();
    void loadCanvasImages();
    void showQuestion(int i);
    void nextQuestion();
    void endQuiz();
    void styleButtonsDefault();

    // Rectángulos relativos
    QRect boardRect()  const;
    QRect canvasRect() const;
};

#endif // MINIGAMEARTE_H





