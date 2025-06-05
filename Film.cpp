#include "Film.h"

Film::Film(int id, const QString& title, const QUrl& url)
    : m_id(id), m_title(title), m_url(url) {
}

int Film::id() const { return m_id; }
QString Film::title() const { return m_title; }
QUrl Film::url() const { return m_url; }

QString Film::displayText() const {
    return QString("%1. %2").arg(QString::number(m_id), m_title);
}