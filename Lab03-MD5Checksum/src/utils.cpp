// Utils functions
#include "utils.h"
#include <iostream>
#include <iomanip>
#include <fstream>

void help() {
    std::cout << "Usage:" << "  MD5 [OPTION] [FILE]" << std::endl;
    std::cout << "  [-h]" << std::endl;
    std::cout << "      Display this help message." << std::endl;
    std::cout << "  [-t]" << std::endl;
    std::cout << "      Run test suite." << std::endl;
    std::cout << "  <FILE>" << std::endl;
    std::cout << "      Calculate MD5 hash of a file." << std::endl;
    std::cout << "  [-s] <STRING>" << std::endl;
    std::cout << "      Calculate MD5 hash of a string." << std::endl;
    std::cout << "  [-S]" << std::endl;
    std::cout << "      Calculate MD5 hash of a string from stdin."
              << " Use Ctrl+D (in Unix-like OS) or Ctrl+Z (in Windows) to end input" << std::endl;
    std::cout << "  [-v] <FILE> <MD5FILE>" << std::endl;
    std::cout << "      Validate the integrity of a given file by the MD5 hash from .md5 file <MD5FILE>" << std::endl;
    std::cout << "  [-i] <FILE>" << std::endl;
    std::cout << "      Validate the integrity of a given file by user input MD5 hash" << std::endl;
}

void TestSuite() {
    std::cout << "MD5 Test Suite:" << std::endl;
    uint8_t *hash = new uint8_t[16];

    MD5String("", hash, true);
    MD5String("a", hash, true);
    MD5String("abc", hash, true);
    MD5String("message digest", hash, true);
    MD5String("abcdefghijklmnopqrstuvwxyz", hash, true);
    MD5String("ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789", hash, true);
    MD5String("12345678901234567890123456789012345678901234567890123456789012345678901234567890", hash, true);

    delete[] hash;
}

void MD5File(const std::string &filename, uint8_t *&hash, bool verbose) {
    std::ifstream file(filename, std::ios::binary);
    if (!file) {
        std::cerr << "Error: Could not open file " << filename << std::endl;
        return;
    }

    MD5 md5;
    md5.Init();
    uint8_t buffer[BUFFER_SIZE];
    while (file.good()) {
        file.read((char *)buffer, BUFFER_SIZE);
        md5.Update(buffer, file.gcount());
    }
    md5.Final();
    md5.GetHash(hash);

    if (verbose) {
        std::cout << "MD5 (" << filename << ") = ";
        PrintMD5Hash(hash);
    }
}

void MD5String(const std::string &data, uint8_t *&hash, bool verbose) {
    MD5 md5;
    md5.Init();
    md5.Update((uint8_t *)data.c_str(), data.length());
    md5.Final();
    md5.GetHash(hash);

    if (verbose) {
        std::cout << "MD5 (\"" << data << "\") = ";
        PrintMD5Hash(hash);
    }
}

void MD5Stdin(uint8_t *&hash, bool verbose) {
    MD5 md5;
    md5.Init();
    uint8_t buffer[BUFFER_SIZE];
    while (std::cin.good()) { // Ctrl+D (in Unix-like OS) or Ctrl+Z (in Windows) to end input
        std::cin.read((char *)buffer, BUFFER_SIZE);
        md5.Update(buffer, std::cin.gcount());
    }
    md5.Final();
    md5.GetHash(hash);

    if (verbose) {
        std::cout << "MD5 = ";
        PrintMD5Hash(hash);
    }
}

std::string MD5HashStr(uint8_t *hash) {
    std::string str;
    str.reserve(32);

    for (size_t i = 0; i < 16; i++) {
        str.push_back("0123456789abcdef"[hash[i] >> 4]);
        str.push_back("0123456789abcdef"[hash[i] & 0x0f]);
    }

    return str;
}

void PrintMD5Hash(uint8_t *hash) {
    for (int i = 0; i < 16; i++) {
        std::cout << std::hex << std::setfill('0') << std::setw(2) << static_cast<int>(hash[i]);
    }
    // Reset to default
    std::cout << std::dec << std::setfill(' ') << std::setw(0) << std::endl;
}

bool CompareMD5Hash(const uint8_t *hash1, const uint8_t *hash2) {
    for (int i = 0; i < 16; i++) {
        if (hash1[i] != hash2[i]) {
            return false;
        }
    }
    return true;
}
