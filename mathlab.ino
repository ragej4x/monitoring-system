#include <WiFi.h>               // Ensure the correct WiFi library for your board
#include <ArduinoHttpClient.h>

const char* ssid = "PLDTHOMEFIBRb8900";
const char* password = "#Fk9lratv123456789";
const char* apiKey = "2I1IYU5MVI5JW6NX";  // Your ThingSpeak Write API Key

WiFiClient wifi;                       // WiFiClient object to communicate with ThingSpeak
HttpClient client = HttpClient(wifi, "api.thingspeak.com", 80);  // HTTP client to communicate with ThingSpeak

void setup() {
  Serial.begin(115200);

  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi...");
  }

  Serial.println("WiFi connected!");

  float temperature = 25.3;  // Example values
  float humidity = 60.2;

  // Build the URL for the ThingSpeak API request
  String url = "/update?api_key=" + String(apiKey);
  url += "&field1=" + String(temperature);
  url += "&field2=" + String(humidity);

  client.beginRequest();
  client.get(url);  // Send GET request to ThingSpeak API
  int statusCode = client.responseStatusCode();  // Get the HTTP response code
  String response = client.responseBody();  // Get the response body

  if (statusCode > 0) {
    Serial.print("Server response: ");
    Serial.println(response);
  } else {
    Serial.print("Failed, error: ");
    Serial.println(statusCode);
  }

  client.endRequest();
}

void loop() {
  // Send data every 15 seconds (ThingSpeak limit)
  delay(15000);
}
