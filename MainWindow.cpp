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
#include <QLineEdit>
#include <QTabWidget>
#include <QFile>

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent),
    m_client(new Client(this)),
    m_videoRenderer(new VideoRenderer(this)),
    m_player(new Player(this)),
    m_notificationManager(new NotificationManager(this)),
    m_tabWidget(nullptr),
    m_menuTab(nullptr),
    m_createRoomBtn(nullptr),
    m_movieList(nullptr),
    m_roomIdEdit(nullptr),
    m_joinRoomBtn(nullptr),
    m_roomTab(nullptr),
    m_centralWidget(nullptr),
    m_mainSplitter(nullptr),
    m_videoContainer(nullptr),
    m_sidebar(nullptr),
    m_participantsList(nullptr),
    m_chatReactions(nullptr),
    m_playPauseBtn(nullptr),
    m_volumeSlider(nullptr),
    m_speedCombo(nullptr),
    m_positionLabel(nullptr),
    m_statusLabel(nullptr),
    m_playIcon(QIcon::fromTheme("media-playback-start")),
    m_pauseIcon(QIcon::fromTheme("media-playback-pause"))
{
    createUI();
    setupConnections();
    applyStyleSheet();

    m_client->connectToServer("localhost", 8888);
}

MainWindow::~MainWindow() {}

void MainWindow::createUI()
{
    // Create tab widget
    m_tabWidget = new QTabWidget(this);
    setCentralWidget(m_tabWidget);

    // ======================
    // Menu Tab
    // ======================
    m_menuTab = new QWidget(this);
    auto menuLayout = new QVBoxLayout(m_menuTab);

    // Create room button
    m_getFilmsBtn = new QPushButton("Get Films", m_menuTab);
    menuLayout->addWidget(m_getFilmsBtn);

    // Movie list
    menuLayout->addWidget(new QLabel("Available Movies:", m_menuTab));
    m_movieList = new QListWidget(m_menuTab);
    menuLayout->addWidget(m_movieList);

    // Film number input
    auto filmNumberContainer = new QWidget(m_menuTab);
    auto filmNumberLayout = new QHBoxLayout(filmNumberContainer);
    filmNumberLayout->setContentsMargins(0, 0, 0, 0);

    filmNumberLayout->addWidget(new QLabel("Film number:", filmNumberContainer));
    m_filmNumberEdit = new QLineEdit(filmNumberContainer);
    filmNumberLayout->addWidget(m_filmNumberEdit);

    m_createRoomBtn = new QPushButton("Create", filmNumberContainer);
    filmNumberLayout->addWidget(m_createRoomBtn);

    menuLayout->addWidget(filmNumberContainer);

    // Join room section
    auto joinContainer = new QWidget(m_menuTab);
    auto joinLayout = new QHBoxLayout(joinContainer);
    joinLayout->setContentsMargins(0, 0, 0, 0);

    joinLayout->addWidget(new QLabel("Room ID:", joinContainer));

    m_roomIdEdit = new QLineEdit(joinContainer);
    joinLayout->addWidget(m_roomIdEdit);

    m_joinRoomBtn = new QPushButton("Join Room", joinContainer);
    joinLayout->addWidget(m_joinRoomBtn);

    menuLayout->addWidget(joinContainer);
    menuLayout->addStretch();

    m_tabWidget->addTab(m_menuTab, "Menu");

    // ======================
    // Room Tab
    // ======================
    m_roomTab = new QWidget(this);
    auto roomLayout = new QVBoxLayout(m_roomTab);

    // Video container
    m_videoContainer = new QVideoWidget(m_roomTab);
    m_videoContainer->setMinimumSize(640, 360);
    m_player->setVideoOutput(m_videoContainer);

    // Sidebar
    m_sidebar = new QWidget(m_roomTab);
    auto sidebarLayout = new QVBoxLayout(m_sidebar);

    sidebarLayout->addWidget(new QLabel("Participants:", m_sidebar));
    m_participantsList = new QListWidget(m_sidebar);
    sidebarLayout->addWidget(m_participantsList);

    sidebarLayout->addWidget(new QLabel("Reactions:", m_sidebar));
    m_chatReactions = new QListWidget(m_sidebar);
    m_chatReactions->addItems({ "👍", "👎", "😂", "😮", "😢", "🔥" });
    sidebarLayout->addWidget(m_chatReactions);

    auto reactionBtn = new QPushButton("Send Reaction", m_sidebar);
    sidebarLayout->addWidget(reactionBtn);
    connect(reactionBtn, &QPushButton::clicked, this, &MainWindow::sendReaction);

    // Splitter for video and sidebar
    m_mainSplitter = new QSplitter(Qt::Horizontal, m_roomTab);
    m_mainSplitter->addWidget(m_videoContainer);
    m_mainSplitter->addWidget(m_sidebar);
    m_mainSplitter->setStretchFactor(0, 3);
    m_mainSplitter->setStretchFactor(1, 1);

    // Controls
    m_playPauseBtn = new QPushButton(m_roomTab);
    m_playPauseBtn->setIcon(m_playIcon);

    m_volumeSlider = new QSlider(Qt::Horizontal, m_roomTab);
    m_volumeSlider->setRange(0, 100);
    m_volumeSlider->setValue(50);

    m_speedCombo = new QComboBox(m_roomTab);
    m_speedCombo->addItems({ "0.5x", "1.0x", "1.5x", "2.0x" });
    m_speedCombo->setCurrentText("1.0x");

    m_positionLabel = new QLabel("00:00", m_roomTab);
    m_statusLabel = new QLabel("Disconnected", m_roomTab);

    auto controlsLayout = new QHBoxLayout;
    controlsLayout->addWidget(m_playPauseBtn);
    controlsLayout->addWidget(new QLabel("Volume:", m_roomTab));
    controlsLayout->addWidget(m_volumeSlider);
    controlsLayout->addWidget(new QLabel("Speed:", m_roomTab));
    controlsLayout->addWidget(m_speedCombo);
    controlsLayout->addStretch();
    controlsLayout->addWidget(m_positionLabel);
    controlsLayout->addWidget(m_statusLabel);

    // Add to room layout
    roomLayout->addWidget(m_mainSplitter);
    roomLayout->addLayout(controlsLayout);

    m_tabWidget->addTab(m_roomTab, "Room");

    // Start with Menu tab
    m_tabWidget->setCurrentIndex(0);
}

void MainWindow::setupConnections()
{
    // Player controls
    connect(m_playPauseBtn, &QPushButton::clicked, this, &MainWindow::handlePlayPause);
    connect(m_volumeSlider, &QSlider::valueChanged, this, &MainWindow::handleVolumeChange);
    connect(m_speedCombo, &QComboBox::currentTextChanged, this, &MainWindow::handleSpeedChange);

    // Player signals
    connect(m_player, &Player::positionChanged, this, &MainWindow::updatePositionDisplay);
    connect(m_player, &Player::playbackStateChanged, this, &MainWindow::updatePlayerControls);
    connect(m_player, &Player::errorOccurred, this, &MainWindow::showNotification);

    // Client signals
    connect(m_client, &Client::connected, [this]() {
        updateConnectionStatus(true);
        showNotification("Connected to server");
        });

    connect(m_client, &Client::disconnected, [this]() {
        updateConnectionStatus(false);
        showNotification("Disconnected from server");
        m_tabWidget->setCurrentIndex(0); // Return to Menu tab
        });

    connect(m_client, &Client::roomCreated, this, &MainWindow::onRoomJoined);
    connect(m_client, &Client::participantsUpdated, this, &MainWindow::onParticipantsUpdated);
    connect(m_client, &Client::videoUrlReceived, this, &MainWindow::handleVideoUrlReceived);
    connect(m_client, &Client::errorOccurred, this, &MainWindow::showNotification);
    connect(m_client, &Client::filmsListReceived, this, &MainWindow::onFilmsListReceived);

    connect(m_getFilmsBtn, &QPushButton::clicked, [this]() {
        m_client->sendMessage("GET_FILMS");
        });

    /*connect(m_createRoomBtn, &QPushButton::clicked, this, [this]() {
        m_client->sendMessage("1"); // Только команда создания
        });*/

    /*connect(m_createRoomBtn, &QPushButton::clicked, this, [this]() {
    int filmIndex = m_movieList->currentRow(); // Получаем выбранный фильм
    m_client->createRoom(filmIndex + 1); // Индексация с 1 на сервере
    });*/

    // Menu tab actions
    connect(m_createRoomBtn, &QPushButton::clicked, this, [this]() {
        if (m_client->currentRoom().isEmpty()) {
            int filmIndex = m_filmNumberEdit->text().toInt();
            m_client->createRoom(filmIndex);
        }
        });

    connect(m_joinRoomBtn, &QPushButton::clicked, this, [this]() {
        QString roomId = m_roomIdEdit->text().trimmed();
        if (roomId.isEmpty()) {
            showNotification("Please enter room ID");
            return;
        }

        if (m_client->currentRoom() != roomId) {
            m_client->joinRoom(roomId);
        }
        else {
            showNotification("You are already in this room");
        }
        });

    // Automatically switch to room tab when room is created
    connect(m_client, &Client::roomCreated, [this]() {
        m_tabWidget->setCurrentIndex(1);
        });

    // Automatically switch to room tab when joining
    connect(m_client, &Client::videoUrlReceived, [this]() {
        m_tabWidget->setCurrentIndex(1);
        });
}

void MainWindow::applyStyleSheet()
{
    setStyleSheet(
        "QMainWindow, QWidget { background-color: #2b2b2b; color: white; }"
        "QLabel, QListWidget, QPushButton, QComboBox, QLineEdit { "
        "   font-size: 14px; "
        "   background-color: #3c3c3c; "
        "   color: white; "
        "   border: 1px solid #555; "
        "   padding: 5px; "
        "}"
        "QTabWidget::pane { border: 0; }"
        "QTabBar::tab { "
        "   background: #3c3c3c; "
        "   color: white; "
        "   padding: 8px; "
        "   border: 1px solid #555; "
        "   border-bottom: none; "
        "   border-top-left-radius: 4px; "
        "   border-top-right-radius: 4px; "
        "}"
        "QTabBar::tab:selected { "
        "   background: #555; "
        "   border-color: #777; "
        "}"
        "QLineEdit { background: #333; }"
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
    cleaned.remove('x');
    bool ok;
    float rate = cleaned.toFloat(&ok);
    if (ok) {
        m_player->setPlaybackRate(rate);
    }
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

void MainWindow::onFilmsListReceived(const QStringList& films)
{
    // Сохраняем в файл
    QFile file("films.txt");
    if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QTextStream out(&file);
        for (const QString& film : films) {
            out << film << "\n";
        }
        file.close();
    }

    // Отображаем в списке
    m_movieList->clear();
    m_movieList->addItems(films);
}

void MainWindow::onRoomJoined(const QString& roomId)
{
    showNotification("Joined room: " + roomId);
    m_roomIdEdit->clear();
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
    m_player->setMedia(url);
    m_player->play();

    updatePlayerControls(true);
    showNotification("Видео загружено: " + url.toString());
}

/*void MainWindow::handleVideoUrlReceived(const QUrl& url) {
    // Проверяем тип URL
    if(url.isLocalFile()) {
        m_player->setMedia(url);
    } else {
        // Для сетевых источников используем QMediaContent
        m_player->setMedia(QMediaContent(url));
    }
    m_player->play();
}*/

void MainWindow::updatePlayerControls(bool isPlaying)
{
    m_playPauseBtn->setIcon(isPlaying ? m_pauseIcon : m_playIcon);
}

void MainWindow::updateConnectionStatus(bool connected)
{
    m_statusLabel->setText(connected ? "Connected" : "Disconnected");
    m_statusLabel->setStyleSheet(connected ? "color: green;" : "color: red;");
}