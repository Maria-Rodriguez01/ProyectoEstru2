#include "mapa.h"

#include <QPainter>
#include <QResizeEvent>
#include <QApplication>
#include <QScreen>
#include <QGuiApplication>
#include <QLabel>
#include <QVBoxLayout>

#include "lobby.h"
#include "nivel1.h"
#include "mainwindow.h"      // Nivel 2
#include "combatenivel3.h"   // Nivel 3
#include "escuela.h"         // Nivel 4

// Ranking + Timer global
#include "gamesession.h"
#include "ranking.h"

// Cambia esta ruta a donde tengas tu fondo
static const char* kMapaBg =
    "C:/Users/Maria Gabriela/OneDrive/Documents/ProyectoEstru2MariaRodriguez/Assets/Mapa.jpg";

Mapa::Mapa(QWidget *parent)
    : QWidget(parent)
{
    setWindowTitle("Mapa del Conocimiento");
    setAttribute(Qt::WA_StyledBackground, true);

    // Tamaño de ventana agradable (90% pantalla)
    const QRect avail = QGuiApplication::primaryScreen()->availableGeometry();
    const int W = int(avail.width()  * 0.90);
    const int H = int(avail.height() * 0.90);
    resize(W, H);

    cargarFondo();
    inicializarGrafo();
    colocarBotones();
    actualizarBotones();
    actualizarRankingBtn();

    // Al entrar al Mapa empieza el cronómetro (nuevo jugador)
    GameSession::instance().start();
}

void Mapa::cargarFondo()
{
    if (!bgOrig_.load(kMapaBg)) {
        bgOrig_ = QPixmap(1200, 750);
        bgOrig_.fill(Qt::darkGray);
        QPainter p(&bgOrig_);
        p.setPen(Qt::white);
        p.drawText(bgOrig_.rect(), Qt::AlignCenter,
                   "No se pudo cargar\n" + QString::fromUtf8(kMapaBg));
    }
    bgScaled_ = bgOrig_.scaled(size(), Qt::KeepAspectRatio, Qt::SmoothTransformation);
}

void Mapa::inicializarGrafo()
{
    niveles_.clear();

    niveles_["Nivel 1"] = {"Nivel 1", QPointF(0.23, 0.20), true,  {"Nivel 2"}, nullptr};
    niveles_["Nivel 2"] = {"Nivel 2", QPointF(0.38, 0.52), false, {"Nivel 1", "Nivel 3"}, nullptr};
    niveles_["Nivel 3"] = {"Nivel 3", QPointF(0.63, 0.72), false, {"Nivel 2", "Nivel 4"}, nullptr};
    niveles_["Nivel 4"] = {"Nivel 4", QPointF(0.58, 0.42), false, {"Nivel 3"}, nullptr};
}

QRect Mapa::drawRect() const
{
    if (bgScaled_.isNull()) return rect();
    const int x = (width()  - bgScaled_.width())  / 2;
    const int y = (height() - bgScaled_.height()) / 2;
    return QRect(QPoint(x, y), bgScaled_.size());
}

void Mapa::colocarBotones()
{
    const QRect R = drawRect();
    const int imgW = R.width();
    const int imgH = R.height();
    const int x0 = R.left();
    const int y0 = R.top();

    const char* css =
        "QPushButton{background:rgba(0,0,0,140); color:#ffd87a; border:2px solid #d2b48c;"
        "border-radius:10px; padding:6px 10px; font: 12pt 'Times New Roman';}"
        "QPushButton:disabled{background:rgba(0,0,0,60); color:#888; border:1px solid #777;}";

    // Botones de niveles
    for (auto &n : niveles_) {
        if (!n.boton) {
            n.boton = new QPushButton(n.nombre, this);
            connect(n.boton, &QPushButton::clicked, this, [this, name = n.nombre]() {
                abrirNivel(name);
            });
            n.boton->setStyleSheet(css);
        }

        const int bw = 120, bh = 40;
        const int x = x0 + int(n.posRel.x() * imgW) - bw/2;
        const int y = y0 + int(n.posRel.y() * imgH) - bh/2;
        n.boton->setGeometry(x, y, bw, bh);
        n.boton->raise();
        n.boton->show();
    }

    // Botón Ranking (abajo-derecha)
    if (!btnRanking_) {
        btnRanking_ = new QPushButton("Ranking", this);
        btnRanking_->setStyleSheet(css);
        connect(btnRanking_, &QPushButton::clicked, this, &Mapa::abrirRanking);
    }
    {
        const int bw = 140, bh = 40, m = 16;
        const int x = R.right() - bw - m;
        const int y = R.bottom() - bh - m;
        btnRanking_->setGeometry(x, y, bw, bh);
        btnRanking_->raise();
        btnRanking_->show();
    }

    // Botón Lobby (abajo-izquierda)
    if (!btnLobby_) {
        btnLobby_ = new QPushButton("Lobby", this);
        btnLobby_->setStyleSheet(css);
        connect(btnLobby_, &QPushButton::clicked, this, &Mapa::volverLobby);
    }
    {
        const int bw = 140, bh = 40, m = 16;
        const int x = R.left() + m;
        const int y = R.bottom() - bh - m;
        btnLobby_->setGeometry(x, y, bw, bh);
        btnLobby_->raise();
        btnLobby_->show();
    }
}

void Mapa::paintEvent(QPaintEvent *)
{
    QPainter p(this);
    if (!bgScaled_.isNull())
        p.drawPixmap(drawRect().topLeft(), bgScaled_);
}

void Mapa::resizeEvent(QResizeEvent *e)
{
    QWidget::resizeEvent(e);
    bgScaled_ = bgOrig_.scaled(size(), Qt::KeepAspectRatio, Qt::SmoothTransformation);
    colocarBotones();
    update();
}

void Mapa::actualizarBotones()
{
    for (auto &n : niveles_) {
        if (n.boton)
            n.boton->setEnabled(n.desbloqueado);
    }
    actualizarRankingBtn();
}

void Mapa::desbloquearVecinos(const QString &nivelActual)
{
    for (const QString &vecino : niveles_[nivelActual].conexiones) {
        niveles_[vecino].desbloqueado = true;
    }
    actualizarBotones();
}

bool Mapa::allLevelsCompleted() const
{
    static const QStringList req = {"Nivel 1","Nivel 2","Nivel 3","Nivel 4"};
    for (const QString &n : req)
        if (!completados_.contains(n)) return false;
    return true;
}

void Mapa::actualizarRankingBtn()
{
    if (!btnRanking_) return;
    // El ranking solo se habilita cuando TODOS están completados
    btnRanking_->setEnabled(allLevelsCompleted());
}

void Mapa::abrirNivel(const QString &nombreNivel)
{
    if (!niveles_[nombreNivel].desbloqueado)
        return;

    nivelActivo_ = nombreNivel;
    qApp->setQuitOnLastWindowClosed(false);

    QWidget *nivelWidget = nullptr;

    if (nombreNivel == "Nivel 1")      nivelWidget = new Nivel1();
    else if (nombreNivel == "Nivel 2") nivelWidget = new MainWindow();
    else if (nombreNivel == "Nivel 3") nivelWidget = new CombateNivel3(1, nullptr);
    else if (nombreNivel == "Nivel 4") nivelWidget = new Escuela();

    if (!nivelWidget) return;

    nivelWidget->setAttribute(Qt::WA_DeleteOnClose);

    // Al cerrar el nivel → volvemos a mostrar el mapa, desbloqueamos vecinos y marcamos completado
    connect(nivelWidget, &QObject::destroyed, this, [this, nombreNivel]() {
        // marcar completado
        completados_.insert(nombreNivel);
        desbloquearVecinos(nombreNivel);

        // Si se cerró Escuela (Nivel 4): detener el timer y marcar la corrida
        if (nombreNivel == "Nivel 4") {
            GameSession::instance().stop();
            lastRunMs_ = GameSession::instance().elapsedMs();

            // marca todos completados (por si faltó marcar alguno manualmente)
            completados_.insert("Nivel 1");
            completados_.insert("Nivel 2");
            completados_.insert("Nivel 3");
            completados_.insert("Nivel 4");
            actualizarRankingBtn();
        }

        this->show();
        this->raise();
        this->activateWindow();
    });

    nivelWidget->show();
    this->hide();
}

void Mapa::abrirRanking()
{
    Ranking dlg(lastRunMs_, this);            // pasa el último tiempo si viene de Nivel 4
    connect(&dlg, &Ranking::closedBackToMap, this, [this]() {
        this->show();
        this->raise();
        this->activateWindow();
    });
    dlg.exec();
}


void Mapa::volverLobby()
{
    // Aquí reseteamos el timer para un NUEVO jugador
    GameSession::instance().reset();

    // Si tienes una clase Lobby real, ábrela aquí. Ejemplo:
     auto* lobbyVentana = new Lobby();
     lobbyVentana->setAttribute(Qt::WA_DeleteOnClose);
     lobbyVentana->show();
     this->close();

}



