// Main Script

#include <iostream>
#include <cstring>
#include <cstdint>
#include "ping.h"
#include "tcp_connect.h"
#include "tcp_syn.h"
#include "tcp_fin.h"
#include "udp.h"
#include "utils.h"

void help() {
    std::cout << "PortScanner: Scan a host for open ports" << std::endl;
    std::cout << "Usage:" << std::endl;
    std::cout << "    PortScanner [options] [hostname/IP] [port/port_min-port_max]" << std::endl;
    std::cout << "Options:" << std::endl;
    std::cout << "    -h: Show this help message" << std::endl;
    std::cout << "    -p <host>: Ping the host (IP or Domain Name)" << std::endl;
    std::cout << "    -c <host> <port>: TCP Connect Scan" << std::endl;
    std::cout << "    -s <host> <port>: TCP SYN Scan" << std::endl;
    std::cout << "    -f <host> <port>: TCP FIN Scan" << std::endl;
    std::cout << "    -u <host> <port>: UDP Scan" << std::endl;
}

int main(int argc, char* argv[]) {
    if (argc < 2) {
        help();
        return 1;
    }

    if (strcmp(argv[1], "-h") == 0) {
        help();
        return 0;
    }

    if (strcmp(argv[1], "-p") == 0) {
        if (argc < 3) {
            help();
            return 1;
        }
        ping(argv[2]);
        return 0;
    }

    if (argc < 4) {
        help();
        return 1;
    }

    std::string host_ip = argv[2];

    std::string port_range = argv[3];
    uint16_t port_begin, port_end;
    size_t dash_pos = port_range.find('-');
    if (dash_pos != std::string::npos) {
        port_begin = (uint16_t)(std::stoi(port_range.substr(0, dash_pos)));
        port_end = (uint16_t)(std::stoi(port_range.substr(dash_pos + 1)));
    } else {
        port_begin = (uint16_t)(std::stoi(port_range));
        port_end = port_begin;
    }

    if (port_begin > port_end || !checkPort(port_begin) || !checkPort(port_end)) {
        std::cerr << "[Error] Invalid port range" << std::endl;
        return 1;
    }

    if (!ping(host_ip)) {
        std::cerr << "[Error] Host is unreachable" << std::endl;
        return 1;
    }

    if (strcmp(argv[1], "-c") == 0) {
        for (uint16_t port = port_begin; port <= port_end; port++) {
            tcpConnectScan(host_ip, port);
        }
        return 0;
    } else if (strcmp(argv[1], "-s") == 0) {
        for (uint16_t port = port_begin; port <= port_end; port++) {
            tcpSynScan(host_ip, port);
        }
        return 0;
    } else if (strcmp(argv[1], "-f") == 0) {
        for (uint16_t port = port_begin; port <= port_end; port++) {
            tcpFinScan(host_ip, port);
        }
        return 0;
    } else if (strcmp(argv[1], "-u") == 0) {
        for (uint16_t port = port_begin; port <= port_end; port++) {
            udpScan(host_ip, port);
        }
        return 0;
    }

    help();
    return 0;
}
