// TCP Connect Scan

#ifndef PORTSCANNER_TCP_CONNECT_H
#define PORTSCANNER_TCP_CONNECT_H

#include <cstdint>
#include <string>

bool tcpConnectScan(const std::string& host_ip, uint16_t port);

#endif //PORTSCANNER_TCP_CONNECT_H
