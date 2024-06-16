// Utils Functions

#ifndef PORTSCANNER_UTILS_H
#define PORTSCANNER_UTILS_H

#include <cstdint>

uint16_t calcChecksum(void* buf, int length);

bool checkPort(uint16_t port);

#endif //PORTSCANNER_UTILS_H
