#ifndef MINIJUEGOHISTORIA_H
#define MINIJUEGOHISTORIA_H

#include <QWidget>
#include <QPixmap>
#include <QVector>
#include <QString>
#include <QPushButton>
#include <QLabel>

class MinijuegoHistoria : public QWidget
{
    Q_OBJECT
public:
    explicit MinijuegoHistoria(QWidget *parent = nullptr);

signals:
    // ► Se emite al terminar. true si todas correctas (perfecto 5/5)
    void quizFinished(bool allCorrect);

protected:
    void paintEvent(QPaintEvent *e) override;
    void resizeEvent(QResizeEvent *e) override;

private slots:
    void handleAnswerClicked();

private:
    struct QA {
        QString question;
        QStringList options; // 4
        int correctIndex;    // 0..3
    };

    // UI
    QPixmap bg_;
    QLabel *titleLabel_ = nullptr;
    QLabel *questionLabel_ = nullptr;
    QPushButton *btn_[4] = {nullptr,nullptr,nullptr,nullptr};
    QLabel *btnText_[4] = {nullptr,nullptr,nullptr,nullptr};
    QLabel *progressLabel_ = nullptr;
    QLabel *puffleLabel_ = nullptr;

    // Feedback (debajo de opciones)
    QLabel *feedbackLabel_ = nullptr;

    // Sprites puffle (4 moods)
    QVector<QPixmap> puffleMoods_;
    int moodIndex_ = 0;

    // Quiz
    QVector<QA> quiz_;
    int current_ = 0;
    int score_   = 0;

    // Helpers
    void buildUI();
    void placeControls();
    void loadQuestions();
    void showQuestion(int i);
    void nextQuestion();
    void endQuiz();
    void styleButtonsDefault();

    // Geometría relativa
    QRect topPanelRect(int W, int H) const;
    QRect questionRect(int W, int H) const;
    QVector<QRect> tableRects(int W, int H) const;
    QRect puffleRect(int W, int H) const;
};

#endif // MINIJUEGOHISTORIA_H





