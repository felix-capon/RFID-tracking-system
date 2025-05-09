#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <ArduinoJson.h>

const char* ssid = 
const char* password = 

// Your Web App URL
const char* scriptURL = 

void setup() {
  Serial.begin(9600);
  WiFi.begin(ssid, password);

  Serial.print("Connecting to WiFi...");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi connected");
}

void loop() {
  if (Serial.available() > 0) {
    String rfid = Serial.readStringUntil('\n');
    rfid.trim();  // Clean any extra whitespace or newline

    if (rfid.length() > 0) {
      Serial.println("RFID Scanned: " + rfid);
      handleRFIDScan(rfid);  // Handle the scanned RFID
    }
  }
}

void handleRFIDScan(String rfid) {
  WiFiClientSecure client;
  client.setInsecure();  // Disable SSL verification for testing

  HTTPClient http;
  String fullURL = String(scriptURL) + "?rfid=" + rfid;
  http.begin(client, fullURL);

  int httpCode = http.GET();  // Perform the GET request
  String payload = http.getString();  // Get the response body
  Serial.println("HTTP Code: " + String(httpCode));
  Serial.println("Payload: " + payload);

  // Check if a redirect occurred (HTTP 301 or 302)
  if (httpCode == 301 || httpCode == 302) {
    String redirectURL = getRedirectURL(payload);  // Extract the redirect URL
    if (redirectURL.length() > 0) {
      Serial.println("Following redirect to: " + redirectURL);
      http.end();  // Close the current connection
      http.begin(client, redirectURL);  // Start a new connection to the redirect URL
      httpCode = http.GET();  // Perform the GET request again
      payload = http.getString();  // Get the new response body
      Serial.println("HTTP Code after redirect: " + String(httpCode));
      Serial.println("Payload after redirect: " + payload);
    }
  }

  // If the response is 200 OK, handle the JSON payload
  if (httpCode == 200) {
    DynamicJsonDocument doc(1024);
    deserializeJson(doc, payload);  // Parse the response as JSON

    String type = doc["type"];   // Get the "type" from the response
    String status = doc["status"];  // Get the "status" from the response
    
    // Log the RFID scan if the type and status are valid
    if (type != "UNKNOWN" && status != "UNKNOWN") {
      sendPutRequest(rfid, type, status);  // Send the data to another endpoint or log it
    }
  }

  http.end();  // End the HTTP request
}

// Helper function to extract the redirect URL from the payload
String getRedirectURL(String payload) {
  int hrefIndex = payload.indexOf("HREF=\"");
  if (hrefIndex == -1) {
    hrefIndex = payload.indexOf("href='");
  }

  if (hrefIndex != -1) {
    int start = hrefIndex + 6;  // Skip the 'HREF="' part
    char quoteChar = payload.charAt(hrefIndex + 5);  // Either '"' or "'"
    int end = payload.indexOf(quoteChar, start);  // Find the end quote
    String redirectURL = payload.substring(start, end);  // Extract the URL
    redirectURL.replace("&amp;", "&");  // Clean up URL if necessary

    return redirectURL;  // Return the redirect URL
  }
  return "";
}

void sendPutRequest(String rfid, String type, String status) {
  // Your PUT request logic here (to log or send the RFID data)
  // Example (you can replace this with your logic):
  Serial.println("Logging RFID Data: " + rfid + ", " + type + ", " + status);
}
