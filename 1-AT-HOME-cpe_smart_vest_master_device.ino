// Master.ino file final code

#include <Wire.h>
#include "fallDetection.h"
#include "configFirebase.h"

// master device
const int mq135Pin = 35; // Mau ni ang MQ7
const int mq7Pin = 32; // Mau ni ang MQ135
const int SLAVE_ADDR = 0x08; // I2C address of Arduino Nano

float dustDensity = 0;
float temperature = 0;
float humidity = 0;
String warningStatus = "1";
String emergencyStatus = "1";

unsigned long lastSendTime = 0;
const unsigned long SEND_INTERVAL = 2000;

String rxBuffer = "";
FALLDETECTION gyro;
CONFIGFIREBASE conf;

void parseSlaveData(String data) {
  int dIdx = data.indexOf("D:");
  int tIdx = data.indexOf(",T:");
  int hIdx = data.indexOf(",H:");
  int wIdx = data.indexOf(",W:");
  int eIdx = data.indexOf(",E:");

  if (dIdx != -1 && tIdx != -1 && hIdx != -1 && wIdx != -1 && eIdx != -1) {
    dustDensity = data.substring(dIdx + 2, tIdx).toFloat();
    temperature = data.substring(tIdx + 3, hIdx).toFloat();
    humidity    = data.substring(hIdx + 3, wIdx).toFloat();

    warningStatus   = data.substring(wIdx + 3, eIdx);
    emergencyStatus = data.substring(eIdx + 3);
  }
}

void setup() {
  Serial.begin(115200);
  Wire.begin();
  gyro.initMPU();
  delay(1000);
  conf.initFirebase();
  delay(1500);
  Serial.println("Master ready");
}

void loop() {
  unsigned long now = millis();

  if (now - lastSendTime >= SEND_INTERVAL) {
    lastSendTime = now;

    int mq135Val = analogRead(mq135Pin);
    int mq7Val   = analogRead(mq7Pin);

    String payload = "CO:" + String(mq135Val) + ",AQ:" + String(mq7Val);
    if (mq135Val > 2500 || mq7Val > 3000) {
      payload += ",ON";
    }

    Wire.beginTransmission(SLAVE_ADDR);
    Wire.write((const uint8_t*)payload.c_str(), payload.length());
    byte error = Wire.endTransmission();
    delay(10);
    if (error == 0) {
      Serial.println("Sent to slave: " + payload);
    } else {
      Serial.print("I2C send error: ");
      Serial.println(error);
    }

    delay(50); 
    Wire.requestFrom(SLAVE_ADDR, 32); 

    String received = "";
    while (Wire.available()) {
      char c = Wire.read();
      if (c != '\0') received += c;
    }

    if (received.length() > 0) {
      Serial.println("From slave: " + received);
      parseSlaveData(received);
    }

    //if (mq135Val < 305)
      //mq135Val = random (10, 20);
    //else if (mq135Val < 515)
      //mq135Val = random (30, 50);
    //else
      //mq135Val = random (75, 100);

    //if (mq7Val < 930)
      //mq7Val = random (10, 20);
    //else if (mq7Val < 970)
      //mq7Val = random (30, 50);
    //else
      //mq7Val = random (75, 100);

    //if (dustDensity < 155)
      //dustDensity = random (10, 20);
    //else if (dustDensity < 625)
      //dustDensity = random (30, 50);
    //else
      //dustDensity = random (75, 100);
    

    conf.sendFirebaseData(
        mq135Val,
        mq7Val,
        dustDensity,
        temperature,
        gyro.isPersonFainted(),
        (emergencyStatus == "1"),
        (warningStatus == "1"),
        humidity
      );
    Serial.print("CO: ");   Serial.println(mq135Val);
    Serial.print("AQ: ");   Serial.println(mq7Val);
    Serial.print("Dust: "); Serial.println(dustDensity);
    Serial.print("Temp: "); Serial.println(temperature);
    Serial.print("Humd: "); Serial.println(humidity);
    Serial.print("Warning Status: "); Serial.println(warningStatus);
    Serial.print("Emergency Status: "); Serial.println(emergencyStatus);

  }
}