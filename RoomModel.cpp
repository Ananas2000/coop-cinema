#include "RoomModel.h"

RoomModel::RoomModel(const QString& id, const QUrl& videoUrl)
    : m_id(id), m_videoUrl(videoUrl) {
}

QString RoomModel::id() const { return m_id; }
QUrl RoomModel::videoUrl() const { return m_videoUrl; }
void RoomModel::setVideoUrl(const QUrl& url) { m_videoUrl = url; }

void RoomModel::addParticipant(const Participant& participant) {
    removeParticipant(participant.nickname());
    m_participants.append(participant);
}

void RoomModel::removeParticipant(const QString& nickname) {
    for (int i = 0; i < m_participants.size(); i++) {
        if (m_participants[i].nickname() == nickname) {
            m_participants.removeAt(i);
            break;
        }
    }
}

void RoomModel::clearParticipants() {
    m_participants.clear();
}

QList<Participant> RoomModel::participants() const {
    return m_participants;
}