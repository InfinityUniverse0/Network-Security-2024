// Chat
#ifndef DES_CHAT_CHAT_H
#define DES_CHAT_CHAT_H

#ifdef _WIN32
    #include <winsock2.h>
    #define close(s) closesocket(s)
    typedef SSIZE_T ssize_t;
    typedef int socklen_t;
#else
    #include <sys/socket.h> // socket
    #include <arpa/inet.h>
    #include <unistd.h>
#endif

#include <thread> // thread
#include <atomic> // atomic
#include "DesOp.h" // DES Operation

#define DEFAULT_SERVER_IP "127.0.0.1" // Default server IP
#define DEFAULT_SERVER_PORT 8888 // Default server port
#define MAX_MESSAGE_LENGTH 512 // Maximum message length
#define EXIT_COMMAND "exit" // Exit command
#define KEY "12345678" // Key

class Chat {
private:
    bool isServer; // Server flag
    int serverSocket; // Server socket
    int clientSocket; // Client socket
    const char* serverIp; // Server IP
    int serverPort; // Server port
    char message[MAX_MESSAGE_LENGTH]; // Message
    char buffer[MAX_MESSAGE_LENGTH]; // Buffer
    bool isRunning; // Running flag
    std::thread receiveThread; // Receive message thread object
    DesOp des; // DES Operation
    void Init(); // Initialize
    void Connect(); // Connect to server
    void Send(); // Send message
    void ReceiveThread(); // Receive message thread function
    void Close(); // Close

public:
    Chat(); // Constructor
    ~Chat(); // Destructor
    void RunServer(); // Run Server
    void RunClient(); // Run Client
};

#endif //DES_CHAT_CHAT_H
