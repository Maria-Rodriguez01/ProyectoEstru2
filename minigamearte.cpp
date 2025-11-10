#include "minigamearte.h"
#include <QPainter>
#include <QMessageBox>
#include <QGraphicsOpacityEffect>
#include <QPropertyAnimation>
#include <QTimer>

// ===== RUTAS (ajústalas a tu proyecto) =====
static const char* kBgPath =
    "C:/Users/Maria Gabriela/OneDrive/Documents/ProyectoEstru2MariaRodriguez/Assets/artebackground.jpg";

static const char* kCanvasPaths[] = {
    "C:/Users/Maria Gabriela/OneDrive/Documents/ProyectoEstru2MariaRodriguez/Assets/pintura1.jpg",
    "C:/Users/Maria Gabriela/OneDrive/Documents/ProyectoEstru2MariaRodriguez/Assets/pintura2.jpg",
    "C:/Users/Maria Gabriela/OneDrive/Documents/ProyectoEstru2MariaRodriguez/Assets/pintura3.jpg",
    "C:/Users/Maria Gabriela/OneDrive/Documents/ProyectoEstru2MariaRodriguez/Assets/pintura4.jpg",
    "C:/Users/Maria Gabriela/OneDrive/Documents/ProyectoEstru2MariaRodriguez/Assets/pintura5.jpg",
    "C:/Users/Maria Gabriela/OneDrive/Documents/ProyectoEstru2MariaRodriguez/Assets/pintura6.jpg"
};
static const int kCanvasCount = sizeof(kCanvasPaths)/sizeof(kCanvasPaths[0]);

// ---------- rects de pizarra ----------
static QRect boardOuterRect(int W, int H) {
    return QRect(int(W*0.41), int(H*0.17), int(W*0.52), int(H*0.52));
}

// 🔧 Asimétrico para empujar a la izquierda
static QRect boardInnerRect(int W, int H) {
    QRect o = boardOuterRect(W,H);
    const int ml  = int(o.width()  * 0.07); // margen izquierda 10%
    const int mr  = int(o.width()  * 0.25); // margen derecha 14%  (más grande)
    const int mt  = int(o.height() * 0.18); // arriba 18%
    const int mb  = int(o.height() * 0.20); // abajo 20%
    return QRect(o.x()+ml, o.y()+mt, o.width()-ml-mr, o.height()-mt-mb);
}

// ---------- rect del canvas (lienzo blanco) ----------
// 🔧 Más a la izquierda y más pequeño
static QRect canvasWhiteRect(int W, int H) {
    const int x = int(W * 0.178); // antes 0.195
    const int y = int(H * 0.410); // leve ajuste
    const int w = int(W * 0.155); // antes 0.175 (más pequeño)
    const int h = int(H * 0.215); // antes 0.235 (más pequeño)
    return QRect(x, y, w, h);
}

MinigameArte::MinigameArte(QWidget *parent)
    : QWidget(nullptr)
{
    Q_UNUSED(parent);
    setWindowFlag(Qt::Window, true);
    setAttribute(Qt::WA_DeleteOnClose);
    setFixedSize(1152, 768);
    setWindowTitle("Mini Juego de Arte");

    background_.load(kBgPath);

    buildUI();
    loadQuestions();
    loadCanvasImages();
    canvasIndex_ = 0;
    showQuestion(0);
}

void MinigameArte::buildUI()
{
    titleLabel_ = new QLabel("", this);
    titleLabel_->setAlignment(Qt::AlignCenter);
    titleLabel_->setStyleSheet(
        "color:#ffffff; background:transparent;"
        "border:none; padding:2px 6px;"
        "font: 700 12pt 'Times New Roman';"
        );

    questionLabel_ = new QLabel(this);
    questionLabel_->setWordWrap(true);
    questionLabel_->setAlignment(Qt::AlignLeft | Qt::AlignTop);
    questionLabel_->setStyleSheet(
        "color:#ffffff; background:transparent;"
        "border:none; padding:0px;"
        "font: 700 11pt 'Times New Roman';"
        );

    const QString btnCss =
        "QPushButton{"
        "  color:#ffffff; background:transparent;"
        "  border:2px solid #c0a060; border-radius:8px;"
        "  padding:6px; font:700 10pt 'Times New Roman';"
        "}"
        "QPushButton:hover{ background:rgba(255,255,255,40); }"
        "QPushButton:pressed{ background:rgba(255,255,255,60); }";

    for (int i=0;i<4;++i) {
        btn_[i] = new QPushButton(this);
        btn_[i]->setStyleSheet(btnCss);
        btn_[i]->setCursor(Qt::PointingHandCursor);
        connect(btn_[i], &QPushButton::clicked, this, &MinigameArte::handleAnswerClicked);
    }

    progressLabel_ = new QLabel(this);
    progressLabel_->setAlignment(Qt::AlignCenter);
    progressLabel_->setStyleSheet(
        "color:#ffffff; background:transparent;"
        "border:none; padding:0px;"
        "font: 600 10pt 'Times New Roman';"
        );

    layoutOnBoard();
}

void MinigameArte::loadQuestions()
{
    quiz_.clear(); quiz_.reserve(5);

    quiz_.push_back({
        "1) Uno de los siguientes personajes fue el encargado de pintar la Capilla Sixtina:",
        {"A) Miguel Ángel.", "B) Donatello.", "C) Leonardo Da Vinci.", "D) Francis Bacon"}, 0
    });
    quiz_.push_back({
        "2) Genio del Renacimiento que esculpió el Moisés, el David y la Pietà:",
        {"A) Miguel Ángel Buonarroti.", "B) Leonardo Da Vinci.", "C) Rafael Sanzio.", "D) Galileo Galilei"}, 0
    });
    quiz_.push_back({
        "3) Durante el Renacimiento el estilo artístico que impregnó el arte y el pensamiento fue:",
        {"A) El Gótico.", "B) El barroco.", "C) El clasicismo.", "D) El Romanticismo"}, 1
    });
    quiz_.push_back({
        "4) Nueva visión del hombre en el Renacimiento (arte, política y ciencias humanas):",
        {"A) Antropocentrismo.", "B) Humanismo.", "C) Paradigma antropológico.", "D) Teocentrismo."}, 1
    });
    quiz_.push_back({
        "5) Leonardo, Donatello, Rafael y Miguel Ángel aparecen en los cómics de:",
        {"A) Las Tortugas Ninja.", "B) Los Caballeros del Zodiaco.", "C) Los Cuatro Fantásticos.", "D) Attack on Titan (antagonistas)"}, 0
    });

    current_ = 0;
    score_   = 0;
}

void MinigameArte::loadCanvasImages()
{
    canvasImgs_.clear(); canvasImgs_.reserve(kCanvasCount);
    for (int i=0;i<kCanvasCount; ++i) {
        canvasImgs_.push_back(QPixmap(QString::fromUtf8(kCanvasPaths[i])));
    }
}

void MinigameArte::showQuestion(int i)
{
    if (i < 0 || i >= quiz_.size()) return;

    locked_ = false;
    styleButtonsDefault();

    const QA &qa = quiz_[i];
    questionLabel_->setText(qa.question);
    for (int k=0;k<4;++k) btn_[k]->setText(qa.options[k]);

    progressLabel_->setText(
        QString("")
            .arg(i+1).arg(quiz_.size()).arg(score_)
        );

    auto eff = new QGraphicsOpacityEffect(questionLabel_);
    questionLabel_->setGraphicsEffect(eff);
    auto anim = new QPropertyAnimation(eff, "opacity");
    anim->setDuration(180);
    anim->setStartValue(0.0);
    anim->setEndValue(1.0);
    anim->start(QAbstractAnimation::DeleteWhenStopped);
}

void MinigameArte::nextQuestion()
{
    ++current_;
    if (current_ >= quiz_.size()) { endQuiz(); return; }
    showQuestion(current_);
}

void MinigameArte::endQuiz()
{
    const bool perfect = (score_ == quiz_.size());
    emit quizFinished(perfect);  // ► informa al MainWindow
    close();                     // ► sin QMessageBox
}


void MinigameArte::handleAnswerClicked()
{
    if (locked_) return;
    locked_ = true;

    QPushButton *senderBtn = qobject_cast<QPushButton*>(sender());
    int idx = -1;
    for (int i=0;i<4;++i) if (btn_[i] == senderBtn) { idx = i; break; }

    const QA &qa = quiz_[current_];
    const int correct = qa.correctIndex;

    const QString okCss =
        "QPushButton{ color:#11dd55; background:transparent; "
        "border:2px solid #11dd55; border-radius:8px; padding:6px; font:700 10pt 'Times New Roman'; }";
    const QString badCss =
        "QPushButton{ color:#ffb3c6; background:transparent; "
        "border:2px solid #cc3355; border-radius:8px; padding:6px; font:700 10pt 'Times New Roman'; }";
    const QString revealCss =
        "QPushButton{ color:#fbf3d5; background:transparent; "
        "border:2px solid #2aa44a; border-radius:8px; padding:6px; font:700 10pt 'Times New Roman'; }";

    if (idx == correct) {
        score_++;
        btn_[idx]->setStyleSheet(okCss);
        if (canvasIndex_ + 1 < canvasImgs_.size())
            canvasIndex_++;
        update();
    } else {
        if (idx >= 0) btn_[idx]->setStyleSheet(badCss);
        btn_[correct]->setStyleSheet(revealCss);
    }

    QTimer::singleShot(1200, this, [this](){ nextQuestion(); });
}

void MinigameArte::styleButtonsDefault()
{
    const QString css =
        "QPushButton{"
        "  color:#ffffff; background:transparent;"
        "  border:2px solid #c0a060; border-radius:8px;"
        "  padding:6px; font:700 10pt 'Times New Roman';"
        "}"
        "QPushButton:hover{ background:rgba(255,255,255,40); }";
    for (int i=0;i<4;++i) btn_[i]->setStyleSheet(css);
}

// ================= Rects relativos ==================
QRect MinigameArte::boardRect() const { return boardOuterRect(width(), height()); }
QRect MinigameArte::canvasRect() const { return canvasWhiteRect(width(), height()); }

void MinigameArte::layoutOnBoard()
{
    const QRect outer = boardOuterRect(width(), height());
    const QRect inner = boardInnerRect(width(), height());

    // Título centrado encima del marco
    titleLabel_->setGeometry(outer.x(), outer.y() - 26, outer.width(), 22);

    // Pregunta: compacta y con padding
    const int qH   = int(inner.height() * 0.33);
    const int padX = 12, padY = 6;
    questionLabel_->setGeometry(inner.x() + padX, inner.y() + padY,
                                inner.width() - padX*2, qH - padY*2);

    // Botones 2x2 en el resto de la pizarra verde
    const int gap  = 14;
    const int btnW = (inner.width() - gap*3) / 2;
    const int btnH = int((inner.height() - qH - gap*3) / 2);
    const int bx   = inner.x() + gap;
    const int by   = inner.y() + qH + gap;

    btn_[0]->setGeometry(bx,                  by,              btnW, btnH);
    btn_[1]->setGeometry(bx + btnW + gap,     by,              btnW, btnH);
    btn_[2]->setGeometry(bx,                  by + btnH + gap, btnW, btnH);
    btn_[3]->setGeometry(bx + btnW + gap,     by + btnH + gap, btnW, btnH);

    // Progreso bajo el marco
    progressLabel_->setGeometry(outer.x(), outer.bottom() + 6, outer.width(), 22);
}

// ================== Pintado ==================
void MinigameArte::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);
    QPainter p(this);

    // Fondo
    if (!background_.isNull())
        p.drawPixmap(rect(), background_.scaled(size(),
                                                Qt::KeepAspectRatioByExpanding, Qt::SmoothTransformation));
    else
        p.fillRect(rect(), Qt::black);

    // Imagen actual dentro del lienzo blanco (más pequeña)
    const QRect cr = canvasRect();
    if (!canvasImgs_.isEmpty()) {
        const QPixmap &pm = canvasImgs_.at(qBound(0, canvasIndex_, canvasImgs_.size()-1));
        if (!pm.isNull()) {
            QRect inner = cr.adjusted(8,8,-8,-10);
            p.setRenderHint(QPainter::SmoothPixmapTransform, true);
            p.drawPixmap(inner, pm.scaled(inner.size(), Qt::KeepAspectRatio, Qt::SmoothTransformation));
        }
    }
}

void MinigameArte::resizeEvent(QResizeEvent *event)
{
    QWidget::resizeEvent(event);
    layoutOnBoard();
}







