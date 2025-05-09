#include <SPI.h>
#include <MFRC522.h>

#define SS_PIN 10  // Pin 10 is used to select the RFID reader
#define RST_PIN 9  // Reset pin for the RFID module

MFRC522 mfrc522(SS_PIN, RST_PIN);  // Create MFRC522 instance

void setup() {
  // Start the serial communication
  Serial.begin(9600);

  // Initialize the MFRC522 RFID reader
  SPI.begin();  // Initialize SPI bus
  mfrc522.PCD_Init();  // Initialize the MFRC522 module

  Serial.println("Scan a card or tag");
}

void loop() {
  // Look for a new card
  if (mfrc522.PICC_IsNewCardPresent()) {
    // Read the card's UID
    if (mfrc522.PICC_ReadCardSerial()) {
      // Print UID to Serial Monitor
      Serial.print("UID: ");
      for (byte i = 0; i < mfrc522.uid.size; i++) {
        Serial.print(mfrc522.uid.uidByte[i], HEX);
      }
      Serial.println();

      // Stop reading the card
      mfrc522.PICC_HaltA();
      mfrc522.PCD_StopCrypto1();
    }
  }
}
