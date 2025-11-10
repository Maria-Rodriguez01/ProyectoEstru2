#include "gamesession.h"

GameSession& GameSession::instance() {
    static GameSession s;
    return s;
}

GameSession::GameSession(QObject* parent) : QObject(parent) {}

void GameSession::start() {
    timer_.restart();
    running_ = true;
}

void GameSession::stop() {
    // No "pausa": solo marcamos que ya no está corriendo.
    running_ = false;
}

void GameSession::reset() {
    running_ = false;
    timer_ = QElapsedTimer(); // invalida el timer → elapsedMs()=0
}
