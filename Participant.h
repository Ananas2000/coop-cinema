#ifndef PARTICIPANT_H
#define PARTICIPANT_H

#include <QString>

class Participant {
public:
    Participant(const QString& nickname = "", const QString& avatarPath = "", bool isYou = false);

    QString nickname() const;
    QString avatarPath() const;
    bool isYou() const;
    QString displayName() const;

private:
    QString m_nickname;
    QString m_avatarPath;
    bool m_isYou;
};

#endif
