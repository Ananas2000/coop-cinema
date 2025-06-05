#ifndef FILM_H
#define FILM_H

#include <QString>
#include <QUrl>

class Film {
public:
    Film(int id = 0, const QString& title = "", const QUrl& url = QUrl());

    int id() const;
    QString title() const;
    QUrl url() const;
    QString displayText() const;

private:
    int m_id;
    QString m_title;
    QUrl m_url;
};

#endif