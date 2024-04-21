// Chat
#include "chat.h"
#include <iostream>
#include <cstring>

Chat::Chat() {
    Init();
#ifdef _WIN32
    // Initialize winsock
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        std::cerr << "Error: Failed to initialize winsock." << std::endl;
        return;
    }
#endif
}

Chat::~Chat() {
    Close();
#ifdef _WIN32
    // Cleanup winsock
    WSACleanup();
#endif
}

void Chat::Init() {
    serverSocket = -1;
    clientSocket = -1;
    serverIp = DEFAULT_SERVER_IP;
    serverPort = DEFAULT_SERVER_PORT;
    isRunning = false;
    exited = false;
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
    } else {
        std::cout << "Connected to server." << std::endl;
    }
}

void Chat::Send() {
    std::cin.getline(message, MAX_MESSAGE_LENGTH);
    char* cipherText = nullptr;
    int cipherTextLength = -1;

    // Exit command
    if (strcmp(message, EXIT_COMMAND) == 0) {
        isRunning = false;
        exited = true;
    }

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
            isRunning = false;
            if (!exited) {
                std::cerr << "Error: Failed to receive message or connection closed." << std::endl;
            }
            break;
        }
        plainText = nullptr;
        plainTextLength = -1;
        des.Decrypt(buffer, len, plainText, plainTextLength);

        // Exit command
        if (strcmp(plainText, EXIT_COMMAND) == 0) {
            isRunning = false;
            delete[] plainText;
            std::cout << info << " exited." << std::endl;
            break;
        }

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
    } else {
        std::cout << "Client connected." << std::endl;
    }

    /* ==================== Key Exchange Begin ==================== */
    // Generate RSA Keys
    const int N_RETRY = 3; // Number of retries for RSA key generation
    for (int i = 0; i < N_RETRY; i++) {
        if (rsa.GenerateKey()) {
            std::cout << "RSA key generated successfully." << std::endl;
            break;
        }

        if (i == N_RETRY - 1) {
            std::cerr << "Error: Failed to generate RSA key." << std::endl;
            std::cerr << "Server Exiting..." << std::endl;
            return;
        } else {
            std::cerr << "Warning: Failed to generate RSA key. Retrying..." << std::endl;
        }
    }

    // Send public key and modulus
    uint64_t e = rsa.GetPublicKey();
    uint64_t n = rsa.GetModulus();
    if (send(clientSocket, &e, sizeof(e), 0) < 0) {
        std::cerr << "Error: Failed to send public key." << std::endl;
        return;
    }
    if (send(clientSocket, &n, sizeof(n), 0) < 0) {
        std::cerr << "Error: Failed to send modulus." << std::endl;
        return;
    }

    // Receive DES key from client
    uint64_t desKey_enc[8];
    if (recv(clientSocket, desKey_enc, sizeof(desKey_enc), 0) < 0) {
        std::cerr << "Error: Failed to receive DES key." << std::endl;
        return;
    }

    // Decrypt DES key using RSA private key
    uint8_t desKey[8];
    for (int i = 0; i < 8; i++) {
        desKey[i] = rsa.Decrypt(desKey_enc[i]);
    }

    // Set DES key
    des.SetKey((char*)desKey);

    std::cout << "Key exchange completed." << std::endl;
    /* ===================== Key Exchange End ===================== */

    // Begin chat
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

    /* ==================== Key Exchange Begin ==================== */
    // Generate DES key
    des.RandomGenKey();
    uint8_t* desKey;
    desKey = des.GetKey();

    // Receive RSA public key and modulus from server
    uint64_t e, n;
    if (recv(clientSocket, &e, sizeof(e), 0) < 0) {
        std::cerr << "Error: Failed to receive public key." << std::endl;
        return;
    }
    if (recv(clientSocket, &n, sizeof(n), 0) < 0) {
        std::cerr << "Error: Failed to receive modulus." << std::endl;
        return;
    }

    // Encrypt DES key using RSA public key
    uint64_t desKey_enc[8];
    for (int i = 0; i < 8; i++) {
        desKey_enc[i] = RSA::Encrypt((uint32_t)desKey[i], e, n);
    }
    delete[] desKey; // Release memory

    // Send DES key to server
    if (send(clientSocket, desKey_enc, sizeof(desKey_enc), 0) < 0) {
        std::cerr << "Error: Failed to send DES key." << std::endl;
        return;
    }

    std::cout << "Key exchange completed." << std::endl;
    /* ===================== Key Exchange End ===================== */

    // Begin chat
    isRunning = true;
    receiveThread = std::thread(&Chat::ReceiveThread, this); // Create a new thread to receive messages

    while (isRunning) {
        Send();
    }

    Close();
}
