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
#include <msxml.h>

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
    menuLayout->setSpacing(10);

    // Get Films button
    m_getFilmsBtn = new QPushButton("Get Films", m_menuTab);
    menuLayout->addWidget(m_getFilmsBtn, 0, Qt::AlignLeft);

    // Создаем горизонтальный лейаут для поиска
    auto searchLayout = new QHBoxLayout();

    m_searchEdit = new QLineEdit(m_menuTab);
    m_searchEdit->setPlaceholderText("Search...");
    m_searchEdit->setClearButtonEnabled(true);
    m_searchEdit->setMaximumWidth(1000);
    searchLayout->addWidget(m_searchEdit, 1000, Qt::AlignLeft);

    menuLayout->addLayout(searchLayout);

    // Movie list
    m_movieList = new QListWidget(m_menuTab);
    m_movieList->setMinimumHeight(300);
    m_movieList->setMaximumWidth(1500);
    menuLayout->addWidget(m_movieList, 1);

    // Film selection - компактная группа
    auto filmSelectionGroup = new QWidget(m_menuTab);
    auto filmLayout = new QHBoxLayout(filmSelectionGroup);
    filmLayout->setContentsMargins(0, 0, 0, 0);
    filmLayout->setSpacing(5);

    // Метка и поле для номера фильма
    auto filmLabel = new QLabel("Film number:", filmSelectionGroup);
    filmLabel->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
    filmLayout->addWidget(filmLabel);

    m_filmNumberEdit = new QLineEdit(filmSelectionGroup);
    m_filmNumberEdit->setAlignment(Qt::AlignCenter);
    m_filmNumberEdit->setFixedWidth(40); // Фиксированная ширина
    m_filmNumberEdit->setValidator(new QIntValidator(1, 999, this));
    filmLayout->addWidget(m_filmNumberEdit);

    // Кнопка создания
    m_createRoomBtn = new QPushButton("Create", filmSelectionGroup);
    m_createRoomBtn->setFixedWidth(80); // Фиксированная ширина
    filmLayout->addWidget(m_createRoomBtn);

    menuLayout->addWidget(filmSelectionGroup, 0, Qt::AlignLeft);

    // Join room section - компактная группа
    auto joinGroup = new QWidget(m_menuTab);
    auto joinLayout = new QHBoxLayout(joinGroup);
    joinLayout->setContentsMargins(0, 0, 0, 0);
    joinLayout->setSpacing(5);

    // Метка и поле для ID комнаты
    auto roomLabel = new QLabel("Room ID:", joinGroup);
    roomLabel->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
    joinLayout->addWidget(roomLabel);

    m_roomIdEdit = new QLineEdit(joinGroup);
    m_roomIdEdit->setAlignment(Qt::AlignCenter);
    m_roomIdEdit->setFixedWidth(80); // Фиксированная ширина
    m_roomIdEdit->setPlaceholderText("ID");
    joinLayout->addWidget(m_roomIdEdit);

    // Кнопка входа
    m_joinRoomBtn = new QPushButton("Join", joinGroup);
    m_joinRoomBtn->setFixedWidth(80); // Фиксированная ширина
    joinLayout->addWidget(m_joinRoomBtn);

    menuLayout->addWidget(joinGroup, 0, Qt::AlignLeft);
    m_tabWidget->addTab(m_menuTab, "Menu");

    // ======================
    // Room Tab
    // ======================
    m_roomTab = new QWidget(this);
    auto roomLayout = new QVBoxLayout(m_roomTab);
    roomLayout->setSpacing(10);

    // Room ID panel
    auto roomIdPanel = new QWidget(m_roomTab);
    auto roomIdLayout = new QHBoxLayout(roomIdPanel);
    roomIdLayout->setContentsMargins(0, 0, 0, 0);
    roomIdLayout->setSpacing(5);

    roomIdLayout->addWidget(new QLabel("Room ID:", roomIdPanel));
    m_roomIdLabel = new QLabel("None", roomIdPanel);
    roomIdLayout->addWidget(m_roomIdLabel);
    roomIdLayout->addStretch();

    roomLayout->addWidget(roomIdPanel);

    // Video container
    m_videoContainer = new QVideoWidget(m_roomTab);
    m_videoContainer->setMinimumSize(640, 360);
    m_player->setVideoOutput(m_videoContainer);

    // Sidebar
    m_sidebar = new QWidget(m_roomTab);
    auto sidebarLayout = new QVBoxLayout(m_sidebar);
    sidebarLayout->setSpacing(5);

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

    m_positionSlider = new QSlider(Qt::Horizontal, m_roomTab);
    m_positionSlider->setRange(0, 100);
    m_positionSlider->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);

    m_durationLabel = new QLabel("/ 00:00", m_roomTab);

    auto controlsLayout = new QHBoxLayout();
    controlsLayout->setSpacing(10);
    controlsLayout->addWidget(m_playPauseBtn);
    controlsLayout->addWidget(new QLabel("Volume:", m_roomTab));
    controlsLayout->addWidget(m_volumeSlider);
    controlsLayout->addWidget(new QLabel("Speed:", m_roomTab));
    controlsLayout->addWidget(m_speedCombo);
    controlsLayout->addStretch();
    controlsLayout->addWidget(m_positionLabel);
    controlsLayout->addWidget(m_statusLabel);
    controlsLayout->addWidget(m_positionSlider);
    controlsLayout->addWidget(m_durationLabel);

    // Add to room layout
    roomLayout->addWidget(m_mainSplitter);
    roomLayout->addLayout(controlsLayout);

    m_leaveRoomBtn = new QPushButton("Leave Room", m_roomTab);
    controlsLayout->addWidget(m_leaveRoomBtn);

    m_tabWidget->addTab(m_roomTab, "Room");

    // Start with Menu tab
    m_tabWidget->setCurrentIndex(0);
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
    // Player controls
    connect(m_playPauseBtn, &QPushButton::clicked, this, &MainWindow::handlePlayPause);
    connect(m_volumeSlider, &QSlider::valueChanged, this, &MainWindow::handleVolumeChange);
    connect(m_speedCombo, &QComboBox::currentTextChanged, this, &MainWindow::handleSpeedChange);

    // Обновление позиции
    connect(m_player, &Player::positionChanged, this, [this](qint64 position) {
        if (!m_positionSlider->isSliderDown()) {
            m_positionSlider->setValue(static_cast<int>(position));
            updatePositionDisplay(position);
        }
        if (m_syncing) return;
        qint64 now = QDateTime::currentMSecsSinceEpoch();
        if (m_player->isPlaying() && (now - m_lastPlayerStateSendTime) >= 1000) {
            m_client->sendPlayerState(!m_player->isPlaying(), position);
            m_lastPlayerStateSendTime = now;
        }
        });
   
    // Обновление длительности
    connect(m_player, &Player::durationChanged, this, [this](qint64 duration) {
        m_positionSlider->setMaximum(static_cast<int>(duration));
        m_durationLabel->setText("/ " + formatTime(duration));
        });

    // Обработка перемотки пользователем
    connect(m_positionSlider, &QSlider::sliderReleased, this, [this]() {
        bool wasPlaying = m_player->isPlaying(); // Сохраняем состояние
        m_player->seek(static_cast<qint64>(m_positionSlider->value()));

        // Восстанавливаем воспроизведение если было запущено
        if (wasPlaying) {
            QTimer::singleShot(100, this, [this]() {
                m_player->play();
                });
        }
        });

    // Player signals
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
    connect(m_client, &Client::playerStateReceived,
        this, &MainWindow::onPlayerStateReceived);

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

    /*
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
    */

    // Automatically switch to room tab when room is created
    connect(m_client, &Client::roomCreated, [this]() {
        m_tabWidget->setCurrentIndex(1);
        });

    connect(m_searchEdit, &QLineEdit::textChanged,
        this, &MainWindow::filterMovies);

    connect(m_movieList, &QListWidget::itemDoubleClicked, this, &MainWindow::onMovieDoubleClicked);


    connect(m_joinRoomBtn, &QPushButton::clicked, this, [this]() {
        QString roomId = m_roomIdEdit->text().trimmed();
        if (roomId.isEmpty() || roomId == " ") {
            showNotification("Please enter room ID");
            return;
        }

        if (m_client->currentRoom() != roomId) {
            m_tabWidget->setCurrentIndex(1);
            m_client->joinRoom(roomId); // Только отправка запроса
        }
        else {
            showNotification("You are already in this room");
        }
        });

    // Обновляем ID комнаты при успешном создании/присоединении
    connect(m_client, &Client::roomCreated, this, [this](const QString& roomId) {
        m_roomIdLabel->setText(roomId);
        });

    connect(m_client, &Client::videoUrlReceived, this, [this](const QUrl& url) {
        // Обновляем ID комнаты при получении видео
        if (!m_client->currentRoom().isEmpty()) {
            m_roomIdLabel->setText(m_client->currentRoom());
        }
        });
    connect(m_leaveRoomBtn, &QPushButton::clicked, this, [this]() {
        if (!m_client->currentRoom().isEmpty()) {
            // Отправляем команду на выход
            m_client->sendMessage("LEAVE_ROOM");

            // Очищаем интерфейс
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

        // Исправленный вызов
        m_client->sendPlayerState(!isPlaying, m_player->currentPosition());
        m_lastPlayerStateSendTime = QDateTime::currentMSecsSinceEpoch();
        });

    connect(m_player, &Player::positionChanged, this, [this](qint64 position) {
        // Удален дублирующий вызов updatePositionDisplay
        if (m_syncing) return;

        qint64 now = QDateTime::currentMSecsSinceEpoch();
        if (m_player->isPlaying() && (now - m_lastPlayerStateSendTime) >= 1000) {
            m_client->sendPlayerState(!m_player->isPlaying(), position);
            m_lastPlayerStateSendTime = now;
        }
        });

    connect(m_positionSlider, &QSlider::sliderReleased, this, [this]() {
        if (m_syncing) return;
        qint64 pos = static_cast<qint64>(m_positionSlider->value());
        m_player->seek(pos);

        // Исправленный вызов
        m_client->sendPlayerState(!m_player->isPlaying(), pos);
        });
}

void MainWindow::applyStyleSheet()
{
    setStyleSheet(
        "QMainWindow, QWidget { background-color: #2b2b2b; color: white; }"

        // Общие стили для текстовых элементов
        "QLabel, QListWidget, QPushButton, QComboBox, QLineEdit { "
        "   font-size: 14px; "
        "   background-color: #3c3c3c; "
        "   color: white; "
        "   border: 1px solid #555; "
        "}"

        // Стили для кнопок
        "QPushButton {"
        "   padding: 6px 12px;"
        "   min-height: 28px;"
        "   border-radius: 4px;"
        "}"
        "QPushButton:hover { background-color: #4a4a4a; }"
        "QPushButton:pressed { background-color: #2a2a2a; }"

        // Стили для полей ввода
        "QLineEdit, QComboBox {"
        "   padding: 5px 8px;"
        "   min-height: 28px;"
        "   background: #333;"
        "   border-radius: 4px;"
        "}"
        "QComboBox::drop-down { width: 20px; }"
        "QLineEdit[maximumWidth=\"40\"] {"
        "   padding: 5px 2px;"  // Уменьшаем горизонтальные отступы
        "   text-align: center;"
        "}"
        "QLineEdit[maximumWidth=\"80\"] {"
        "   padding: 5px 4px;"  // Уменьшаем горизонтальные отступы
        "   text-align: center;"
        "}"

        "QLineEdit[placeholderText=\"Type to filter movies...\"] {"
        "   max-width: 300px;" // Ограничиваем ширину
        "   background: #333;"
        "   border: 1px solid #555;"
        "   border-radius: 4px;"
        "   padding: 5px;"
        "}"

        // Стили для списков
        "QListWidget {"
        "   padding: 3px;"
        "   background: #333;"
        "   border-radius: 4px;"
        "}"

        // Стили для слайдеров
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

        // Стили для вкладок
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

        // Статусная метка
        "QLabel#statusLabel {"
        "   padding: 2px 6px;"
        "   border-radius: 3px;"
        "}"
    );

    // Применяем дополнительные стили к статусной метке
    m_statusLabel->setObjectName("statusLabel");
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
    m_searchEdit->clear();
}

void MainWindow::onRoomJoined(const QString& roomId)
{
    showNotification("Joined room: " + roomId);
    m_roomIdEdit->clear();
}

/*
void MainWindow::onParticipantsUpdated(const QStringList& users)
{
    m_participantsList->clear();
    m_participantsList->addItems(users);
}
*/

void MainWindow::onParticipantsUpdated(const QStringList& users) {
    m_participantsList->clear();

    // Установите желаемый размер иконок для списка
    const int iconSize = 64; // Увеличьте это значение для больших аватарок
    m_participantsList->setIconSize(QSize(iconSize, iconSize));

    for (const QString& user : users) {
        QStringList parts = user.split('|');
        if (parts.size() < 2) continue;

        QString nickname = parts[0];
        QString avatarPath = parts[1];

        QListWidgetItem* item = new QListWidgetItem;
        item->setText(nickname);

        // Загрузка и масштабирование аватарки
        QPixmap pixmap;
        if (QFile::exists(avatarPath)) {
            pixmap.load(avatarPath);
        }
        else {
            pixmap.load(":/avatars/0.png");
        }

        // Масштабируем изображение до нужного размера
        if (!pixmap.isNull()) {
            // Увеличьте размер здесь (например, 64x64)
            pixmap = pixmap.scaled(iconSize, iconSize,
                Qt::KeepAspectRatio,
                Qt::SmoothTransformation);
            item->setIcon(QIcon(pixmap));

            // Установите размер элемента списка
            item->setSizeHint(QSize(-1, iconSize + 10)); // +10 для текста
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
    m_player->play();

    updatePlayerControls(true);
    showNotification("Видео загружено: " + url.toString());
}

void MainWindow::onPlayerStateReceived(const QString& roomId,
    const QString& senderNick,
    bool isPaused,
    qint64 position)
{
    Q_UNUSED(roomId);

    // Всегда применяем состояние независимо от отправителя
    m_syncing = true;

    // Синхронизация состояния паузы
    if (m_player->isPlaying() != !isPaused) {
        if (isPaused) {
            m_player->pause();
        }
        else {
            m_player->play();
        }
    }

    // Синхронизация позиции
    m_player->seek(position);

    m_syncing = false;
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