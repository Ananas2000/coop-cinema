#ifndef REACTION_H
#define REACTION_H

#include <QString>
#include <QDateTime>

class Reaction {
public:
    Reaction(const QString& user = "", const QString& content = "",
        const QDateTime& timestamp = QDateTime::currentDateTime());

    QString user() const;
    QString content() const;
    QDateTime timestamp() const;
    QString displayText() const;

private:
    QString m_user;
    QString m_content;
    QDateTime m_timestamp;
};

#endif
