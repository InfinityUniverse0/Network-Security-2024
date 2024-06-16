// TCP FIN Scan

#ifndef PORTSCANNER_TCP_FIN_H
#define PORTSCANNER_TCP_FIN_H

#include <cstdint>
#include <string>

bool tcpFinScan(const std::string& host_ip, uint16_t port);

#endif //PORTSCANNER_TCP_FIN_H
