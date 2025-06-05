#include "ReactionManager.h"

ReactionManager::ReactionManager(QObject* parent) : QObject(parent) {}

void ReactionManager::addReaction(const QString& user, const QString& reaction) {
    m_reactions.append(Reaction(user, reaction));
    emit newReactionAdded();
}

QList<Reaction> ReactionManager::reactions() const {
    return m_reactions;
}

void ReactionManager::clear() {
    m_reactions.clear();
}