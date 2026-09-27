#ifndef BLUECRYPT_H
#define BLUECRYPT_H

#include <Arduino.h>
#include <avr/pgmspace.h>

#define BC_MAX_SLOTS 128
#define BC_DEFAULT_SLOTS 128
#define BC_SBOX_SIZE 256
#define BC_HMAC_SIZE 2

enum BlueCryptStatus {
    BC_SUCCESS = 0,
    BC_INVALID_SLOT_COUNT,
    BC_HMAC_MISMATCH,
    BC_BUFFER_OVERFLOW
};

class BlueCryptClass {
private:
    uint8_t _pipeline[BC_MAX_SLOTS];
    uint8_t _activeSlotCount;
    uint8_t _customSBox[BC_SBOX_SIZE];
    bool _useCustomSBox;
    uint16_t _hmacKey;

    uint8_t substituteByte(uint8_t inputVal);
    uint8_t reverseSubstituteByte(uint8_t inputVal);
    uint16_t calculateHMAC(const uint8_t* data, size_t length);

public:
    BlueCryptClass();

    // 1. Initializer: BlueCrypt.begin('A');
    void begin(uint8_t seedKey = 'A');

    // 2. Dynamic Pipeline Config: BlueCrypt.module(modules);
    void module(const uint8_t* slotList, size_t count);

    template<size_t N>
    void module(const uint8_t (&slotList)[N]) {
        module(slotList, N);
    }

    // 3. Optional S-Box Table
    void table(const uint8_t* customTable);
    void resetTable();

    // Raw byte buffer methods
    size_t encrypt(const uint8_t* input, size_t inputLen, uint8_t* output);
    BlueCryptStatus decrypt(const uint8_t* input, size_t inputLen, uint8_t* output, size_t* outputLen);

    // 4 & 5. Simplified One-Line String & Buffer Overloads
    String encrypt(const String& msg);
    String decrypt(const String& cipherText);
    size_t encrypt(uint8_t* buffer, size_t len);
};

// ==========================================================
// OPTION A KEY: Pre-instantiates global 'BlueCrypt' object
// ==========================================================
extern BlueCryptClass BlueCrypt;

#endif // BLUECRYPT_H
