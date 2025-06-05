#include "FilmManager.h"
#include "Film.h"

FilmManager::FilmManager(QObject* parent) : QObject(parent) {}

void FilmManager::loadFilms(const QStringList& filmStrings) {
    m_films.clear();
    
    for (int i = 0; i < filmStrings.size(); i++) {
        QString filmStr = filmStrings[i];
        QStringList parts = filmStr.split(". ");
        if (parts.size() < 2) continue;
        
        int id = parts[0].toInt();
        QString title = parts[1]; // Берем только название
        
        m_films.append(Film(id, title));
    }
    
    emit filmsUpdated();
}

QList<Film> FilmManager::films() const {
    return m_films;
}

Film FilmManager::filmById(int id) const {
    for (const Film& film : m_films) {
        if (film.id() == id) {
            return film;
        }
    }
    return Film();
}

void FilmManager::filterFilms(const QString& text) {
    Q_UNUSED(text);
    emit filmsUpdated();
}