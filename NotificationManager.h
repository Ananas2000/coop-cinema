#ifndef NOTIFICATIONMANAGER_H
#define NOTIFICATIONMANAGER_H

#include <QObject>
#include <QWidget>
#include <QQueue>
#include <QTimer>
#include <QLabel>
#include <QPropertyAnimation>

class NotificationManager : public QObject
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

    void showNotification(const QString& message,
        NotificationType type = Info,
        int durationMs = 3000);

public slots:
    void setNotificationsEnabled(bool enabled);
    void setDisplayDuration(int durationMs);

signals:
    void notificationClicked(const QString& message);

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