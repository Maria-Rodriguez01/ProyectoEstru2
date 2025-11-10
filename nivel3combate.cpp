#include "nivel3combate.h"

#include <QPixmap>
#include <QPainter>
#include <QDebug>
#include <QResizeEvent>
#include <QGridLayout>
#include <QEasingCurve>
#include <QTimer>
#include <QScreen>
#include <QGuiApplication>
#include <QMediaPlayer>
#include <QAudioOutput>


// Rutas de assets
static const char* kCombateBg =
    "C:/Users/Maria Gabriela/OneDrive/Documents/ProyectoEstru2MariaRodriguez/Assets/combate.jpg";

static const char* kEmpSheet =
    "C:/Users/Maria Gabriela/OneDrive/Documents/ProyectoEstru2MariaRodriguez/Assets/Empiristas.png";
static const char* kRacSheet =
    "C:/Users/Maria Gabriela/OneDrive/Documents/ProyectoEstru2MariaRodriguez/Assets/Racionalistas.png";

static const char* kFireL2R =
    "C:/Users/Maria Gabriela/OneDrive/Documents/ProyectoEstru2MariaRodriguez/Assets/ataque.png";
static const char* kFireR2L =
    "C:/Users/Maria Gabriela/OneDrive/Documents/ProyectoEstru2MariaRodriguez/Assets/ataque2.png";

static const char* kSongRac =
    "C:/Users/Maria Gabriela/OneDrive/Documents/ProyectoEstru2MariaRodriguez/Assets/Racionalista.mp3";
static const char* kSongEmp =
    "C:/Users/Maria Gabriela/OneDrive/Documents/ProyectoEstru2MariaRodriguez/Assets/Empirista.mp3";


// ====== ANCLA DE DISPARO (mano/espada) ======
static QPointF handAnchor(QGraphicsPixmapItem* item,
                          const QPixmap& sheet,
                          bool isPlayer /*true jugador, false cpu*/,
                          qreal handY = 0.58,
                          qreal xRight = 0.78,
                          qreal xLeft  = 0.22)
{
    if (!item || sheet.isNull()) return item ? item->pos() : QPointF();

    const int fw = sheet.width()  / 4;
    const int fh = sheet.height() / 3;
    const qreal sc = item->scale();

    const qreal xRel = isPlayer ? xRight : xLeft;
    QPointF p = item->pos();
    p.rx() += fw * sc * xRel;
    p.ry() += fh * sc * handY;
    return p;
}

Nivel3Combate::Nivel3Combate(Side playerSide, int heartsPlayer, QWidget *parent)
    : QGraphicsView(parent),
    playerSide_(playerSide),
    playerHearts_(qMax(1, heartsPlayer)),
    cpuHearts_(4)
{
    setWindowTitle("Nivel 3 — Combate Final");
    setRenderHint(QPainter::Antialiasing, true);
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setFrameShape(QFrame::NoFrame);
    setBackgroundBrush(QColor(15, 15, 15));
    setFocusPolicy(Qt::StrongFocus);

    scene_ = new QGraphicsScene(this);
    setScene(scene_);

    bgItem_ = new QGraphicsPixmapItem();
    scene_->addItem(bgItem_);

    playerItem_ = new QGraphicsPixmapItem();
    cpuItem_    = new QGraphicsPixmapItem();
    playerItem_->setZValue(5);
    cpuItem_->setZValue(5);
    scene_->addItem(playerItem_);
    scene_->addItem(cpuItem_);

    fireball_ = new QGraphicsPixmapItem();
    fireball_->setVisible(false);
    fireball_->setZValue(20);
    scene_->addItem(fireball_);

    fireAnim_ = new QVariantAnimation(this);
    fireAnim_->setEasingCurve(QEasingCurve::Linear);
    connect(fireAnim_, &QVariantAnimation::valueChanged, this, [this](const QVariant &v){
        fireball_->setPos(v.toPointF());
    });
    connect(fireAnim_, &QVariantAnimation::finished, this, &Nivel3Combate::onFireballArrived);

    buildUI();

    // Banco de preguntas
    questions_ = {
        { "Para algunos de los siguientes filósofos, el criterio de verdad es la evidencia sensible:",
         {"Empiristas", "Criticistas", "Racionalistas", "Dogmáticos"} },
        { "De las siguientes, una de ellas es la corriente filosófica que en general tiende a negar la posibilidad de la metafísica y a sostener que hay conocimiento únicamente de los fenómenos:",
         {"Racionalistas", "Empiristas", "Escolásticos", "Escépticos"} },
        { "Para unos de los siguientes filósofos, la experiencia como única fuente del conocimiento:",
         {"Epistemólogos", "Racionalistas", "Empiristas", "Escépticos"} },
        { "Filósofos para quienes la única fuente del conocimiento es la razón:",
         {"Epistemólogos", "Racionalistas", "Empiristas", "Escépticos"} },
        { "Filósofos que postulan las ideas innatas en el sujeto:",
         {"Empiristas", "Idealistas", "Racionalistas", "Innatistas"} },
        { "De los siguientes filósofos seleccione el que no se considera Racionalista:",
         {"David Hume", "John Locke", "Nicolas Malebranch", "Francis Bacon"} },
        { "Es la doctrina que establece que todos nuestros conocimientos provienen de la razón:",
         {"Empirismo", "Criticismo", "Racionalismo", "Epistemología"} },
        { "Uno de los siguientes filósofos, postula las ideas innatas en el sujeto:",
         {"George Berkeley", "David Hume", "Leibniz", "Hipatía"} }
    };
    // índices correctos (0-based):
    answers_ = { 0, 1, 2, 1, 2, 2, 2, 2 };

    loadBackground();

    // El lado del jugador define su sheet:
    QPixmap emp = safeLoad(kEmpSheet, QSize(256,256));
    QPixmap rac = safeLoad(kRacSheet, QSize(256,256));
    const bool playerIsEmp = (playerSide_ == Side::Empirista);
    playerSheet_ = (playerIsEmp ? emp : rac);
    cpuSheet_    = (playerIsEmp ? rac : emp);

    placeFighters();
    setIdle();
    updateHeartsUI();
    askNextQuestion();
    // --- Audio ---
    mediaPlayer_ = new QMediaPlayer(this);
    audioOut_    = new QAudioOutput(this);
    mediaPlayer_->setAudioOutput(audioOut_);
    audioOut_->setVolume(0.6); // 60% (ajústalo si quieres)
}


QPixmap Nivel3Combate::safeLoad(const QString &path, const QSize &fallback)
{
    QPixmap pm;
    if (!pm.load(path)) {
        qWarning() << "[Nivel3Combate]  No se pudo cargar:" << path;
        pm = QPixmap(fallback);
        pm.fill(Qt::darkGray);
        QPainter p(&pm);
        p.setPen(Qt::white);
        p.drawText(pm.rect(), Qt::AlignCenter, "No se pudo cargar:\n" + path);
        p.end();
    }
    return pm;
}

void Nivel3Combate::loadBackground()
{
    QPixmap bg = safeLoad(kCombateBg, QSize(1125, 683));
    bgItem_->setPixmap(bg);
    scene_->setSceneRect(QRectF(QPointF(0,0), bg.size()));
    fitView();
}

void Nivel3Combate::buildUI()
{
    // Pregunta
    questionLabel_ = new QLabel("Pregunta");
    questionLabel_->setWordWrap(true);
    questionLabel_->setStyleSheet(
        "color:#fff; background:rgba(0,0,0,150);"
        "border:1px solid #d2b48c; border-radius:10px; padding:10px 14px; "
        "font: bold 12pt 'Times New Roman';");
    questionLabel_->setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
    questionProxy_ = scene_->addWidget(questionLabel_);
    questionProxy_->setZValue(1000);

    // Botón Responder
    btnResponder_ = new QPushButton("Responder");
    btnResponder_->setStyleSheet(
        "background:#5b3a17; color:#f2d295; border:2px solid #d2b48c; border-radius:6px; "
        "padding:8px 18px; font: 11pt 'Times New Roman';");
    respProxy_ = scene_->addWidget(btnResponder_);
    respProxy_->setZValue(1000);
    connect(btnResponder_, &QPushButton::clicked, this, &Nivel3Combate::onPressResponder);

    // Panel de opciones
    optionsPanel_ = new QWidget();
    auto *lay = new QGridLayout(optionsPanel_);
    lay->setContentsMargins(8,8,8,8); lay->setSpacing(8);
    for (int i=0;i<4;i++){
        auto *b = new QPushButton(QString("Opción %1").arg(i+1));
        b->setStyleSheet(
            "background:#2d2d2d; color:#fff; border:1px solid #888; border-radius:6px; padding:8px;");
        lay->addWidget(b, i/2, i%2);
        optionBtns_.push_back(b);
        connect(b, &QPushButton::clicked, this, &Nivel3Combate::onAnswerChosen);
    }
    optionsPanel_->setStyleSheet("background:rgba(0,0,0,120);");
    optionsProxy_ = scene_->addWidget(optionsPanel_);
    optionsProxy_->setZValue(1000);
    optionsProxy_->setVisible(false);

    // HUD corazones
    hudLabel_ = new QLabel();
    hudLabel_->setStyleSheet(
        "color:#fff; background:rgba(0,0,0,130); border:1px solid #d2b48c; "
        "border-radius:6px; padding:6px 10px; font: 10pt 'Times New Roman';");
    hudProxy_ = scene_->addWidget(hudLabel_);
    hudProxy_->setZValue(1000);
}

void Nivel3Combate::placeFighters()
{
    // Normaliza escala por altura de FRAME (mismo alto)
    const qreal TARGET_FRAME_H = 170.0;

    playerItem_->setScale( scaleForSheetHeight(playerSheet_, SHEET_ROWS, TARGET_FRAME_H) );
    cpuItem_->setScale   ( scaleForSheetHeight(cpuSheet_,    SHEET_ROWS, TARGET_FRAME_H) );

    const QRectF S = scene_->sceneRect();
    const qreal groundY = S.height()*0.45;

    playerItem_->setPos(S.width()*0.18, groundY);
    cpuItem_->setPos   (S.width()*0.58, groundY);

    // CPU mira hacia el jugador
    cpuItem_->setTransform(QTransform().scale(-1, 1), false);
    cpuItem_->setTransformOriginPoint(cpuItem_->boundingRect().width()/2.0, 0);
}

qreal Nivel3Combate::scaleForSheetHeight(const QPixmap &sheet, int rows, qreal targetFrameHeightPx) const
{
    if (sheet.isNull() || rows <= 0) return 1.0;
    const qreal frameH = sheet.height() / qreal(rows);
    if (frameH <= 1.0) return 1.0;
    return targetFrameHeightPx / frameH;
}

QSize Nivel3Combate::frameSize(const QPixmap &sheet) const
{
    if (sheet.isNull()) return QSize(1,1);
    return QSize(sheet.width()/SHEET_COLS, sheet.height()/SHEET_ROWS);
}

void Nivel3Combate::showEvent(QShowEvent *e)
{
    QGraphicsView::showEvent(e);

    resizeToScreen();            // 1) tamaño y fitInView
    const QRect g = QGuiApplication::primaryScreen()->availableGeometry();
    move(g.center() - rect().center());   // 2) centrar DESPUÉS de resize
}

void Nivel3Combate::resizeToScreen()
{
    const QRect a = QGuiApplication::primaryScreen()->availableGeometry();
    const int W = int(a.width()  * 0.90);
    const int H = int(a.height() * 0.90);
    resize(W, H);
    fitView();                   // <-- importantísimo
}

void Nivel3Combate::fitView()
{
    if (!scene_) return;
    if (scene_->sceneRect().isEmpty() && bgItem_ && !bgItem_->pixmap().isNull())
        scene_->setSceneRect(bgItem_->pixmap().rect());
    resetTransform();
    fitInView(scene_->sceneRect(), Qt::KeepAspectRatio);
    layoutUI();                  // ️ importante
}


void Nivel3Combate::layoutUI()
{
    const QRectF S = scene_->sceneRect();
    const qreal margin = 14.0;

    // Pregunta (arriba)
    const qreal qW = S.width() - 2*margin;
    questionProxy_->setPos(S.left() + margin, S.top() + 10.0);
    questionLabel_->setFixedWidth(int(qW));
    updateQuestionLayout();

    // Botón Responder
    respProxy_->setPos(S.center().x() - btnResponder_->sizeHint().width()/2.0,
                       questionProxy_->y() + questionProxy_->size().height() + 6);

    // Opciones
    optionsProxy_->setPos(S.width()*0.30,
                          questionProxy_->y() + questionProxy_->size().height() + 48);
    optionsProxy_->resize(S.width()*0.40, 150);

    // --- HUD corazones ABAJO ---
    hudLabel_->adjustSize(); // asegúrate de tener tamaño real
    const qreal hudX = S.center().x() - hudLabel_->width()/2.0;           // centrado
    const qreal hudY = S.bottom() - hudLabel_->height() - margin;         // pegado abajo
    hudProxy_->setPos(hudX, hudY);

    // Si lo prefieres bottom-left o bottom-right:
    hudProxy_->setPos(S.left()+margin, S.bottom()-hudLabel_->height()-margin);        // BL
    // hudProxy_->setPos(S.right()-hudLabel_->width()-margin, S.bottom()-hudLabel_->height()-margin); // BR
}


void Nivel3Combate::updateQuestionLayout()
{
    QFontMetrics fm(questionLabel_->font());
    const int textW = questionLabel_->width() - 24;
    const QString txt = questionLabel_->text();
    const QRect br = fm.boundingRect(0,0, textW, 0, Qt::TextWordWrap, txt);
    const int wantedH = qMax(44, br.height() + 20);
    questionLabel_->setFixedHeight(wantedH);
    questionProxy_->resize(questionLabel_->size());
}

void Nivel3Combate::resizeEvent(QResizeEvent *e)
{
    QGraphicsView::resizeEvent(e);
    fitView();   //  nunca queda pequeño
}

void Nivel3Combate::askNextQuestion()
{
    currentQuestion_++;
    if (currentQuestion_ >= questions_.size()) currentQuestion_ = 0;

    const auto &q = questions_[currentQuestion_];
    questionLabel_->setText(q.first);
    updateQuestionLayout();

    correctIndex_ = answers_.value(currentQuestion_, 0);
    for (int i=0;i<4;i++)
        optionBtns_[i]->setText(q.second.value(i));

    optionsProxy_->setVisible(false);
    optionsVisible_ = false;
    btnResponder_->setEnabled(true);
}

void Nivel3Combate::onPressResponder()
{
    if (fireInFlight_) return;
    optionsProxy_->setVisible(true);
    optionsVisible_ = true;
    btnResponder_->setEnabled(false);
}

void Nivel3Combate::onAnswerChosen()
{
    if (fireInFlight_ || !optionsVisible_) return;

    auto *btn = qobject_cast<QPushButton*>(sender());
    int idx = optionBtns_.indexOf(btn);
    const bool playerCorrect = (idx == correctIndex_);

    optionsProxy_->setVisible(false);
    optionsVisible_ = false;

    evaluateAnswer(playerCorrect);
}

void Nivel3Combate::evaluateAnswer(bool playerCorrect)
{
    if (playerCorrect) {
        launchFireball(true);   // jugador → CPU
    } else {
        launchFireball(false);  // CPU → jugador
    }
}

void Nivel3Combate::launchFireball(bool fromPlayer)
{
    if (fireInFlight_) return;
    fireInFlight_ = true;

    // Tamaño pequeño proporcional
    const int fireW = int(scene_->width() * 0.07);
    const int fireH = int(scene_->height() * 0.05);

    if (fromPlayer) {
        QPixmap pm = safeLoad(kFireL2R).scaled(fireW, fireH, Qt::KeepAspectRatio, Qt::SmoothTransformation);
        fireball_->setPixmap(pm);

        QPointF start = handAnchor(playerItem_, playerSheet_, /*isPlayer=*/true);
        QPointF end   = handAnchor(cpuItem_,    cpuSheet_,    /*isPlayer=*/false);

        fireball_->setPos(start);
        fireball_->setVisible(true);

        fireAnim_->stop();
        fireAnim_->setStartValue(start);
        fireAnim_->setEndValue(end);
        fireAnim_->setDuration(480);
        fireAnim_->start();

        fireball_->setData(1, QVariant(true));   // daño a CPU
    } else {
        QPixmap pm = safeLoad(kFireR2L).scaled(fireW, fireH, Qt::KeepAspectRatio, Qt::SmoothTransformation);
        fireball_->setPixmap(pm);

        QPointF start = handAnchor(cpuItem_,    cpuSheet_,    /*isPlayer=*/false);
        QPointF end   = handAnchor(playerItem_, playerSheet_, /*isPlayer=*/true);

        fireball_->setPos(start);
        fireball_->setVisible(true);

        fireAnim_->stop();
        fireAnim_->setStartValue(start);
        fireAnim_->setEndValue(end);
        fireAnim_->setDuration(480);
        fireAnim_->start();

        fireball_->setData(1, QVariant(false));  // daño al jugador
    }
}

void Nivel3Combate::onFireballArrived()
{
    fireball_->setVisible(false);
    bool toCPU = fireball_->data(1).toBool();
    applyDamage(toCPU);

    if (playerHearts_ > 0 && cpuHearts_ > 0) {
        askNextQuestion();
        btnResponder_->setEnabled(true);
        btnResponder_->setFocus();
    } else {
        btnResponder_->setEnabled(false);
        for (auto *b : optionBtns_) b->setEnabled(false);
    }

    fireInFlight_ = false;
}

void Nivel3Combate::applyDamage(bool toCPU)
{
    if (toCPU) {
        cpuHearts_ = qMax(0, cpuHearts_-1);
        if (cpuHearts_ == 0) { endMatch(true);  return; }
    } else {
        playerHearts_ = qMax(0, playerHearts_-1);
        if (playerHearts_ == 0) { endMatch(false); return; }
    }
    updateHeartsUI();
}

void Nivel3Combate::updateHeartsUI()
{
    auto hearts = [](int n){ QString s; for (int i=0;i<n;i++) s += "♥"; return s; };
    QString txt = QString("Tú: %1  (%2)    Rival: %3  (%4)")
                      .arg(hearts(playerHearts_)).arg(playerHearts_)
                      .arg(hearts(cpuHearts_)).arg(cpuHearts_);
    hudLabel_->setText(txt);
    hudLabel_->adjustSize();
    layoutUI(); //️ recoloca abajo con el tamaño nuevo
}

void Nivel3Combate::endMatch(bool playerWon)
{
    questionLabel_->setText(playerWon
                                ? "¡Ganaste! El conocimiento te dio la victoria."
                                : "Has perdido… ¡pero puedes intentarlo de nuevo!");
    updateQuestionLayout();
    optionsProxy_->setVisible(false);
    btnResponder_->setEnabled(false);
    for (auto *b : optionBtns_) b->setEnabled(false);

    // Determinar quién ganó (bando)
    Side winnerSide = Side::None;
    if (playerWon) {
        winnerSide = playerSide_;
    } else {
        // si pierde el jugador, gana el opuesto
        winnerSide = (playerSide_ == Side::Empirista) ? Side::Racionalista : Side::Empirista;
    }

    // Reproducir tema de victoria 10s
    playVictoryTheme(winnerSide);

    // Volver al mapa después de 10 segundos (cuando termina la música)
    QTimer::singleShot(20000, this, [this](){
        emit matchFinished();
        close();
    });
}

void Nivel3Combate::playVictoryTheme(Side winner)
{
    if (!mediaPlayer_) return;

    // Detener cualquier reproducción previa
    mediaPlayer_->stop();

    const char* path = nullptr;
    if (winner == Side::Racionalista) path = kSongRac;
    else if (winner == Side::Empirista) path = kSongEmp;

    if (!path) return;

    mediaPlayer_->setSource(QUrl::fromLocalFile(QString::fromUtf8(path)));
    mediaPlayer_->play();

    // Detener tras 10 segundos
    QTimer::singleShot(20000, this, [this]() {
        if (mediaPlayer_) mediaPlayer_->stop();
    });
}

void Nivel3Combate::drawFrame(QGraphicsPixmapItem *item, const QPixmap &sheet,
                              int col, int row, bool mirror)
{
    const int fw = sheet.width()  / SHEET_COLS;
    const int fh = sheet.height() / SHEET_ROWS;

    QRect src(col * fw, row * fh, fw, fh);
    QPixmap frame = sheet.copy(src);

    if (mirror) frame = frame.transformed(QTransform().scale(-1,1));

    item->setPixmap(frame);
}

void Nivel3Combate::setIdle()
{
    drawFrame(playerItem_, playerSheet_, 3, 1, false);
    drawFrame(cpuItem_,    cpuSheet_,    0, IDLE_ROW, true);
}

