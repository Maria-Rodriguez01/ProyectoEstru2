#ifndef GAMESESSION_H
#define GAMESESSION_H

#include <QObject>
#include <QElapsedTimer>

class GameSession : public QObject {
    Q_OBJECT
public:
    static GameSession& instance();

    void start();       // iniciar/reiniciar y comenzar a contar
    void stop();        // detener, sin borrar el tiempo acumulado
    void reset();       // poner a cero

    bool isRunning() const { return running_; }
    qint64 elapsedMs() const { return timer_.isValid() ? timer_.elapsed() : 0; }

private:
    explicit GameSession(QObject* parent = nullptr);

    QElapsedTimer timer_;
    bool running_ = false;
};

#endif // GAMESESSION_H
