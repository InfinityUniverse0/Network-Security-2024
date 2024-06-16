// UDP Scan

#ifndef PORTSCANNER_UDP_H
#define PORTSCANNER_UDP_H

#include <cstdint>
#include <string>

bool udpScan(const std::string& host_ip, uint16_t port);

#endif //PORTSCANNER_UDP_H
