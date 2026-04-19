#include "fallDetection.h"
#include <Arduino.h>
#include <Wire.h>
// master code final
unsigned long fallPrevTime = 0;
unsigned long prevTime = 0;
unsigned long interval = 9000;
bool isPersonFall = false;
bool isPersonFainted = false;

const float faintedThresholdVal = 1.07;
const float fallThresholdVal = 20.0;

FALLDETECTION::FALLDETECTION()
    : _state(State::MONITORING), _faintStartTime(0) {}


void FALLDETECTION::initMPU() {
    Wire.begin();
    _mpu.initialize();

    if (!_mpu.testConnection()) {
        Serial.println("[FallDetection] MPU6050 connection FAILED");
        return;
    }

    _mpu.setFullScaleAccelRange(MPU6050_ACCEL_FS_8); 
    _mpu.setFullScaleGyroRange(MPU6050_GYRO_FS_500);

    Serial.println("[FallDetection] MPU6050 ready");
}

float FALLDETECTION::getAccelMagnitude() {
    int16_t ax, ay, az, gx, gy, gz;
    _mpu.getMotion6(&ax, &ay, &az, &gx, &gy, &gz);

    float fx = ax / 4096.0f;
    float fy = ay / 4096.0f;
    float fz = az / 4096.0f;
    return sqrt(fx*fx + fy*fy + fz*fz);
}

float FALLDETECTION::getGyroMagnitude() {
    int16_t ax, ay, az, gx, gy, gz;
    _mpu.getMotion6(&ax, &ay, &az, &gx, &gy, &gz);

    float rx = gx / 65.5f;
    float ry = gy / 65.5f;
    float rz = gz / 65.5f;
    return sqrt(rx*rx + ry*ry + rz*rz);
}


bool FALLDETECTION::isPersonFainted() {
    float accel = getAccelMagnitude();
    float gyro  = getGyroMagnitude();

    Serial.print("Accel value ");
    Serial.println(accel);
    Serial.print("Gyro ");
    Serial.println(gyro);

    unsigned long currentTime = millis();

    // Step 1: Detect accel spike and START timer
    if (accel > faintedThresholdVal) {
        prevTime = currentTime;
        Serial.printf("[FaintedDetection] Accel trigger started\n");
    }

    // Step 2: Check if within 9 seconds window
    if ((currentTime - prevTime <= interval) && accel > faintedThresholdVal) {
        Serial.printf("[FaintedDetection] Within 9s window accel=%.2f g\n", accel);
        return true;
    }

    // Step 3: Gyro fall detection (1 second window)
    if (gyro > fallThresholdVal) {
        if (currentTime - fallPrevTime <= 1000) {
            Serial.printf("[FallDetection] Gyro trigger within 1s: %.2f\n", gyro);
            return true;
        }
        fallPrevTime = currentTime; // start timer
    }

    return false;
}