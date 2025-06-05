#include <iostream>
#include <string>
#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <queue>
#include <random>
#include <thread>
#include <mutex>
#include <atomic>
#include <cstdlib>
#include <utility>
#include <fstream>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <json/json.h>

using namespace std;

#pragma comment(lib, "ws2_32.lib")
#pragma comment(lib, "jsoncpp.lib")

const vector<string> DEFAULT_NICKS = {
    "Tralalelo Tralala", "Tung Tung Tung Tung Sahur", "Bombardiro Crocodilo",
    "Udindindindun Madindindindun", "Shpioniro Golubiro", "Cappuccina Ballerina",
    "Glorbo Fruttodrilo", "Cappuccino Assassino", "Shimpanzini Bananini", "Bobrito Bandito"
};

struct Film {
    string title;
    string director;
    string path;
};

class Room {
public:
    SOCKET creator;
    string name;
    string filmTitle;
    string filmDirector;
    string filmPath;
    unordered_map<SOCKET, pair<string, string>> users;
    queue<string> availableNicks;
    mutable mutex roomMutex;

    Room(string name, const Film & film, SOCKET creatorSocket)
        : name(move(name)), filmTitle(film.title),
        filmDirector(film.director), filmPath(film.path),
        creator(creatorSocket)
    {
        for (const auto& nick : DEFAULT_NICKS) {
            availableNicks.push(nick);
        }
    }

    pair<bool, string> addUser(SOCKET socket, const string& ip) {
        lock_guard<mutex> lock(roomMutex);
        if (users.size() >= 10) return { false, "" };
        if (availableNicks.empty()) return { false, "" };

        string nick = availableNicks.front();
        availableNicks.pop();
        users[socket] = { ip, nick };
        return { true, nick };
    }

    pair<bool, bool> removeUser(SOCKET socket) {
        lock_guard<mutex> lock(roomMutex);
        auto it = users.find(socket);
        if (it == users.end()) return { false, false };

        availableNicks.push(it->second.second);
        users.erase(it);
        return { true, users.empty() };
    }

    vector<pair<string, string>> getUsersInfo() const {
        lock_guard<mutex> lock(roomMutex);
        vector<pair<string, string>> result;
        for (const auto& [sock, info] : users) {
            result.push_back(info);
        }
        return result;
    }

    size_t userCount() const {
        lock_guard<mutex> lock(roomMutex);
        return users.size();
    }
};

class Server {
    WSADATA wsaData;
    SOCKET serverSocket = INVALID_SOCKET;
    atomic<bool> isRunning{ false };

    unordered_map<string, Room> rooms;
    unordered_map<SOCKET, string> socketToRoom;
    unordered_map<SOCKET, string> connectedClients;
    vector<Film> films;

    mutable mutex roomsMutex;
    mutable mutex clientsMutex;

    string getClientIP(SOCKET socket) {
        sockaddr_in addr;
        int addrSize = sizeof(addr);
        getpeername(socket, (sockaddr*)&addr, &addrSize);
        char ip[INET_ADDRSTRLEN];
        inet_ntop(AF_INET, &addr.sin_addr, ip, INET_ADDRSTRLEN);
        return ip;
    }

    void loadFilms() {
        ifstream filmsFile("films.json");
        if (!filmsFile.is_open()) {
            throw runtime_error("Не удалось открыть films.json");
        }

        Json::Value root;
        Json::CharReaderBuilder reader;
        string errs;
        if (!Json::parseFromStream(reader, filmsFile, &root, &errs)) {
            throw runtime_error("Ошибка парсинга JSON: " + errs);
        }

        for (const auto& filmJson : root["films"]) {
            Film film{
                filmJson["title"].asString(),
                filmJson["director"].asString(),
                filmJson["path"].asString()
            };
            films.push_back(film);
        }

        if (films.empty()) {
            throw runtime_error("Нет фильмов в films.json");
        }
    }

    void printHelp() {
        cout << "Серверные команды:\n"
            << "list   - Список комнат\n"
            << "users  - Список пользователей\n"
            << "help   - Показать помощь\n"
            << "exit   - Завершить работу\n";
    }

    void handleConsoleInput() {
        string command;
        while (isRunning) {
            system("cls");
            cout << "=== СЕРВЕР КИНОТЕАТРА ===\n";
            printHelp();

            cout << "\nВведите команду: ";
            getline(cin, command);

            system("cls");

            if (command == "list") {
                lock_guard<mutex> lock(roomsMutex);
                cout << "Активные комнаты (" << rooms.size() << "):\n";
                for (const auto& [name, room] : rooms) {
                    cout << "- " << name << " (" << room.userCount() << "/10) - "
                        << room.filmTitle << " (" << room.filmDirector << ")\n";
                }
            }
            else if (command == "users") {
                lock_guard<mutex> lock(clientsMutex);
                cout << "Подключенные пользователи (" << connectedClients.size() << "):\n";

                lock_guard<mutex> roomLock(roomsMutex);
                for (const auto& [socket, ip] : connectedClients) {
                    auto roomIt = socketToRoom.find(socket);
                    if (roomIt != socketToRoom.end()) {
                        const Room& room = rooms.at(roomIt->second);
                        lock_guard<mutex> roomLock(room.roomMutex);
                        auto userIt = room.users.find(socket);
                        if (userIt != room.users.end()) {
                            const auto& [userIp, nick] = userIt->second;
                            cout << "- " << userIp << " : " << roomIt->second
                                << " <" << nick << ">\n";
                        }
                    }
                    else {
                        cout << "- " << ip << " : Не в комнате\n";
                    }
                }
            }
            else if (command == "help") {
                printHelp();
            }
            else if (command == "exit") {
                isRunning = false;
                cout << "Завершение работы сервера...\n";
                closesocket(serverSocket);
                break;
            }

            if (command != "exit") {
                cout << "\nНажмите Enter для продолжения...";
                cin.ignore();
            }
        }
    }

    string generateRoomName() {
        static const char chars[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";
        random_device rd;
        mt19937 gen(rd());
        uniform_int_distribution<> dist(0, 35);

        lock_guard<mutex> lock(roomsMutex);
        string name;
        do {
            name.clear();
            for (int i = 0; i < 6; ++i) name += chars[dist(gen)];
        } while (rooms.count(name));
        return name;
    }

    void sendMessage(SOCKET socket, const string& message) {
        send(socket, message.c_str(), static_cast<int>(message.size()), 0);
    }

    void broadcastParticipantsUpdate(Room& room) {
        vector<pair<string, string>> usersInfo;
        {
            lock_guard<mutex> lock(room.roomMutex);
            for (const auto& [sock, info] : room.users) {
                usersInfo.push_back(info);
            }
        }

        // УДАЛЕНО: сортировка по нику (причина проблемы #2)
        // sort(usersInfo.begin(), usersInfo.end(), ...);

        string updateMsg = "PARTICIPANTS_UPDATE";
        for (const auto& [ip, nick] : usersInfo) {
            // Отправляем только один ник (исправление проблемы #1)
            updateMsg += "|" + nick;
        }
        updateMsg += "\n";

        for (const auto& user : room.users) {
            sendMessage(user.first, updateMsg);
        }
    }


    void handleClient(SOCKET clientSocket) {
        char buffer[1024];
        string clientIP = getClientIP(clientSocket);

        {
            lock_guard<mutex> lock(clientsMutex);
            connectedClients[clientSocket] = clientIP;
        }

        Room* currentRoom = nullptr;
        string clientNick;
        enum class ClientState { MainMenu, ChoosingFilm, InRoom };
        ClientState currentState = ClientState::MainMenu;
        Film* selectedFilm = nullptr;

        cout << "Подключился: " << clientIP << endl;

        while (isRunning) {
            memset(buffer, 0, sizeof(buffer));
            int bytesReceived = recv(clientSocket, buffer, sizeof(buffer), 0);
            if (bytesReceived <= 0) break;

            string command(buffer, static_cast<size_t>(bytesReceived));

            command.erase(std::remove(command.begin(), command.end(), '\n'), command.end());
            command.erase(std::remove(command.begin(), command.end(), '\r'), command.end());

            // Разбиваем команду на части
            std::vector<std::string> parts;
            size_t start = 0;
            size_t end = command.find('|');
            while (end != std::string::npos) {
                parts.push_back(command.substr(start, end - start));
                start = end + 1;
                end = command.find('|', start);
            }
            parts.push_back(command.substr(start));

            if (parts.empty()) continue;

            const std::string& cmd = parts[0];

            if (cmd == "PLAYER_STATE") {
                if (parts.size() < 4) continue;

                string roomId = parts[1]; // Индекс 1 вместо 0
                string isPausedStr = parts[2];
                string positionStr = parts[3];

                if (currentRoom && currentRoom->name == roomId) {
                    string msg = "PLAYER_STATE|" + roomId + "|" +
                        clientNick + "|" + isPausedStr + "|" +
                        positionStr + "\n";

                    // Рассылаем ВСЕМ участникам комнаты включая отправителя
                    for (const auto& user : currentRoom->users) {
                        sendMessage(user.first, msg);
                    }
                }
            }
            else if (currentState == ClientState::MainMenu) {
                if (command == "CREATE_ROOM") {
                    /*
                    // Отправляем список фильмов
                    string filmsList = "FILMS_LIST";
                    for (const auto& film : films) {
                        filmsList += "|" + film.title + "|" + film.director;
                    }
                    sendMessage(clientSocket, filmsList + "\n");
                    */
                    int bytesReceived = recv(clientSocket, buffer, sizeof(buffer), 0);
                    if (bytesReceived <= 0) break;
                    int filmIndex = std::stoi(string(buffer, bytesReceived));

                    string roomName = generateRoomName();
                    {
                        lock_guard<mutex> lock(roomsMutex);
                        auto it = rooms.emplace(
                            piecewise_construct,
                            forward_as_tuple(roomName),
                            forward_as_tuple(roomName, films[filmIndex - 1], clientSocket) 
                        );

                        if (it.second) {
                            Room& newRoom = it.first->second;
                            auto [success, nick] = newRoom.addUser(clientSocket, clientIP);
                            if (success) {
                                clientNick = nick;
                                socketToRoom[clientSocket] = roomName;
                                currentRoom = &newRoom;
                                currentState = ClientState::InRoom;

                                string videoUrl = "VIDEO_URL|http://localhost:8000/" + films[filmIndex - 1].path + "\n";
                                sendMessage(clientSocket, "ROOM_CREATED|" + roomName + "\n");
                                sendMessage(clientSocket, videoUrl);
                                sendMessage(clientSocket, "TEMPORARY_PROFILE|" + nick + "\n");
                                broadcastParticipantsUpdate(newRoom);
                                cout << clientIP << " создал комнату " << roomName
                                    << " с фильмом " << films[filmIndex - 1].title
                                    << " как " << clientNick << endl;
                            }
                            else {
                                rooms.erase(roomName);
                                sendMessage(clientSocket, "Ошибка при создании комнаты!\n");
                            }
                        }
                        else {
                            sendMessage(clientSocket, "Ошибка при создании комнаты!\n");
                        }
                    }
                }
                else if (command == "GET_FILMS") {
                    string response = "FILMS_LIST";
                    sendMessage(clientSocket, response + "\n");
                    for (const auto& film : films) {
                        response += "|" + film.title + "|" + film.director;
                    }
                    sendMessage(clientSocket, response + "\n");
                    continue;
                }
                else if (command == "JOIN_ROOM") {
                    bytesReceived = recv(clientSocket, buffer, sizeof(buffer), 0);
                    if (bytesReceived <= 0) break;

                    string roomName(buffer, static_cast<size_t>(bytesReceived));
                    lock_guard<mutex> lock(roomsMutex);
                    auto it = rooms.find(roomName);
                    if (it != rooms.end()) {
                        auto [success, nick] = it->second.addUser(clientSocket, clientIP);
                        if (success) {
                            clientNick = nick;
                            currentRoom = &it->second;
                            socketToRoom[clientSocket] = roomName;
                            
                            string roomJoinedMsg = "ROOM_ENTERED|" + roomName + "\n";
                            sendMessage(clientSocket, roomJoinedMsg);

                            string videoUrl = "VIDEO_URL|http://localhost:8000/" + it->second.filmPath + "\n";
                            sendMessage(clientSocket, videoUrl);

                            sendMessage(clientSocket, "TEMPORARY_PROFILE|" + nick + "\n");

                            broadcastParticipantsUpdate(*currentRoom);
                            cout << clientIP << " присоединился к " << roomName
                                << " как " << clientNick << endl;
                            currentState = ClientState::InRoom;
                        }
                        else {
                            sendMessage(clientSocket, "Комната заполнена!\n");
                        }
                    }
                    else {
                        sendMessage(clientSocket, "Комната не найдена!\n");
                    }
                }
                else if (command == "GET_VIDEO") {
                    if (!currentRoom) {
                        sendMessage(clientSocket, "Вы не в комнате\n");
                        continue;
                    }

                    string videoPath;
                    {
                        lock_guard<mutex> lock(roomsMutex);
                        videoPath = "videos/" + currentRoom->filmTitle + ".mp4";
                    }

                    ifstream videoFile(videoPath, ios::binary);
                    if (!videoFile.is_open()) {
                        sendMessage(clientSocket, "Ошибка: Видео не найдено\n");
                        continue;
                    }

                    sendMessage(clientSocket, "START_VIDEO\n");

                    char buffer[4096];
                    while (videoFile.read(buffer, sizeof(buffer))) {
                        send(clientSocket, buffer, sizeof(buffer), 0);
                    }

                    if (videoFile.gcount() > 0) {
                        send(clientSocket, buffer, static_cast<int>(videoFile.gcount()), 0);
                    }

                    sendMessage(clientSocket, "END_VIDEO\n");
                    videoFile.close();
                }
            }
            else if (cmd == "REACTION") {
                if (parts.size() < 3) continue;

                // Исправлено: правильные индексы
                string roomId = parts[1];
                string reaction = parts[2];

                if (currentRoom && currentRoom->name == roomId) {
                    // Рассылаем реакцию всем участникам комнаты
                    string msg = "REACTION|" + clientNick + "|" + reaction + "\n";
                    for (const auto& user : currentRoom->users) {
                        sendMessage(user.first, msg);
                    }
                }
             }
            else if (currentState == ClientState::InRoom) {
                if (command == "LEAVE_ROOM") {
                    string roomName = currentRoom->name;
                    auto [removed, nowEmpty] = currentRoom->removeUser(clientSocket);
                    if (removed) {
                        socketToRoom.erase(clientSocket);
                        clientNick.clear();

                        // Сообщение в консоль о ручном выходе
                        cout << "[" << clientIP << "] Покинул комнату: " << roomName << endl;

                        // Отправка подтверждения клиенту
                        sendMessage(clientSocket, "LEFT_ROOM|" + roomName + "\n");

                        if (!nowEmpty) {
                            broadcastParticipantsUpdate(*currentRoom);
                        }

                        if (nowEmpty) {
                            rooms.erase(roomName);
                            cout << "Комната удалена (пуста): " << roomName << endl;
                        }
                    }
                    currentRoom = nullptr;
                    currentState = ClientState::MainMenu;
                }
            }
        }

        if (currentRoom) {
            string roomName = currentRoom->name;
            auto [removed, nowEmpty] = currentRoom->removeUser(clientSocket);
            if (removed) {
                socketToRoom.erase(clientSocket);

                // Выводим сообщение о разрыве соединения
                cout << "[" << clientIP << "] Разрыв соединения. Покинул комнату: " << roomName << endl;

                if (!nowEmpty) {
                    // Обновляем список участников для оставшихся пользователей
                    broadcastParticipantsUpdate(*currentRoom);
                }
                else {
                    // Удаляем пустую комнату
                    lock_guard<mutex> lock(roomsMutex);
                    rooms.erase(roomName);
                    cout << "Комната удалена (пуста): " << roomName << endl;
                }
            }
        }
        else {
            // Если пользователь не был в комнате
            cout << "Отключился: " << clientIP << endl;
        }

        {
            lock_guard<mutex> lock(clientsMutex);
            connectedClients.erase(clientSocket);
        }

        closesocket(clientSocket);

        cout << "Отключился: " << clientIP << endl;
    }

public:
    Server() {
        if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
            throw runtime_error("Ошибка инициализации Winsock");

        serverSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
        if (serverSocket == INVALID_SOCKET)
            throw runtime_error("Ошибка создания сокета");

        sockaddr_in serverAddr{};
        serverAddr.sin_family = AF_INET;
        serverAddr.sin_addr.s_addr = INADDR_ANY;
        serverAddr.sin_port = htons(8888);

        if (bind(serverSocket, (sockaddr*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR)
            throw runtime_error("Ошибка привязки");

        listen(serverSocket, SOMAXCONN);
        isRunning = true;
        loadFilms();
        cout << "Сервер запущен на порту 8888\n";
    }

    void start() {
        thread consoleThread(&Server::handleConsoleInput, this);
        while (isRunning) {
            SOCKET clientSocket = accept(serverSocket, nullptr, nullptr);
            if (clientSocket == INVALID_SOCKET) continue;
            thread(&Server::handleClient, this, clientSocket).detach();
        }
        consoleThread.join();
    }

    ~Server() {
        isRunning = false;
        closesocket(serverSocket);
        WSACleanup();
        cout << "Сервер остановлен\n";
    }
};

int main() {
    setlocale(LC_ALL, "Russian");
    try {
        Server server;
        server.start();
    }
    catch (const exception& e) {
        cerr << "ОШИБКА: " << e.what() << endl;
        return 1;
    }
    return 0;
}