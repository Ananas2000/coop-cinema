#include "RoomData.h"
#include <QMetaType>

static void registerRoomDataTypes()
{
    qRegisterMetaType<UserProfile>("UserProfile");
    qRegisterMetaType<Room>("Room");
    qRegisterMetaType<QList<UserProfile>>("QList<UserProfile>");
}

namespace {
    const bool registered = []() {
        registerRoomDataTypes();
        return true;
        }();
}