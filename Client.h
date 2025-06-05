#ifndef CLIENT_H
#define CLIENT_H

#include <QObject>
#include <QTcpSocket>
#include <QUrl>
#include <QStringList>
#include <QMap>

class Client : public QObject
{
    Q_OBJECT
public:
    explicit Client(QObject* parent = nullptr);

    void connectToServer(const QString& host, quint16 port);
    void disconnectFromServer();

    void createRoom(int filmIndex);
    void joinRoom(const QString& roomId);
    void sendReaction(const QString& reaction);
    void sendMessage(const QString& message);
    void sendPlayerState(bool isPaused, qint64 position);

    QString currentRoom() const { return m_currentRoom; }
    QString userNickname() const { return m_userNickname; }

signals:
    void connected();
    void disconnected();
    void errorOccurred(const QString& error);
    void roomCreated(const QString& roomId);
    void participantsUpdated(const QStringList& users);
    void videoUrlReceived(const QUrl& url);
    void chatMessageReceived(const QString& user, const QString& message);
    void syncPositionReceived(qint64 position);
    void filmsListReceived(const QStringList& films);
    void playerStateReceived(const QString& roomId, const QString& senderNick, bool isPaused, qint64 position);

private slots:
    void onConnected();
    void onDisconnected();
    void onTextMessageReceived(const QString& message);
    void onErrorOccurred(QAbstractSocket::SocketError error);
    void onReadyRead();

private:
    void processVideoData(const QByteArray& data);
    void assignAvatar(const QString& userNickname);

    QTcpSocket* m_socket;
    QString m_currentRoom;
    QString m_userNickname;
    QString m_userAvatar;

    QStringList m_avatars;
    int m_avatarIndex = 0;
    QMap<QString, QString> m_userAvatars;
};
#endif // CLIENT_H