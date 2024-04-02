// Chat
#include "chat.h"
#include <iostream>
#include <arpa/inet.h>
#include <unistd.h>

Chat::Chat() {
    Init();
}

Chat::~Chat() {
    Close();
}

void Chat::Init() {
    serverSocket = -1;
    clientSocket = -1;
    serverIp = DEFAULT_SERVER_IP;
    serverPort = DEFAULT_SERVER_PORT;
    isRunning = false;

    des.SetKey(KEY);
}

void Chat::Connect() {
    clientSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (clientSocket < 0) {
        std::cerr << "Error: Failed to create socket." << std::endl;
        return;
    }

    struct sockaddr_in serverAddr;
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(serverPort);
    serverAddr.sin_addr.s_addr = inet_addr(serverIp);

    if (connect(clientSocket, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) < 0) {
        std::cerr << "Error: Failed to connect to server." << std::endl;
        return;
    }
    isRunning = true;

    receiveThread = std::thread(&Chat::ReceiveThread, this); // Create a new thread to receive messages
}

void Chat::Send() {
    std::cin.getline(message, MAX_MESSAGE_LENGTH);
    char* cipherText = nullptr;
    int cipherTextLength = -1;
    des.Encrypt(message, strlen(message), cipherText, cipherTextLength);
    if (send(clientSocket, cipherText, cipherTextLength, 0) < 0) {
        std::cerr << "Error: Failed to send message." << std::endl;
        delete[] cipherText;
        return;
    }
    delete[] cipherText;
}

void Chat::ReceiveThread() {
    const char* info;
    if (isServer) {
        info = "Client";
    } else {
        info = "Server";
    }
    char* plainText = nullptr;
    int plainTextLength = -1;
    while (isRunning) {
        memset(buffer, 0, sizeof(buffer));
        ssize_t len = recv(clientSocket, buffer, sizeof(buffer), 0);
        if (len <= 0) {
            std::cerr << "Error: Failed to receive message or connection closed." << std::endl;
            isRunning = false;
            break;
        }
        plainText = nullptr;
        plainTextLength = -1;
        des.Decrypt(buffer, len, plainText, plainTextLength);
        std::cout << info << ": " << plainText << std::endl;
        delete[] plainText;
    }
}

void Chat::Close() {
    isRunning = false;
    if (serverSocket >= 0) {
        close(serverSocket);
    }
    if (clientSocket >= 0) {
        close(clientSocket);
    }
    if (receiveThread.joinable()) {
        receiveThread.join();
    }
}

void Chat::RunServer() {
    isServer = true;

    // Bind
    serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (serverSocket < 0) {
        std::cerr << "Error: Failed to create socket." << std::endl;
        return;
    }

    struct sockaddr_in serverAddr;
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(serverPort);
    serverAddr.sin_addr.s_addr = INADDR_ANY; // Accept any IP

    if (bind(serverSocket, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) < 0) {
        std::cerr << "Error: Failed to bind." << std::endl;
        return;
    }

    // Listen
    if (listen(serverSocket, 1) < 0) {
        std::cerr << "Error: Failed to listen." << std::endl;
        return;
    }

    // Accept
    struct sockaddr_in clientAddr;
    socklen_t clientAddrLen = sizeof(clientAddr);
    clientSocket = accept(serverSocket, (struct sockaddr*)&clientAddr, &clientAddrLen);
    if (clientSocket < 0) {
        std::cerr << "Error: Failed to accept." << std::endl;
        return;
    }

    isRunning = true;
    receiveThread = std::thread(&Chat::ReceiveThread, this); // Create a new thread to receive messages

    while (isRunning) {
        Send();
    }

    Close();
}

void Chat::RunClient() {
    isServer = false;
    Connect();
    while (isRunning) {
        Send();
    }
    Close();
}
