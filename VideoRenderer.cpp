#include "VideoRenderer.h"
#include <QMutexLocker>
#include <QDebug>
#include <QThread>

#define AV_CODEC_STATIC
#define AVFORMAT_STATIC
#define AVUTIL_STATIC
#define SWSCALE_STATIC
#define SWRESAMPLE_STATIC
#define AVFILTER_STATIC
#define POSTPROC_STATIC
#define AVDEVICE_STATIC

extern "C" {
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libswscale/swscale.h>
#include <libavutil/avutil.h>
#include <libavutil/imgutils.h>
#include <libavfilter/avfilter.h>
#include <libpostproc/postprocess.h>
#include <libavdevice/avdevice.h>
}

#pragma comment(lib, "avcodec.lib")
#pragma comment(lib, "avformat.lib")
#pragma comment(lib, "avutil.lib")
#pragma comment(lib, "swscale.lib")
#pragma comment(lib, "swresample.lib")
#pragma comment(lib, "avfilter.lib")
#pragma comment(lib, "postproc.lib")
#pragma comment(lib, "avdevice.lib")

VideoRenderer::VideoRenderer(QObject* parent)
    : QObject(parent),
    m_processingThread(nullptr),
    m_videoSink(nullptr),
    m_isRunning(false),
    m_isPaused(false),
    m_playbackRate(1.0f),
    m_lastAudioPosition(0),
    m_baseTimestamp(0),
    m_frameDuration(40),
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

    qint64 presentationTime = m_lastAudioPosition + m_frameQueue.size() * m_frameDuration;

    QVideoFrame timedFrame(frame);
    timedFrame.setStartTime(presentationTime);

    m_frameQueue.enqueue(timedFrame);
    m_frameAvailable.wakeOne();

    if (m_frameQueue.size() > 30) {
        m_frameQueue.dequeue();
    }
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

void VideoRenderer::setFrameDuration(qint64 durationMs)
{
    m_frameDuration = qMax(1LL, durationMs);
}

void VideoRenderer::seek(qint64 positionMs)
{
    QMutexLocker locker(&m_queueMutex);
    m_lastAudioPosition = positionMs;
    m_baseTimestamp = positionMs;
    m_frameQueue.clear(); // Очищаем очередь при перемотке
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

    // Корректируем время отображения кадра
    adjustPresentationTime(frame);

    if (m_videoSink && frame.isValid()) {
        m_videoSink->setVideoFrame(frame);
        emit frameProcessed(frame);
    }

    emit positionChanged(currentPosition());
}

QVideoFrame VideoRenderer::decodeFrame(const QByteArray& data)
{
    static bool ffmpegInitialized = []() {
        avformat_network_init();
        return true;
        }();
    Q_UNUSED(ffmpegInitialized);

    static const AVCodec* codec = nullptr;
    static AVCodecContext* codecContext = nullptr;
    static AVFrame* avFrame = nullptr;
    static AVFrame* swsFrame = nullptr;
    static struct SwsContext* swsContext = nullptr;

    if (!codec) {
        codec = avcodec_find_decoder(AV_CODEC_ID_H264);
        if (!codec) {
            emit errorOccurred("H.264 decoder not found");
            return QVideoFrame();
        }

        codecContext = avcodec_alloc_context3(codec);
        if (!codecContext) {
            emit errorOccurred("Could not allocate codec context");
            return QVideoFrame();
        }

        if (avcodec_open2(codecContext, codec, nullptr) < 0) {
            emit errorOccurred("Could not open codec");
            return QVideoFrame();
        }

        avFrame = av_frame_alloc();
        swsFrame = av_frame_alloc();
        if (!avFrame || !swsFrame) {
            emit errorOccurred("Could not allocate frames");
            return QVideoFrame();
        }
    }

    AVPacket* packet = av_packet_alloc();
    if (!packet) {
        emit errorOccurred("Could not allocate packet");
        return QVideoFrame();
    }

    packet->data = reinterpret_cast<uint8_t*>(const_cast<char*>(data.constData()));
    packet->size = data.size();

    int ret = avcodec_send_packet(codecContext, packet);
    av_packet_free(&packet);

    if (ret < 0) {
        emit errorOccurred("Error sending packet to decoder");
        return QVideoFrame();
    }

    ret = avcodec_receive_frame(codecContext, avFrame);
    if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF) {
        return QVideoFrame();
    }
    else if (ret < 0) {
        emit errorOccurred("Error during decoding");
        return QVideoFrame();
    }

    if (!swsContext || avFrame->width != m_width || avFrame->height != m_height) {
        if (swsContext) {
            sws_freeContext(swsContext);
            swsContext = nullptr;
        }

        if (swsFrame->data[0]) {
            av_frame_unref(swsFrame);
        }

        swsContext = sws_getContext(
            avFrame->width,
            avFrame->height,
            static_cast<AVPixelFormat>(avFrame->format),
            m_width,
            m_height,
            AV_PIX_FMT_RGB32,
            SWS_BILINEAR,
            nullptr, nullptr, nullptr
        );

        if (!swsContext) {
            emit errorOccurred("Could not create scaling context");
            return QVideoFrame();
        }

        m_width = avFrame->width;
        m_height = avFrame->height;
    }

    if (!swsFrame->data[0]) {
        swsFrame->format = AV_PIX_FMT_RGB32;
        swsFrame->width = m_width;
        swsFrame->height = m_height;
        if (av_frame_get_buffer(swsFrame, 0) < 0) {
            emit errorOccurred("Could not allocate frame buffer");
            return QVideoFrame();
        }
    }

    sws_scale(
        swsContext,
        avFrame->data,
        avFrame->linesize,
        0,
        avFrame->height,
        swsFrame->data,
        swsFrame->linesize
    );

    QImage image(
        swsFrame->data[0],
        m_width,
        m_height,
        swsFrame->linesize[0],
        QImage::Format_RGB32
    );

    QImage frameCopy = image.copy();
    return QVideoFrame(frameCopy);
}

void VideoRenderer::adjustPresentationTime(QVideoFrame& frame)
{
    if (!frame.isValid()) return;

    const qint64 currentTime = currentPosition();
    const qint64 frameTime = frame.startTime();

    const qint64 delta = frameTime - currentTime;

    if (delta > 1) {
        QThread::msleep(static_cast<unsigned long>(delta));
    }

    else if (delta < -2 * m_frameDuration) {
        qDebug() << "Dropping late frame, delta:" << delta << "ms";
        frame = QVideoFrame();
    }
}

void VideoRenderer::dropLateFrames()
{
    QMutexLocker locker(&m_queueMutex);
    while (m_frameQueue.size() > 30) {
        m_frameQueue.dequeue();
    }
}