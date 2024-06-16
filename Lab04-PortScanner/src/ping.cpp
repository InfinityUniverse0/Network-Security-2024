// ping

#include "ping.h"
#include <iostream>
#include <sys/socket.h>
#include <netinet/ip_icmp.h>
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


bool ping(const std::string &host_ip) {
    /*
     * ping - Send ICMP ECHO to Network Hosts
     * @param host_ip: IP address of the host or domain name
     */

    // Create socket
    int sockfd = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP);
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

    // ICMP packet
    icmp_hdr_t icmp;
    icmp.icmp_type = ICMP_ECHO;
    icmp.icmp_code = 0;
    icmp.icmp_id = getpid();  // Use PID as ICMP ID to identify the received ICMP packet
    icmp.icmp_seq = 0;
    icmp.icmp_cksum = 0;
    icmp.icmp_cksum = calcChecksum(&icmp, sizeof(icmp));

    // Send ICMP ECHO packet
    if (sendto(sockfd, &icmp, sizeof(icmp), 0, (sockaddr*)&addr, sizeof(addr)) <= 0) {
        std::cerr << "[Error] Failed to send ICMP packet: " << strerror(errno) << std::endl;
        close(sockfd);
        return false;
    }

    // Set socket to non-blocking mode
    if (fcntl(sockfd, F_SETFL, O_NONBLOCK) < 0) {
        std::cerr << "[Error] Failed to set socket to non-blocking mode: " << strerror(errno) << std::endl;
        close(sockfd);
        return false;
    }

    // Receive ICMP ECHO REPLY
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

            if (icmp_header->icmp_type == ICMP_ECHOREPLY &&
                icmp_header->icmp_id == icmp.icmp_id &&
                icmp_header->icmp_seq == icmp.icmp_seq &&
                recv_addr.sin_addr.s_addr == addr.sin_addr.s_addr) {
                std::cout << "Ping " << host_ip << " successful!" << std::endl;
                close(sockfd);
                return true;
            }
        }
        auto end = std::chrono::high_resolution_clock::now();
        auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
        if (elapsed > TIMEOUT_MS) {
            std::cout << "Ping " << host_ip << " failed!" << std::endl;
            close(sockfd);
            return false;  // Timeout
        }
    }

    std::cout << "Ping " << host_ip << " failed!" << std::endl;
    close(sockfd);
    return false;
}
