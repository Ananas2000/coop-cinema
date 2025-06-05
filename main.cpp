#include <QApplication>
#include "MainWindow.h"
#include "RoomData.h"

int main(int argc, char* argv[])
{
    qRegisterMetaType<UserProfile>("UserProfile");
    qRegisterMetaType<Room>("Room");
    qRegisterMetaType<QList<UserProfile>>("QList<UserProfile>");

    Client client;
    client.connectToServer("localhost", 8888);

    QApplication app(argc, argv);
    MainWindow mainWindow;
    mainWindow.showMaximized();
    return app.exec();
}