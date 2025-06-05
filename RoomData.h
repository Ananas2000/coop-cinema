#ifndef ROOMDATA_H
#define ROOMDATA_H

#include <QString>
#include <QList>
#include <QUrl>
#include <QMetaType>

struct UserProfile {
    QString nickname;
    QString avatarUrl;
    QString sessionId;

    bool operator==(const UserProfile& other) const {
        return nickname == other.nickname &&
            avatarUrl == other.avatarUrl &&
            sessionId == other.sessionId;
    }

    Q_GADGET
        Q_PROPERTY(QString nickname MEMBER nickname)
        Q_PROPERTY(QString avatarUrl MEMBER avatarUrl)
        Q_PROPERTY(QString sessionId MEMBER sessionId)
};

struct Room {
    QString roomId;
    QList<UserProfile> members;
    QUrl currentVideo;
    qint64 playbackPosition;
    bool isPlaying;

    bool operator==(const Room& other) const {
        return roomId == other.roomId &&
            members == other.members &&
            currentVideo == other.currentVideo &&
            playbackPosition == other.playbackPosition &&
            isPlaying == other.isPlaying;
    }

    Q_GADGET
        Q_PROPERTY(QString roomId MEMBER roomId)
        Q_PROPERTY(QList<UserProfile> members MEMBER members)
        Q_PROPERTY(QUrl currentVideo MEMBER currentVideo)
        Q_PROPERTY(qint64 playbackPosition MEMBER playbackPosition)
        Q_PROPERTY(bool isPlaying MEMBER isPlaying)
};

Q_DECLARE_METATYPE(UserProfile)
Q_DECLARE_METATYPE(Room)

#endif