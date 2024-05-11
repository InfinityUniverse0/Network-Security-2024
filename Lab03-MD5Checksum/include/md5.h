// MD5
#ifndef MD5_MD5_H
#define MD5_MD5_H

#include <cstdint>
#include <string>
#include <cstring>

class MD5 {
    uint32_t state[4]; // state (ABCD)
    uint32_t count[2]; // number of bits, modulo 2^64 (LSB first)
    uint8_t buffer[64]; // input buffer
    uint8_t digest[16]; // message digest
    static uint8_t PADDING[64]; // padding for final block

public:
    MD5() = default;
    ~MD5() = default;

    void Init(); // MD5 initialization
    void Update(const uint8_t *data, size_t len); // MD5 block update operation
    void Final(); // MD5 finalization

    void GetHash(uint8_t* &hash);
    std::string GetHashStr();

private:
    void Transform(const uint8_t block[64]); // MD5 basic transformation
    static void Encode(const uint32_t *input, uint8_t *output, size_t length); // Encodes input (uint32_t) into output (uint8_t)
    static void Decode(const uint8_t *input, uint32_t *output, size_t length); // Decodes input (uint8_t) into output (uint32_t)
    static void MD5_memcpy(uint8_t *output, const uint8_t *input, size_t length); // Copies data from input to output
    static void MD5_memset(uint8_t *output, uint8_t value, size_t length); // Set output to value

};

#endif //MD5_MD5_H
