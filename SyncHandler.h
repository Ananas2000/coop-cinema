#ifndef SYNCHANDLER_H
#define SYNCHANDLER_H

#include <QObject>
#include <QTimer>
#include <QElapsedTimer>
#include <QDateTime>

class SyncHandler : public QObject
{
    Q_OBJECT
public:
    explicit SyncHandler(QObject* parent = nullptr);

    void startSync(int intervalMs = 2000);
    void stopSync();
    void forceImmediateSync();

    qint64 adjustedPosition(qint64 localPosition) const;

signals:
    void syncRequest(qint64 clientTime, qint64 playbackPos);

    void syncCorrection(qint64 serverTime, qint64 adjustment);

public slots:
    void handleServerSync(qint64 clientSentTime,
        qint64 serverReceiveTime,
        qint64 serverTransmitTime,
        qint64 playbackPosition);

    void updateLocalPosition(qint64 position);

private slots:
    void sendSyncPacket();

private:
    void calculateOffset(qint64 t1, qint64 t2, qint64 t3);

    qint64 currentTimestamp() const;

    QTimer m_syncTimer;
    QElapsedTimer m_offsetTimer;

    qint64 m_lastLocalPosition = 0;
    qint64 m_timeOffset = 0;
    qint64 m_roundTripTime = 0;

    struct SyncPoint {
        qint64 clientSend;
        qint64 serverReceive;
        qint64 serverSend;
    };

    QVector<SyncPoint> m_syncHistory;
    bool m_syncActive = false;
};

#endif
