#include <Arduino.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>

const char* ssid = "lenguyen";
const char* password = "16082001";

const char* serverName = "https://ap-southeast-1.aws.data.mongodb-api.com/app/application-0-wjqhy/endpoint/coordination";

double x, y, z;

unsigned long lastTime = 0;  
unsigned long timerDelay = 10000; 

void setupClient () {
    Serial.begin(115200);

    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED) {
        delay(1000);
        Serial.println("Connecting to WiFi...");
    }
    Serial.println("Connected to WiFi");
}

void sendRequest () {
    if ((WiFi.status() == WL_CONNECTED)) { 
    if ((millis() - lastTime) > timerDelay) {
      lastTime = millis();
      HTTPClient http;

      http.begin(serverName);
      int httpResponseCode = http.GET();

      if (httpResponseCode>0) {
        String payload = http.getString();
        DynamicJsonDocument doc(1024);
        deserializeJson(doc, payload);

        x = doc["x"]["$numberDouble"].as<double>();
        y = doc["y"]["$numberDouble"].as<double>();
        z = doc["z"]["$numberDouble"].as<double>();

        Serial.print("x: ");
        Serial.println(x);
        Serial.print("y: ");
        Serial.println(y);
        Serial.print("z: ");
        Serial.println(z);
      }
      else {
        Serial.print("Error code: ");
        Serial.println(httpResponseCode);
      }
      http.end();
    }
  }
}

void setup() {
    setupClient();
}

void loop() {
    sendRequest();
}
