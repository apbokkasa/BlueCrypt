#include <Arduino.h>
#include <BlueCrypt.h>
#include <AutoPack.h>

byte modules[128];
byte nums[256];
int len = 0; 

void BlueCryptSetup() {
  Serial.println(F(" - ARDUINO - Enter secret mixing key(Eg: 0xAAAAAAAA)."));
  Serial.print(F(" - USER - "));
  while(Serial.available() == 0) {}
  
  if (Serial.available() > 0) {
    String input = Serial.readStringUntil('\n');
    input.trim(); 
    uint32_t seedKey = strtoul(input.c_str(), NULL, 16);
    BlueCrypt.begin(seedKey);
  }
  Serial.readString(); 
  Serial.println();
  
  Serial.println(F(" - ARDUINO - Would you like to set the module order(y/n)? "));
  Serial.print(F(" - USER - "));
  while(Serial.available() == 0) {}
  
  char ans = Serial.read();
  if(ans == 'y' || ans == 'Y') {
    Serial.readString();
    Serial.println();
    Serial.println(F(" - ARDUINO - Enter module order."));
    Serial.print(F(" - USER - "));
    while(Serial.available() == 0) {}
    
    if(Serial.available() > 0) {
      String inputStr = Serial.readStringUntil('\n');
      inputStr.trim();
      char buf[inputStr.length() + 1];
      inputStr.toCharArray(buf, sizeof(buf));

      AutoPack.delimit(", ");
      len = AutoPack.parseByte(buf, modules, 128);
    }
    BlueCrypt.module(modules, len);
  } else {
    BlueCrypt.module((const uint8_t[]){1, 2, 3, 4}, 4); 
  }
  
  len = 0;
  Serial.readString();
  Serial.println();
  
  Serial.println(F(" - ARDUINO - Would you like to edit the S-Box(WARNING: It involves 1536 characters)(y/n)?"));
  Serial.print(F(" - USER - "));
  while(Serial.available() == 0) {}
  
  ans = Serial.read();
  Serial.readString();
  
  if(ans == 'y' || ans == 'Y') {
    Serial.println();
    Serial.println(F(" - ARDUINO - Enter hex values(0x00 to 0xFF)"));
    Serial.print(F(" - USER - "));
    while(Serial.available() == 0) {}
    
    if (Serial.available() > 0) {
      String inputString = Serial.readStringUntil('\n');
      inputString.trim();

      if (inputString.length() > 0) {
        memset(nums, 0, sizeof(nums));
        char buf[inputString.length() + 1];
        inputString.toCharArray(buf, sizeof(buf));

        AutoPack.delimit(", ");
        AutoPack.parseByte(buf, nums, 256);
        BlueCrypt.table(nums);
      }
    }
  } else {
    BlueCrypt.resetTable();
  }
  
  Serial.println();
  Serial.println();
  Serial.println(F(" - ARDUINO - BlueCrypt Setup complete. Initializing..."));
  delay(1000);
  Serial.println(F(" - ARDUINO - BlueCrypt Initialization complete. Initializing Bootup Sequence..."));
  delay(4000);
  Serial.println(F(" - ARDUINO - BlueCrypt Bootup Sequence Complete."));
  delay(500);
}

void setup() {
  Serial.begin(9600);
  BlueCryptSetup();
}

void loop() {
  Serial.println(F(" - ARDUINO - Would you like to encrypt or decrypt a message(e(encryption)/d(decryption))?"));
  Serial.print(F(" - USER - "));
  
  while(Serial.available() == 0) {}
  
  char ans = Serial.read(); 
  Serial.readString(); 
  
  if(ans == 'e' || ans == 'E') {
    Serial.println();
    Serial.println();
    Serial.println(F("- ARDUINO - Enter message to encrypt. ")); 
    Serial.print(F(" - USER - ")); 
    while(Serial.available() == 0) {} 
    
    String msg = Serial.readStringUntil('\n'); 
    msg.trim();
    Serial.println();
    Serial.println(F(" - ARDUINO - Message recieved. Encrypting...")); 
    
    unsigned long startTime = micros(); 
    String encryptedHex = BlueCrypt.encrypt(msg); 
    unsigned long endTime = micros(); 
    
    Serial.println(F(" - ARDUINO - Encryption process complete. Data: "));
    Serial.print(F(" - ARDUINO - Duration: "));
    Serial.print(endTime - startTime);
    Serial.println(F(" us"));
    
    Serial.print(F(" - ARDUINO - Encrypted (HEX): "));
    Serial.println(encryptedHex);
    Serial.println();
  }
  else if(ans == 'd' || ans == 'D') {
    Serial.println(F("- ARDUINO - Enter HEX message to decrypt (space separated). ")); 
    Serial.print(F(" - USER - ")); 
    while(Serial.available() == 0) {} 
    
    String hexInput = Serial.readStringUntil('\n'); 
    hexInput.trim();
    Serial.println(F(" - ARDUINO - Message recieved. Decrypting...")); 
    
    // --- DIAGNOSTIC DEBUG OUTPUT ---
    Serial.print(F(" [DEBUG] Raw Input length: "));
    Serial.println(hexInput.length());
    // -------------------------------
    
    unsigned long startTime = micros(); 
    String decrypted = BlueCrypt.decrypt(hexInput); 
    unsigned long endTime = micros(); 
    
    // If the library returns a blank string, catch the error explicitly
    if (decrypted.length() == 0) {
        Serial.println(F(" [ERROR] HMAC Mismatch or Corrupted Payload!"));
        Serial.println(F(" [ERROR] Make sure you ONLY copied the hex numbers, not the text 'Encrypted (HEX):'"));
        Serial.println();
    } else {
        Serial.println(F(" - ARDUINO - Decryption process complete. Data: "));
        Serial.print(F(" - ARDUINO - Duration: "));
        Serial.print(endTime - startTime);
        Serial.println(F(" us"));
        
        Serial.print(F(" - ARDUINO - Decrypted (Plaintext): "));
        Serial.println(decrypted);
        Serial.println();
    }
  }
  else {
    Serial.println();
    Serial.println(F(" - ARDUINO - Invalid selection. Please enter 'e' or 'd'."));
    Serial.println();
  }
}