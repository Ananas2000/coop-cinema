#ifndef NOTIFICATIONMANAGER_H
#define NOTIFICATIONMANAGER_H

#include <QObject>
#include <QWidget>
#include <QQueue>
#include <QTimer>
#include <QLabel>
#include <QPropertyAnimation>
#include <vector>

class Observer {
public:
    virtual ~Observer() = default;
    virtual void update(const QString& message,
        int type = 0,
        int durationMs = 3000) = 0;
};

class Subject {
public:
    virtual ~Subject() = default;
    void registerObserver(Observer* observer) {
        observers_.push_back(observer);
    }
    void removeObserver(Observer* observer) {
        auto it = std::find(observers_.begin(), observers_.end(), observer);
        if (it != observers_.end())
            observers_.erase(it);
    }
    void notifyObservers(const QString& message,
        int type = 0,
        int durationMs = 3000) {
        for (auto* observer : observers_)
            observer->update(message, type, durationMs);
    }

private:
    std::vector<Observer*> observers_;
};

class NotificationManager : public QObject, public Observer
{
    Q_OBJECT
public:
    enum NotificationType {
        Info,
        Warning,
        Error,
        Reaction
    };
    Q_ENUM(NotificationType)

        explicit NotificationManager(QWidget* parentWidget = nullptr, QObject* parent = nullptr);
    ~NotificationManager();

    void update(const QString& message, int type, int durationMs) override;

public slots:
    void setNotificationsEnabled(bool enabled);
    void setDisplayDuration(int durationMs);

private slots:
    void processNextNotification();
    void hideCurrentNotification();

private:
    struct Notification {
        QString message;
        NotificationType type;
        int duration;
    };

    void setupUI();
    void applyStyle(NotificationType type);
    void startAnimation(bool show);

    QWidget* m_parentWidget;
    QLabel* m_notificationLabel;
    QPropertyAnimation* m_animation;
    QQueue<Notification> m_notificationQueue;
    QTimer m_displayTimer;
    bool m_isShowing = false;
    bool m_enabled = true;
    int m_defaultDuration = 3000;
};

#endif