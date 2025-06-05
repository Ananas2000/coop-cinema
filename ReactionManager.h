#ifndef REACTIONMANAGER_H
#define REACTIONMANAGER_H

#include <QObject>
#include <QList>
#include "Reaction.h"

class ReactionManager : public QObject {
    Q_OBJECT
public:
    explicit ReactionManager(QObject* parent = nullptr);

    void addReaction(const QString& user, const QString& reaction);
    QList<Reaction> reactions() const;
    void clear();

signals:
    void newReactionAdded();

private:
    QList<Reaction> m_reactions;
};

#endif
