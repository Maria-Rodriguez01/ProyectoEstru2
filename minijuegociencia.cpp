#include "minijuegociencia.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPainter>
#include <QRandomGenerator>
#include <QFileInfo>
#include <QMessageBox>
#include <QtMath>

QString minijuegociencia::ASSETS(const QString &rel)
{
    static const QString kBase =
        "C:/Users/Maria Gabriela/OneDrive/Documents/ProyectoEstru2MariaRodriguez/Assets/";
    return kBase + rel;
}

minijuegociencia::minijuegociencia(QWidget *parent)
    : QWidget(parent)
{
    setWindowTitle("Puffle Trivia — Ciencia");
    setMinimumSize(1000, 650);
    bg = QPixmap(ASSETS("ciencia.png"));

    // Layout raíz
    auto *root = new QVBoxLayout(this);
    root->setContentsMargins(0,0,0,0);
    root->setSpacing(0);

    // Título
    lblTitulo = new QLabel("¡Atrapa el puffle correcto!", this);
    lblTitulo->setAlignment(Qt::AlignHCenter | Qt::AlignTop);
    lblTitulo->setStyleSheet("color:#0b1738; font: 700 18px 'Segoe UI'; padding: 10px;");
    root->addWidget(lblTitulo, 0);

    // HUD inferior
    hud = new QWidget(this);
    hud->setFixedHeight(HUD_H);
    hud->setStyleSheet("background: rgba(0,0,0,0.55);");
    auto *hudLay = new QVBoxLayout(hud);
    hudLay->setContentsMargins(14,10,14,10);
    hudLay->setSpacing(8);

    lblPregunta = new QLabel(this);
    lblPregunta->setWordWrap(true);
    lblPregunta->setStyleSheet("color:#ffffff; font: 600 14px 'Segoe UI';");
    hudLay->addWidget(lblPregunta);

    lblOpciones = new QLabel(this);
    lblOpciones->setWordWrap(true);
    lblOpciones->setStyleSheet("color:#dff3ff; font: 12px 'Segoe UI';");
    hudLay->addWidget(lblOpciones);

    root->addWidget(hud, 0, Qt::AlignBottom);

    // Crear los 4 puffles
    const QString letras[4] = {"A","B","C","D"};
    puffles.resize(4);
    for (int i = 0; i < puffles.size(); ++i) {
        auto &p = puffles[i];
        p.letra = letras[i];
        p.btn = new QPushButton(this);
        p.btn->setFixedSize(PUFFLE_SIZE, PUFFLE_SIZE);
        p.btn->setCursor(Qt::PointingHandCursor);
        p.btn->show();
        applyPuffleSkin(p.btn, p.letra);
        connect(p.btn, &QPushButton::clicked, this, &minijuegociencia::onPuffleClicked);

        const double ang = QRandomGenerator::global()->generateDouble() * (2.0*M_PI);
        const double speed = SPEED_MIN + QRandomGenerator::global()->generateDouble() * (SPEED_MAX - SPEED_MIN);
        p.vel = QPointF(qCos(ang)*speed, qSin(ang)*speed);
    }

    // Cargar preguntas
    cargarPreguntas();
    setPreguntaActual(0);

    for (auto &p : puffles) placeRandom(p.btn);

    timer = new QTimer(this);
    connect(timer, &QTimer::timeout, this, &minijuegociencia::tick);
    timer->start(FPS_MS);
    clock.start();
}

minijuegociencia::~minijuegociencia() {}

void minijuegociencia::paintEvent(QPaintEvent *)
{
    QPainter pr(this);
    pr.setRenderHint(QPainter::SmoothPixmapTransform, true);
    if (!bg.isNull()) pr.drawPixmap(rect(), bg);
    else pr.fillRect(rect(), QColor(25,25,25));
}

void minijuegociencia::resizeEvent(QResizeEvent *e)
{
    QWidget::resizeEvent(e);
    hud->move(0, height() - HUD_H);
    hud->resize(width(), HUD_H);
}

bool minijuegociencia::fileExists(const QString &absPath) const
{
    QFileInfo fi(absPath);
    return fi.exists() && fi.isFile();
}

QRect minijuegociencia::playArea() const
{
    const int greenTop = qRound(GREEN_TOP_RATIO * height());
    QRect r(0, greenTop, width(), height() - greenTop - HUD_H);
    r.adjust(8, 8, -PUFFLE_SIZE-8, -PUFFLE_SIZE-8);
    return r;
}

void minijuegociencia::placeRandom(QWidget *w)
{
    const QRect r = playArea();
    const int x = QRandomGenerator::global()->bounded(r.left(),  r.right());
    const int y = QRandomGenerator::global()->bounded(r.top(),   r.bottom());
    w->move(x, y);
}

void minijuegociencia::applyPuffleSkin(QPushButton *btn, const QString &letter)
{
    const QString path = ASSETS("sprites/puffle_" + letter + ".png");
    if (fileExists(path)) {
        btn->setStyleSheet(
            "QPushButton{border-radius:48px;"
            "background-color: transparent;"
            "background-image: url(" + path + ");"
                     "background-repeat: no-repeat;"
                     "background-position: center;"
                     "background-size: contain;}"
            );
        btn->setText("");
    } else {
        btn->setText(letter);
        btn->setStyleSheet(
            "QPushButton{border-radius:48px; background:#86d7ff; color:#fff; font:700 22px 'Segoe UI';}"
            );
    }
}

void minijuegociencia::tick()
{
    const double dt = qMin<double>(clock.restart()/1000.0, 0.05);
    const QRect r = playArea();

    for (auto &p : puffles) {
        QPointF pos = p.btn->pos();
        pos += p.vel * dt;

        if (pos.x() < r.left())   { pos.setX(r.left());   p.vel.setX(qAbs(p.vel.x())); }
        if (pos.x() > r.right())  { pos.setX(r.right());  p.vel.setX(-qAbs(p.vel.x())); }
        if (pos.y() < r.top())    { pos.setY(r.top());    p.vel.setY(qAbs(p.vel.y())); }
        if (pos.y() > r.bottom()) { pos.setY(r.bottom()); p.vel.setY(-qAbs(p.vel.y())); }

        p.btn->move(pos.toPoint());
    }
}

void minijuegociencia::onPuffleClicked()
{
    auto *btn = qobject_cast<QPushButton*>(sender());
    if (!btn) return;

    QString letra;
    for (const auto &p : puffles)
        if (p.btn == btn)
            letra = p.letra;

    const QItem &q = banco[qIndex];
    const QString correctaLetra = QString("ABCD").mid(q.correcta, 1);

    // Mostrar resultado directamente en lblTitulo
    if (letra == correctaLetra) {
        lblTitulo->setText("¡Has atrapado el puffle correcto!");
        lblTitulo->setStyleSheet("color: #114510; font: 700 18px 'Segoe UI'; padding: 10px;");
        ++aciertos;
    } else {
        lblTitulo->setText("Oops, has agarrado el puffle incorrecto!");
        lblTitulo->setStyleSheet("color: #7a0f0f; font: 700 18px 'Segoe UI'; padding: 10px;");
    }

    // Esperar un segundo antes de pasar a la siguiente pregunta
    QTimer::singleShot(1000, this, [this]() {
        qIndex++;
        if (qIndex >= banco.size()) {
            // Fin del juego
            if (aciertos == banco.size()) {
                lblTitulo->setText("¡Atrapaste todos los puffles!");
                lblTitulo->setStyleSheet("color: #114510; font: 700 18px 'Segoe UI'; padding: 10px;");
            } else {
                lblTitulo->setText("No atrapaste todos los puffles");
                lblTitulo->setStyleSheet("color: #7a0f0f; font: 700 18px 'Segoe UI'; padding: 10px;");
            }

            emit juegoCompletado(aciertos == banco.size(), aciertos, banco.size());
            QTimer::singleShot(2000, this, &QWidget::close);
        } else {
            lblTitulo->setText("¡Atrapa el puffle correcto!");
            lblTitulo->setStyleSheet("color: #071152; font: 700 18px 'Segoe UI'; padding: 10px;");
            setPreguntaActual(qIndex);
        }
    });
}



void minijuegociencia::cargarPreguntas()
{
    banco.clear();
    banco.push_back({"Entre los siguientes renacentistas seleccione, uno de los precursores filósofo-científico del heliocentrismo (teoría que afirma que el sol es el centro del universo):",
                     {"Tomas Moro","Galileo","Platón","Arquimedes"}, 1});
    banco.push_back({"El método científico se introduce por el interés de tres filósofos. Entre los siguientes uno de los mencionados NO es precursor del método científico:",
                     {"Francis Bacon","Galileo Galilei","Nicolas Maquiavelo","René Descartes"}, 2});
    banco.push_back({"Es uno de los precursores del pensamiento Moderno:",
                     {"Isaac Newton","René Descartes","Erasmo de Roterdam","Francis Bacon"}, 1});
    banco.push_back({"De los siguientes filósofos niega el geocentrismo (teoría que afirma que el centro de nuestro sistema solar es la tierra):",
                     {"Aristóteles","Nicolás Copérnico","Tomás de Aquino","Isaac Newton"}, 1});
    banco.push_back({"Uno de los inventos que suscitó un conocimiento ilimitado, fue el de Gutenberg:",
                     {"El astrolabio","La imprenta","La Nao y la Carabela","El Telescopio"}, 1});
}

void minijuegociencia::setPreguntaActual(int idx)
{
    const QItem &q = banco[idx];
    lblPregunta->setText(QString("Pregunta %1/%2: %3")
                             .arg(idx+1).arg(banco.size()).arg(q.texto));
    lblOpciones->setText(
        QString("A) %1    B) %2    C) %3    D) %4")
            .arg(q.opciones.value(0))
            .arg(q.opciones.value(1))
            .arg(q.opciones.value(2))
            .arg(q.opciones.value(3))
        );
}

