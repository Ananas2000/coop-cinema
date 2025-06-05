#include "MainWindow.h"
#include "Client.h"
#include "Player.h"
#include "NotificationManager.h"

#include <QVideoWidget>
#include <QListWidget>
#include <QPushButton>
#include <QApplication>
#include <QMouseEvent>
#include <QClipboard>
#include <QSlider>
#include <QLabel>
#include <QComboBox>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QSplitter>
#include <QIcon>
#include <QUrl>
#include <QLineEdit>
#include <QTabWidget>
#include <QDateTime>
#include <QFile>

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent),
    m_client(new Client(this)),
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

    m_roomIdLabel->installEventFilter(this);

    m_client->connectToServer("localhost", 8888);

    QTimer::singleShot(100, this, [this]() {
        showMaximized();
        });
}

MainWindow::~MainWindow() {}

void MainWindow::createUI()
{
    m_tabWidget = new QTabWidget(this);
    setCentralWidget(m_tabWidget);

    m_menuTab = new QWidget(this);
    auto menuLayout = new QVBoxLayout(m_menuTab);
    menuLayout->setSpacing(10);

    m_getFilmsBtn = new QPushButton("Get Films", m_menuTab);
    menuLayout->addWidget(m_getFilmsBtn, 0, Qt::AlignLeft);

    auto searchLayout = new QHBoxLayout();
    m_searchEdit = new QLineEdit(m_menuTab);
    m_searchEdit->setPlaceholderText("Search...");
    m_searchEdit->setClearButtonEnabled(true);
    m_searchEdit->setMaximumWidth(1000);
    searchLayout->addWidget(m_searchEdit, 1000, Qt::AlignLeft);
    menuLayout->addLayout(searchLayout);

    m_movieList = new QListWidget(m_menuTab);
    m_movieList->setMinimumHeight(300);
    m_movieList->setMaximumWidth(1500);
    menuLayout->addWidget(m_movieList, 1);

    auto filmSelectionGroup = new QWidget(m_menuTab);
    auto filmLayout = new QHBoxLayout(filmSelectionGroup);
    filmLayout->setContentsMargins(0, 0, 0, 0);
    filmLayout->setSpacing(5);

    auto filmLabel = new QLabel("Film number:", filmSelectionGroup);
    filmLabel->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
    filmLayout->addWidget(filmLabel);

    m_filmNumberEdit = new QLineEdit(filmSelectionGroup);
    m_filmNumberEdit->setAlignment(Qt::AlignCenter);
    m_filmNumberEdit->setFixedWidth(40);
    m_filmNumberEdit->setValidator(new QIntValidator(1, 999, this));
    filmLayout->addWidget(m_filmNumberEdit);

    m_createRoomBtn = new QPushButton("Create", filmSelectionGroup);
    m_createRoomBtn->setFixedWidth(80);
    filmLayout->addWidget(m_createRoomBtn);
    menuLayout->addWidget(filmSelectionGroup, 0, Qt::AlignLeft);

    auto joinGroup = new QWidget(m_menuTab);
    auto joinLayout = new QHBoxLayout(joinGroup);
    joinLayout->setContentsMargins(0, 0, 0, 0);
    joinLayout->setSpacing(5);

    auto roomLabel = new QLabel("Room ID:", joinGroup);
    roomLabel->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
    joinLayout->addWidget(roomLabel);

    m_roomIdEdit = new QLineEdit(joinGroup);
    m_roomIdEdit->setAlignment(Qt::AlignCenter);
    m_roomIdEdit->setFixedWidth(80);
    m_roomIdEdit->setPlaceholderText("ID");
    joinLayout->addWidget(m_roomIdEdit);

    m_joinRoomBtn = new QPushButton("Join", joinGroup);
    m_joinRoomBtn->setFixedWidth(80);
    joinLayout->addWidget(m_joinRoomBtn);
    menuLayout->addWidget(joinGroup, 0, Qt::AlignLeft);
    m_tabWidget->addTab(m_menuTab, "Menu");

    m_roomTab = new QWidget(this);
    auto roomLayout = new QVBoxLayout(m_roomTab);
    roomLayout->setSpacing(10);

    auto roomIdPanel = new QWidget(m_roomTab);
    auto roomIdLayout = new QHBoxLayout(roomIdPanel);
    roomIdLayout->setContentsMargins(5, 2, 5, 2);
    roomIdLayout->setSpacing(5);

    roomIdLayout->addWidget(new QLabel("Room ID:", roomIdPanel));
    m_roomIdLabel = new QLabel("None", roomIdPanel);
    m_roomIdLabel->setStyleSheet("padding: 2px 5px;");
    m_roomIdLabel->setCursor(Qt::PointingHandCursor);
    roomIdLayout->addWidget(m_roomIdLabel);
    roomIdLayout->addStretch();

    roomIdPanel->setFixedHeight(40);
    roomIdPanel->setFixedWidth(150);

    m_videoContainer = new QVideoWidget(m_roomTab);
    m_videoContainer->setMaximumSize(1700, 750);
    m_player->setVideoOutput(m_videoContainer);

    m_sidebar = new QWidget(m_roomTab);
    auto sidebarLayout = new QVBoxLayout(m_sidebar);
    sidebarLayout->setSpacing(5);

    sidebarLayout->addWidget(new QLabel("Participants:", m_sidebar));
    m_participantsList = new QListWidget(m_sidebar);
    sidebarLayout->addWidget(m_participantsList);

    m_chatReactions = new QListWidget(m_sidebar);
    sidebarLayout->addWidget(new QLabel("Reactions:", m_sidebar));
    sidebarLayout->addWidget(m_chatReactions);

    QWidget* reactionButtonsContainer = new QWidget(m_sidebar);
    QHBoxLayout* buttonsLayout = new QHBoxLayout(reactionButtonsContainer);
    buttonsLayout->setContentsMargins(0, 0, 0, 0);

    const QString reactions[] = { "👍", "👎", "😂", "😮", "🔥" };
    for (int i = 0; i < 5; ++i) {
        m_reactionButtons[i] = new QPushButton(reactions[i], reactionButtonsContainer);
        m_reactionButtons[i]->setFixedSize(40, 40);
        m_reactionButtons[i]->setStyleSheet("font-size: 18px;");
        buttonsLayout->addWidget(m_reactionButtons[i]);
    }

    sidebarLayout->addWidget(reactionButtonsContainer);

    m_mainSplitter = new QSplitter(Qt::Horizontal, m_roomTab);
    m_mainSplitter->addWidget(m_videoContainer);
    m_mainSplitter->addWidget(m_sidebar);
    m_mainSplitter->setStretchFactor(0, 3);
    m_mainSplitter->setStretchFactor(1, 1);

    m_playPauseBtn = new QPushButton(m_roomTab);
    m_playPauseBtn->setIcon(m_playIcon);
    m_playPauseBtn->setFixedSize(32, 32);

    m_positionSlider = new QSlider(Qt::Horizontal, m_roomTab);
    m_positionSlider->setRange(0, 100);
    m_positionSlider->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    m_positionSlider->setFixedHeight(40);

    m_positionLabel = new QLabel("00:00", m_roomTab);
    m_positionLabel->setAlignment(Qt::AlignCenter);
    m_positionLabel->setFixedHeight(40);
    m_durationLabel = new QLabel("/ 00:00", m_roomTab);
    m_durationLabel->setAlignment(Qt::AlignCenter);
    m_durationLabel->setFixedHeight(40);

    QWidget* timeContainer = new QWidget(m_roomTab);
    QHBoxLayout* timeLayout = new QHBoxLayout(timeContainer);
    timeLayout->setContentsMargins(0, 0, 0, 0);
    timeLayout->setSpacing(0);
    timeLayout->addWidget(m_positionLabel);
    timeLayout->addWidget(m_durationLabel);

    m_volumeSlider = new QSlider(Qt::Vertical, m_roomTab);
    m_volumeSlider->setRange(0, 100);
    m_volumeSlider->setValue(50);
    m_volumeSlider->setFixedHeight(70);

    m_speedCombo = new QComboBox(m_roomTab);
    m_speedCombo->addItems({ "0.5x", "1.0x", "1.5x", "2.0x" });
    m_speedCombo->setCurrentText("1.0x");
    m_speedCombo->setFixedWidth(65);
    m_speedCombo->setFixedHeight(40);

    m_statusLabel = new QLabel("Disconnected", m_roomTab);
    m_statusLabel->setAlignment(Qt::AlignCenter);
    m_statusLabel->setFixedHeight(40);

    m_volLabel = new QLabel("Vol:", m_roomTab);
    m_volLabel->setFixedHeight(40);
    m_volLabel->setAlignment(Qt::AlignVCenter);

    m_leaveRoomBtn = new QPushButton("Leave Room", m_roomTab);
    m_leaveRoomBtn->setFixedHeight(40);

    auto controlsLayout = new QHBoxLayout();
    controlsLayout->setSpacing(10);
    controlsLayout->setContentsMargins(5, 5, 5, 5);

    controlsLayout->addWidget(m_playPauseBtn);
    controlsLayout->addWidget(m_positionSlider, 5);
    controlsLayout->addWidget(timeContainer);
    controlsLayout->addWidget(m_volLabel);
    controlsLayout->addWidget(m_volumeSlider);
    controlsLayout->addWidget(m_speedCombo);
    controlsLayout->addWidget(m_statusLabel);
    controlsLayout->addWidget(m_leaveRoomBtn);

    roomLayout->addWidget(m_mainSplitter);
    roomLayout->addLayout(controlsLayout);

    m_tabWidget->addTab(m_roomTab, "Room");
    m_tabWidget->setCurrentIndex(0);
    m_speedCombo->hide();
}

QString MainWindow::formatTime(qint64 ms) const
{
    int seconds = static_cast<int>(ms / 1000) % 60;
    int minutes = static_cast<int>(ms / 60000);
    return QString("%1:%2")
        .arg(minutes, 2, 10, QLatin1Char('0'))
        .arg(seconds, 2, 10, QLatin1Char('0'));
}

void MainWindow::setupConnections()
{
    connect(m_playPauseBtn, &QPushButton::clicked, this, &MainWindow::handlePlayPause);
    connect(m_volumeSlider, &QSlider::valueChanged, this, &MainWindow::handleVolumeChange);
    connect(m_speedCombo, &QComboBox::currentTextChanged, this, &MainWindow::handleSpeedChange);

    connect(m_player, &Player::positionChanged, this, [this](qint64 position) {
        if (!m_positionSlider->isSliderDown()) {
            m_positionSlider->setValue(static_cast<int>(position));
            updatePositionDisplay(position);
        }
        });

    connect(m_player, &Player::durationChanged, this, [this](qint64 duration) {
        m_positionSlider->setMaximum(static_cast<int>(duration));
        m_durationLabel->setText("/ " + formatTime(duration));
        });

    connect(m_positionSlider, &QSlider::sliderReleased, this, [this]() {
        bool wasPlaying = m_player->isPlaying();
        m_player->seek(static_cast<qint64>(m_positionSlider->value()));
        if (wasPlaying) {
            QTimer::singleShot(100, this, [this]() {
                m_player->play();
                });
        }
        });

    connect(m_player, &Player::playbackStateChanged, this, &MainWindow::updatePlayerControls);
    connect(m_player, &Player::errorOccurred, this, &MainWindow::showNotification);

    connect(m_client, &Client::connected, [this]() {
        updateConnectionStatus(true);
        showNotification("Connected to server");
        });

    connect(m_client, &Client::disconnected, [this]() {
        updateConnectionStatus(false);
        showNotification("Disconnected from server");
        m_tabWidget->setCurrentIndex(0);
        });

    connect(m_client, &Client::roomCreated, this, &MainWindow::onRoomJoined);
    connect(m_client, &Client::participantsUpdated, this, &MainWindow::onParticipantsUpdated);
    connect(m_client, &Client::videoUrlReceived, this, &MainWindow::handleVideoUrlReceived);
    connect(m_client, &Client::errorOccurred, this, &MainWindow::showNotification);
    connect(m_client, &Client::reactionReceived,
        this, &MainWindow::onReactionReceived);
    connect(m_client, &Client::filmsListReceived, this, &MainWindow::onFilmsListReceived);
    connect(m_client, &Client::playerStateReceived,
        this, &MainWindow::onPlayerStateReceived);

    connect(m_getFilmsBtn, &QPushButton::clicked, [this]() {
        m_client->sendMessage("GET_FILMS");
        });

    connect(m_createRoomBtn, &QPushButton::clicked, this, [this]() {
        if (m_client->currentRoom().isEmpty()) {
            int filmIndex = m_filmNumberEdit->text().toInt();
            m_client->createRoom(filmIndex);
        }
        });

    connect(m_client, &Client::roomCreated, [this]() {
        m_tabWidget->setCurrentIndex(1);
        });

    connect(m_roomIdLabel, &QLabel::linkActivated, this, [this](const QString&) {
        QClipboard* clipboard = QApplication::clipboard();
        clipboard->setText(m_roomIdLabel->text());
        showNotification("Room ID copied to clipboard");
        });

    connect(m_searchEdit, &QLineEdit::textChanged,
        this, &MainWindow::filterMovies);

    connect(m_movieList, &QListWidget::itemDoubleClicked, this, &MainWindow::onMovieDoubleClicked);

    for (int i = 0; i < 5; ++i) {
        connect(m_reactionButtons[i], &QPushButton::clicked,
            this, [this, i]() { handleReaction(m_reactionButtons[i]->text()); });
    }

    connect(m_joinRoomBtn, &QPushButton::clicked, this, [this]() {
        QString roomId = m_roomIdEdit->text().trimmed();
        if (roomId.isEmpty() || roomId == " ") {
            showNotification("Please enter room ID");
            return;
        }

        if (m_client->currentRoom() != roomId) {
            m_tabWidget->setCurrentIndex(1);
            m_client->joinRoom(roomId);
        }
        else {
            showNotification("You are already in this room");
        }
        });

    connect(m_client, &Client::roomCreated, this, [this](const QString& roomId) {
        m_roomIdLabel->setText(roomId);
        });

    connect(m_client, &Client::videoUrlReceived, this, [this](const QUrl& url) {
        if (!m_client->currentRoom().isEmpty()) {
            m_roomIdLabel->setText(m_client->currentRoom());
        }
        });
    connect(m_leaveRoomBtn, &QPushButton::clicked, this, [this]() {
        if (!m_client->currentRoom().isEmpty()) {
            m_client->sendMessage("LEAVE_ROOM");
            m_tabWidget->setCurrentIndex(0);
            m_roomIdLabel->setText("None");
            m_player->stop();
            m_participantsList->clear();
        }
        });
    connect(m_player, &Player::playbackStateChanged, this, [this](QMediaPlayer::PlaybackState state) {
        bool isPlaying = (state == QMediaPlayer::PlayingState);
        updatePlayerControls(isPlaying);
        if (m_syncing) return;
        m_client->sendPlayerState(!isPlaying, m_player->currentPosition());
        m_lastPlayerStateSendTime = QDateTime::currentMSecsSinceEpoch();
        });

    connect(m_positionSlider, &QSlider::sliderReleased, this, [this]() {
        if (m_syncing) return;
        qint64 pos = static_cast<qint64>(m_positionSlider->value());
        m_player->seek(pos);
        m_client->sendPlayerState(!m_player->isPlaying(), pos);
        });

    connect(m_speedCombo, &QComboBox::currentTextChanged, this, [this](const QString& speed) {
        handleSpeedChange(speed);
        if (!m_client->currentRoom().isEmpty()) {
            m_client->sendPlayerState(!m_player->isPlaying(), m_player->currentPosition());
        }
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
        "}"
        "QPushButton {"
        "   padding: 6px 12px;"
        "   min-height: 28px;"
        "   border-radius: 4px;"
        "}"
        "QPushButton:hover { background-color: #4a4a4a; }"
        "QPushButton:pressed { background-color: #2a2a2a; }"
        "QLabel#roomIdLabel {"
        "   background-color: #3c3c3c;"
        "   border: 1px solid #555;"
        "   border-radius: 4px;"
        "   padding: 2px 5px;"
        "}"
        "QLabel#roomIdLabel:hover {"
        "   background-color: #4a4a4a;"
        "}"
        "QLineEdit, QComboBox {"
        "   padding: 5px 8px;"
        "   min-height: 28px;"
        "   background: #333;"
        "   border-radius: 4px;"
        "}"
        "QComboBox::drop-down { width: 20px; }"
        "QLineEdit[maximumWidth=\"40\"] {"
        "   padding: 5px 2px;"
        "   text-align: center;"
        "}"
        "QLineEdit[maximumWidth=\"80\"] {"
        "   padding: 5px 4px;"
        "   text-align: center;"
        "}"
        "QLineEdit[placeholderText=\"Type to filter movies...\"] {"
        "   max-width: 300px;"
        "   background: #333;"
        "   border: 1px solid #555;"
        "   border-radius: 4px;"
        "   padding: 5px;"
        "}"
        "QListWidget {"
        "   padding: 3px;"
        "   background: #333;"
        "   border-radius: 4px;"
        "}"
        "QSlider::groove:horizontal {"
        "   height: 6px;"
        "   background: #444;"
        "   border-radius: 3px;"
        "}"
        "QSlider::handle:horizontal {"
        "   width: 16px;"
        "   margin: -5px 0;"
        "   background: #666;"
        "   border-radius: 8px;"
        "}"
        "QSlider::groove:vertical {"
        "   width: 6px;"
        "   background: #444;"
        "   border-radius: 3px;"
        "}"
        "QSlider::handle:vertical {"
        "   height: 16px;"
        "   margin: 0 -5px;"
        "   background: #666;"
        "   border-radius: 8px;"
        "}"
        "QWidget#timeContainer {"
        "   background: transparent;"
        "   border: 1px solid #555;"
        "   border-radius: 4px;"
        "   min-width: 100px;"
        "}"
        "QLabel {"
        "   padding: 0 5px;"
        "}"
        "QTabWidget::pane { border: 0; }"
        "QTabBar::tab { "
        "   padding: 8px 16px;"
        "   background: #3c3c3c;"
        "   color: white;"
        "   border: 1px solid #555;"
        "   border-bottom: none;"
        "   border-top-left-radius: 4px;"
        "   border-top-right-radius: 4px;"
        "}"
        "QTabBar::tab:selected { "
        "   background: #555;"
        "   border-color: #777;"
        "}"
        "QLabel#statusLabel {"
        "   padding: 2px 6px;"
        "   border-radius: 3px;"
        "}"
    );

    m_statusLabel->setObjectName("statusLabel");
    m_roomIdLabel->setObjectName("roomIdLabel");
}

void MainWindow::handlePlayPause()
{
    if (m_player->isPlaying()) {
        m_player->pause();
        m_client->sendPlayerState(true, m_player->currentPosition());
    }
    else {
        m_player->play();
        m_client->sendPlayerState(false, m_player->currentPosition());
    }
}

void MainWindow::filterMovies(const QString& text)
{
    for (int i = 0; i < m_movieList->count(); ++i) {
        QListWidgetItem* item = m_movieList->item(i);
        bool match = item->text().contains(text, Qt::CaseInsensitive);
        item->setHidden(!match);
    }
}

void MainWindow::handleReaction(const QString& reaction) {
    m_client->sendReaction(reaction);
}

void MainWindow::onReactionReceived(const QString& user, const QString& reaction)
{
    QString avatarPath = m_client->getAvatarPath(user);

    QListWidgetItem* item = new QListWidgetItem(
        QIcon(avatarPath),
        QString("%1 - %2").arg(reaction, user)
    );

    item->setSizeHint(QSize(-1, 50));
    m_chatReactions->addItem(item);
    m_chatReactions->scrollToBottom();

    m_notificationManager->showNotification(
        QString("%1: %2").arg(user, reaction),
        NotificationManager::Reaction,
        2000
    );
}

bool MainWindow::eventFilter(QObject* watched, QEvent* event)
{
    if (watched == m_roomIdLabel && event->type() == QEvent::MouseButtonPress) {
        QMouseEvent* mouseEvent = static_cast<QMouseEvent*>(event);
        if (mouseEvent->button() == Qt::LeftButton) {
            copyRoomId();
            return true;
        }
    }
    return QMainWindow::eventFilter(watched, event);
}

void MainWindow::copyRoomId()
{
    QClipboard* clipboard = QApplication::clipboard();
    clipboard->setText(m_roomIdLabel->text());
    showNotification("Room ID copied to clipboard");
}

void MainWindow::onMovieDoubleClicked(QListWidgetItem* item)
{
    int row = m_movieList->row(item);
    m_filmNumberEdit->setText(QString::number(row + 1));
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
    QFile file("films.txt");
    if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QTextStream out(&file);
        for (const QString& film : films) {
            out << film << "\n";
        }
        file.close();
    }

    m_movieList->clear();
    m_movieList->addItems(films);
    m_searchEdit->clear();
}

void MainWindow::onRoomJoined(const QString& roomId)
{
    showNotification("Joined room: " + roomId);
    m_roomIdEdit->clear();
}

void MainWindow::onParticipantsUpdated(const QStringList& users) {
    m_participantsList->clear();
    const int iconSize = 64;
    m_participantsList->setIconSize(QSize(iconSize, iconSize));

    for (const QString& user : users) {
        QStringList parts = user.split('|');
        if (parts.size() < 2) continue;

        QString nickname = parts[0];
        QString avatarPath = parts[1];

        QString displayName = nickname;
        if (nickname == m_client->userNickname()) {
            displayName += " (you)";
        }

        QListWidgetItem* item = new QListWidgetItem;
        item->setText(displayName);

        QPixmap pixmap(avatarPath);
        if (pixmap.isNull()) {
            pixmap.load(":/images/default_avatar.png");
        }

        if (!pixmap.isNull()) {
            pixmap = pixmap.scaled(iconSize, iconSize, Qt::KeepAspectRatio, Qt::SmoothTransformation);
            item->setIcon(QIcon(pixmap));
            item->setSizeHint(QSize(-1, iconSize + 10));
        }

        m_participantsList->addItem(item);
    }
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
    m_player->pause();
    updatePlayerControls(true);
    showNotification("Видео загружено: " + url.toString());
}

void MainWindow::onPlayerStateReceived(const QString& roomId,
    const QString& senderNick,
    bool isPaused,
    qint64 position)
{
    Q_UNUSED(roomId);
    Q_UNUSED(senderNick);

    if (m_player->isPlaying() == isPaused) {
        if (isPaused) {
            m_player->pause();
        }
        else {
            m_player->play();
        }
    }

    if (qAbs(m_player->currentPosition() - position) > 500) {
        m_player->seek(position);
    }
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