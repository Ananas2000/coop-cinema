#include "MainWindow.h"
#include "Client.h"
#include "VideoRenderer.h"
#include "Player.h"
#include "NotificationManager.h"

#include <QVideoWidget>
#include <QListWidget>
#include <QPushButton>
#include <QSlider>
#include <QLabel>
#include <QComboBox>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QSplitter>
#include <QMessageBox>
#include <QIcon>
#include <QUrl>
#include <QStringList>

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent),
    m_client(new Client(this)),
    m_videoRenderer(new VideoRenderer(this)),
    m_player(new Player(this)),
    m_notificationManager(new NotificationManager(this)),
    m_centralWidget(new QWidget(this)),
    m_mainSplitter(new QSplitter(Qt::Horizontal, this)),
    m_videoContainer(new QVideoWidget(this)),
    m_sidebar(new QWidget(this)),
    m_participantsList(new QListWidget(this)),
    m_chatReactions(new QListWidget(this)),
    m_playPauseBtn(new QPushButton(this)),
    m_volumeSlider(new QSlider(Qt::Horizontal, this)),
    m_speedCombo(new QComboBox(this)),
    m_positionLabel(new QLabel("00:00", this)),
    m_statusLabel(new QLabel("Disconnected", this)),
    m_playIcon(QIcon::fromTheme("media-playback-start")),
    m_pauseIcon(QIcon::fromTheme("media-playback-pause"))
{
    createUI();
    setupConnections();
    applyStyleSheet();

    m_client->connectToServer("localhost", 12345); // Пример подключения
}

MainWindow::~MainWindow() {}

void MainWindow::createUI()
{
    setCentralWidget(m_centralWidget);

    m_videoContainer->setMinimumSize(640, 360);
    m_player->setVideoOutput(m_videoContainer);

    // Layout боковой панели
    auto sidebarLayout = new QVBoxLayout;
    sidebarLayout->addWidget(new QLabel("Participants:", this));
    sidebarLayout->addWidget(m_participantsList);

    sidebarLayout->addWidget(new QLabel("Reactions:", this));
    sidebarLayout->addWidget(m_chatReactions);

    auto reactionBtn = new QPushButton("Send Reaction", this);
    sidebarLayout->addWidget(reactionBtn);
    connect(reactionBtn, &QPushButton::clicked, this, &MainWindow::sendReaction);

    m_sidebar->setLayout(sidebarLayout);

    m_mainSplitter->addWidget(m_videoContainer);
    m_mainSplitter->addWidget(m_sidebar);
    m_mainSplitter->setStretchFactor(0, 3);
    m_mainSplitter->setStretchFactor(1, 1);

    // Нижняя панель управления
    m_playPauseBtn->setIcon(m_playIcon);
    m_volumeSlider->setRange(0, 100);
    m_volumeSlider->setValue(50);
    m_speedCombo->addItems({ "0.5x", "1.0x", "1.5x", "2.0x" });
    m_speedCombo->setCurrentText("1.0x");

    auto controlsLayout = new QHBoxLayout;
    controlsLayout->addWidget(m_playPauseBtn);
    controlsLayout->addWidget(new QLabel("Volume:", this));
    controlsLayout->addWidget(m_volumeSlider);
    controlsLayout->addWidget(new QLabel("Speed:", this));
    controlsLayout->addWidget(m_speedCombo);
    controlsLayout->addStretch();
    controlsLayout->addWidget(m_positionLabel);
    controlsLayout->addWidget(m_statusLabel);

    auto mainLayout = new QVBoxLayout(m_centralWidget);
    mainLayout->addWidget(m_mainSplitter);
    mainLayout->addLayout(controlsLayout);
}

void MainWindow::setupConnections()
{
    connect(m_playPauseBtn, &QPushButton::clicked, this, &MainWindow::handlePlayPause);
    connect(m_volumeSlider, &QSlider::valueChanged, this, &MainWindow::handleVolumeChange);
    connect(m_speedCombo, &QComboBox::currentTextChanged, this, &MainWindow::handleSpeedChange);

    connect(m_player, &Player::positionChanged, this, &MainWindow::updatePositionDisplay);
    connect(m_player, &Player::playbackStateChanged, this, &MainWindow::updatePlayerControls);
    connect(m_player, &Player::errorOccurred, this, &MainWindow::showNotification);

    connect(m_client, &Client::connected, [this]() { updateConnectionStatus(true); });
    connect(m_client, &Client::disconnected, [this]() { updateConnectionStatus(false); });
    connect(m_client, &Client::roomCreated, this, &MainWindow::onRoomJoined);
    connect(m_client, &Client::participantsUpdated, this, &MainWindow::onParticipantsUpdated);
    connect(m_client, &Client::videoUrlReceived, this, &MainWindow::handleVideoUrlReceived);
}

void MainWindow::applyStyleSheet()
{
    setStyleSheet(
        "QMainWindow { background-color: #2b2b2b; color: white; }"
        "QLabel, QListWidget, QPushButton, QComboBox { font-size: 14px; }"
    );
}

void MainWindow::handlePlayPause()
{
    if (m_player->isPlaying()) {
        m_player->pause();
    }
    else {
        m_player->play();
    }
}

void MainWindow::handleVolumeChange(int volume)
{
    m_player->setVolume(volume);
}

void MainWindow::handleSpeedChange(const QString& speed)
{
    QString cleaned = speed;
    cleaned.remove('x'); // Исправленная строка
    float rate = cleaned.toFloat();
    m_player->setPlaybackRate(rate);
}

void MainWindow::sendReaction()
{
    auto selected = m_chatReactions->currentItem();
    if (!selected) return;

    m_client->sendReaction(selected->text());
}

void MainWindow::updatePositionDisplay(qint64 position)
{
    int seconds = static_cast<int>(position / 1000);
    int minutes = seconds / 60;
    seconds %= 60;
    m_positionLabel->setText(QString::asprintf("%02d:%02d", minutes, seconds));
}

void MainWindow::onRoomJoined(const QString& roomId)
{
    showNotification("Joined room: " + roomId);
}

void MainWindow::onParticipantsUpdated(const QStringList& users)
{
    m_participantsList->clear();
    m_participantsList->addItems(users);
}

void MainWindow::showNotification(const QString& message)
{
    m_notificationManager->showNotification(message);
}

void MainWindow::handleVideoUrlReceived(const QUrl& url)
{
    m_player->stop();
    m_player->setPlaybackRate(1.0);
    m_client->sendMessage("START_PLAYBACK|" + url.toString()); // Исправленная строка
    m_player->play();
}

void MainWindow::updatePlayerControls(bool isPlaying)
{
    m_playPauseBtn->setIcon(isPlaying ? m_pauseIcon : m_playIcon);
}

void MainWindow::updateConnectionStatus(bool connected)
{
    m_statusLabel->setText(connected ? "Connected" : "Disconnected");
    m_statusLabel->setStyleSheet(connected ? "color: green;" : "color: red;");
}