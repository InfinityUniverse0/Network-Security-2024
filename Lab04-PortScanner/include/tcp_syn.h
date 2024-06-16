// TCP SYN Scan

#ifndef PORTSCANNER_TCP_SYN_H
#define PORTSCANNER_TCP_SYN_H

#include <cstdint>
#include <string>

bool tcpSynScan(const std::string& host_ip, uint16_t port);

#endif //PORTSCANNER_TCP_SYN_H
