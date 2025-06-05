#ifndef CLIENT_H
#define CLIENT_H

#include <QObject>
#include <QWebSocket>
#include <QUrl>
#include <QStringList>

class Client : public QObject
{
    Q_OBJECT
public:
    explicit Client(QObject* parent = nullptr);

    void connectToServer(const QString& host, quint16 port);
    void disconnectFromServer();
    
    void createRoom();
    void joinRoom(const QString& roomId);
    void sendReaction(const QString& reaction);
    void sendMessage(const QString& message);
    void sendPlayerState(bool isPaused, qint64 position);

signals:
    void connected();
    void disconnected();
    void errorOccurred(const QString& error);
    void roomCreated(const QString& roomId);
    void participantsUpdated(const QStringList& users);
    void videoUrlReceived(const QUrl& url);
    void chatMessageReceived(const QString& user, const QString& message);
    void syncPositionReceived(qint64 position);

private slots:
    void onConnected();
    void onDisconnected();
    void onTextMessageReceived(const QString& message);
    void onBinaryMessageReceived(const QByteArray& message);
    void onErrorOccurred(QAbstractSocket::SocketError error);

private:
    void processVideoData(const QByteArray& data);

    QWebSocket* m_socket;
    QString m_currentRoom;
    QString m_userNickname;
    QString m_userAvatar;
};

#endif // CLIENT_H