#include "VideoRenderer.h"
#include <QMutexLocker>
#include <QDebug>

VideoRenderer::VideoRenderer(QObject* parent)
    : QObject(parent),
    m_processingThread(nullptr),
    m_videoSink(nullptr),
    m_isRunning(false),
    m_isPaused(false),
    m_playbackRate(1.0f),
    m_lastAudioPosition(0),
    m_baseTimestamp(0),
    m_width(0),
    m_height(0)
{
    m_processingThread = QThread::create([this]() {
        while (m_isRunning) {
            processFrames();
        }
        });
}

VideoRenderer::~VideoRenderer()
{
    stop();
    if (m_processingThread) {
        m_processingThread->quit();
        m_processingThread->wait();
        delete m_processingThread;
    }
}

bool VideoRenderer::initialize(QVideoSink* sink, int width, int height)
{
    if (!sink) {
        emit errorOccurred("VideoSink is null.");
        return false;
    }
    m_videoSink = sink;
    m_width = width;
    m_height = height;
    return true;
}

void VideoRenderer::start()
{
    if (m_isRunning) return;
    m_isRunning = true;
    m_isPaused = false;
    m_renderTimer.restart();
    if (m_processingThread && !m_processingThread->isRunning()) {
        m_processingThread->start();
    }
}

void VideoRenderer::stop()
{
    m_isRunning = false;
    m_frameAvailable.wakeAll();
}

void VideoRenderer::pause(bool paused)
{
    m_isPaused = paused;
}

void VideoRenderer::enqueueFrame(const QVideoFrame& frame)
{
    QMutexLocker locker(&m_queueMutex);
    m_frameQueue.enqueue(frame);
    m_frameAvailable.wakeOne();
}

void VideoRenderer::enqueueEncodedData(const QByteArray& data)
{
    QVideoFrame frame = decodeFrame(data);
    if (frame.isValid()) {
        enqueueFrame(frame);
    }
    else {
        emit errorOccurred("Failed to decode video frame.");
    }
}

void VideoRenderer::setPlaybackRate(float rate)
{
    m_playbackRate = rate;
}

void VideoRenderer::seek(qint64 positionMs)
{
    m_lastAudioPosition = positionMs;
    m_baseTimestamp = positionMs;
    m_renderTimer.restart();
    emit positionChanged(positionMs);
}

bool VideoRenderer::isPlaying() const
{
    return m_isRunning && !m_isPaused;
}

qint64 VideoRenderer::currentPosition() const
{
    return m_baseTimestamp + static_cast<qint64>(m_renderTimer.elapsed() * m_playbackRate);
}

int VideoRenderer::bufferedFrames() const
{
    QMutexLocker locker(&m_queueMutex);
    return m_frameQueue.size();
}

void VideoRenderer::handleAudioPositionChanged(qint64 position)
{
    m_lastAudioPosition = position;
}

void VideoRenderer::processFrames()
{
    QMutexLocker locker(&m_queueMutex);
    if (m_isPaused || m_frameQueue.isEmpty()) {
        m_frameAvailable.wait(&m_queueMutex, 10);
        return;
    }

    QVideoFrame frame = m_frameQueue.dequeue();
    locker.unlock();

    if (m_videoSink) {
        m_videoSink->setVideoFrame(frame);
        emit frameProcessed(frame);
    }

    emit positionChanged(currentPosition());
}

QVideoFrame VideoRenderer::decodeFrame(const QByteArray& data)
{
    Q_UNUSED(data);
    return QVideoFrame();
}

void VideoRenderer::adjustPresentationTime(QVideoFrame& frame)
{
    Q_UNUSED(frame);
}

void VideoRenderer::dropLateFrames()
{
    QMutexLocker locker(&m_queueMutex);
    while (m_frameQueue.size() > 30) {
        m_frameQueue.dequeue();
    }
}