// Utils Functions

#include "utils.h"
#include "defs.h"

uint16_t calcChecksum(void* buf, int length) {
    auto* buffer = (uint16_t*)buf;
    uint64_t sum = 0;

    for (; length > 1; length -= 2) {
        sum += *buffer++;
    }

    if (length == 1) {
        sum += *(uint8_t *)buffer;
    }

    sum = (sum >> 16) + (sum & 0xFFFF);
    sum += (sum >> 16);
    return (uint16_t)(~sum);
}

bool checkPort(const uint16_t port) {
    return port >= MIN_PORT && port <= MAX_PORT;
}
