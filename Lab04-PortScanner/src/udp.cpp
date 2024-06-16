// UDP Scan

#include "udp.h"
#include <iostream>
#include <string.h>
#include <chrono>
#include <sys/socket.h>
#include <netinet/ip_icmp.h>
#include <netinet/ip.h>
#include <netinet/udp.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <fcntl.h>
#include <netdb.h>
#include "defs.h"
#include "utils.h"

#ifdef __APPLE__
    // On macOS, use icmp
    typedef struct icmp icmp_hdr_t;
#elif __linux__
    // On Linux, use icmphdr
    typedef struct icmphdr icmp_hdr_t;
#else
    #error "Unknown platform"
#endif

bool udpScan(const std::string& host_ip, const uint16_t port) {
    // Create socket
    int sockfd = socket(AF_INET, SOCK_RAW, IPPROTO_UDP);
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
    sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = 0;
    // addr.sin_addr.s_addr = inet_addr(host_ip.c_str());
    addr.sin_addr = ((struct sockaddr_in*)result->ai_addr)->sin_addr;

//    // Set IP_HDRINCL to 1 (on)
//    int on = 1;
//    if (setsockopt(sockfd, IPPROTO_IP, IP_HDRINCL, &on, sizeof(on)) < 0) {
//        std::cerr << "[Error] Failed to set IP_HDRINCL: " << strerror(errno) << std::endl;
//        close(sockfd);
//        return false;
//    }

//    // Create IP Header & UDP Header
//    auto* buffer = new uint8_t[sizeof(struct ip) + sizeof(struct udphdr)];

    // Create Pseudo Header & UDP Header
    auto* buffer = new uint8_t[sizeof(struct pseudo_header) + sizeof(struct udphdr)];
    memset(buffer, 0, sizeof(struct pseudo_header) + sizeof(struct udphdr));
    auto* pseudo_header = (struct pseudo_header*)buffer;
    auto* udp_header = (struct udphdr*)(buffer + sizeof(struct pseudo_header));

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
    pseudo_header->dest_address = addr.sin_addr.s_addr;
    pseudo_header->placeholder = 0;
    pseudo_header->protocol = IPPROTO_UDP;
    pseudo_header->length = htons(sizeof(struct udphdr));

    // Fill in the UDP header
    udp_header->uh_sport = local_addr.sin_port;
    udp_header->uh_dport = addr.sin_port;
    udp_header->uh_ulen = htons(sizeof(struct udphdr));
    udp_header->uh_sum = 0;

    // Calculate UDP Checksum
    udp_header->uh_sum = calcChecksum(buffer, sizeof(struct pseudo_header) + sizeof(struct udphdr));

    // Send UDP packet
    if (sendto(sockfd, udp_header, sizeof(struct udphdr), 0, (sockaddr*)&addr, sizeof(addr)) <= 0) {
        std::cerr << "[Error] Failed to send UDP packet: " << strerror(errno) << std::endl;
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

    // Receive ICMP packet
    uint8_t recv_buffer[MAX_BUFFER_SIZE];
    sockaddr_in recv_addr;
    socklen_t addr_len = sizeof(recv_addr);
    ssize_t ret;
    auto start = std::chrono::high_resolution_clock::now();
    while (true) {
        ret = recvfrom(sockfd, recv_buffer, sizeof(recv_buffer), 0, (sockaddr*)&recv_addr, &addr_len);
        if (ret > 0) {
            auto* ip_hdr = (struct ip*)recv_buffer;
            int ip_hdr_len = ip_hdr->ip_hl << 2;
            auto* icmp_header = (icmp_hdr_t*)(recv_buffer + ip_hdr_len);

            // Check if ICMP packet is an ICMP Port Unreachable
            #ifdef __APPLE__
            if (icmp_header->icmp_type == ICMP_UNREACH && icmp_header->icmp_code == ICMP_UNREACH_PORT) {
                std::cout << "UDP Scan " << host_ip << ":" << port << " Port closed" << std::endl;
                close(sockfd);
                delete[] buffer;
                return false;
            } else {
                std::cout << "UDP Scan " << host_ip << ":" << port << " Unknown response" << std::endl;
            }
            #elif __linux__
            if (icmp_header->type == ICMP_DEST_UNREACH && icmp_header->code == ICMP_PORT_UNREACH) {
                std::cout << "UDP Scan " << host_ip << ":" << port << " Port closed" << std::endl;
                close(sockfd);
                delete[] buffer;
                return false;
            } else {
                std::cout << "UDP Scan " << host_ip << ":" << port << " Unknown response" << std::endl;
            }
            #endif
        }

        auto end = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
        if (duration > TIMEOUT_MS) {
            std::cout << "UDP Scan " << host_ip << ":" << port << " Port open (May be lost)" << std::endl;
            close(sockfd);
            delete[] buffer;
            return true;
        }
    }
}
