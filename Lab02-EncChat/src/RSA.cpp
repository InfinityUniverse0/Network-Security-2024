#include "RSA.h"
#include <cassert>

uint64_t RSA::ModExp(uint64_t base, uint64_t exp, uint64_t mod) {
    // Use the square-and-multiply algorithm
    base = base % mod;
    uint64_t idx = (1LL << 63);
    while (!(exp & idx)) {
        idx >>= 1;
    }

    uint128_t result = 1; // In case of overflow
    while (idx) {
        result = (uint128_t)((result * result) % mod);
        if (exp & idx) {
            result = (uint128_t)((result * base) % mod);
        }
        idx >>= 1;
    }

    return (uint64_t)result;
}

uint64_t RSA::ModInv(uint64_t a, uint64_t m) {
    // Use the extended Euclidean algorithm
    /*
     * The extended Euclidean algorithm finds the modular multiplicative inverse of a modulo m.
     * If a and m are coprime, then a^(-1) mod m exists and the function returns the modular multiplicative inverse of a modulo m.
     * If a and m are not coprime, then a^(-1) mod m does not exist and the function returns 0.
     */

    assert(a < m); // Ensure a < m

    // NOTE: Use signed integer for that the result may be negative; use 128-bit integer to avoid overflow
    int128_t r0 = m, r = a; // Remainder sequence
    int128_t q = -1; // Quotient sequence
    int128_t s0 = 1, s = 0;
    int128_t t0 = 0, t = 1;

    while (r0 % r) { // If the remainder is not 0, continue the loop
        int128_t tmp = r0;
        r0 = r;
        r = tmp % r0;

        q = tmp / r0;

        tmp = s0;
        s0 = s;
        s = tmp - s0 * q;

        tmp = t0;
        t0 = t;
        t = tmp - t0 * q;
    }

    if (r == 1) { // If a and m are coprime, then a^(-1) mod m exists
        // Ensure the result is positive
        if (s < 0)
            s += a;
        if (t < 0)
            t += m;
        return t;
    }

    return 0; // a and m are not coprime, then a^(-1) mod m does not exist
}

bool RSA::MillerRabin(uint64_t n, int round) {
    if (n == 2) return true;
    if (n < 2 || (n & 1) == 0) return false;

    // Write n as 2^k * m + 1
    uint64_t m = n - 1;
    uint64_t k = 0;
    while ((m & 1) == 0) {
        m >>= 1;
        k++;
    }

    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<uint64_t> dis(2, n - 1);

    while (round--) { // Loop `round` times
        uint64_t a = dis(gen); // random init: [2, n-1]
        uint64_t b = ModExp(a, m, n);
        if (b == 1 || b == n - 1) continue;

        for (int i = 0; i < k; i++) {
            b = ModExp(b, 2, n);
            if ((b == n - 1) && (i < k - 1)) {
                b = 1;
                break;
            }
            if (b == 1) return false;
        }

        if (b != 1) return false; // Failed Fermat test
    }
    return true;
}


RSA::RSA() {
    p = 0; q = 0; n = 0; phi = 0; e = 0; d = 0;
}

RSA::~RSA() = default;

bool RSA::GenerateKey() {
    // Generate prime numbers p and q
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<uint64_t> dis(0x20000000, 0xFFFFFFFF); // [2^29, 2^32-1]

    const int MAX_ROUND = 50;
    do {
        p = dis(gen);
    } while (!MillerRabin(p, MAX_ROUND));

    do {
        q = dis(gen);
    } while (!MillerRabin(q, MAX_ROUND));

    n = p * q;
    phi = (p - 1) * (q - 1);

    // Generate public key e
    /*
     * The public key e must be coprime with phi(n) and 1 < e < phi(n).
     * Because phi(n) is even, e is odd.
     * The public key e is commonly set to 65537 (2^16 + 1) because it is prime and has a short binary representation with only two bits set.
     */
    e = 65537; // Commonly used public key
    while (e < phi) {
        // if ((d = ModInv(e, phi))) { break;}

        /*
         * Below we use a more efficient way to find e
         * Here we let e:
         * 1. e is odd
         * 2. e is not a factor of phi
         * 3. e is prime
         * NOTE: This is a more strict condition than the original one
         * You can also use the original one to find e with a higher complexity by using the above code
         */
        if ((phi % e != 0) && (MillerRabin(e, MAX_ROUND))) {
            break;
        }
        e += 2; // Ensure e is odd
    }

    if (e >= phi) {
        // Failed to find e
        // Reset the key
        p = 0; q = 0; n = 0; phi = 0; e = 0; d = 0;
        return false;
    }

    d = ModInv(e, phi);
    return true;
}

uint64_t RSA::Encrypt(uint32_t plainText, uint64_t e, uint64_t n) {
    return ModExp((uint64_t)plainText, e, n);
}

uint32_t RSA::Decrypt(uint64_t cipherText) {
    return (uint32_t)ModExp(cipherText, d, n);
}
