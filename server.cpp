#include <iostream>
#include <string>
#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <random>
#include <thread>
#include <mutex>
#include <atomic>
#include <cstdlib>
#include <winsock2.h>
#include <ws2tcpip.h>

#pragma comment(lib, "ws2_32.lib")

class Room {
public:
    std::string name;
    std::unordered_set<std::string> users;
    std::mutex roomMutex;

    explicit Room(std::string name) : name(std::move(name)) {}

    bool addUser(const std::string& user) {
        std::lock_guard<std::mutex> lock(roomMutex);
        return users.insert(user).second;
    }

    bool removeUser(const std::string& user) {
        std::lock_guard<std::mutex> lock(roomMutex);
        return users.erase(user) > 0;
    }

    std::vector<std::string> getUsers() {
        std::lock_guard<std::mutex> lock(roomMutex);
        return { users.begin(), users.end() };
    }
};

class Server {
    WSADATA wsaData;
    SOCKET serverSocket = INVALID_SOCKET;
    std::atomic<bool> isRunning{ false };

    std::unordered_map<std::string, Room> rooms;
    std::unordered_set<std::string> connectedUsers;
    std::unordered_map<std::string, std::string> userRooms;
    std::mutex roomsMutex;
    std::mutex usersMutex;
    std::mutex userRoomsMutex;

    void printHelp() {
        std::cout << "��������� �������:\n"
            << "list   - ������ ������\n"
            << "users  - ������ �������������\n"
            << "help   - �������� ������\n"
            << "exit   - ��������� ������\n";
    }

    void handleConsoleInput() {
        std::string command;
        while (isRunning) {
            system("cls");
            std::cout << "=== ���������� �������� ===\n";
            printHelp();

            std::cout << "\n������� �������: ";
            std::getline(std::cin, command);

            system("cls");

            if (command == "list") {
                std::lock_guard<std::mutex> lock(roomsMutex);
                std::cout << "�������� ������� (" << rooms.size() << "):\n";
                for (const auto& [name, room] : rooms) {
                    std::cout << "- " << name << " (" << room.users.size() << " ����������)\n";
                }
            }
            else if (command == "users") {
                std::lock_guard<std::mutex> lockUsers(usersMutex);
                std::lock_guard<std::mutex> lockRooms(userRoomsMutex);
                std::cout << "������������ ������������ (" << connectedUsers.size() << "):\n";
                for (const auto& user : connectedUsers) {
                    auto it = userRooms.find(user);
                    std::cout << "- " << user << (it != userRooms.end() ? " : ������� " + it->second : " : �� � �������") << "\n";
                }
            }
            else if (command == "help") {
                printHelp();
            }
            else if (command == "exit") {
                isRunning = false;
                std::cout << "���������� ������ �������...\n";
                break;
            }

            if (command != "exit") {
                std::cout << "\n������� Enter ��� �����������...";
                std::cin.ignore();
            }
        }
    }

    std::string generateRoomName() {
        static const char chars[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_int_distribution<> dist(0, 35);

        std::lock_guard<std::mutex> lock(roomsMutex);
        std::string name;
        do {
            name.clear();
            for (int i = 0; i < 6; ++i) name += chars[dist(gen)];
        } while (rooms.count(name));
        return name;
    }

    void sendMessage(SOCKET socket, const std::string& message) {
        send(socket, message.c_str(), static_cast<int>(message.size()), 0);
    }

    void handleClient(SOCKET clientSocket) {
        char buffer[1024];
        std::string username;
        Room* currentRoom = nullptr;

        while (true) {
            sendMessage(clientSocket, "������� ���: ");
            int bytesReceived = recv(clientSocket, buffer, sizeof(buffer), 0);
            if (bytesReceived <= 0) {
                closesocket(clientSocket);
                return;
            }

            std::string name(buffer, static_cast<size_t>(bytesReceived));
            std::lock_guard<std::mutex> lock(usersMutex);
            if (!connectedUsers.count(name)) {
                connectedUsers.insert(name);
                username = name;
                break;
            }
            sendMessage(clientSocket, "��� ������! ���������� ������: ");
        }

        std::cout << "�����������: " << username << std::endl;

        while (isRunning) {
            std::string menu = currentRoom ?
                "\n���� �������:\n1. ���������\n2. ����\n3. �����\n4. �����\n�����: " :
                "\n������� ����:\n1. �������\n2. ��������������\n3. ������\n4. �����\n�����: ";

            sendMessage(clientSocket, menu);

            memset(buffer, 0, sizeof(buffer));
            int bytesReceived = recv(clientSocket, buffer, sizeof(buffer), 0);
            if (bytesReceived <= 0) break;

            std::string command(buffer, static_cast<size_t>(bytesReceived));

            if (command == "1") {
                if (!currentRoom) {
                    std::string roomName = generateRoomName();
                    {
                        std::lock_guard<std::mutex> lock(roomsMutex);
                        rooms.emplace(
                            std::piecewise_construct,
                            std::forward_as_tuple(roomName),
                            std::forward_as_tuple(roomName)
                        );
                        rooms.at(roomName).addUser(username);
                        std::lock_guard<std::mutex> lockUR(userRoomsMutex);
                        userRooms[username] = roomName;
                    }
                    currentRoom = &rooms.at(roomName);
                    sendMessage(clientSocket, "������� �������: " + roomName + "\n");
                    std::cout << "������������ " << username << " ������ ������� " << roomName << std::endl;
                }
                else {
                    auto members = currentRoom->getUsers();
                    std::string msg = "���������:\n";
                    for (const auto& m : members) msg += m + "\n";
                    sendMessage(clientSocket, msg);
                }
            }
            else if (command == "2") {
                if (!currentRoom) {
                    sendMessage(clientSocket, "������� ��� �������: ");
                    bytesReceived = recv(clientSocket, buffer, sizeof(buffer), 0);
                    if (bytesReceived <= 0) break;

                    std::string roomName(buffer, static_cast<size_t>(bytesReceived));
                    std::lock_guard<std::mutex> lock(roomsMutex);
                    auto it = rooms.find(roomName);
                    if (it != rooms.end() && it->second.addUser(username)) {
                        currentRoom = &it->second;
                        {
                            std::lock_guard<std::mutex> lockUR(userRoomsMutex);
                            userRooms[username] = roomName;
                        }
                        sendMessage(clientSocket, "�������������� �: " + roomName + "\n");
                        std::cout << "������������ " << username << " ������������� � " << roomName << std::endl;
                    }
                    else {
                        sendMessage(clientSocket, "������ �����������!\n");
                    }
                }
                else {
                    sendMessage(clientSocket, "�������: " + currentRoom->name + "\n");
                }
            }
            else if (command == "3") {
                if (!currentRoom) {
                    std::lock_guard<std::mutex> lock(roomsMutex);
                    std::string list = "��������� �������:\n";
                    for (const auto& pair : rooms)
                        list += "- " + pair.first + " (" + std::to_string(pair.second.users.size()) + ")\n";
                    sendMessage(clientSocket, list.empty() ? "��� �������� ������\n" : list);
                }
                else {
                    currentRoom->removeUser(username);
                    {
                        std::lock_guard<std::mutex> lockUR(userRoomsMutex);
                        userRooms.erase(username);
                    }
                    sendMessage(clientSocket, "����� ��: " + currentRoom->name + "\n");
                    currentRoom = nullptr;
                    std::cout << "������������ " << username << " ������� �������\n";
                }
            }
            else if (command == "4") {
                sendMessage(clientSocket, "�� ��������!\n");
                break;
            }
            else sendMessage(clientSocket, "����������� �������!\n");
        }

        if (currentRoom) {
            currentRoom->removeUser(username);
            std::lock_guard<std::mutex> lockUR(userRoomsMutex);
            userRooms.erase(username);
        }
        {
            std::lock_guard<std::mutex> lock(usersMutex);
            connectedUsers.erase(username);
        }
        closesocket(clientSocket);
        std::cout << "����������: " << username << std::endl;
    }

public:
    Server() {
        if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
            throw std::runtime_error("������ ������������� Winsock");

        serverSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
        if (serverSocket == INVALID_SOCKET)
            throw std::runtime_error("������ �������� ������");

        sockaddr_in serverAddr{};
        serverAddr.sin_family = AF_INET;
        serverAddr.sin_addr.s_addr = INADDR_ANY;
        serverAddr.sin_port = htons(8888);

        if (bind(serverSocket, (sockaddr*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR)
            throw std::runtime_error("������ ��������");

        listen(serverSocket, SOMAXCONN);
        isRunning = true;
        std::cout << "������ ������� �� ����� 8888\n";
    }

    void start() {
        std::thread consoleThread(&Server::handleConsoleInput, this);
        while (isRunning) {
            SOCKET clientSocket = accept(serverSocket, nullptr, nullptr);
            if (clientSocket == INVALID_SOCKET) continue;
            std::thread(&Server::handleClient, this, clientSocket).detach();
        }
        consoleThread.join();
    }

    ~Server() {
        isRunning = false;
        closesocket(serverSocket);
        WSACleanup();
        std::cout << "������ ����������\n";
    }
};

int main() {
    setlocale(LC_ALL, "Russian");

    
    try {
        Server server;
        server.start();
    }
    catch (const std::exception& e) {
        std::cerr << "������: " << e.what() << std::endl;
        return 1;
    }


    return 0;
}