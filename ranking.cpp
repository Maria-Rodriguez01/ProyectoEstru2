#include "ranking.h"
#include <QHeaderView>
#include <QInputDialog>
#include <QCloseEvent>

Ranking::Ranking(qint64 newTimeMs, QWidget *parent)
    : QDialog(parent), newTimeMs_(newTimeMs)
{
    setWindowTitle("Ranking de Tiempos");
    setModal(true);
    resize(520, 420);
    buildUI();

    // Si vienes de acabar el juego: pedir nombre y guardar
    if (newTimeMs_ >= 0) saveNewScore();

    reloadTable();
}

void Ranking::buildUI()
{
    auto *lay = new QVBoxLayout(this);

    auto *title = new QLabel("Mejores tiempos");
    title->setStyleSheet("font: bold 14pt 'Times New Roman';");
    lay->addWidget(title);

    table_ = new QTableWidget(this);
    table_->setColumnCount(3);
    table_->setHorizontalHeaderLabels({"Posición", "Jugador", "Tiempo"});
    table_->horizontalHeader()->setSectionResizeMode(0, QHeaderView::ResizeToContents);
    table_->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Stretch);
    table_->horizontalHeader()->setSectionResizeMode(2, QHeaderView::ResizeToContents);
    table_->setEditTriggers(QAbstractItemView::NoEditTriggers);
    table_->setSelectionMode(QAbstractItemView::NoSelection);
    lay->addWidget(table_);

    btnBack_ = new QPushButton("Volver al mapa");
    connect(btnBack_, &QPushButton::clicked, this, &Ranking::close);
    lay->addWidget(btnBack_, 0, Qt::AlignRight);
}

QString Ranking::fmtTime(qint64 ms)
{
    qint64 s  = ms / 1000;
    qint64 msR = ms % 1000;
    qint64 m  = s / 60;
    qint64 sR = s % 60;
    return QString("%1:%2.%3")
        .arg(m, 2, 10, QLatin1Char('0'))
        .arg(sR,2,10,QLatin1Char('0'))
        .arg(msR/10,2,10,QLatin1Char('0')); // centésimas
}

void Ranking::reloadTable()
{
    QVector<ScoreRecord> v = store_.load(); // ya ordenado (quicksort)
    table_->setRowCount(v.size());
    for (int i=0;i<v.size();++i) {
        const ScoreRecord& r = v[i];
        const QString name = QString::fromUtf8(r.name).trimmed();
        table_->setItem(i, 0, new QTableWidgetItem(QString::number(i+1)));
        table_->setItem(i, 1, new QTableWidgetItem(name));
        table_->setItem(i, 2, new QTableWidgetItem(fmtTime(r.millis)));
    }
}

void Ranking::saveNewScore()
{
    // Pide nombre (bloquea hasta que el usuario escriba algo o cancele)
    bool ok = false;
    const QString name = QInputDialog::getText(
        this, "Nuevo record",
        QString("Tu tiempo fue: <b>%1</b>\n\nEscribe tu nombre para guardar en el ranking:")
            .arg(fmtTime(newTimeMs_)),
        QLineEdit::Normal, "", &ok);

    if (ok && !name.trimmed().isEmpty()) {
        store_.add(name.trimmed(), newTimeMs_);
    }
}

void Ranking::closeEvent(QCloseEvent *e)
{
    emit closedBackToMap();
    QDialog::closeEvent(e);
}
