// TCP FIN Scan

#include "tcp_fin.h"
#include <iostream>
#include <string.h>
#include <chrono>
#include <sys/socket.h>
#include <netinet/tcp.h>
#include <netinet/ip.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <netdb.h>
#include <fcntl.h>
#include "defs.h"
#include "utils.h"

bool tcpFinScan(const std::string& host_ip, const uint16_t port) {
    // Create socket
    int sockfd = socket(AF_INET, SOCK_RAW, IPPROTO_TCP);
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

    // Create Pseudo Header & TCP Header
    auto* buffer = new uint8_t[sizeof(struct pseudo_header) + sizeof(struct tcphdr)];
    memset(buffer, 0, sizeof(struct pseudo_header) + sizeof(struct tcphdr));
    auto* pseudo_header = (struct pseudo_header*)buffer;
    auto* tcp_header = (struct tcphdr*)(buffer + sizeof(struct pseudo_header));

    // Get local info
    struct sockaddr_in local_addr;
    socklen_t local_addr_len = sizeof(local_addr);
    if (getsockname(sockfd, (struct sockaddr*)&local_addr, &local_addr_len) != 0) {
        std::cerr << "[Error] Failed to get local address: " << strerror(errno) << std::endl;
        close(sockfd);
        delete[] buffer;
        return false;
    }

    // Fill in the pseudo header
    pseudo_header->source_address = local_addr.sin_addr.s_addr;
    pseudo_header->dest_address = dest.sin_addr.s_addr;
    pseudo_header->placeholder = 0;
    pseudo_header->protocol = IPPROTO_TCP;
    pseudo_header->length = htons(sizeof(struct tcphdr));

    // Fill in the TCP Header
    tcp_header->th_sport = local_addr.sin_port;
    tcp_header->th_dport = dest.sin_port;
    tcp_header->th_seq = 0;
    tcp_header->th_ack = 0;
    tcp_header->th_x2 = 0;
    tcp_header->th_off = 5;
    tcp_header->th_flags = TH_FIN;  // FIN Scan
    tcp_header->th_win = htons(65535);
    tcp_header->th_sum = 0;
    tcp_header->th_urp = 0;

    // Calculate TCP Checksum
    tcp_header->th_sum = calcChecksum(buffer, sizeof(struct pseudo_header) + sizeof(struct tcphdr));

    // Send TCP FIN packet
    if (sendto(sockfd, tcp_header, sizeof(struct tcphdr), 0, (struct sockaddr*)&dest, sizeof(dest)) <= 0) {
        std::cerr << "[Error] Failed to send TCP FIN packet: " << strerror(errno) << std::endl;
        close(sockfd);
        delete[] buffer;
        return false;
    }

    // Set socket to non-blocking mode
    if (fcntl(sockfd, F_SETFL, O_NONBLOCK) < 0) {
        std::cerr << "[Error] Failed to set socket to non-blocking mode: " << strerror(errno) << std::endl;
        close(sockfd);
        delete[] buffer;
        return false;
    }

    // Receive TCP packet
    uint8_t recv_buffer[MAX_BUFFER_SIZE];
    struct sockaddr_in recv_addr;
    socklen_t addr_len = sizeof(recv_addr);
    ssize_t ret;
    auto start = std::chrono::high_resolution_clock::now();
    while (true) {
        ret = recvfrom(sockfd, recv_buffer, sizeof(recv_buffer), 0, (struct sockaddr*)&recv_addr, &addr_len);
        if (ret > 0) {
            auto* ip_hdr = (struct ip*)recv_buffer;
            int ip_hdr_len = ip_hdr->ip_hl << 2;
            auto* recv_tcp_header = (struct tcphdr*)(recv_buffer + ip_hdr_len);

            // Check if the received packet is a TCP RST packet
            if (recv_tcp_header->th_flags == TH_RST) { // RST
                std::cout << "TCP FIN Scan " << host_ip << ":" << port << " Port closed" << std::endl;
                close(sockfd);
                delete[] buffer;
                return false;
            } else {
                std::cout << "TCP FIN Scan " << host_ip << ":" << port << " Unknown response" << std::endl;
            }
        }

        auto end = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
        if (duration > TIMEOUT_MS) {
            std::cout << "TCP FIN Scan " << host_ip << ":" << port << " Port open" << std::endl;
            close(sockfd);
            delete[] buffer;
            return true;
        }
    }
}