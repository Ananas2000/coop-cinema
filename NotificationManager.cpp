#include "NotificationManager.h"

#include <QVBoxLayout>
#include <QGraphicsOpacityEffect>
#include <QMouseEvent>

NotificationManager::NotificationManager(QWidget* parentWidget, QObject* parent)
    : QObject(parent),
    m_parentWidget(parentWidget),
    m_notificationLabel(new QLabel(parentWidget)),
    m_animation(new QPropertyAnimation(this)),
    m_isShowing(false)
{
    setupUI();

    connect(&m_displayTimer, &QTimer::timeout, this, &NotificationManager::hideCurrentNotification);
    connect(m_animation, &QPropertyAnimation::finished, this, [this]() {
        if (!m_isShowing) {
            m_notificationLabel->hide();
            processNextNotification();
        }
        });
}

void NotificationManager::setupUI()
{
    m_notificationLabel->setWindowFlags(Qt::FramelessWindowHint | Qt::ToolTip);
    m_notificationLabel->setAttribute(Qt::WA_TranslucentBackground);
    m_notificationLabel->setAlignment(Qt::AlignCenter);
    m_notificationLabel->setMargin(10);
    m_notificationLabel->setWordWrap(true);
    m_notificationLabel->setStyleSheet("border-radius: 6px; padding: 10px; font-size: 14px;");

    QGraphicsOpacityEffect* effect = new QGraphicsOpacityEffect(m_notificationLabel);
    m_notificationLabel->setGraphicsEffect(effect);

    m_animation->setTargetObject(effect);
    m_animation->setPropertyName("opacity");
    m_animation->setDuration(400);

    m_notificationLabel->hide();

    if (m_parentWidget)
        m_notificationLabel->setParent(m_parentWidget);
}

void NotificationManager::showNotification(const QString& message, NotificationType type, int durationMs)
{
    if (!m_enabled)
        return;

    Notification notif{ message, type, durationMs > 0 ? durationMs : m_defaultDuration };
    m_notificationQueue.enqueue(notif);

    if (!m_isShowing) {
        processNextNotification();
    }
}

void NotificationManager::processNextNotification()
{
    if (m_notificationQueue.isEmpty()) {
        m_isShowing = false;
        return;
    }

    Notification notif = m_notificationQueue.dequeue();
    m_notificationLabel->setText(notif.message);
    applyStyle(notif.type);

    if (m_parentWidget) {
        int w = m_parentWidget->width();
        int h = 60;
        m_notificationLabel->setFixedWidth(w / 2);
        m_notificationLabel->move((w - m_notificationLabel->width()) / 2, 50);
    }

    m_notificationLabel->show();
    startAnimation(true);

    m_displayTimer.start(notif.duration);
    m_isShowing = true;
}

void NotificationManager::hideCurrentNotification()
{
    m_displayTimer.stop();
    startAnimation(false);
    m_isShowing = false;
}

void NotificationManager::startAnimation(bool show)
{
    auto* effect = qobject_cast<QGraphicsOpacityEffect*>(m_notificationLabel->graphicsEffect());
    if (!effect)
        return;

    m_animation->stop();
    m_animation->setStartValue(show ? 0.0 : 1.0);
    m_animation->setEndValue(show ? 1.0 : 0.0);
    m_animation->start();
}

void NotificationManager::applyStyle(NotificationType type)
{
    QString bgColor;
    switch (type) {
    case Info:     bgColor = "#2d8cf0"; break;
    case Warning:  bgColor = "#f90";    break;
    case Error:    bgColor = "#ed4014"; break;
    case Reaction: bgColor = "#19be6b"; break;
    }

    m_notificationLabel->setStyleSheet(QString(
        "background-color: %1; color: white; border-radius: 6px; padding: 10px; font-size: 14px;")
        .arg(bgColor));
}

void NotificationManager::setNotificationsEnabled(bool enabled)
{
    m_enabled = enabled;
}

void NotificationManager::setDisplayDuration(int durationMs)
{
    m_defaultDuration = durationMs;
}
