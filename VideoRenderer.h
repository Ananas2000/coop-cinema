#ifndef VIDEORENDERER_H
#define VIDEORENDERER_H

#include <QObject>
#include <QVideoSink>
#include <QVideoFrame>
#include <QQueue>
#include <QMutex>
#include <QWaitCondition>
#include <QElapsedTimer>
#include <QThread>

class VideoRenderer : public QObject
{
    Q_OBJECT
public:
    explicit VideoRenderer(QObject* parent = nullptr);
    ~VideoRenderer();

    bool initialize(QVideoSink* sink, int width, int height);
    void start();
    void stop();
    void pause(bool paused);
    void enqueueFrame(const QVideoFrame& frame);
    void enqueueEncodedData(const QByteArray& data);
    void setPlaybackRate(float rate);
    void setFrameDuration(qint64 durationMs); // Добавленный метод
    void seek(qint64 positionMs);
    bool isPlaying() const;
    qint64 currentPosition() const;
    int bufferedFrames() const;

signals:
    void frameProcessed(const QVideoFrame& frame);
    void errorOccurred(const QString& error);
    void positionChanged(qint64 position);

public slots:
    void handleAudioPositionChanged(qint64 position);

private slots:
    void processFrames();

private:
    QVideoFrame decodeFrame(const QByteArray& data);
    void adjustPresentationTime(QVideoFrame& frame);
    void dropLateFrames();

    bool m_initialized = false;
    void initializeFFmpeg();

    QVideoSink* m_videoSink = nullptr;
    QThread* m_processingThread = nullptr;
    QQueue<QVideoFrame> m_frameQueue;
    mutable QMutex m_queueMutex;
    QWaitCondition m_frameAvailable;
    QElapsedTimer m_renderTimer;

    bool m_isRunning = false;
    bool m_isPaused = false;
    float m_playbackRate = 1.0f;
    qint64 m_lastAudioPosition = 0;
    qint64 m_baseTimestamp = 0;
    qint64 m_frameDuration = 40;
    int m_width = 0;
    int m_height = 0;
};

#endif