// Defines

#ifndef PORTSCANNER_DEFS_H
#define PORTSCANNER_DEFS_H

#include <cstdint>

#define TIMEOUT_MS 3000  // Timeout in Milliseconds

#define MAX_BUFFER_SIZE 1024  // Maximum Buffer Size

#define MIN_PORT 1       // Minimum Port Number
#define MAX_PORT 65535   // Maximum Port Number

struct pseudo_header
{
    uint32_t source_address;
    uint32_t dest_address;
    uint8_t placeholder;
    uint8_t protocol;
    uint16_t length;
};

#endif //PORTSCANNER_DEFS_H
