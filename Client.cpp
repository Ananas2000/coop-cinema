#include "Client.h"
#include <QTcpSocket>
#include <QStringList>
#include <QDebug>
#include <QApplication>
#include <QDir>
#include <QDataStream>
#include <thread>

Client::Client(QObject* parent)
    : QObject(parent),
    m_socket(new QTcpSocket(this)),
    m_avatarIndex(0)
{
    connect(m_socket, &QTcpSocket::connected, this, &Client::onConnected);
    connect(m_socket, &QTcpSocket::disconnected, this, &Client::onDisconnected);
    connect(m_socket, &QTcpSocket::readyRead, this, &Client::onReadyRead); // Чтение данных
    connect(m_socket, &QTcpSocket::errorOccurred, this, &Client::onErrorOccurred);

    QString avatarsDir = QApplication::applicationDirPath() + "/avatars/";

    QDir dir;
    if (!dir.exists(avatarsDir)) {
        dir.mkpath(avatarsDir);
    }

    for (int i = 0; i < 10; i++) {
        QString avatarPath = avatarsDir + QString::number(i) + ".png";
        if (QFile::exists(avatarPath)) {
            m_avatars.append(avatarPath);
        }
        else {
            m_avatars.append(":/images/default_avatar.png");
            qWarning() << "Avatar file missing:" << avatarPath;
        }
    }
}

void Client::connectToServer(const QString& host, quint16 port)
{
    m_socket->connectToHost(host, port);
}

void Client::disconnectFromServer()
{
    if (m_socket->state() == QAbstractSocket::ConnectedState) {
        m_socket->disconnectFromHost();
    }
}

void Client::createRoom(int filmIndex) {
    sendMessage("CREATE_ROOM");
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    sendMessage(QString::number(filmIndex));
}

void Client::joinRoom(const QString& roomId)
{
    m_currentRoom = roomId;
    sendMessage("JOIN_ROOM"); // Команда присоединения
    sendMessage(roomId); // Отправка ID комнаты
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
    m_avatarIndex = 0;
    m_userAvatars.clear();
    emit disconnected();
}

void Client::onReadyRead() // Обработка входящих данных
{
    while (m_socket->canReadLine()) {
        QString message = QString::fromUtf8(m_socket->readLine()).trimmed();
        onTextMessageReceived(message); // Передаём данные в обработчик
    }
}

void Client::onTextMessageReceived(const QString& message)
{
    QStringList parts = message.split("|", Qt::SkipEmptyParts);
    if (parts.isEmpty()) return;

    const QString command = parts[0];
    parts.pop_front();

    if (command == "FILMS_LIST") {
        QStringList films;
        int counter = 1;
        for (int i = 0; i < parts.size(); i += 2) {
            if (i + 1 < parts.size()) {
                QString current = QString::number(counter) + ". " + parts[i] + " (" + parts[i + 1] + ")";
                films << current;
                counter++;
            }
        }
        emit filmsListReceived(films);
    }

    else if (command == "ROOM_CREATED") {
        if (!parts.isEmpty()) {
            m_currentRoom = parts[0];
            emit roomCreated(parts[0]);
        }
    }

    else if (command == "PARTICIPANTS_UPDATE") {
        for (const QString& user : parts) {
            assignAvatar(user);
        }

        QStringList usersWithAvatars;
        for (const QString& user : parts) {
            usersWithAvatars << user + "|" + m_userAvatars.value(user, ":/images/default_avatar.png");
        }

        emit participantsUpdated(usersWithAvatars);
    }
    else if (command == "CHAT_MESSAGE") {
        if (parts.size() >= 2) emit chatMessageReceived(parts[0], parts[1]);
    }
    else if (command == "PLAYER_STATE") {
        if (parts.size() >= 4) {
            QString roomId = parts[0];
            QString senderNick = parts[1];
            bool isPaused = (parts[2] == "1");
            qint64 position = parts[3].toLongLong();
            emit playerStateReceived(roomId, senderNick, isPaused, position);
        }
    }
    else if (command == "TEMPORARY_PROFILE") {
        if (parts.size() >= 2) {
            m_userNickname = parts[0];
            m_userAvatar = parts[1];
        }
    }
    else if (command == "VIDEO_URL") {
        if (!parts.isEmpty()) {
            QUrl url(parts[0]);

            emit videoUrlReceived(url);
        }
    }
    else if (command == "ERROR") {
        emit errorOccurred(parts.join("|"));
    }
    else if (command == "ROOM_ENTERED") {
        if (!parts.isEmpty()) {
            m_currentRoom = parts[0];
            emit roomCreated(parts[0]); // Используем тот же сигнал, что и при создании комнаты
        }
    }
}

void Client::onErrorOccurred(QAbstractSocket::SocketError error)
{
    Q_UNUSED(error)
        emit errorOccurred(m_socket->errorString());
}

void Client::sendMessage(const QString& message)
{
    if (m_socket->state() == QAbstractSocket::ConnectedState) {
        m_socket->write(message.toUtf8());
        m_socket->flush();
    }
}

void Client::processVideoData(const QByteArray& data)
{
    Q_UNUSED(data)
}

void Client::assignAvatar(const QString& userNickname)
{
    if (m_userAvatars.contains(userNickname)) {
        return;
    }

    // Используем относительные пути вместо абсолютных
    QString avatar = QString("avatars/%0.png").arg(m_avatarIndex % 10);
    m_userAvatars[userNickname] = avatar;
    m_avatarIndex++;

    qDebug() << "Assigned avatar" << avatar << "to user" << userNickname;
}