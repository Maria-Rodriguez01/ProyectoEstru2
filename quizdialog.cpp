// QuizDialog.cpp
#include "QuizDialog.h"
#include <QVBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QButtonGroup>

QuizDialog::QuizDialog(const MCQ& q, QWidget* parent)
    : QDialog(parent)
{
    setWindowTitle("Pregunta de la puerta");
    setModal(true);
    auto *lay = new QVBoxLayout(this);

    auto *lbl = new QLabel(q.question, this);
    lbl->setWordWrap(true);
    lay->addWidget(lbl);

    auto *grp = new QButtonGroup(this);
    grp->setExclusive(true);

    for (int i=0;i<q.options.size();++i) {
        auto *btn = new QPushButton(q.options[i], this);
        btn->setCheckable(true);
        grp->addButton(btn, i);
        lay->addWidget(btn);
    }

    auto *ok = new QPushButton("Responder", this);
    lay->addWidget(ok);

    connect(ok, &QPushButton::clicked, this, [this, grp](){
        chosen_ = grp->checkedId();
        if (chosen_ == -1) return;
        accept();
    });

    resize(520, 300);
}
