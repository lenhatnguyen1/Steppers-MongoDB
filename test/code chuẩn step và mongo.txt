// Include libraries
#include <Arduino.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <AccelStepper.h>
#include <MultiStepper.h>

// Define stepper motor connections and motor interface type: 
#define step1 12 
#define dir1 13 

#define step2 25 
#define dir2 26 

#define step3 22
#define dir3 23

#define driverMode 1

// Create a new instance of the AccelStepper class for each stepper motor: 
AccelStepper stepper1 = AccelStepper(driverMode, step1, dir1);
AccelStepper stepper2 = AccelStepper(driverMode, step2, dir2);
AccelStepper stepper3 = AccelStepper(driverMode, step3, dir3);

// Create a new instance of the MultiStepper class: 
MultiStepper steppers;

const float maxSpeed = 1000.0;
const float maxAccel = 800.0;
const float constSpeed = 1000;

// Declare wifi and API
const char* ssid = "Hong Hai";
const char* password = "ngoclan64";
const char* serverName = "https://ap-southeast-1.aws.data.mongodb-api.com/app/application-0-wjqhy/endpoint/steppers";

// Declare variables
double theta1, theta2, theta3;
unsigned long lastTime = 0;  
unsigned long timerDelay = 4000;

// Create function to convert angle to position
float angleToPosition (float angle) {
  float pos = angle/360*200;
  return pos;
}

// Create function to run steppers to position with constant speed
void stepRun(float pos1, float pos2, float pos3) {
   long positions[3]; // Array of desired stepper positions
   positions[0] = pos1;
   positions[1] = pos2;
   positions[2] = pos3;
   steppers.moveTo(positions);
   steppers.runSpeedToPosition(); // Blocks until all are in position

}

void setupSteppers() {
     // Add each stepper motor to the MultiStepper object: 
   steppers.addStepper(stepper1);
   steppers.addStepper(stepper2);
   steppers.addStepper(stepper3);

    // Set up each stepper motor: 
    stepper1.setMaxSpeed(maxSpeed);
    stepper1.setAcceleration(maxAccel);
    stepper1.setSpeed(constSpeed);

    
    stepper2.setMaxSpeed(maxSpeed);
    stepper2.setAcceleration(maxAccel);
    stepper2.setSpeed(constSpeed);
    
    stepper3.setMaxSpeed(maxSpeed);
    stepper3.setAcceleration(maxAccel);
    stepper3.setSpeed(constSpeed);

    stepper1.moveTo(0);
    stepper2.moveTo(0);
    stepper3.moveTo(0);
}

// Create function to setup http client
void setupClient () {
    Serial.begin(115200);
    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED) {
        delay(1000);
        Serial.println("Connecting to WiFi...");
    }
    Serial.println("Connected to WiFi");
}

// Create function to send request to API
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
                theta1 = doc["theta1"].as<double>();
                theta2 = doc["theta2"].as<double>();
                theta3 = doc["theta3"].as<double>();
                Serial.print("theta1: ");
                Serial.println(theta1);
                Serial.print("theta2: ");
                Serial.println(theta2);
                Serial.print("theta3: ");
                Serial.println(theta3);
                Serial.println("--------------");
            }
            else {
                Serial.print("Error code: ");
                Serial.println(httpResponseCode);
            }
            http.end();
        }
    }
}

void Task1code( void * pvParameters ){
    Serial.print("Task1 running on core ");
    Serial.println(xPortGetCoreID());
    for(;;){
        sendRequest();
    } 
}

void Task2code( void * pvParameters ){
    Serial.print("Task2 running on core ");
    Serial.println(xPortGetCoreID());
    for(;;){
      double pos1 = angleToPosition(theta1);
      double pos2 = angleToPosition(theta2);
      double pos3 = angleToPosition(theta3);
      delay(100);
      if (pos1 != stepper1.currentPosition() || pos2 != stepper2.currentPosition() || pos3 != stepper3.currentPosition()) {
          stepRun(pos1, pos2, pos3);
      }
    }
}

void setup() {
    setupClient();
    setupSteppers();
    xTaskCreatePinnedToCore(Task1code, "Task1", 4096, NULL, 0, NULL, 0);                  
    delay(500); 
    xTaskCreatePinnedToCore(Task2code, "Task2", 4096, NULL, 1, NULL, 1);
    delay(500); 
}

void loop() {
  
}
