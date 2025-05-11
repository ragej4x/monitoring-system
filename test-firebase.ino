#include <WiFiS3.h>
#include <ArduinoJson.h>

// WiFi credentials
#define WIFI_SSID "PLDTHOMEFIBRb8900"
#define WIFI_PASSWORD "#Fk9lratv123456789"

// Firebase credentials
#define FIREBASE_HOST "iot-monitoringsys-default-rtdb.asia-southeast1.firebasedatabase.app"
#define FIREBASE_AUTH "Bfblx5ubfwbbPw6vbM13Scw3zGdDw6iZPOgV3RaW"

// Use WiFiSSLClient for HTTPS instead of standard WiFiClient
WiFiSSLClient client;

void setup() {
  Serial.begin(115200);
  
  // Connect to WiFi
  Serial.print("Connecting to WiFi");
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(300);
  }
  Serial.println();
  Serial.print("Connected with IP: ");
  Serial.println(WiFi.localIP());
}

void loop() {
  // Example: Send a random sensor value
  sendToFirebase("/sensors/temperature.json", String(random(20, 30)));
  
  delay(5000); // Wait 5 seconds before sending another value
}

void sendToFirebase(String path, String value) {
  // Create JSON document
  StaticJsonDocument<200> doc;
  doc.set(value);
  
  // Serialize JSON to string
  String jsonStr;
  serializeJson(doc, jsonStr);
  
  Serial.println("Connecting to Firebase via HTTPS...");
  
  // Connect to Firebase host using SSL
  if (client.connect(FIREBASE_HOST, 443)) {
    Serial.println("Connected to Firebase");
    
    // Create the full path with authentication
    String fullPath = path + "?auth=" + String(FIREBASE_AUTH);
    
    // Send HTTP PUT request
    client.println("PUT " + fullPath + " HTTP/1.1");
    client.println("Host: " + String(FIREBASE_HOST));
    client.println("Connection: close");
    client.println("Content-Type: application/json");
    client.print("Content-Length: ");
    client.println(jsonStr.length());
    client.println();
    client.println(jsonStr);
    
    Serial.println("Data sent to Firebase");
    
    // Wait for response
    unsigned long timeout = millis();
    while (client.available() == 0) {
      if (millis() - timeout > 10000) {  // Increased timeout for SSL handshake
        Serial.println("Request timeout");
        client.stop();
        return;
      }
    }
    
    // Read response
    String response = "";
    while (client.available()) {
      char c = client.read();
      response += c;
      
      // Print in chunks to avoid Serial buffer overflow
      if (response.length() > 80) {
        Serial.print(response);
        response = "";
      }
    }
    
    // Print any remaining response
    if (response.length() > 0) {
      Serial.print(response);
    }
    
    Serial.println();
    Serial.println("Request completed");
    
    // Close connection
    client.stop();
  } else {
    Serial.println("Connection to Firebase failed");
  }
}
