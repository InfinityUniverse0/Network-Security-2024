// DES Operation
#ifndef DES_CHAT_DESOP_H
#define DES_CHAT_DESOP_H

#include <cstdint> // uint8_t
#include <random> // Generate random number

#define get_bit(x, i) (((x) >> (i)) & 0x01)

class DesOp {
private:
    uint8_t key[8]; // 64-bit key
    uint8_t subKeys[16][6]; // 16 48-bit subkeys
    static uint8_t IP[64]; // Initial Permutation
    static uint8_t IP_INV[64]; // Inverse Initial Permutation
    static uint8_t E[48]; // Expansion Permutation (E-Box)
    static uint8_t S[8][4][16]; // S-Boxes
    static uint8_t P[32]; // Permutation (P-Box)
    static uint8_t PC1[2][28]; // Permuted Choice 1
    static uint8_t LS[16]; // Left Shifts
    static uint8_t PC2[48]; // Permuted Choice 2
    void GenerateSubKeys();
    void F(uint8_t* R, uint8_t* subKey, uint8_t* result);
    void DES(uint8_t* plainText, uint8_t* cipherText, bool isEncrypt);

    // Useful functions
    static void Xor(uint8_t* a, uint8_t* b, int length);
    static void Copy(uint8_t* a, uint8_t* b, int length);
    static void ByteToBit(uint8_t* byte, uint8_t* bit, int length);
    static void BitToByte(uint8_t* bit, uint8_t* byte, int length);
public:
    DesOp();
    ~DesOp();
    void RandomGenKey(); // Generate a random key
    uint8_t* GetKey(); // Get key
    void SetKey(const char* key);
    void Encrypt(char* plainText, int plainTextLength, char*& cipherText, int& cipherTextLength);
    void Decrypt(char* cipherText, int cipherTextLength, char*& plainText, int& plainTextLength);
};

#endif //DES_CHAT_DESOP_H
