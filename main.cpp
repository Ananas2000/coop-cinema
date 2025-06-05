#include <QApplication>
#include "MainWindow.h"
#include "RoomData.h"

int main(int argc, char* argv[])
{
    qRegisterMetaType<UserProfile>("UserProfile");
    qRegisterMetaType<Room>("Room");
    qRegisterMetaType<QList<UserProfile>>("QList<UserProfile>");

    QApplication app(argc, argv);
    MainWindow mainWindow;
    mainWindow.show();
    return app.exec();
}