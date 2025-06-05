#ifndef PLAYER_H
#define PLAYER_H

#include <QObject>
#include <QMediaPlayer>
#include <QAudioOutput>
#include <QVideoWidget>
#include <QTimer>
#include <QUrl>
#include "VideoRenderer.h"
#include "SyncHandler.h"

class Player : public QObject
{
    Q_OBJECT
public:
    explicit Player(QWidget* parentWidget = nullptr, QObject* parent = nullptr);
    ~Player();

    void play();
    void pause();
    void stop();
    void seek(qint64 positionMs);

    void setPlaybackRate(float rate);
    void setVolume(int volume);
    void setVideoOutput(QWidget* container);
    void setMedia(const QUrl& url);

    bool isPlaying() const;
    qint64 currentPosition() const;
    qint64 duration() const;

signals:
    void positionChanged(qint64 position);
    void playbackStateChanged(QMediaPlayer::PlaybackState state);
    void errorOccurred(const QString& error);

public slots:
    void updateSyncPosition(qint64 serverPosition);
    void handleVideoFrame(const QVideoFrame& frame);

private slots:
    void onMediaStatusChanged(QMediaPlayer::MediaStatus status);
    void onErrorOccurred(QMediaPlayer::Error error, const QString& errorString);
    void updatePosition();

private:
    void initializePlayer();
    void setupVideoRenderer();
    void applySyncCorrection(qint64 serverTime);

    QMediaPlayer* m_mediaPlayer;
    QAudioOutput* m_audioOutput;
    QVideoWidget* m_videoWidget;
    VideoRenderer* m_videoRenderer;
    SyncHandler* m_syncHandler;
    QTimer* m_positionTimer;

    qint64 m_lastSyncPosition;
    float m_playbackRate;
    bool m_externalSync;
};

#endif