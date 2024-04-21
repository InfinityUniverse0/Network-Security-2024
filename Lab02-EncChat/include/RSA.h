// RSA
#ifndef ENCCHAT_RSA_H
#define ENCCHAT_RSA_H

#include <cstdint>
#include <random>

// Define 128-bit integer type (NOTE: May not be supported by all compilers)
#define uint128_t __uint128_t
#define int128_t __int128_t

class RSA {
private:
    uint64_t p; // Prime number p
    uint64_t q; // Prime number q
    uint64_t n; // n = p * q
    uint64_t phi; // Euler's totient function
    uint64_t e; // Public key e
    uint64_t d; // Private key d

    static uint64_t ModExp(uint64_t base, uint64_t exp, uint64_t mod); // Modular exponentiation
    static uint64_t ModInv(uint64_t a, uint64_t m); // Modular inverse
    static bool MillerRabin(uint64_t n, int round = 50); // Miller-Rabin primality test

public:
    RSA();
    ~RSA();
    bool GenerateKey(); // Generate public and private keys
    inline uint64_t GetPublicKey() {return e;}; // Get public key
    inline uint64_t GetModulus() {return n;}; // Get modulus
    static uint64_t Encrypt(uint32_t plainText, uint64_t e, uint64_t n); // Encrypt
    uint32_t Decrypt(uint64_t cipherText); // Decrypt
};

#endif //ENCCHAT_RSA_H
