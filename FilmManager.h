#ifndef FILMMANAGER_H
#define FILMMANAGER_H

#include <QObject>
#include <QList>
#include "Film.h"

class FilmManager : public QObject {
    Q_OBJECT
public:
    explicit FilmManager(QObject* parent = nullptr);

    void loadFilms(const QStringList& filmStrings);
    QList<Film> films() const;
    Film filmById(int id) const;
    void filterFilms(const QString& text);

signals:
    void filmsUpdated();

private:
    QList<Film> m_films;
};

#endif