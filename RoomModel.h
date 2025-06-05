#ifndef ROOMMODEL_H
#define ROOMMODEL_H

#include <QString>
#include <QUrl>
#include <QList>
#include "Participant.h"

class RoomModel {
public:
    RoomModel(const QString& id = "", const QUrl& videoUrl = QUrl());

    QString id() const;
    QUrl videoUrl() const;
    void setVideoUrl(const QUrl& url);
    void addParticipant(const Participant& participant);
    void removeParticipant(const QString& nickname);
    void clearParticipants();
    QList<Participant> participants() const;

private:
    QString m_id;
    QUrl m_videoUrl;
    QList<Participant> m_participants;
};

#endif