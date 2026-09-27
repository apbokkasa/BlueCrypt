#include <Arduino.h>
#include <BlueCrypt.h>

void setup() {
    Serial.begin(9600);
    while (!Serial);

    // Initializer and module configuration
    BlueCrypt.begin('A');
    
    const uint8_t modules[] = {1, 3, 2, 4, 2, 4, 3, 2, 1, 2, 1, 3, 4, 2, 4};
    BlueCrypt.module(modules);

    String myMessage = "Hello World!";
    String encrypted = BlueCrypt.encrypt(myMessage);

    // --- PRINTING ENCRYPTED PAYLOAD IN HEX FORMAT ---
    Serial.print(F("Encrypted (HEX): "));
    for (size_t i = 0; i < encrypted.length(); i++) {
        uint8_t b = (uint8_t)encrypted[i];
        
        // Print leading zero for single-digit hex values (e.g., 0x0F -> "0F")
        if (b < 0x10) {
            Serial.print('0');
        }
        
        // Print byte value in Hexadecimal uppercase
        Serial.print(b, HEX);
        Serial.print(' ');
    }
    Serial.println();

    // Decrypt and display original string
    String decrypted = BlueCrypt.decrypt(encrypted);
    Serial.print(F("Decrypted Text: "));
    Serial.println(decrypted);
}

void loop() {
    // Idle
}
