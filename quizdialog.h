// QuizDialog.h
#pragma once
#include <QDialog>
#include "Quiz.h"

class QuizDialog : public QDialog {
    Q_OBJECT
public:
    explicit QuizDialog(const MCQ& q, QWidget* parent=nullptr);
    int chosenIndex() const { return chosen_; }

private:
    int chosen_ = -1;
};
