// main

#include "utils.h"
#include <iostream>
#include <fstream>

int main(int argc, char *argv[]) {
    if (argc < 2) {
        help();
        return 1;
    }

    if (std::string(argv[1]) == "-h") {
        help();
    } else if (std::string(argv[1]) == "-t") {
        TestSuite();
    } else if (std::string(argv[1]) == "-s") {
        if (argc < 3) {
            std::cerr << "Error: Missing arguments" << std::endl;
            help();
            return 1;
        }
        std::string data = argv[2];
        uint8_t *hash = new uint8_t[16];
        MD5String(data, hash, true);
        delete[] hash;
    } else if (std::string(argv[1]) == "-S") {
        uint8_t *hash = new uint8_t[16];
        MD5Stdin(hash, true);
        delete[] hash;
    } else if (std::string(argv[1]) == "-v") {
        if (argc < 4) {
            std::cerr << "Error: Missing arguments" << std::endl;
            help();
            return 1;
        }

        std::string filename = argv[2];
        std::string md5file = argv[3];

        uint8_t *hash = new uint8_t[16];
        MD5File(filename, hash);

        std::ifstream file(md5file);
        if (!file) {
            std::cerr << "Error: Could not open file " << md5file << std::endl;
            delete[] hash;
            return 1;
        }
        std::string md5str;
        file >> md5str;
        uint8_t *md5hash = new uint8_t[16];
        for (int i = 0; i < 16; i++) {
            sscanf(md5str.substr(i * 2, 2).c_str(), "%2hhx", &md5hash[i]);
        }

        if (CompareMD5Hash(hash, md5hash)) {
            std::cout << "MD5 hash matched" << std::endl;
            std::cout << filename << ": Validated SUCCESS" << std::endl;
        } else {
            std::cout << "MD5 hash mismatched" << std::endl;
            std::cout << filename << ": Validated FAILED" << std::endl;
        }

        // Print MD5 hash
        std::cout << "Calculated MD5 (" << filename << ") = ";
        PrintMD5Hash(hash);
        std::cout << "Expected MD5 (" << md5file << ") = ";
        PrintMD5Hash(md5hash);

        delete[] hash;
        delete[] md5hash;
    } else if (std::string(argv[1]) == "-i") {
        if (argc < 3) {
            std::cerr << "Error: Missing arguments" << std::endl;
            help();
            return 1;
        }

        std::string filename = argv[2];

        uint8_t *hash = new uint8_t[16];
        MD5File(filename, hash);

        std::cout << "Enter MD5 hash: ";
        std::string md5str;
        std::cin >> md5str;
        while (md5str.length() != 32) {
            std::cerr << "Error: Invalid MD5 hash" << std::endl;
            std::cout << "Enter MD5 hash again: ";
            std::cin >> md5str;
        }
        uint8_t *md5hash = new uint8_t[16];
        for (int i = 0; i < 16; i++) {
            sscanf(md5str.substr(i * 2, 2).c_str(), "%2hhx", &md5hash[i]);
        }
        
        if (CompareMD5Hash(hash, md5hash)) {
            std::cout << "MD5 hash matched" << std::endl;
            std::cout << filename << ": Validated SUCCESS" << std::endl;
        } else {
            std::cout << "MD5 hash mismatched" << std::endl;
            std::cout << filename << ": Validated FAILED" << std::endl;
        }

        // Print MD5 hash
        std::cout << "Calculated MD5 (" << filename << ") = ";
        PrintMD5Hash(hash);
        std::cout << "Expected MD5 = ";
        PrintMD5Hash(md5hash);

        delete[] hash;
        delete[] md5hash;
    } else {
        std::string filename = argv[1];
        uint8_t *hash = new uint8_t[16];
        MD5File(filename, hash, true);
        delete[] hash;
    }

    return 0;
}
