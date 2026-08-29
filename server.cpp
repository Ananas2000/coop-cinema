#include <iostream>
#include <string>
#include <vector>
#include <unordered_map>
#include <queue>
#include <random>
#include <thread>
#include <mutex>
#include <atomic>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <cstdlib>
#include <ctime>

#include <winsock2.h>
#include <ws2tcpip.h>
#include <windows.h>
#include <filesystem>

#include <json/json.h>

using namespace std;
namespace fs = std::filesystem;

#pragma comment(lib, "ws2_32.lib")
#pragma comment(lib, "jsoncpp.lib")

// Исключения для HTTP и TCP серверов
class HttpException : public std::runtime_error {
public:
    explicit HttpException(const std::string& msg)
        : std::runtime_error(msg) {}
};

class TcpException : public std::runtime_error {
public:
    explicit TcpException(const std::string& msg)
        : std::runtime_error(msg) {}
};

struct Film {
    string title;
    string director;
    string path;
};

class FilmManager {
public:
    static FilmManager& getInstance() {
        static FilmManager instance;
        return instance;
    }

    void loadFilms(const string& filename) {
        ifstream filmsFile(filename);
        if (!filmsFile.is_open())
            throw runtime_error("Не удалось открыть файл films.json");

        Json::Value root;
        Json::CharReaderBuilder reader;
        string errs;
        if (!Json::parseFromStream(reader, filmsFile, &root, &errs))
            throw runtime_error("Ошибка парсинга JSON: " + errs);

        for (const auto& filmJson : root["films"]) {
            Film film;
            film.title = filmJson["title"].asString();
            film.director = filmJson["director"].asString();
            film.path = filmJson["path"].asString();
            films_.push_back(film);
        }

        if (films_.empty())
            throw runtime_error("Нет фильмов в films.json");
    }

    const vector<Film>& getFilms() const {
        return films_;
    }

    const Film& getFilmByIndex(size_t idx) const {
        return films_.at(idx);
    }

    size_t size() const {
        return films_.size();
    }

private:
    FilmManager() = default;
    vector<Film> films_;
};

enum class UserState { MainMenu, ChoosingFilm, InRoom };

class User {
public:
    User(SOCKET socket, const string& ip)
        : socket_(socket), ip_(ip), state_(UserState::MainMenu) {
    }

    SOCKET getSocket() const { return socket_; }
    const string& getIP() const { return ip_; }
    const string& getNick() const { return nick_; }
    void setNick(const string& nick) { nick_ = nick; }
    UserState getState() const { return state_; }
    void setState(UserState st) { state_ = st; }
    const string& getRoomName() const { return roomName_; }
    void setRoomName(const string& rn) { roomName_ = rn; }

private:
    SOCKET socket_;
    string ip_;
    string nick_;
    string roomName_;
    UserState state_;
};

class Room {
public:
    Room(const string& name, const Film& film, SOCKET creatorSocket)
        : name_(name), film_(film), creatorSocket_(creatorSocket) {
        for (const auto& nick : DEFAULT_NICKS_)
            availableNicks_.push(nick);
    }

    pair<bool, string> addUser(shared_ptr<User> user) {
        lock_guard<mutex> lock(mutex_);
        if (users_.size() >= maxUsers_)
            return { false, "" };
        if (availableNicks_.empty())
            return { false, "" };

        string nick = availableNicks_.front();
        availableNicks_.pop();
        user->setNick(nick);
        user->setRoomName(name_);
        users_[user->getSocket()] = user;
        return { true, nick };
    }

    pair<bool, bool> removeUser(shared_ptr<User> user) {
        lock_guard<mutex> lock(mutex_);
        auto it = users_.find(user->getSocket());
        if (it == users_.end())
            return { false, false };

        availableNicks_.push(it->second->getNick());
        users_.erase(it);
        bool emptyNow = users_.empty();
        return { true, emptyNow };
    }

    vector<string> getParticipantNicks() const {
        lock_guard<mutex> lock(mutex_);
        vector<string> result;
        for (const auto& [sock, u] : users_)
            result.push_back(u->getNick());
        return result;
    }

    size_t userCount() const {
        lock_guard<mutex> lock(mutex_);
        return users_.size();
    }

    const string& getName() const { return name_; }
    const Film& getFilm() const { return film_; }
    const unordered_map<SOCKET, shared_ptr<User>>& getUsers() const { return users_; }

private:
    string name_;
    Film film_;
    SOCKET creatorSocket_;
    unordered_map<SOCKET, shared_ptr<User>> users_;
    queue<string> availableNicks_;
    mutable mutex mutex_;
    static constexpr size_t maxUsers_ = 10;
    static const vector<string> DEFAULT_NICKS_;
};

const vector<string> Room::DEFAULT_NICKS_ = {
    "Tralalelo Tralala","Tung Tung Tung Tung Sahur","Bombardiro Crocodilo",
    "Udindindindun Madindindindun","Shpioniro Golubiro","Cappuccina Ballerina",
    "Glorbo Fruttodrilo","Cappuccino Assassino","Shimpanzini Bananini","Bobrito Bandito"
};

class RoomManager {
public:
    static RoomManager& getInstance() {
        static RoomManager instance;
        return instance;
    }

    shared_ptr<User> registerUser(SOCKET socket, const string& ip) {
        auto user = make_shared<User>(socket, ip);
        lock_guard<mutex> lock(usersMutex_);
        users_[socket] = user;
        return user;
    }

    void unregisterUser(SOCKET socket) {
        lock_guard<mutex> lock(usersMutex_);
        auto it = users_.find(socket);
        if (it != users_.end())
            users_.erase(it);
    }

    shared_ptr<User> getUser(SOCKET socket) {
        lock_guard<mutex> lock(usersMutex_);
        auto it = users_.find(socket);
        return (it != users_.end() ? it->second : nullptr);
    }

    string createRoom(SOCKET ownerSocket, size_t filmIndex) {
        string roomName = generateRoomName();
        const Film& film = FilmManager::getInstance().getFilmByIndex(filmIndex);
        auto room = make_shared<Room>(roomName, film, ownerSocket);
        {
            lock_guard<mutex> lock(roomsMutex_);
            rooms_[roomName] = room;
        }
        auto user = getUser(ownerSocket);
        auto [success, nick] = room->addUser(user);
        if (!success) {
            lock_guard<mutex> lock(roomsMutex_);
            rooms_.erase(roomName);
            return "";
        }
        user->setState(UserState::InRoom);
        cout << "Создана комната " << roomName << " с фильмом " << film.title << endl;
        return roomName;
    }

    bool joinRoom(SOCKET socket, const string& roomName, string& outNick) {
        shared_ptr<Room> room;
        {
            lock_guard<mutex> lock(roomsMutex_);
            auto it = rooms_.find(roomName);
            if (it == rooms_.end())
                return false;
            room = it->second;
        }
        auto user = getUser(socket);
        auto [success, nick] = room->addUser(user);
        if (!success)
            return false;
        user->setState(UserState::InRoom);
        outNick = nick;
        cout << "Пользователь " << user->getIP() << " присоединился к комнате " << roomName << endl;
        return true;
    }

    pair<bool, bool> leaveRoom(SOCKET socket) {
        auto user = getUser(socket);
        if (!user)
            return { false, false };
        string roomName = user->getRoomName();
        shared_ptr<Room> room;
        {
            lock_guard<mutex> lock(roomsMutex_);
            auto it = rooms_.find(roomName);
            if (it == rooms_.end())
                return { false, false };
            room = it->second;
        }
        auto result = room->removeUser(user);
        user->setState(UserState::MainMenu);
        cout << "Пользователь " << user->getIP() << " покинул комнату " << roomName << endl;
        if (result.second) {
            lock_guard<mutex> lock(roomsMutex_);
            rooms_.erase(roomName);
            cout << "Комната удалена: " << roomName << endl;
        }
        return result;
    }

    void broadcastParticipantUpdate(const string& roomName) {
        shared_ptr<Room> room;
        {
            lock_guard<mutex> lock(roomsMutex_);
            auto it = rooms_.find(roomName);
            if (it == rooms_.end())
                return;
            room = it->second;
        }
        auto nicks = room->getParticipantNicks();
        string msg = "PARTICIPANTS_UPDATE";
        for (const auto& ni : nicks)
            msg += "|" + ni;
        msg += "\n";
        for (const auto& [sock, userPtr] : room->getUsers()) {
            send(sock, msg.c_str(), (int)msg.size(), 0);
        }
    }

    vector<pair<string, size_t>> getRoomsInfo() {
        vector<pair<string, size_t>> res;
        lock_guard<mutex> lock(roomsMutex_);
        for (const auto& [name, roomPtr] : rooms_)
            res.emplace_back(name, roomPtr->userCount());
        return res;
    }

    vector<pair<string, string>> getAllUsersInfo() {
        vector<pair<string, string>> res;
        lock_guard<mutex> lock1(usersMutex_);
        lock_guard<mutex> lock2(roomsMutex_);
        for (const auto& [sock, userPtr] : users_) {
            string room = userPtr->getRoomName();
            string ip = userPtr->getIP();
            if (room.empty())
                res.emplace_back(ip, "не в комнате");
            else
                res.emplace_back(ip, room + " <" + userPtr->getNick() + ">");
        }
        return res;
    }

    shared_ptr<Room> getRoom(const string& name) {
        lock_guard<mutex> lock(roomsMutex_);
        auto it = rooms_.find(name);
        return (it != rooms_.end() ? it->second : nullptr);
    }

private:
    RoomManager() = default;

    string generateRoomName() {
        static const char chars[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";
        random_device rd;
        mt19937 gen(rd());
        uniform_int_distribution<> dist(0, 35);
        string name;
        lock_guard<mutex> lock(roomsMutex_);
        do {
            name.clear();
            for (int i = 0; i < 6; ++i)
                name += chars[dist(gen)];
        } while (rooms_.count(name));
        return name;
    }

    unordered_map<string, shared_ptr<Room>> rooms_;
    unordered_map<SOCKET, shared_ptr<User>> users_;
    mutable mutex roomsMutex_;
    mutable mutex usersMutex_;
};

class ConsoleCommandHandler {
public:
    ConsoleCommandHandler() = default;

    void run() {
        while (isRunning_) {
            system("cls");
            cout << "=== Сервер работает ===\n";
            cout << "Доступные команды:\n";
            cout << "list   - список комнат\n";
            cout << "users  - список подключенных\n";
            cout << "exit   - остановить сервер\n\n";
            string cmd;
            getline(cin, cmd);
            system("cls");
            if (cmd == "list") {
                auto rooms = RoomManager::getInstance().getRoomsInfo();
                cout << "Активные комнаты (" << rooms.size() << "):\n";
                for (const auto& [name, cnt] : rooms) {
                    auto roomPtr = RoomManager::getInstance().getRoom(name);
                    cout << "- " << name << " (" << cnt << "/10) - "
                        << roomPtr->getFilm().title << " ("
                        << roomPtr->getFilm().director << ")\n";
                }
            }
            else if (cmd == "users") {
                auto users = RoomManager::getInstance().getAllUsersInfo();
                cout << "Подключенные клиенты (" << users.size() << "):\n";
                for (const auto& [ip, desc] : users) {
                    cout << "- " << ip << " : " << desc << "\n";
                }
            }
            else if (cmd == "exit") {
                isRunning_ = false;
                closesocket(tcpListenSocket_);
                break;
            }
            if (cmd != "exit") {
                cout << "Нажмите Enter для продолжения..." << endl;
                cin.ignore();
            }
        }
    }

    void stop() {
        isRunning_ = false;
    }

    void setListenSocket(SOCKET s) {
        tcpListenSocket_ = s;
    }

    bool isRunning() const {
        return isRunning_;
    }

private:
    atomic<bool> isRunning_{ true };
    SOCKET tcpListenSocket_{ INVALID_SOCKET };
};

class ClientHandler {
public:
    ClientHandler(SOCKET clientSocket)
        : clientSocket_(clientSocket) {
    }

    void operator()() {
        string clientIP = getClientIP(clientSocket_);
        cout << "\nПодключился: " << clientIP << endl;
        auto user = RoomManager::getInstance().registerUser(clientSocket_, clientIP);
        char buffer[1024];

        while (true) {
            memset(buffer, 0, sizeof(buffer));
            int bytes = recv(clientSocket_, buffer, sizeof(buffer), 0);
            if (bytes <= 0) {
                cout << "Разрыв соединения: " << clientIP << endl;
                break;
            }

            string command(buffer, bytes);
            command.erase(remove(command.begin(), command.end(), '\n'), command.end());
            command.erase(remove(command.begin(), command.end(), '\r'), command.end());

            vector<string> parts;
            size_t start = 0;
            size_t pos = command.find('|');
            while (pos != string::npos) {
                parts.push_back(command.substr(start, pos - start));
                start = pos + 1;
                pos = command.find('|', start);
            }
            parts.push_back(command.substr(start));

            if (parts.empty())
                continue;

            const string& cmd = parts[0];

            if (cmd == "PLAYER_STATE") {
                if (parts.size() < 4)
                    continue;
                string roomId = parts[1];
                string isPaused = parts[2];
                string position = parts[3];

                auto roomPtr = RoomManager::getInstance().getRoom(roomId);
                if (roomPtr && user->getRoomName() == roomId) {
                    string msg = "PLAYER_STATE|" + roomId + "|" + user->getNick() + "|" + isPaused + "|" + position + "\n";
                    for (const auto& [sock, uPtr] : roomPtr->getUsers()) {
                        send(sock, msg.c_str(), (int)msg.size(), 0);
                    }
                }
            }
            else if (user->getState() == UserState::MainMenu) {
                if (cmd == "CREATE_ROOM") {
                    int bytes2 = recv(clientSocket_, buffer, sizeof(buffer), 0);
                    if (bytes2 <= 0) {
                        cout << "Ошибка получения индекса фильма от " << clientIP << endl;
                        break;
                    }
                    int filmIndex = stoi(string(buffer, bytes2));
                    string roomName = RoomManager::getInstance().createRoom(clientSocket_, filmIndex - 1);
                    if (roomName.empty()) {
                        string err = "ERROR|Создание комнаты не удалось\n";
                        send(clientSocket_, err.c_str(), (int)err.size(), 0);
                        cout << "Ошибка создания комнаты для " << clientIP << endl;
                    }
                    else {
                        string msg1 = "ROOM_CREATED|" + roomName + "\n";
                        send(clientSocket_, msg1.c_str(), (int)msg1.size(), 0);

                        const Film& film = FilmManager::getInstance().getFilmByIndex(filmIndex - 1);
                        string videoUrl = "VIDEO_URL|http://localhost:8000/" + film.path + "\n";
                        send(clientSocket_, videoUrl.c_str(), (int)videoUrl.size(), 0);

                        string profileMsg = "TEMPORARY_PROFILE|" + user->getNick() + "\n";
                        send(clientSocket_, profileMsg.c_str(), (int)profileMsg.size(), 0);

                        RoomManager::getInstance().broadcastParticipantUpdate(roomName);
                        user->setState(UserState::InRoom);
                    }
                }
                else if (cmd == "GET_FILMS") {
                    const auto& films = FilmManager::getInstance().getFilms();
                    string header = "FILMS_LIST\n";
                    send(clientSocket_, header.c_str(), (int)header.size(), 0);

                    string response = "FILMS_LIST";
                    for (const auto& f : films)
                        response += "|" + f.title + "|" + f.director;
                    response += "\n";
                    send(clientSocket_, response.c_str(), (int)response.size(), 0);
                }
                else if (cmd == "JOIN_ROOM") {
                    int bytes2 = recv(clientSocket_, buffer, sizeof(buffer), 0);
                    if (bytes2 <= 0) {
                        break;
                    }
                    string roomName(buffer, bytes2);
                    string nick;
                    if (RoomManager::getInstance().joinRoom(clientSocket_, roomName, nick)) {
                        user->setNick(nick);
                        string roomEntered = "ROOM_ENTERED|" + roomName + "\n";
                        send(clientSocket_, roomEntered.c_str(), (int)roomEntered.size(), 0);

                        auto roomPtr = RoomManager::getInstance().getRoom(roomName);
                        string videoUrl = "VIDEO_URL|http://localhost:8000/" + roomPtr->getFilm().path + "\n";
                        send(clientSocket_, videoUrl.c_str(), (int)videoUrl.size(), 0);

                        string profileMsg = "TEMPORARY_PROFILE|" + nick + "\n";
                        send(clientSocket_, profileMsg.c_str(), (int)profileMsg.size(), 0);

                        RoomManager::getInstance().broadcastParticipantUpdate(roomName);
                        user->setState(UserState::InRoom);
                    }
                    else {
                        string err = "ERROR|Комната не найдена или переполнена\n";
                        send(clientSocket_, err.c_str(), (int)err.size(), 0);
                    }
                }
                else if (cmd == "GET_VIDEO") {
                    if (user->getRoomName().empty()) {
                        string err = "ERROR|Вы не в комнате\n";
                        send(clientSocket_, err.c_str(), (int)err.size(), 0);
                        continue;
                    }
                    fs::path videoPath = "videos/" + RoomManager::getInstance().getRoom(user->getRoomName())->getFilm().title + ".mp4";
                    ifstream videoFile(videoPath, ios::binary);
                    if (!videoFile.is_open()) {
                        string err = "ERROR|Файл не найден\n";
                        send(clientSocket_, err.c_str(), (int)err.size(), 0);
                        continue;
                    }
                    send(clientSocket_, "START_VIDEO\n", 12, 0);
                    char buf[4096];
                    while (videoFile.read(buf, sizeof(buf)))
                        send(clientSocket_, buf, (int)sizeof(buf), 0);
                    if (videoFile.gcount() > 0)
                        send(clientSocket_, buf, (int)videoFile.gcount(), 0);
                    send(clientSocket_, "END_VIDEO\n", 10, 0);
                    videoFile.close();
                }
            }
            else if (cmd == "REACTION" && parts.size() >= 3) {
                string roomId = parts[1];
                string reaction = parts[2];
                auto roomPtr = RoomManager::getInstance().getRoom(roomId);
                if (roomPtr && user->getRoomName() == roomId) {
                    string msg = "REACTION|" + user->getNick() + "|" + reaction + "\n";
                    for (const auto& [sock, uPtr] : roomPtr->getUsers()) {
                        send(sock, msg.c_str(), (int)msg.size(), 0);
                    }
                }
            }
            else if (user->getState() == UserState::InRoom && cmd == "LEAVE_ROOM") {
                auto [removed, nowEmpty] = RoomManager::getInstance().leaveRoom(clientSocket_);
                if (removed) {
                    string resp = "LEFT_ROOM|" + user->getRoomName() + "\n";
                    send(clientSocket_, resp.c_str(), (int)resp.size(), 0);
                    if (!nowEmpty)
                        RoomManager::getInstance().broadcastParticipantUpdate(user->getRoomName());
                }
                user->setRoomName("");
                user->setState(UserState::MainMenu);
            }
        }

        if (user->getState() == UserState::InRoom) {
            auto [removed, nowEmpty] = RoomManager::getInstance().leaveRoom(clientSocket_);
            if (!nowEmpty)
                RoomManager::getInstance().broadcastParticipantUpdate(user->getRoomName());
        }
        RoomManager::getInstance().unregisterUser(clientSocket_);
        closesocket(clientSocket_);
    }

private:
    string getClientIP(SOCKET s) {
        sockaddr_in addr;
        int len = sizeof(addr);
        getpeername(s, (sockaddr*)&addr, &len);
        char buf[INET_ADDRSTRLEN];
        inet_ntop(AF_INET, &addr.sin_addr, buf, INET_ADDRSTRLEN);
        return string(buf);
    }

    SOCKET clientSocket_;
};

class NetworkUtils {
public:
    static void initWinsock() {
        WSADATA wsaData;
        if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
            throw runtime_error("Ошибка инициализации Winsock");
    }

    static void cleanupWinsock() {
        WSACleanup();
    }

    class HttpServer {
    public:
        explicit HttpServer(const string& dir)
            : filmsDir_(dir), running_(false), listenSocket_(INVALID_SOCKET) {
        }

        void start() {
            running_ = true;
            thread([this]() { run(); }).detach();
        }

        void stop() {
            running_ = false;
            if (listenSocket_ != INVALID_SOCKET)
                closesocket(listenSocket_);
        }

    private:
        void run() {
            listenSocket_ = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
            if (listenSocket_ == INVALID_SOCKET)
                throw HttpException("Ошибка создания сокета HTTP сервера");

            sockaddr_in httpAddr{};
            httpAddr.sin_family = AF_INET;
            httpAddr.sin_addr.s_addr = INADDR_ANY;
            httpAddr.sin_port = htons(8000);

            if (bind(listenSocket_, (sockaddr*)&httpAddr, sizeof(httpAddr)) == SOCKET_ERROR) {
                closesocket(listenSocket_);
                throw HttpException("Ошибка привязки HTTP сервера");
            }

            if (listen(listenSocket_, SOMAXCONN) == SOCKET_ERROR) {
                closesocket(listenSocket_);
                throw HttpException("Ошибка прослушивания HTTP сервера");
            }

            cout << "HTTP сервер запущен на порту 8000\n";

            while (running_) {
                SOCKET clientSock = accept(listenSocket_, nullptr, nullptr);
                if (clientSock == INVALID_SOCKET)
                    continue;
                thread([this, clientSock]() { handleClient(clientSock); }).detach();
            }
            closesocket(listenSocket_);
        }

        void handleClient(SOCKET clientSock) {
            const int bufferSize = 4096;
            vector<char> buffer(bufferSize);
            int received = recv(clientSock, buffer.data(), bufferSize, 0);
            if (received <= 0) {
                closesocket(clientSock);
                return;
            }

            string request(buffer.data(), received);
            size_t posLineEnd = request.find("\r\n");
            if (posLineEnd == string::npos) {
                closesocket(clientSock);
                return;
            }
            string requestLine = request.substr(0, posLineEnd);
            istringstream lineStream(requestLine);
            string method, url, protocol;
            lineStream >> method >> url >> protocol;

            cout << "HTTP запрос: " << method << " " << url << endl;

            size_t fileStart = 0, fileEnd = 0;
            bool isPartial = false;
            size_t posHeader = request.find("Range:");
            if (posHeader != string::npos) {
                size_t posLineBreak = request.find("\r\n", posHeader);
                string rangeLine = request.substr(posHeader, posLineBreak - posHeader);
                auto bytesPos = rangeLine.find("bytes=");
                if (bytesPos != string::npos) {
                    string byteRange = rangeLine.substr(bytesPos + 6);
                    auto dashPos = byteRange.find('-');
                    string startStr = byteRange.substr(0, dashPos);
                    string endStr = byteRange.substr(dashPos + 1);
                    fileStart = startStr.empty() ? 0 : stoull(startStr);
                    isPartial = true;
                    fileEnd = endStr.empty() ? ULLONG_MAX : stoull(endStr);
                }
            }

            string relativePath = url;
            if (!relativePath.empty() && relativePath.front() == '/')
                relativePath.erase(0, 1);
            if (relativePath.empty()) {
                string resp = "HTTP/1.1 404 Not Found\r\nContent-Length: 0\r\n\r\n";
                send(clientSock, resp.c_str(), (int)resp.size(), 0);
                closesocket(clientSock);
                return;
            }

            fs::path filePath = fs::path(filmsDir_) / fs::path(relativePath);
            if (!fs::exists(filePath) || fs::is_directory(filePath)) {
                string resp = "HTTP/1.1 404 Not Found\r\nContent-Length: 0\r\n\r\n";
                send(clientSock, resp.c_str(), (int)resp.size(), 0);
                closesocket(clientSock);
                return;
            }

            uint64_t totalSize = fs::file_size(filePath);
            if (isPartial) {
                if (fileEnd == ULLONG_MAX || fileEnd >= totalSize)
                    fileEnd = totalSize - 1;
            }
            else {
                fileStart = 0;
                fileEnd = totalSize - 1;
            }
            if (fileStart > fileEnd || fileStart >= totalSize) {
                string resp = "HTTP/1.1 416 Range Not Satisfiable\r\nContent-Length: 0\r\n\r\n";
                send(clientSock, resp.c_str(), (int)resp.size(), 0);
                closesocket(clientSock);
                return;
            }

            uint64_t contentLength = fileEnd - fileStart + 1;
            string statusLine = isPartial ? "HTTP/1.1 206 Partial Content\r\n" : "HTTP/1.1 200 OK\r\n";
            ostringstream headers;
            headers << statusLine;
            headers << "Content-Type: video/mp4\r\n";
            headers << "Accept-Ranges: bytes\r\n";
            headers << "Content-Length: " << contentLength << "\r\n";
            if (isPartial)
                headers << "Content-Range: bytes " << fileStart << "-" << fileEnd << "/" << totalSize << "\r\n";
            headers << "\r\n";

            send(clientSock, headers.str().c_str(), (int)headers.str().size(), 0);

            ifstream fileStream(filePath, ios::binary);
            if (!fileStream.is_open()) {
                closesocket(clientSock);
                return;
            }
            fileStream.seekg(fileStart, ios::beg);

            const size_t chunkSize = 8192;
            vector<char> sendBuffer(chunkSize);
            uint64_t bytesToSend = contentLength;
            while (bytesToSend > 0) {
                size_t thisRead = (size_t)min<uint64_t>(chunkSize, bytesToSend);
                fileStream.read(sendBuffer.data(), thisRead);
                streamsize actuallyRead = fileStream.gcount();
                if (actuallyRead <= 0)
                    break;
                send(clientSock, sendBuffer.data(), (int)actuallyRead, 0);
                bytesToSend -= (uint64_t)actuallyRead;
            }
            fileStream.close();
            closesocket(clientSock);
        }

        string filmsDir_;
        atomic<bool> running_;
        SOCKET listenSocket_;
    };
};

int wmain(int argc, wchar_t* argv[]) {
    setlocale(LC_ALL, "Russian");
    srand((unsigned)time(nullptr));
    bool httpMode = (argc == 2 && wcscmp(argv[1], L"--http") == 0);

    if (httpMode) {
        try {
            try {
                NetworkUtils::initWinsock();
            }
            catch (const std::exception& e) {
                throw HttpException(e.what());
            }
            NetworkUtils::HttpServer httpServer("films");
            httpServer.start();
            while (true)
                Sleep(1000);
            httpServer.stop();
            NetworkUtils::cleanupWinsock();
        }
        catch (...) {
            return 1;
        }
        return 0;
    }

    // Запуск HTTP-сервера в новом процессе
    wchar_t exePath[MAX_PATH];
    GetModuleFileNameW(nullptr, exePath, MAX_PATH);
    wstring cmdLine = L"\"";
    cmdLine += exePath;
    cmdLine += L"\" --http";

    STARTUPINFOW si;
    PROCESS_INFORMATION pi;
    ZeroMemory(&si, sizeof(si));
    si.cb = sizeof(si);
    ZeroMemory(&pi, sizeof(pi));
    CreateProcessW(nullptr, cmdLine.data(), nullptr, nullptr, FALSE, CREATE_NEW_CONSOLE, nullptr, nullptr, &si, &pi);
    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);

    try {
        try {
            NetworkUtils::initWinsock();
        }
        catch (const std::exception& e) {
            throw TcpException(e.what());
        }

        FilmManager::getInstance().loadFilms("films.json");

        SOCKET listenSock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
        if (listenSock == INVALID_SOCKET)
            throw TcpException("Ошибка создания сокета");

        sockaddr_in serverAddr{};
        serverAddr.sin_family = AF_INET;
        serverAddr.sin_addr.s_addr = INADDR_ANY;
        serverAddr.sin_port = htons(8888);

        if (bind(listenSock, (sockaddr*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR)
            throw TcpException("Ошибка привязки");

        if (listen(listenSock, SOMAXCONN) == SOCKET_ERROR)
            throw TcpException("Ошибка прослушивания");

        cout << "TCP сервер запущен на порту 8888";

        ConsoleCommandHandler consoleHandler;
        consoleHandler.setListenSocket(listenSock);
        thread consoleThread(&ConsoleCommandHandler::run, &consoleHandler);

        while (consoleHandler.isRunning()) {
            SOCKET clientSock = accept(listenSock, nullptr, nullptr);
            if (clientSock == INVALID_SOCKET)
                continue;
            thread(ClientHandler(clientSock)).detach();
        }

        consoleThread.join();
        closesocket(listenSock);
        NetworkUtils::cleanupWinsock();
        cout << "\nСервер закрыт";
    }
    catch (const TcpException& e) {
        cerr << "TCP Ошибка: " << e.what() << endl;
        return 1;
    }
    catch (const exception& e) {
        cerr << "Ошибка: " << e.what() << endl;
        return 1;
    }

    return 0;
}