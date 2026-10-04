#include "BlueCrypt.h"
#include <string.h>
#include <stdlib.h>

// Instantiate Option A global object instance
BlueCryptClass BlueCrypt;

// Hardened 256-byte Non-Linear Substitution Box stored in Flash Memory (0 SRAM Used)
const uint8_t DEFAULT_SBOX_FLASH[BC_SBOX_SIZE] PROGMEM = {
    0x63, 0x7c, 0x77, 0x7b, 0xf2, 0x6b, 0x6f, 0xc5, 0x30, 0x01, 0x67, 0x2b, 0xfe, 0xd7, 0xab, 0x76,
    0xca, 0x82, 0xc9, 0x7d, 0xfa, 0x59, 0x47, 0xf0, 0xad, 0xd4, 0xa2, 0xaf, 0x9c, 0xa4, 0x72, 0xc0,
    0xb7, 0xfd, 0x93, 0x26, 0x36, 0x3f, 0xf7, 0xcc, 0x34, 0xa5, 0xe5, 0xf1, 0x71, 0xd8, 0x31, 0x15,
    0x04, 0xc7, 0x23, 0xc3, 0x18, 0x96, 0x05, 0x9a, 0x07, 0x12, 0x80, 0xe2, 0xeb, 0x27, 0xb2, 0x75,
    0x09, 0x83, 0x2c, 0x1a, 0x1b, 0x6e, 0x5a, 0xa0, 0x52, 0x3b, 0xd6, 0xb3, 0x29, 0xe3, 0x2f, 0x84,
    0x53, 0xd1, 0x00, 0xed, 0x20, 0xfc, 0xb1, 0x5b, 0x6a, 0xcb, 0xbe, 0x39, 0x4a, 0x4c, 0x58, 0xcf,
    0xd0, 0xef, 0xaa, 0xfb, 0x43, 0x4d, 0x33, 0x85, 0x45, 0xf9, 0x02, 0x7f, 0x50, 0x3c, 0x9f, 0xa8,
    0x51, 0xa3, 0x40, 0x8f, 0x92, 0x9d, 0x38, 0xf5, 0xbc, 0xb6, 0xda, 0x21, 0x10, 0xff, 0xf3, 0xd2,
    0xcd, 0x0c, 0x13, 0xec, 0x5f, 0x97, 0x44, 0x17, 0xc4, 0xa7, 0x7e, 0x3d, 0x64, 0x5d, 0x19, 0x73,
    0x60, 0x81, 0x4f, 0xdc, 0x22, 0x2a, 0x90, 0x88, 0x46, 0xee, 0xb8, 0x14, 0xde, 0x5e, 0x0b, 0xdb,
    0xe0, 0x32, 0x3a, 0x0a, 0x49, 0x06, 0x24, 0x5c, 0xc2, 0xd3, 0xac, 0x62, 0x91, 0x95, 0xe4, 0x79,
    0xe7, 0xc8, 0x37, 0x6d, 0x8d, 0xd5, 0x4e, 0xa9, 0x6c, 0x56, 0xf4, 0xea, 0x65, 0x7a, 0xae, 0x08,
    0xba, 0x78, 0x25, 0x2e, 0x1c, 0xa6, 0xb4, 0xc6, 0xe8, 0xdd, 0x74, 0x1f, 0x4b, 0xbd, 0x8b, 0x8a,
    0x70, 0x3e, 0xb5, 0x66, 0x48, 0x03, 0xf6, 0x0e, 0x61, 0x35, 0x57, 0xb9, 0x86, 0xc1, 0x1d, 0x9e,
    0xe1, 0xf8, 0x98, 0x11, 0x69, 0xd9, 0x8e, 0x94, 0x9b, 0x1e, 0x87, 0xe9, 0xce, 0x55, 0x28, 0xdf,
    0x8c, 0xa1, 0x89, 0x0d, 0xbf, 0xe6, 0x42, 0x68, 0x41, 0x99, 0x2d, 0x0f, 0xb0, 0x54, 0xbb, 0x16
};

// Constructor
BlueCryptClass::BlueCryptClass() {
    _activeSlotCount = BC_DEFAULT_SLOTS;
    _useCustomSBox = false;
    _hmacKey = 0xABCD;
}

// Initializer method
void BlueCryptClass::begin(uint8_t seedKey) {
    uint8_t currentKey = seedKey;
    _activeSlotCount = BC_DEFAULT_SLOTS;
    for (uint8_t i = 0; i < _activeSlotCount; i++) {
        currentKey = (currentKey * 33) + i;
        _pipeline[i] = currentKey;
    }
}

// Pipeline slot array initializer
void BlueCryptClass::module(const uint8_t* slotList, size_t count) {
    _activeSlotCount = (count > BC_MAX_SLOTS) ? BC_MAX_SLOTS : count;
    for (size_t i = 0; i < _activeSlotCount; i++) {
        _pipeline[i] = slotList[i];
    }
}

// Custom S-Box table loader
void BlueCryptClass::table(const uint8_t* customTable) {
    memcpy(_customSBox, customTable, BC_SBOX_SIZE);
    _useCustomSBox = true;
}

// Flash PROGMEM reset
void BlueCryptClass::resetTable() {
    _useCustomSBox = false;
}

// Substitution lookup
uint8_t BlueCryptClass::substituteByte(uint8_t inputVal) {
    if (_useCustomSBox) {
        return _customSBox[inputVal];
    } else {
        return pgm_read_byte(&(DEFAULT_SBOX_FLASH[inputVal]));
    }
}

// Reverse substitution lookup
uint8_t BlueCryptClass::reverseSubstituteByte(uint8_t inputVal) {
    if (_useCustomSBox) {
        for (int16_t i = 0; i < BC_SBOX_SIZE; i++) {
            if (_customSBox[i] == inputVal) return (uint8_t)i;
        }
    } else {
        for (int16_t i = 0; i < BC_SBOX_SIZE; i++) {
            if (pgm_read_byte(&(DEFAULT_SBOX_FLASH[i])) == inputVal) return (uint8_t)i;
        }
    }
    return inputVal;
}

// 2-Byte HMAC generator
uint16_t BlueCryptClass::calculateHMAC(const uint8_t* data, size_t length) {
    uint16_t crc = _hmacKey;
    for (size_t i = 0; i < length; i++) {
        crc ^= (uint16_t)data[i] << 8;
        for (uint8_t bit = 0; bit < 8; bit++) {
            uint16_t mask = -(crc & 1);
            crc = (crc >> 1) ^ (0xA001 & mask);
        }
    }
    return crc;
}

// Buffer helper overload
size_t BlueCryptClass::encrypt(uint8_t* buffer, size_t len) {
    uint8_t temp[128];
    size_t encLen = encrypt(buffer, len, temp);
    memcpy(buffer, temp, encLen);
    return encLen;
}

// Low-level buffer encryption
size_t BlueCryptClass::encrypt(const uint8_t* input, size_t inputLen, uint8_t* output) {
    memcpy(output, input, inputLen);

    // Pass 1: Forward Sweep
    for (size_t byteIdx = 0; byteIdx < inputLen; byteIdx++) {
        for (uint8_t slotIdx = 0; slotIdx < _activeSlotCount; slotIdx++) {
            uint8_t key = _pipeline[slotIdx];
            output[byteIdx] ^= key;
            output[byteIdx] = substituteByte(output[byteIdx]);
        }
    }

    // Avalanche Diffusion: Nibble Swapping
    for (size_t byteIdx = 0; byteIdx < inputLen; byteIdx++) {
        uint8_t val = output[byteIdx];
        output[byteIdx] = ((val & 0x0F) << 4) | ((val & 0xF0) >> 4);
    }

    // Pass 2: Reverse Sweep
    for (size_t byteIdx = 0; byteIdx < inputLen; byteIdx++) {
        for (int16_t slotIdx = _activeSlotCount - 1; slotIdx >= 0; slotIdx--) {
            uint8_t key = _pipeline[slotIdx];
            output[byteIdx] = substituteByte(output[byteIdx]);
            output[byteIdx] ^= key;
        }
    }

    // Append 2-Byte HMAC Tag
    uint16_t hmacTag = calculateHMAC(output, inputLen);
    output[inputLen] = (uint8_t)(hmacTag >> 8);
    output[inputLen + 1] = (uint8_t)(hmacTag & 0xFF);

    return inputLen + BC_HMAC_SIZE;
}

// Low-level buffer decryption
BlueCryptStatus BlueCryptClass::decrypt(const uint8_t* input, size_t inputLen, uint8_t* output, size_t* outputLen) {
    if (inputLen <= BC_HMAC_SIZE) {
        return BC_BUFFER_OVERFLOW;
    }

    size_t payloadLen = inputLen - BC_HMAC_SIZE;

    uint16_t receivedHMAC = ((uint16_t)input[payloadLen] << 8) | input[payloadLen + 1];
    uint16_t computedHMAC = calculateHMAC(input, payloadLen);

    if (receivedHMAC != computedHMAC) {
        return BC_HMAC_MISMATCH;
    }

    memcpy(output, input, payloadLen);

    for (size_t byteIdx = 0; byteIdx < payloadLen; byteIdx++) {
        for (uint8_t slotIdx = 0; slotIdx < _activeSlotCount; slotIdx++) {
            uint8_t key = _pipeline[slotIdx];
            output[byteIdx] ^= key;
            output[byteIdx] = reverseSubstituteByte(output[byteIdx]);
        }
    }

    for (size_t byteIdx = 0; byteIdx < payloadLen; byteIdx++) {
        uint8_t val = output[byteIdx];
        output[byteIdx] = ((val & 0x0F) << 4) | ((val & 0xF0) >> 4);
    }

    for (size_t byteIdx = 0; byteIdx < payloadLen; byteIdx++) {
        for (int16_t slotIdx = _activeSlotCount - 1; slotIdx >= 0; slotIdx--) {
            uint8_t key = _pipeline[slotIdx];
            output[byteIdx] = reverseSubstituteByte(output[byteIdx]);
            output[byteIdx] ^= key;
        }
    }

    *outputLen = payloadLen;
    return BC_SUCCESS;
}

// =========================================================================
// V2.0.1 UPDATE: Hex-Formatted String Overloads
// =========================================================================

// Simplified String Overload: Encrypt (Outputs Space-Separated Hex)
String BlueCryptClass::encrypt(const String& msg) {
    size_t inputLen = msg.length();
    if (inputLen == 0) return "";

    uint8_t* tempOut = new uint8_t[inputLen + BC_HMAC_SIZE];
    size_t outLen = encrypt((const uint8_t*)msg.c_str(), inputLen, tempOut);

    // Convert raw encrypted bytes to space-separated Hex
    char* hexBuf = new char[outLen * 3 + 1];
    size_t pos = 0;
    const char hexDigits[] = "0123456789ABCDEF";

    for (size_t i = 0; i < outLen; i++) {
        hexBuf[pos++] = hexDigits[tempOut[i] >> 4];
        hexBuf[pos++] = hexDigits[tempOut[i] & 0x0F];
        hexBuf[pos++] = ' ';
    }

    // Replace the trailing space with a null terminator
    if (pos > 0) hexBuf[pos - 1] = '\0';
    else hexBuf[0] = '\0';

    String hexOutput = String(hexBuf);
    
    // Clean up dynamic allocations safely
    delete[] hexBuf;
    delete[] tempOut;

    return hexOutput;
}

// Simplified String Overload: Decrypt (Accepts Space-Separated Hex)
String BlueCryptClass::decrypt(const String& cipherText) {
    // Copy the const string so we can trim and tokenize it
    String inputCopy = cipherText;
    inputCopy.trim();
    if (inputCopy.length() == 0) return "";

    // Estimate max bytes (2 chars + 1 space per byte)
    size_t maxBytes = (inputCopy.length() / 2) + 1;
    uint8_t* rawBuffer = new uint8_t[maxBytes];
    size_t rawLen = 0;

    char* buf = new char[inputCopy.length() + 1];
    inputCopy.toCharArray(buf, inputCopy.length() + 1);

    char* token = strtok(buf, " ");
    while (token != NULL) {
        rawBuffer[rawLen++] = (uint8_t)strtol(token, NULL, 16);
        token = strtok(NULL, " ");
    }

    // Safety check against buffer underruns
    if (rawLen <= BC_HMAC_SIZE) {
        delete[] rawBuffer;
        delete[] buf;
        return "";
    }

    uint8_t* tempOut = new uint8_t[rawLen];
    size_t decLen = 0;

    BlueCryptStatus status = decrypt(rawBuffer, rawLen, tempOut, &decLen);

    // Halt decryption and return empty if HMAC integrity check fails
    if (status != BC_SUCCESS) {
        delete[] tempOut;
        delete[] rawBuffer;
        delete[] buf;
        return "";
    }

    // Convert decrypted raw bytes back into a standard text String
    char* outBuf = new char[decLen + 1];
    memcpy(outBuf, tempOut, decLen);
    outBuf[decLen] = '\0';

    String decryptedPlaintext = String(outBuf);

    // Clean up all dynamically allocated memory
    delete[] outBuf;
    delete[] tempOut;
    delete[] rawBuffer;
    delete[] buf;

    return decryptedPlaintext;
}
