// TCP Connect Scan

#include "tcp_connect.h"
#include <iostream>
#include <string.h>
#include <sys/socket.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <netdb.h>

bool tcpConnectScan(const std::string& host_ip, const uint16_t port) {
    // Create socket
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        std::cerr << "[Error] Failed to create socket: " << strerror(errno) << std::endl;
        return false;
    }

    // Resolve the hostname to an IP address
    struct addrinfo hints, *result;
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;
    if (getaddrinfo(host_ip.c_str(), NULL, &hints, &result) != 0) {
        std::cerr << "[Error] Failed to resolve hostname" << std::endl;
        close(sockfd);
        return false;
    }

    // Set destination
    struct sockaddr_in dest;
    memset(&dest, 0, sizeof(dest));
    dest.sin_family = AF_INET;
    dest.sin_port = htons(port);
    // dest.sin_addr.s_addr = inet_addr(host_ip.c_str());
    dest.sin_addr = ((struct sockaddr_in*)result->ai_addr)->sin_addr;

    // Connect
    if (connect(sockfd, (struct sockaddr *)&dest, sizeof(dest)) != 0) {
        std::cout << "TCP Connect Scan " << host_ip << ":" << port << " Port closed" << std::endl;
        close(sockfd);
        return false;
    }

    std::cout << "TCP Connect Scan " << host_ip << ":" << port << " Port open" << std::endl;
    close(sockfd);
    return true;
}
