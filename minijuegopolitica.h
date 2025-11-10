#ifndef MINIJUEGOPOLITICA_H
#define MINIJUEGOPOLITICA_H

#include <QWidget>
#include <QPixmap>
#include <QTimer>
#include <array>
#include "Maze.h"

// Forward Qt
class QGraphicsScene;
class QGraphicsView;
class QGraphicsPixmapItem;
class QLabel;              // <-- ya estaba forward-declarado
class QButtonGroup;
class QRadioButton;
class QPushButton;
class QResizeEvent;
class QKeyEvent;

class minijuegopolitica : public QWidget
{
    Q_OBJECT
public:
    explicit minijuegopolitica(QWidget *parent = nullptr);
    bool loadCSV(const QString& absPath);

signals:
    void juegoCompletado(bool exito);

protected:
    void keyPressEvent(QKeyEvent *e) override;
    void resizeEvent(QResizeEvent *ev) override;

private slots:
    void tickAnim();
    void onAnswerClicked();

private:
    void buildScene(bool fitViewNow);
    bool canWalk(int nr, int nc) const;
    void onEnter(int r, int c);
    void setDirection(int dirIndex);
    void showDoorQuestion(int doorIndex);
    void unlockDoorBit(int doorIndex);

    // -------- NUEVO: aviso flotante --------
    void showToast(const QString& msg, int ms = 1600);

private:
    // ---- vista/escena
    QGraphicsView*       view_  = nullptr;
    QGraphicsScene*      scene_ = nullptr;

    // ---- laberinto
    Maze mz_;
    int  tile = 48;

    // ---- tiles
    QPixmap pxFloor, pxWall, pxStart, pxExit, pxExitLocked;
    std::array<QPixmap,5> pxDoor;

    // ---- puffle
    QGraphicsPixmapItem* puffleItem_ = nullptr;
    QPixmap puffleSheet_;
    int p_r_ = 0, p_c_ = 0;
    int animDir_ = 0;
    int animFrame_ = 0;
    bool moving_ = false;
    QTimer animTimer_;

    // ---- preguntas arriba
    QWidget*       quizPanel_  = nullptr;
    QLabel*        quizLabel_  = nullptr;
    QButtonGroup*  quizGroup_  = nullptr;
    QRadioButton*  opt_[4]     = {nullptr,nullptr,nullptr,nullptr};
    QPushButton*   answerBtn_  = nullptr;

    int pendingDoorIndex_ = -1;
    unsigned visitedMask_ = 0;

    static constexpr int kDoorCount = 5;
    static constexpr unsigned kAllVisitedMask = (1u << kDoorCount) - 1;

    // -------- NUEVO: aviso flotante + temporizador de cierre --------
    QLabel* toast_ = nullptr;
};

#endif // MINIJUEGOPOLITICA_H
