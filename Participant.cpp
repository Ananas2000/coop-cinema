#include "Participant.h"

Participant::Participant(const QString& nickname, const QString& avatarPath, bool isYou)
    : m_nickname(nickname), m_avatarPath(avatarPath), m_isYou(isYou) {
}

QString Participant::nickname() const { return m_nickname; }
QString Participant::avatarPath() const { return m_avatarPath; }
bool Participant::isYou() const { return m_isYou; }

QString Participant::displayName() const {
    return m_isYou ? m_nickname + " (you)" : m_nickname;
}