#include <SPI.h>
#include <MFRC522.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

#define RST_PIN 9
#define SS_PIN 10

MFRC522 mfrc522(SS_PIN, RST_PIN);
LiquidCrystal_I2C lcd(0x27, 16, 2);  // Adjust I2C address if needed

bool wifiConnected = false;

void displayMessage(const String& line1, const String& line2 = "") {
  lcd.setCursor(0, 0);
  lcd.print(line1 + String("                ").substring(line1.length()));
  lcd.setCursor(0, 1);
  lcd.print(line2 + String("                ").substring(line2.length()));
}

void waitForWiFi() {
  displayMessage("Waiting for WiFi");

  while (!wifiConnected) {
    if (Serial.available()) {
      String line = Serial.readStringUntil('\n');
      line.trim();
      if (line.startsWith("#WIFI_CONNECTED")) {
        wifiConnected = true;
        displayMessage("WiFi Connected");
        delay(500);  // Reduced delay
        displayMessage("Scan RFID...");
      }
    }
  }
}

void setup() {
  Serial.begin(115200);  // Increased baud rate
  SPI.begin();
  mfrc522.PCD_Init();
  lcd.init();
  lcd.backlight();

  waitForWiFi();
  displayMessage("Scan RFID...");
}

void loop() {
  if (!wifiConnected) {
    waitForWiFi();
    return;
  }

  // Wait for RFID card
  if (!mfrc522.PICC_IsNewCardPresent() || !mfrc522.PICC_ReadCardSerial()) {
    return;
  }

  // Read RFID UID
  String rfid = "";
  for (byte i = 0; i < mfrc522.uid.size; i++) {
    if (mfrc522.uid.uidByte[i] < 0x10) rfid += "0";
    rfid += String(mfrc522.uid.uidByte[i], HEX);
  }
  rfid.toUpperCase();

  // Send RFID to ESP
  Serial.println("&" + rfid);
  displayMessage("Sending...");
  // Show "Sending..."
  delay(2000);  // Simulate short send time

  // Show "Logging..."
  displayMessage("Logging...");
  delay(2000);  // Simulate logging time

  // Show "Waiting for", "response..."
  displayMessage("Waiting for", "response...");

  // Wait for a valid response that starts with #
  String response = "";
  unsigned long startTime = millis();
  const unsigned long timeout = 15000;

  while (millis() - startTime < timeout) {
    if (Serial.available()) {
      String line = Serial.readStringUntil('\n');
      line.trim();

      if (line.startsWith("#")) {
        response = line.substring(1);  // Remove #
        break;
      }
      // Ignore all other lines
    }
  }

  if (response == "") {
    displayMessage("No Response", "Try Again");
    delay(1000);  // Reduced delay
    displayMessage("Scan RFID...");
    return;
  }

  // Process valid response
  if (response.indexOf("TYPE") >= 0 && response.indexOf("STATUS") >= 0) {
    if (response.indexOf("PERSON") >= 0) {
      if (response.indexOf("IN") >= 0) {
        displayMessage("Session Started");
      } else if (response.indexOf("OUT") >= 0) {
        displayMessage("Session Ended");
      } else {
        displayMessage("Person Unknown");
      }
    } else if (response.indexOf("TOOL") >= 0) {
      if (response.indexOf("IN") >= 0) {
        displayMessage("Tool Checked In");
      } else if (response.indexOf("OUT") >= 0) {
        displayMessage("Tool Checked Out");
      } else {
        displayMessage("Tool Unknown");
      }
    } else {
      displayMessage("Unknown Type");
    }
  } else if (response.indexOf("Succesfully created entry") >= 0) {
    displayMessage("New Entry Added");
  } else if (response.indexOf("UNAUTHORIZED") >= 0) {
    displayMessage("Unauthorized:", "Check in first");
  } else if (response.indexOf("HTTP Error") >= 0 || response.indexOf("WiFi Error") >= 0) {
    displayMessage("Comm Error");
  } else if (response.indexOf("WIFI_DISCONNECTED") >= 0) {
    wifiConnected = false;
    displayMessage("WiFi Lost");
    delay(500);  // Reduced delay
    waitForWiFi();
    return;
  } else {
    displayMessage("Other Response:", response.substring(0, 16));
  }

  delay(1000);  // Reduced delay
  displayMessage("Scan RFID...");
}
