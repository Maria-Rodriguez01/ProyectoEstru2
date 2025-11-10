#include "minijuegohistoria.h"

#include <QPainter>
#include <QTimer>
#include <QLabel>
#include <QPushButton>
#include <QMessageBox>
#include <QGraphicsOpacityEffect>
#include <QPropertyAnimation>
#include <QAbstractAnimation>
#include <QtMath>

// ==== RUTAS (ajusta a tus archivos locales) ====
static const char* kBgPath =
    "C:/Users/Maria Gabriela/OneDrive/Documents/ProyectoEstru2MariaRodriguez/Assets/historia.png";
static const char* kPufflePath =
    "C:/Users/Maria Gabriela/OneDrive/Documents/ProyectoEstru2MariaRodriguez/Assets/puffle.png";

// ------------------------------------------------
// ctor
// ------------------------------------------------
MinijuegoHistoria::MinijuegoHistoria(QWidget *parent)
    : QWidget(parent)
{
    setAttribute(Qt::WA_DeleteOnClose);
    setWindowTitle("Historia — Quiz");
    setFixedSize(1152, 648);

    // Fondo
    bg_.load(QString::fromUtf8(kBgPath));

    // Puffle (corta 4 moods de grilla 2x2)
    QPixmap sheet(QString::fromUtf8(kPufflePath));
    if (!sheet.isNull()) {
        const int cols = 2, rows = 2;
        const int cw = sheet.width() / cols;
        const int ch = sheet.height() / rows;
        for (int r=0; r<rows; ++r)
            for (int c=0; c<cols; ++c)
                puffleMoods_.push_back(sheet.copy(c*cw, r*ch, cw, ch));
    }
    if (puffleMoods_.isEmpty()) {
        for (int i=0;i<4;++i) puffleMoods_.push_back(QPixmap());
    }

    buildUI();
    loadQuestions();
    showQuestion(0);
}

// ---------- Geometría relativa ----------
QRect MinijuegoHistoria::topPanelRect(int W, int H) const {
    return QRect(int(W*0.245), int(H*0.155), int(W*0.455), int(H*0.185));
}
QRect MinijuegoHistoria::questionRect(int W, int H) const {
    QRect p = topPanelRect(W,H);
    const int padL = int(p.width()*0.15);
    const int padR = int(p.width()*0.15);
    const int padT = int(p.height()*0.26);
    const int padB = int(p.height()*0.20);
    return QRect(p.x()+padL, p.y()+padT, p.width()-padL-padR, p.height()-padT-padB);
}
QVector<QRect> MinijuegoHistoria::tableRects(int W, int H) const {
    QVector<QRect> r; r.reserve(4);
    const int y   = int(H * 0.525);
    const int h   = int(H * 0.13);
    const int w   = int(W * 0.185);
    const int gap = int(W * 0.038);
    int x = int(W * 0.055);
    for (int i=0;i<4;++i) { r.push_back(QRect(x, y, w, h)); x += w + gap; }
    return r;
}
QRect MinijuegoHistoria::puffleRect(int W, int H) const {
    const int pw = int(W * 0.115);
    const int ph = int(H * 0.235);
    const int px = int(W * 0.400);
    const int py = int(H * 0.40) - ph/2;
    return QRect(px, py, pw, ph);
}

// ---------- UI ----------
void MinijuegoHistoria::buildUI()
{
    titleLabel_ = new QLabel("", this);
    titleLabel_->setAlignment(Qt::AlignCenter);
    titleLabel_->setStyleSheet(
        "color:#2b1a10; background:transparent; font: 700 12pt 'Times New Roman';");

    questionLabel_ = new QLabel(this);
    questionLabel_->setWordWrap(true);
    questionLabel_->setAlignment(Qt::AlignLeft | Qt::AlignTop);
    questionLabel_->setStyleSheet(
        "color:#2b1a10; background:transparent; font: 700 11pt 'Times New Roman';");

    for (int i = 0; i < 4; ++i) {
        btn_[i] = new QPushButton(this);
        btn_[i]->setFlat(true);
        btn_[i]->setCursor(Qt::PointingHandCursor);
        btn_[i]->setStyleSheet(
            "QPushButton{ background:transparent; border:none; }"
            "QPushButton:hover{ background:rgba(255,255,255,0.08); }"
            "QPushButton:pressed{ background:rgba(255,255,255,0.16); }");

        btnText_[i] = new QLabel(btn_[i]);
        btnText_[i]->setWordWrap(true);
        btnText_[i]->setAlignment(Qt::AlignCenter);
        btnText_[i]->setStyleSheet(
            "color:#2b1a10; background:transparent; font: 700 10pt 'Times New Roman';");
        btnText_[i]->setAttribute(Qt::WA_TransparentForMouseEvents, true);

        connect(btn_[i], &QPushButton::clicked, this, &MinijuegoHistoria::handleAnswerClicked);
    }

    progressLabel_ = new QLabel(this);
    progressLabel_->setAlignment(Qt::AlignCenter);
    progressLabel_->setStyleSheet(
        "color:#2b1a10; background:transparent; font: 600 10pt 'Times New Roman';");

    puffleLabel_ = new QLabel(this);
    puffleLabel_->setScaledContents(true);
    puffleLabel_->setPixmap(puffleMoods_[0]);

    // Feedback debajo de las opciones (fondo transparente)
    feedbackLabel_ = new QLabel(this);
    feedbackLabel_->setAlignment(Qt::AlignCenter);
    feedbackLabel_->setWordWrap(true);
    feedbackLabel_->setStyleSheet(
        "color:#000000;"
        "background:transparent;"
        "border:none;"
        "font: 700 11pt 'Times New Roman';"
        );
    feedbackLabel_->setText(""); // inicia vacío

    placeControls();
}

void MinijuegoHistoria::placeControls()
{
    const int W = width(), H = height();

    const QRect panel = topPanelRect(W,H);
    titleLabel_->setGeometry(panel.x(), panel.y()-int(panel.height()*0.55),
                             panel.width(), int(panel.height()*0.42));

    const QRect qrect = questionRect(W,H);
    questionLabel_->setGeometry(qrect);

    const auto tables = tableRects(W,H);
    for (int i=0;i<4 && i<tables.size(); ++i) {
        const int insetX = int(tables[i].width()  * 0.11);
        const int insetT = int(tables[i].height() * 0.30);
        const int insetB = int(tables[i].height() * 0.26);
        const QRect t = tables[i].adjusted(insetX, insetT, -insetX, -insetB);
        btn_[i]->setGeometry(t);

        const int m = 6;
        btnText_[i]->setGeometry(m, m, t.width()-2*m, t.height()-2*m);
        btnText_[i]->setMaximumSize(t.width()-2*m, t.height()-2*m);
    }

    progressLabel_->setGeometry(panel.x(),
                                panel.bottom()+int(panel.height()*0.14),
                                panel.width(),
                                int(panel.height()*0.34));

    const QRect pr = puffleRect(W,H);
    puffleLabel_->setGeometry(pr);

    // Colocar feedback debajo de la fila inferior de opciones
    if (!tables.isEmpty()) {
        const int left   = tables.first().x();
        const int right  = tables.last().right();
        const int bottom = qMax(tables[2].bottom(), tables[3].bottom());
        const int fbY    = bottom + int(H * 0.025);
        const int fbH    = int(H * 0.06);
        const int fbW    = right - left + 1;
        feedbackLabel_->setGeometry(left, fbY, fbW, fbH);
    }
}

// ---------- Estilos por defecto ----------
void MinijuegoHistoria::styleButtonsDefault()
{
    const QString okCssBase =
        "QPushButton{ background:transparent; border:none; }"
        "QPushButton:hover{ background:rgba(255,255,255,0.08); }"
        "QPushButton:pressed{ background:rgba(255,255,255,0.16); }";
    for (int i=0;i<4;++i) if (btn_[i]) btn_[i]->setStyleSheet(okCssBase);
}

// ---------- Datos ----------
void MinijuegoHistoria::loadQuestions()
{
    quiz_.clear(); quiz_.reserve(5);

    quiz_.push_back({
        "1) Después del feudalismo medieval acudimos al surgimiento de una nueva clase social conocida como la:",
        {"A) La monarquía.", "B) El mercantilismo.", "C) La burguesía.", "D) El proletariado"},
        2
    });
    quiz_.push_back({
        "2) El Renacimiento supone una época de absolutismo y nacionalismos, con el nacimiento de fuertes monarquías europeas centralizadas como:",
        {"A) Grecia.", "B) Inglaterra.", "C) Yugoslavia.", "D) Egipto"},
        1
    });
    quiz_.push_back({
        "3) Antes de la consolidación del estado moderno, Italia estuvo dividida en pequeñas ciudades-estado normalmente enfrentadas entre sí, como el caso de:",
        {"A) Florencia–Nápoli.", "B) Ámsterdam–Cracovia.", "C) Reims–Colonia.", "D) Milán–Lourdes."},
        0
    });
    quiz_.push_back({
        "4) La toma de Constantinopla bloqueó la ruta de la seda y ocurrió en lo que hoy es:",
        {"A) Eslovaquia.", "B) Estambul (Turquía).", "C) Mesopotamia.", "D) Jerusalén."},
        1
    });
    quiz_.push_back({
        "5) Resurge el interés por Grecia y Roma, declive del feudalismo, crecimiento del comercio e innovaciones como:",
        {"A) La imprenta y la brújula.", "B) La rueda y la escritura.", "C) Máquinas de vapor y producción en masa.", "D) La pólvora y la rueda."},
        0
    });

    current_ = 0;
    score_   = 0;
    moodIndex_ = 0;
}

void MinijuegoHistoria::showQuestion(int i)
{
    if (i < 0 || i >= quiz_.size()) return;

    styleButtonsDefault();

    const auto &qa = quiz_[i];
    questionLabel_->setText(qa.question);

    for (int k = 0; k < 4; ++k) if (btnText_[k]) btnText_[k]->setText(qa.options[k]);

    progressLabel_->setText(QString(""));
    if (feedbackLabel_) feedbackLabel_->setText(""); // limpia feedback
}

void MinijuegoHistoria::nextQuestion()
{
    ++current_;
    if (current_ >= quiz_.size()) { endQuiz(); return; }
    showQuestion(current_);
}

void MinijuegoHistoria::endQuiz()
{
    const bool perfect = (score_ == quiz_.size());

    // Mensaje final
    const QString msg = perfect
                            ? "Mantuviste al puffle alegre, mereces un corazón"
                            : "No mantuviste al puffle alegre, no mereces un corazón";

    // Mostrar el mensaje en el mismo label (en negro, sin fondo)
    if (feedbackLabel_) {
        feedbackLabel_->setText(msg);
        feedbackLabel_->setStyleSheet(
            "color:#000000;"
            "background:transparent;"
            "border:none;"
            "font: 700 11pt 'Times New Roman';"
            );

        // Efecto de aparición
        auto effect = new QGraphicsOpacityEffect(feedbackLabel_);
        feedbackLabel_->setGraphicsEffect(effect);

        auto fadeIn = new QPropertyAnimation(effect, "opacity");
        fadeIn->setDuration(400);
        fadeIn->setStartValue(0.0);
        fadeIn->setEndValue(1.0);
        fadeIn->start(QAbstractAnimation::DeleteWhenStopped);
    }

    // Si quieres cerrar automáticamente el minijuego luego de unos segundos:
    QTimer::singleShot(3000, this, [this]() {
        emit quizFinished(score_ == quiz_.size());
        close();
    });
}

// ---------- Lógica de respuesta ----------
void MinijuegoHistoria::handleAnswerClicked()
{
    QPushButton *senderBtn = qobject_cast<QPushButton*>(sender());
    if (!senderBtn) return;

    int idx = -1;
    for (int i=0;i<4;++i) if (btn_[i] == senderBtn) { idx = i; break; }

    const auto &qa = quiz_[current_];
    const int correct = qa.correctIndex;

    const QString okCss =
        "QPushButton{ background:transparent; border:2px solid #2aa44a; border-radius:8px; }";
    const QString badCss =
        "QPushButton{ background:transparent; border:2px solid #cc3355; border-radius:8px; }";
    const QString revealCss =
        "QPushButton{ background:transparent; border:2px solid #2aa44a; border-radius:8px; }";

    auto letterFor = [](int id){ return QChar('A' + qBound(0, id, 3)); };

    QString text;
    if (idx == correct) {
        score_++;
        btn_[idx]->setStyleSheet(okCss);
        text = "Correcto!";
    } else {
        if (idx >= 0) btn_[idx]->setStyleSheet(badCss);
        btn_[correct]->setStyleSheet(revealCss);
        const QChar correctLetter = letterFor(correct);
        text = QString("Incorrecto.El puffle no esta alegre con tu respuesta.").arg(correctLetter);
        // una vez triste, no vuelve a estar más feliz
        moodIndex_ = qMin(3, moodIndex_ + 1);
    }

    // Feedback con fade (texto negro, fondo transparente)
    if (feedbackLabel_) {
        feedbackLabel_->setText(text);
        feedbackLabel_->setStyleSheet(
            "color:#000000;"
            "background:transparent;"
            "border:none;"
            "font: 700 11pt 'Times New Roman';"
            );

        auto effect = new QGraphicsOpacityEffect(feedbackLabel_);
        feedbackLabel_->setGraphicsEffect(effect);

        auto fadeIn = new QPropertyAnimation(effect, "opacity");
        fadeIn->setDuration(200);
        fadeIn->setStartValue(0.0);
        fadeIn->setEndValue(1.0);
        fadeIn->start(QAbstractAnimation::DeleteWhenStopped);

        QTimer::singleShot(1500, [effect]() {
            auto fadeOut = new QPropertyAnimation(effect, "opacity");
            fadeOut->setDuration(400);
            fadeOut->setStartValue(1.0);
            fadeOut->setEndValue(0.0);
            fadeOut->start(QAbstractAnimation::DeleteWhenStopped);
        });
    }

    if (!puffleMoods_.isEmpty())
        puffleLabel_->setPixmap(puffleMoods_[qBound(0, moodIndex_, 3)]);

    QTimer::singleShot(1800, this, [this](){ nextQuestion(); });
}

// ---------- Pintado / Resize ----------
void MinijuegoHistoria::paintEvent(QPaintEvent *e)
{
    Q_UNUSED(e);
    QPainter p(this);
    p.setRenderHint(QPainter::SmoothPixmapTransform, true);

    if (!bg_.isNull())
        p.drawPixmap(rect(), bg_.scaled(size(), Qt::KeepAspectRatioByExpanding, Qt::SmoothTransformation));
    else
        p.fillRect(rect(), Qt::black);
}

void MinijuegoHistoria::resizeEvent(QResizeEvent *e)
{
    QWidget::resizeEvent(e);
    placeControls();
}


