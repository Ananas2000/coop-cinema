#include "Client.h"
#include <QWebSocket>
#include <QStringList>

Client::Client(QObject* parent)
    : QObject(parent),
    m_socket(new QWebSocket())
{
    connect(m_socket, &QWebSocket::connected, this, &Client::onConnected);
    connect(m_socket, &QWebSocket::disconnected, this, &Client::onDisconnected);
    connect(m_socket, &QWebSocket::textMessageReceived, this, &Client::onTextMessageReceived);
    connect(m_socket, &QWebSocket::binaryMessageReceived, this, &Client::onBinaryMessageReceived);
    connect(m_socket, &QWebSocket::errorOccurred, this, &Client::onErrorOccurred);
}

void Client::connectToServer(const QString& host, quint16 port)
{
    QUrl url(QStringLiteral("ws://%1:%2").arg(host).arg(port));
    m_socket->open(url);
}

void Client::disconnectFromServer()
{
    if (m_socket->state() == QAbstractSocket::ConnectedState) {
        m_socket->close();
    }
}

void Client::createRoom()
{
    sendMessage("CREATE_ROOM");
}

void Client::joinRoom(const QString& roomId)
{
    m_currentRoom = roomId;
    sendMessage("JOIN_ROOM|" + roomId);
}

void Client::sendReaction(const QString& reaction)
{
    sendMessage("REACTION|" + m_currentRoom + "|" + reaction);
}

void Client::sendPlayerState(bool isPaused, qint64 position)
{
    sendMessage(QString("PLAYER_STATE|%1|%2|%3")
        .arg(m_currentRoom)
        .arg(isPaused ? "1" : "0")
        .arg(position));
}

void Client::onConnected()
{
    emit connected();
}

void Client::onDisconnected()
{
    emit disconnected();
}

void Client::onTextMessageReceived(const QString& message)
{
    QStringList parts = message.split("|", Qt::SkipEmptyParts);
    if (parts.isEmpty()) return;

    const QString command = parts[0];
    parts.pop_front();

    if (command == "ROOM_CREATED") {
        if (!parts.isEmpty()) emit roomCreated(parts[0]);
    }
    else if (command == "PARTICIPANTS_UPDATE") {
        emit participantsUpdated(parts);
    }
    else if (command == "VIDEO_URL") {
        if (!parts.isEmpty()) emit videoUrlReceived(QUrl(parts[0]));
    }
    else if (command == "CHAT_MESSAGE") {
        if (parts.size() >= 2) emit chatMessageReceived(parts[0], parts[1]);
    }
    else if (command == "SYNC_POSITION") {
        if (!parts.isEmpty()) emit syncPositionReceived(parts[0].toLongLong());
    }
    else if (command == "TEMPORARY_PROFILE") {
        if (parts.size() >= 2) {
            m_userNickname = parts[0];
            m_userAvatar = parts[1];
        }
    }
    else if (command == "ERROR") {
        emit errorOccurred(parts.join("|"));
    }
}

void Client::onBinaryMessageReceived(const QByteArray& data)
{
    processVideoData(data);
}

void Client::onErrorOccurred(QAbstractSocket::SocketError error)
{
    Q_UNUSED(error)
        emit errorOccurred(m_socket->errorString());
}

void Client::sendMessage(const QString& message)
{
    if (m_socket->state() == QAbstractSocket::ConnectedState) {
        m_socket->sendTextMessage(message);
    }
}

void Client::processVideoData(const QByteArray& data)
{
    // Реализация обработки бинарных данных видео
    Q_UNUSED(data)
}