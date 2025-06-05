#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QSplitter>
#include <QPixmap>
#include <QIcon>
#include <QListWidgetItem> // ???????? ????????? ??? QListWidgetItem
#include "Client.h"
#include "VideoRenderer.h"
#include "Player.h"
#include "NotificationManager.h"

QT_BEGIN_NAMESPACE
class QVideoWidget;
class QListWidget;
class QListWidgetItem; // ????????? ??????????????? ??????????
class QPushButton;
class QSlider;
class QLabel;
class QComboBox;
class QTabWidget;
class QLineEdit;
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
    void onPlayerStateReceived(const QString& roomId, const QString& senderNick, bool isPaused, qint64 position);

    void onRoomJoined(const QString& roomId);
    void onParticipantsUpdated(const QStringList& users);
    void onFilmsListReceived(const QStringList& films);
    void showNotification(const QString& message);
    void handleVideoUrlReceived(const QUrl& url);
    void filterMovies(const QString& text);
    void onMovieDoubleClicked(QListWidgetItem* item);
    void copyRoomId();

    void updatePlayerControls(bool isPlaying);
    void updateConnectionStatus(bool connected);

private:
    void createUI();
    void setupConnections();
    void applyStyleSheet();

    bool m_syncing = false;
    qint64 m_lastPlayerStateSendTime = 0;

    Client* m_client;
    VideoRenderer* m_videoRenderer;
    Player* m_player;
    NotificationManager* m_notificationManager;

    // UI Elements
    QTabWidget* m_tabWidget;
    QPushButton* m_getFilmsBtn;
    QLineEdit* m_filmNumberEdit;
    QPushButton* m_createRoomBtn;
    QLineEdit* m_searchEdit;
    QListWidget* m_movieList;

    // Menu Tab
    QWidget* m_menuTab;
    QLineEdit* m_roomIdEdit;
    QPushButton* m_joinRoomBtn;

    // Room Tab
    QWidget* m_roomTab;
    QWidget* m_centralWidget;
    QSplitter* m_mainSplitter;
    QVideoWidget* m_videoContainer;
    QWidget* m_sidebar;
    QListWidget* m_participantsList;
    QListWidget* m_chatReactions;
    QPushButton* m_playPauseBtn;
    QSlider* m_volumeSlider;
    QSlider* m_positionSlider;
    QLabel* m_durationLabel;
    QLabel* m_volLabel;
    QLabel* m_speedLabel;
    QComboBox* m_speedCombo;
    QLabel* m_positionLabel;
    QLabel* m_statusLabel;
    QLabel* m_roomIdLabel;
    QPushButton* m_leaveRoomBtn;
    QString formatTime(qint64 ms) const;
    bool eventFilter(QObject* watched, QEvent* event) override;

    QIcon m_playIcon;
    QIcon m_pauseIcon;
};

#endif