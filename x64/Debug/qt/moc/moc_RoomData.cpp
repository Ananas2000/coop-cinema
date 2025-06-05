/****************************************************************************
** Meta object code from reading C++ file 'RoomData.h'
**
** Created by: The Qt Meta Object Compiler version 69 (Qt 6.9.0)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../../../RoomData.h"
#include <QtCore/qmetatype.h>
#include <QtCore/QList>

#include <QtCore/qtmochelpers.h>

#include <memory>


#include <QtCore/qxptype_traits.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'RoomData.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 69
#error "This file was generated using the moc from 6.9.0. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

#ifndef Q_CONSTINIT
#define Q_CONSTINIT
#endif

QT_WARNING_PUSH
QT_WARNING_DISABLE_DEPRECATED
QT_WARNING_DISABLE_GCC("-Wuseless-cast")
namespace {
struct qt_meta_tag_ZN11UserProfileE_t {};
} // unnamed namespace

template <> constexpr inline auto UserProfile::qt_create_metaobjectdata<qt_meta_tag_ZN11UserProfileE_t>()
{
    namespace QMC = QtMocConstants;
    QtMocHelpers::StringRefStorage qt_stringData {
        "UserProfile",
        "nickname",
        "avatarUrl",
        "sessionId"
    };

    QtMocHelpers::UintData qt_methods {
    };
    QtMocHelpers::UintData qt_properties {
        // property 'nickname'
        QtMocHelpers::PropertyData<QString>(1, QMetaType::QString, QMC::DefaultPropertyFlags | QMC::Writable),
        // property 'avatarUrl'
        QtMocHelpers::PropertyData<QString>(2, QMetaType::QString, QMC::DefaultPropertyFlags | QMC::Writable),
        // property 'sessionId'
        QtMocHelpers::PropertyData<QString>(3, QMetaType::QString, QMC::DefaultPropertyFlags | QMC::Writable),
    };
    QtMocHelpers::UintData qt_enums {
    };
    return QtMocHelpers::metaObjectData<UserProfile, qt_meta_tag_ZN11UserProfileE_t>(QMC::PropertyAccessInStaticMetaCall, qt_stringData,
            qt_methods, qt_properties, qt_enums);
}
Q_CONSTINIT const QMetaObject UserProfile::staticMetaObject = { {
    nullptr,
    qt_staticMetaObjectStaticContent<qt_meta_tag_ZN11UserProfileE_t>.stringdata,
    qt_staticMetaObjectStaticContent<qt_meta_tag_ZN11UserProfileE_t>.data,
    qt_static_metacall,
    nullptr,
    qt_staticMetaObjectRelocatingContent<qt_meta_tag_ZN11UserProfileE_t>.metaTypes,
    nullptr
} };

void UserProfile::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    auto *_t = reinterpret_cast<UserProfile *>(_o);
    if (_c == QMetaObject::ReadProperty) {
        void *_v = _a[0];
        switch (_id) {
        case 0: *reinterpret_cast<QString*>(_v) = _t->nickname; break;
        case 1: *reinterpret_cast<QString*>(_v) = _t->avatarUrl; break;
        case 2: *reinterpret_cast<QString*>(_v) = _t->sessionId; break;
        default: break;
        }
    }
    if (_c == QMetaObject::WriteProperty) {
        void *_v = _a[0];
        switch (_id) {
        case 0: QtMocHelpers::setProperty(_t->nickname, *reinterpret_cast<QString*>(_v)); break;
        case 1: QtMocHelpers::setProperty(_t->avatarUrl, *reinterpret_cast<QString*>(_v)); break;
        case 2: QtMocHelpers::setProperty(_t->sessionId, *reinterpret_cast<QString*>(_v)); break;
        default: break;
        }
    }
}
namespace {
struct qt_meta_tag_ZN4RoomE_t {};
} // unnamed namespace

template <> constexpr inline auto Room::qt_create_metaobjectdata<qt_meta_tag_ZN4RoomE_t>()
{
    namespace QMC = QtMocConstants;
    QtMocHelpers::StringRefStorage qt_stringData {
        "Room",
        "roomId",
        "members",
        "QList<UserProfile>",
        "currentVideo",
        "playbackPosition",
        "isPlaying"
    };

    QtMocHelpers::UintData qt_methods {
    };
    QtMocHelpers::UintData qt_properties {
        // property 'roomId'
        QtMocHelpers::PropertyData<QString>(1, QMetaType::QString, QMC::DefaultPropertyFlags | QMC::Writable),
        // property 'members'
        QtMocHelpers::PropertyData<QList<UserProfile>>(2, 0x80000000 | 3, QMC::DefaultPropertyFlags | QMC::Writable | QMC::EnumOrFlag),
        // property 'currentVideo'
        QtMocHelpers::PropertyData<QUrl>(4, QMetaType::QUrl, QMC::DefaultPropertyFlags | QMC::Writable),
        // property 'playbackPosition'
        QtMocHelpers::PropertyData<qint64>(5, QMetaType::LongLong, QMC::DefaultPropertyFlags | QMC::Writable),
        // property 'isPlaying'
        QtMocHelpers::PropertyData<bool>(6, QMetaType::Bool, QMC::DefaultPropertyFlags | QMC::Writable),
    };
    QtMocHelpers::UintData qt_enums {
    };
    return QtMocHelpers::metaObjectData<Room, qt_meta_tag_ZN4RoomE_t>(QMC::PropertyAccessInStaticMetaCall, qt_stringData,
            qt_methods, qt_properties, qt_enums);
}
Q_CONSTINIT const QMetaObject Room::staticMetaObject = { {
    nullptr,
    qt_staticMetaObjectStaticContent<qt_meta_tag_ZN4RoomE_t>.stringdata,
    qt_staticMetaObjectStaticContent<qt_meta_tag_ZN4RoomE_t>.data,
    qt_static_metacall,
    nullptr,
    qt_staticMetaObjectRelocatingContent<qt_meta_tag_ZN4RoomE_t>.metaTypes,
    nullptr
} };

void Room::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    auto *_t = reinterpret_cast<Room *>(_o);
    if (_c == QMetaObject::RegisterPropertyMetaType) {
        switch (_id) {
        default: *reinterpret_cast<int*>(_a[0]) = -1; break;
        case 1:
            *reinterpret_cast<int*>(_a[0]) = qRegisterMetaType< QList<UserProfile> >(); break;
        }
    }
    if (_c == QMetaObject::ReadProperty) {
        void *_v = _a[0];
        switch (_id) {
        case 0: *reinterpret_cast<QString*>(_v) = _t->roomId; break;
        case 1: *reinterpret_cast<QList<UserProfile>*>(_v) = _t->members; break;
        case 2: *reinterpret_cast<QUrl*>(_v) = _t->currentVideo; break;
        case 3: *reinterpret_cast<qint64*>(_v) = _t->playbackPosition; break;
        case 4: *reinterpret_cast<bool*>(_v) = _t->isPlaying; break;
        default: break;
        }
    }
    if (_c == QMetaObject::WriteProperty) {
        void *_v = _a[0];
        switch (_id) {
        case 0: QtMocHelpers::setProperty(_t->roomId, *reinterpret_cast<QString*>(_v)); break;
        case 1: QtMocHelpers::setProperty(_t->members, *reinterpret_cast<QList<UserProfile>*>(_v)); break;
        case 2: QtMocHelpers::setProperty(_t->currentVideo, *reinterpret_cast<QUrl*>(_v)); break;
        case 3: QtMocHelpers::setProperty(_t->playbackPosition, *reinterpret_cast<qint64*>(_v)); break;
        case 4: QtMocHelpers::setProperty(_t->isPlaying, *reinterpret_cast<bool*>(_v)); break;
        default: break;
        }
    }
}
QT_WARNING_POP
