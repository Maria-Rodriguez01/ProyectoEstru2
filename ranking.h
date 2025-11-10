#ifndef RANKING_H
#define RANKING_H

#include <QDialog>
#include <QTableWidget>
#include <QPushButton>
#include <QVBoxLayout>
#include <QLabel>
#include "rankingstore.h"

class Ranking : public QDialog
{
    Q_OBJECT
public:
    explicit Ranking(qint64 newTimeMs = -1, QWidget *parent = nullptr);

signals:
    void closedBackToMap();

protected:
    void closeEvent(QCloseEvent *e) override;

private slots:
    void saveNewScore(); // si aplica

private:
    qint64 newTimeMs_;
    RankingStore store_;
    QTableWidget* table_ = nullptr;
    QPushButton*  btnBack_ = nullptr;

    void buildUI();
    void reloadTable();
    static QString fmtTime(qint64 ms);
};

#endif // RANKING_H
