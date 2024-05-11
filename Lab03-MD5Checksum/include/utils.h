// Utils functions
#ifndef MD5_UTILS_H
#define MD5_UTILS_H

#include "md5.h"

#define BUFFER_SIZE 1024

// Print help info
void help();

// Run test suite
void TestSuite();

// Calculate MD5 of a file
void MD5File(const std::string &filename, uint8_t* &hash, bool verbose = false);

// Calculate MD5 of a string
void MD5String(const std::string &data, uint8_t* &hash, bool verbose = false);

// Calculate MD5 of a string from stdin
void MD5Stdin(uint8_t* &hash, bool verbose = false);

// Get MD5 hash as a string
std::string MD5HashStr(uint8_t* hash);

// Print MD5 hash in hex
void PrintMD5Hash(uint8_t* hash);

// Compare two MD5 hashes
bool CompareMD5Hash(const uint8_t* hash1, const uint8_t* hash2);

#endif //MD5_UTILS_H
