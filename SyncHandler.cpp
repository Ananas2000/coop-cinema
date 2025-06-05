#include "SyncHandler.h"

SyncHandler::SyncHandler(QObject* parent)
    : QObject(parent)
{
    connect(&m_syncTimer, &QTimer::timeout, this, &SyncHandler::sendSyncPacket);
    m_syncTimer.setSingleShot(false);
    m_offsetTimer.start();
}

void SyncHandler::startSync(int intervalMs)
{
    if (!m_syncActive) {
        m_syncTimer.start(intervalMs);
        m_syncActive = true;
    }
}

void SyncHandler::stopSync()
{
    m_syncTimer.stop();
    m_syncActive = false;
}

void SyncHandler::forceImmediateSync()
{
    sendSyncPacket();
}

void SyncHandler::sendSyncPacket()
{
    const qint64 clientTime = currentTimestamp();
    emit syncRequest(clientTime, m_lastLocalPosition);
}

void SyncHandler::updateLocalPosition(qint64 position)
{
    m_lastLocalPosition = position;
}

void SyncHandler::handleServerSync(qint64 clientSentTime,
    qint64 serverReceiveTime,
    qint64 serverTransmitTime,
    qint64 playbackPosition)
{
    const qint64 clientReceiveTime = currentTimestamp();

    SyncPoint sp{ clientSentTime, serverReceiveTime, serverTransmitTime };
    m_syncHistory.append(sp);

    calculateOffset(clientSentTime, serverReceiveTime, serverTransmitTime);

    const qint64 estimatedServerNow = serverTransmitTime + (m_roundTripTime / 2);
    const qint64 clientNow = clientReceiveTime;
    const qint64 expectedPlaybackPosition = playbackPosition + (clientNow - estimatedServerNow);

    const qint64 adjustment = expectedPlaybackPosition - m_lastLocalPosition;

    emit syncCorrection(serverTransmitTime, adjustment);
}

void SyncHandler::calculateOffset(qint64 t1, qint64 t2, qint64 t3)
{
    const qint64 t4 = currentTimestamp();

    const qint64 rtt = (t4 - t1) - (t3 - t2);
    const qint64 offset = ((t2 - t1) + (t3 - t4)) / 2;

    m_roundTripTime = rtt;
    m_timeOffset = offset;
}

qint64 SyncHandler::adjustedPosition(qint64 localPosition) const
{
    return localPosition + m_timeOffset;
}

qint64 SyncHandler::currentTimestamp() const
{
    return m_offsetTimer.elapsed();
}
