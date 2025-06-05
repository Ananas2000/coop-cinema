#include "Player.h"
#include <QVBoxLayout>
#include <QNetworkRequest>
#include <QDebug>

Player::Player(QWidget* parentWidget, QObject* parent)
    : QObject(parent),
    m_mediaPlayer(new QMediaPlayer(this)),
    m_audioOutput(new QAudioOutput(this)),
    m_videoWidget(new QVideoWidget(parentWidget)),
    m_videoRenderer(new VideoRenderer(this)),
    m_syncHandler(new SyncHandler(this)),
    m_positionTimer(new QTimer(this)),
    m_lastSyncPosition(0),
    m_playbackRate(1.0f),
    m_externalSync(false)
{
    initializePlayer();
    setupVideoRenderer();

    m_videoWidget->setMinimumSize(640, 360);
    m_videoWidget->show();

    connect(m_positionTimer, &QTimer::timeout, this, &Player::updatePosition);
    connect(m_syncHandler, &SyncHandler::syncCorrection, this, &Player::updateSyncPosition);
    connect(m_videoRenderer, &VideoRenderer::frameProcessed, this, &Player::handleVideoFrame);

    m_positionTimer->start(500);
}

Player::~Player()
{
    stop();
    delete m_videoWidget;
}

void Player::initializePlayer()
{
    m_mediaPlayer->setAudioOutput(m_audioOutput);
    m_mediaPlayer->setVideoOutput(m_videoWidget);
    connect(m_mediaPlayer, &QMediaPlayer::mediaStatusChanged,
        this, &Player::onMediaStatusChanged);
    connect(m_mediaPlayer, &QMediaPlayer::errorOccurred,
        this, &Player::onErrorOccurred);
    connect(m_mediaPlayer, &QMediaPlayer::positionChanged,
        this, &Player::positionChanged);
    connect(m_mediaPlayer, &QMediaPlayer::playbackStateChanged,
        this, &Player::playbackStateChanged);
}

void Player::setupVideoRenderer()
{
    m_videoRenderer->initialize(nullptr, 640, 360);
    m_videoRenderer->start();
}

void Player::play()
{
    m_mediaPlayer->play();
    m_videoRenderer->pause(false);
}

void Player::pause()
{
    m_mediaPlayer->pause();
    m_videoRenderer->pause(true);
}

void Player::stop()
{
    m_mediaPlayer->stop();
    m_videoRenderer->stop();
    m_positionTimer->stop();
}

void Player::seek(qint64 positionMs)
{
    m_mediaPlayer->setPosition(positionMs);
    m_videoRenderer->seek(positionMs);
}

void Player::setPlaybackRate(float rate)
{
    m_playbackRate = rate;
    m_mediaPlayer->setPlaybackRate(rate);
    m_videoRenderer->setPlaybackRate(rate);
}

void Player::setVolume(int volume)
{
    const float normalizedVolume = qBound(0, volume, 100) / 100.0f;
    m_audioOutput->setVolume(normalizedVolume);
}

void Player::setVideoOutput(QWidget* container) {
    if (container) {
        auto layout = new QVBoxLayout(container);
        layout->addWidget(m_videoWidget);
        m_videoRenderer->initialize(m_videoWidget->videoSink(), 640, 360); // Инициализируем
    }
}

void Player::setMedia(const QUrl& url) {
    if (url.isValid()) {
        m_mediaPlayer->setSource(url);
        m_videoRenderer->initialize(m_videoWidget->videoSink(), 640, 360);
    }
}

bool Player::isPlaying() const
{
    return m_mediaPlayer->playbackState() == QMediaPlayer::PlayingState;
}

qint64 Player::currentPosition() const
{
    return m_mediaPlayer->position();
}

qint64 Player::duration() const
{
    return m_mediaPlayer->duration();
}

void Player::updateSyncPosition(qint64 serverPosition)
{
    m_lastSyncPosition = serverPosition;
    m_externalSync = true;
    applySyncCorrection(serverPosition);
}

void Player::handleVideoFrame(const QVideoFrame& frame)
{
    Q_UNUSED(frame);
    // Реализация обработки кадра при необходимости
}

void Player::onMediaStatusChanged(QMediaPlayer::MediaStatus status)
{
    if (status == QMediaPlayer::EndOfMedia) {
        emit playbackStateChanged(QMediaPlayer::StoppedState);
    }
}

void Player::onErrorOccurred(QMediaPlayer::Error error, const QString& errorString)
{
    Q_UNUSED(error);
    emit errorOccurred(errorString);
}

void Player::updatePosition()
{
    emit positionChanged(currentPosition());
}

void Player::applySyncCorrection(qint64 serverTime)
{
    qint64 localTime = currentPosition();
    qint64 delta = serverTime - localTime;

    if (std::abs(delta) > 200) {
        seek(serverTime);
    }
}