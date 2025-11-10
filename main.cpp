#include "lobby.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    Lobby lobby;
    lobby.show();
    return a.exec();
}

