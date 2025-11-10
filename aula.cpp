#include "aula.h"

#include <QGuiApplication>
#include <QScreen>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QKeyEvent>
#include <QPainter>
#include <QLineF>
#include <QResizeEvent>
#include <algorithm>

#include "personaje.h"

// ----- rutas de assets -----
static const char* kAulaBg =
    "C:/Users/Maria Gabriela/OneDrive/Documents/ProyectoEstru2MariaRodriguez/Assets/Aula.png";
static const char* kKantPng =
    "C:/Users/Maria Gabriela/OneDrive/Documents/ProyectoEstru2MariaRodriguez/Assets/EmmanuelKant.png";

// ----------------------------------------------------

Aula::Aula(QWidget *parent)
    : QGraphicsView(parent)
{
    setWindowTitle("Aula — Busca el apunte de Kant");
    setFrameShape(QFrame::NoFrame);
    setRenderHint(QPainter::Antialiasing, true);
    setRenderHint(QPainter::SmoothPixmapTransform, true);
    setAlignment(Qt::AlignCenter);
    setBackgroundBrush(QColor(20,20,20));

    const QRect avail = QGuiApplication::primaryScreen()->availableGeometry();
    const double sx = (avail.width()  * 0.90) / BG_W;
    const double sy = (avail.height() * 0.90) / BG_H;
    const double scale = std::min(1.0, std::min(sx, sy));
    const int  W = int(BG_W * scale);
    const int  H = int(BG_H * scale);
    setMinimumSize(W, H);
    setMaximumSize(W, H);
    resize(W, H);

    scene_ = new QGraphicsScene(this);
    setScene(scene_);
    scene_->setSceneRect(0,0,BG_W,BG_H);

    loadBackground();
    buildCharacters();
    buildBooks();
    buildQuestions();
    buildUI();

    // ⏱️ loop
    timer_ = new QTimer(this);
    timer_->setInterval(100);
    connect(timer_, &QTimer::timeout, this, &Aula::tick);
    timer_->start();

    fitView();

    // 🔔 Emite estado inicial (0 resueltas de 6)
    emitAulaProgress();
}

// ---------- Imagen de fondo ----------
QPixmap Aula::safeLoad(const QString& path, const QSize& fb)
{
    QPixmap pm;
    if (!pm.load(path)) {
        pm = QPixmap(fb);
        pm.fill(Qt::darkGray);
        QPainter p(&pm);
        p.setPen(Qt::white);
        p.drawText(pm.rect(), Qt::AlignCenter, "No se pudo cargar:\n" + path);
    }
    return pm;
}

void Aula::loadBackground()
{
    QPixmap bg = safeLoad(QString::fromUtf8(kAulaBg), QSize(BG_W, BG_H))
    .scaled(BG_W, BG_H, Qt::KeepAspectRatioByExpanding, Qt::SmoothTransformation);
    if (bgItem_) scene_->removeItem(bgItem_);
    bgItem_ = scene_->addPixmap(bg);
    bgItem_->setZValue(-100);
    scene_->setSceneRect(bg.rect());
}

void Aula::fitView()
{
    if (!scene_) return;
    resetTransform();
    fitInView(scene_->sceneRect(), Qt::KeepAspectRatio);
}

// ---------- Construcción ----------
void Aula::buildCharacters()
{
    // Kant
    QPixmap kant = safeLoad(QString::fromUtf8(kKantPng), QSize(200,200))
                       .scaled(180, 180, Qt::KeepAspectRatio, Qt::SmoothTransformation);
    npcKant_ = scene_->addPixmap(kant);
    npcKant_->setZValue(10);
    npcKant_->setPos(BG_W/2.0 - 55, BG_H - 380);

    // Pingüino
    penguin_ = new Personaje();
    scene_->addItem(penguin_);
    penguin_->setSceneBounds(scene_->sceneRect().adjusted(20, 100, -20, -20));
    penguin_->setScale(1.2);
    penguin_->setZValue(12);
    penguin_->setPos(BG_W/2.0 - 40, BG_H - 160);
}

void Aula::buildBooks()
{
    const QList<QRectF> zones = {
        QRectF(150, 530,  70, 60),
        QRectF(365, 360,  70, 60),
        QRectF(275, 360,  70, 60),
        QRectF(220, 430,  70, 60),
        QRectF(845, 430,  70, 60),
        QRectF(665, 360,  70, 60)
    };
    libros_.reserve(zones.size());
    for (const QRectF& r : zones) {
        auto* it = scene_->addRect(r, QPen(Qt::NoPen), QBrush(Qt::NoBrush));
        it->setZValue(5);
        libros_.push_back(it);
    }
}

void Aula::buildQuestions()
{
    questions_ = {
        "1) Seleccione el mandato cuya obligación viene del miedo al castigo o la búsqueda de un premio:",
        "2) Para Kant, es posible conocer lo que las cosas nos permiten a través de los sentidos:",
        "3) Kant decía que el lema de la ilustración era “Sapere aude”, que significa:",
        "4) Kant cambia el centro del conocimiento del objeto al sujeto. A esto se le llama:",
        "5) La postura conciliadora de Kant (entre empiristas y racionalistas) se llama:",
        "6) De las siguientes obras de Kant, seleccione la que define su epistemología:"
    };
    optA_ = { "Imperativo Hipotético", "Conocimiento Nouménico", "Sopesa tus acciones",
             "Subjetivismo", "Racionalismo", "Crítica de la razón práctica" };
    optB_ = { "Imperativo categórico", "Conocimiento Fenoménico", "Atrévete a saber por ti mismo",
             "Prejuicio", "Empirismo", "Crítica de la razón pura" };
    optC_ = { "Ambos", "Conocimiento Empírico", "Saber a la fuerza",
             "Giro Copernicano", "Criticismo", "Crítica del juicio" };
    optD_ = { "Ninguno", "Conocimiento Racional", "Someterse al conocimiento",
             "Suerte", "Escepticismo", "Crítica fenomenológica" };
    answers_ = { 0, 1, 1, 2, 2, 1 };
    pistas_[1] = "1) Su primera edición fue en 1781.";
    pistas_[2] = "2) Es su obra principal.";
    pistas_[3] = "3) Es una indagación 'trascendental'.";
    pistas_[4] = "4) Intenta conjuntar racionalismo y empirismo.";
    pistas_[5] = "5) Está dividida en dos grandes secciones.";
    pistas_[6] = "6) Estudia la razón para comprender su funcionamiento y estructura.";
}

void Aula::buildUI()
{
    // --- Hints ---
    hintLabel_ = new QLabel(" ");
    hintLabel_->setStyleSheet(
        "background:rgba(248,229,151,220);"
        "border:2px solid #b58a3a; border-radius:10px;"
        "padding:6px 12px; color:#2b1d02; font: 12pt 'Times New Roman';");
    hintLabel_->setAttribute(Qt::WA_TransparentForMouseEvents);
    hintProxy_ = scene_->addWidget(hintLabel_);
    hintProxy_->setZValue(1000);
    hintProxy_->setPos(16, 12);
    hintProxy_->hide();

    // --- Panel de preguntas ---
    qPanel_ = new QWidget();
    qPanel_->setStyleSheet("background:rgba(0,0,0,180); border:1px solid #d2b48c;");
    auto* v = new QVBoxLayout(qPanel_);
    v->setContentsMargins(14,12,14,12);
    v->setSpacing(8);
    qTitle_ = new QLabel("Pregunta");
    qTitle_->setStyleSheet("color:#ffd87a; font:bold 14pt 'Times New Roman';");
    qText_  = new QLabel("...");
    qText_->setStyleSheet("color:white; font:12pt 'Times New Roman';");
    qText_->setWordWrap(true);
    v->addWidget(qTitle_);
    v->addWidget(qText_);

    auto* opts = new QWidget();
    auto* h = new QHBoxLayout(opts);
    h->setContentsMargins(0,0,0,0);
    h->setSpacing(10);
    for (int i=0;i<4;i++){
        auto* b = new QPushButton(QString("Opción %1").arg(i+1));
        b->setStyleSheet(
            "QPushButton{background:#2d2d2d; color:#eee; border:1px solid #888; "
            "border-radius:6px; padding:8px;} "
            "QPushButton:hover{background:#3a3a3a;}");
        connect(b, &QPushButton::clicked, this, &Aula::onOptionClicked);
        h->addWidget(b);
        qOpts_.push_back(b);
    }
    v->addWidget(opts);
    auto* pp = scene_->addWidget(qPanel_);
    pp->setZValue(1500);
    pp->setPos(BG_W*0.12, BG_H*0.12);
    qPanel_->hide();

    // --- Panel de respuesta final ---
    answerPanel_ = new QWidget();
    answerPanel_->setStyleSheet(
        "background:rgba(0,0,0,180); border:1px solid #d2b48c; border-radius:10px;");
    auto *av = new QVBoxLayout(answerPanel_);
    av->setContentsMargins(14,12,14,12);
    av->setSpacing(8);
    answerTitle_ = new QLabel("¿En qué libro estaba el apunte?");
    answerTitle_->setStyleSheet("color:#ffd87a; font:bold 14pt 'Times New Roman';");
    answerEdit_ = new QLineEdit();
    answerEdit_->setPlaceholderText("Escribe aquí el nombre del libro…");
    answerEdit_->setStyleSheet(
        "QLineEdit{background:#1e1e1e; color:#eee; border:1px solid #888; "
        "border-radius:6px; padding:8px; font:11pt 'Times New Roman';}"
        "QLineEdit:focus{border:1px solid #d2b48c;}");
    answerBtn_ = new QPushButton("Confirmar");
    answerBtn_->setStyleSheet(
        "QPushButton{background:#5b3a17; color:#f2d295; border:1px solid #d2b48c; "
        "border-radius:6px; padding:8px 14px; font: 11pt 'Times New Roman';}"
        "QPushButton:hover{background:#6a4420;}");
    connect(answerBtn_, &QPushButton::clicked, this, &Aula::submitAnswer);
    av->addWidget(answerTitle_);
    av->addWidget(answerEdit_);
    av->addWidget(answerBtn_, 0, Qt::AlignRight);
    answerProxy_ = scene_->addWidget(answerPanel_);
    answerProxy_->setZValue(1600);
    answerProxy_->setPos(BG_W*0.22, BG_H*0.20);
    answerPanel_->hide();
}

// ---------- Toast ----------
void Aula::ensureToast() {
    if (toastLabel_) return;

    toastLabel_ = new QLabel;
    toastLabel_->setAttribute(Qt::WA_TranslucentBackground);
    toastLabel_->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    toastLabel_->setWordWrap(true);
    toastLabel_->setMargin(10);
    toastLabel_->setStyleSheet(
        "QLabel { background: rgba(0,0,0,180); color: #f8f1d2; "
        "border: 1px solid #d2b48c; border-radius: 10px; font: 12pt 'Times New Roman'; }");
    toastProxy_ = scene_->addWidget(toastLabel_);
    toastProxy_->setZValue(3000);
    toastProxy_->hide();
    toastTimer_ = new QTimer(this);
    toastTimer_->setSingleShot(true);
    connect(toastTimer_, &QTimer::timeout, this, [this]() { if (toastProxy_) toastProxy_->hide(); });
}

void Aula::showToast(const QString &text, int msec) {
    ensureToast();
    toastLabel_->setText(text);
    toastLabel_->adjustSize();
    const QRectF R = scene_->sceneRect();
    const QSize S = toastLabel_->size();
    const int margin = 14;
    const QPointF pos(R.center().x() - S.width()/2.0, R.bottom() - S.height() - margin);
    toastProxy_->setPos(pos);
    toastProxy_->show();
    toastTimer_->start(msec);
}

// ---------- Proximidad ----------
int Aula::nearestBook() const {
    if (!penguin_) return -1;
    const QPointF hero = penguin_->sceneBoundingRect().center();
    int best=-1; qreal bestD=1e9;
    for (int i=0;i<libros_.size();++i){
        const QPointF c = libros_[i]->rect().center();
        const qreal d = QLineF(hero, c).length();
        if (d<bestD){bestD=d;best=i;}
    }
    if (best>=0 && bestD<=BOOK_RADIUS) return best;
    return -1;
}

bool Aula::nearNPC() const {
    if (!penguin_ || !npcKant_) return false;
    const QPointF a = penguin_->sceneBoundingRect().center();
    const QPointF b = npcKant_->sceneBoundingRect().center();
    return (QLineF(a, b).length() <= NPC_RADIUS);
}

void Aula::setHintText(const QString& t){ hintLabel_->setText(t); hintLabel_->adjustSize(); hintProxy_->show(); }
void Aula::placeHintOver(const QRectF&){ hintProxy_->setPos(16,12); }

// ---------- Bucle ----------
void Aula::tick()
{
    if (panelAbierto_) { hintProxy_->hide(); return; }
    hintProxy_->hide();
    const int nb = nearestBook();
    if (nb >= 0 && !librosResueltos_.contains(nb)) { setHintText("Pulsa E para leer"); return; }
    if (nearNPC()) { setHintText("Pulsa E para hablar"); return; }
}

void Aula::resizeEvent(QResizeEvent *event)
{
    QGraphicsView::resizeEvent(event);
    fitView();
}

void Aula::keyPressEvent(QKeyEvent *e)
{
    if (panelAbierto_ && answerPanel_ && answerPanel_->isVisible()) {
        QGraphicsView::keyPressEvent(e);
        return;
    }
    if (!penguin_) return;
    if (e->isAutoRepeat()) return;

    switch (e->key()) {
    case Qt::Key_W: case Qt::Key_Up:    penguin_->startMove(Personaje::Direction::Up);    break;
    case Qt::Key_S: case Qt::Key_Down:  penguin_->startMove(Personaje::Direction::Down);  break;
    case Qt::Key_A: case Qt::Key_Left:  penguin_->startMove(Personaje::Direction::Left);  break;
    case Qt::Key_D: case Qt::Key_Right: penguin_->startMove(Personaje::Direction::Right); break;

    case Qt::Key_E: {
        const int nb = nearestBook();
        if (nb >= 0 && !librosResueltos_.contains(nb)) { abrirPregunta(nb); return; }
        if (nearNPC()) {
            if (librosResueltos_.size() == libros_.size()) showAnswerPanel();
            else showToast("Kant: “Ayúdame buscando entre los libros…”", 2000);
            return;
        }
        break;
    }
    default: break;
    }
}

void Aula::keyReleaseEvent(QKeyEvent *e)
{
    if (panelAbierto_ && answerPanel_ && answerPanel_->isVisible()) {
        QGraphicsView::keyPressEvent(e);
        return;
    }
    if (!penguin_ || e->isAutoRepeat()) return;
    switch (e->key()) {
    case Qt::Key_W: case Qt::Key_S: case Qt::Key_A: case Qt::Key_D:
    case Qt::Key_Up: case Qt::Key_Down: case Qt::Key_Left: case Qt::Key_Right:
        penguin_->stopMove(); break;
    default: break;
    }
}

// ---------- Preguntas ----------
void Aula::abrirPregunta(int idx) {
    currentBook_ = idx;
    const int total = questions_.size();
    const int qIdx = std::max(0, std::min(idx, total - 1));

    qTitle_->setText(QString("Pregunta %1").arg(qIdx + 1));
    qText_->setText(questions_[qIdx]);
    qOpts_[0]->setText("A) " + optA_[qIdx]);
    qOpts_[1]->setText("B) " + optB_[qIdx]);
    qOpts_[2]->setText("C) " + optC_[qIdx]);
    qOpts_[3]->setText("D) " + optD_[qIdx]);

    for (auto* b : qOpts_) b->setVisible(true);
    qPanel_->show();
    showingHint_ = false;
}

void Aula::onOptionClicked()
{
    if (currentBook_ < 0) return;

    auto* btn    = qobject_cast<QPushButton*>(sender());
    const int chosen = qOpts_.indexOf(btn);
    if (chosen < 0) return;

    const int total  = questions_.size();
    const int qIdx   = std::max(0, std::min(currentBook_, total - 1));
    const int correct= answers_.value(qIdx, 0);

    if (chosen == correct) {
        librosResueltos_.insert(currentBook_);
        const QString pista = pistas_.value(qIdx + 1, "Pista no disponible");

        if (qPanel_) qPanel_->hide();
        showToast(QString("¡Correcto!\n\n%1").arg(pista), 3000);

        // 🔔 Emite progreso tras acertar
        emitAulaProgress();

        if (librosResueltos_.size() == libros_.size()) {
            QTimer::singleShot(1800, [this](){ showToast("Vuelve con Kant y di el nombre del libro.", 2600); });
        }
        currentBook_ = -1;
    } else {
        if (qTitle_) qTitle_->setText("Respuesta incorrecta");
        if (qText_)  qText_->setText("Inténtalo de nuevo.");
    }
}

// ---------- Respuesta final ----------
void Aula::showAnswerPanel()
{
    panelAbierto_ = true;
    answerPanel_->show();
    answerProxy_->show();
    answerProxy_->setFocus(Qt::OtherFocusReason);
    scene_->setFocusItem(answerProxy_);
    answerEdit_->setFocus(Qt::OtherFocusReason);
    answerEdit_->selectAll();
}

QString Aula::normalize(const QString &s)
{
    QString t = s.simplified().toLower();
    t.replace(u'á', u'a'); t.replace(u'é', u'e'); t.replace(u'í', u'i');
    t.replace(u'ó', u'o'); t.replace(u'ú', u'u'); t.replace(u'ü', u'u');
    t.replace(u'ñ', u'n');
    return t;
}

void Aula::submitAnswer()
{
    const QString txt = normalize(answerEdit_->text());
    const QString correcto = normalize("Crítica de la razón pura");

    if (txt == correcto) {
        showToast("¡Correcto!", 1800);
        if (answerProxy_) answerProxy_->hide();
        if (answerPanel_) answerPanel_->hide();
        panelAbierto_ = false;

        // Asegura que quedamos marcados como completado (por si acaso)
        emitAulaProgress();

        // Cerrar Aula; (deja que Escuela controle el flujo)
        QTimer::singleShot(1200, this, [this](){close();});
    } else {
        showToast("Casi... intenta escribirlo tal cual.", 1800);
        answerEdit_->setFocus();
        answerEdit_->selectAll();
    }
}

// ---------- Emisor de progreso ----------
void Aula::emitAulaProgress()
{
    const int respondidas = librosResueltos_.size();
    const int total       = libros_.size(); // 6
    const bool done       = (respondidas >= total);
    emit progresoAula(done, respondidas, total);
}
