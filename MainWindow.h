#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QSplitter>
#include "Client.h"
#include "VideoRenderer.h"
#include "Player.h"
#include "NotificationManager.h"

QT_BEGIN_NAMESPACE
class QVideoWidget;
class QListWidget;
class QPushButton;
class QSlider;
class QLabel;
class QComboBox;
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget* parent = nullptr);
    ~MainWindow();

private slots:
    void handlePlayPause();
    void handleVolumeChange(int volume);
    void handleSpeedChange(const QString& speed);
    void sendReaction();
    void updatePositionDisplay(qint64 position);

    void onRoomJoined(const QString& roomId);
    void onParticipantsUpdated(const QStringList& users);
    void showNotification(const QString& message);
    void handleVideoUrlReceived(const QUrl& url);

    void updatePlayerControls(bool isPlaying);
    void updateConnectionStatus(bool connected);

private:
    void createUI();
    void setupConnections();
    void applyStyleSheet();

    Client* m_client;
    VideoRenderer* m_videoRenderer;
    Player* m_player;
    NotificationManager* m_notificationManager;

    QWidget* m_centralWidget;
    QSplitter* m_mainSplitter;
    QVideoWidget* m_videoContainer;
    QWidget* m_sidebar;
    QListWidget* m_participantsList;
    QListWidget* m_chatReactions;
    QPushButton* m_playPauseBtn;
    QSlider* m_volumeSlider;
    QComboBox* m_speedCombo;
    QLabel* m_positionLabel;
    QLabel* m_statusLabel;

    QIcon m_playIcon;
    QIcon m_pauseIcon;
};

#endif