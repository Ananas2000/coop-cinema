#include "Reaction.h"

Reaction::Reaction(const QString& user, const QString& content, const QDateTime& timestamp)
    : m_user(user), m_content(content), m_timestamp(timestamp) {
}

QString Reaction::user() const { return m_user; }
QString Reaction::content() const { return m_content; }
QDateTime Reaction::timestamp() const { return m_timestamp; }

QString Reaction::displayText() const {
    return QString("[%1] %2: %3")
        .arg(m_timestamp.toString("hh:mm"), m_user, m_content);
}