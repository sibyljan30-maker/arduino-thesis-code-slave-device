#include "configFirebase.h"

#ifdef ESP32
#include <WiFi.h>
#include <FirebaseESP32.h>
#endif

#include <addons/TokenHelper.h>
#include <addons/RTDBHelper.h>
// Master code final
#define WIFI_SSID       "GlobeAtHome_2F27D_2.4"
#define WIFI_PASSWORD   "3F4E2F27"

#define API_KEY         "AIzaSyB0uV3eUPE4g39vfG6CZJ3m9OVTT9ivqXU"
#define DATABASE_URL    "https://savest-94b43-default-rtdb.firebaseio.com"
#define USER_EMAIL      "admin@gmail.com"
#define USER_PASSWORD   "admin123"

FirebaseData fbdo;
FirebaseAuth auth;
FirebaseConfig config;

static unsigned long prevTime = 0;
static unsigned long lastWarnRead = 0;

const unsigned long interval = 2000;

CONFIGFIREBASE::CONFIGFIREBASE() {}

void CONFIGFIREBASE::initTime() {
  configTime(8 * 3600, 0, "pool.ntp.org", "time.nist.gov");
  Serial.print("Syncing time");
  time_t now = time(nullptr);
  unsigned long start = millis();
  while (now < 100000 && millis() - start < 10000) { // 10s timeout
    delay(500);
    Serial.print(".");
    now = time(nullptr);
  }
  Serial.println(now >= 100000 ? "\nTime synchronized" : "\nTime sync failed, continuing...");
}

bool CONFIGFIREBASE::initFirebase() {
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("Connecting to WiFi");

  unsigned long wifiStart = millis();
  unsigned long lastDot = millis();
  bool connected = false;

  while (!connected) {
    unsigned long now = millis();

    if (WiFi.status() == WL_CONNECTED) {
      connected = true;
      Serial.println("\nWiFi Connected!");
      break;
    }

    if (now - wifiStart >= 45000) {
      Serial.println("\nWiFi connection timeout! Escaping to continue...");
      break;
    }

    if (now - lastDot >= 1000) {
      Serial.print(".");
      lastDot = now;
    }

    delay(10);
  }

  if (connected) {
    initTime();
  } else {
    Serial.println("Proceeding without WiFi...");
  }

  config.api_key = API_KEY;
  config.database_url = DATABASE_URL;

  auth.user.email = USER_EMAIL;
  auth.user.password = USER_PASSWORD;

  config.token_status_callback = tokenStatusCallback;

  Firebase.reconnectWiFi(true);
  fbdo.setBSSLBufferSize(4096, 1024);

  Firebase.begin(&config, &auth);
  Firebase.setDoubleDigits(5);
  return true;
}

void CONFIGFIREBASE::sendFirebaseData(int mq135Val, int mq7Val, int dust, float temperature, bool isPersonFall, bool warningStatus, bool emergencyStatus, float humidity)
{
  if (Firebase.ready() && millis() - prevTime >= interval) {

    bool isTempSuccess = Firebase.setInt(fbdo, "/monitoring/vest1/temp", temperature);
    bool isHumdSuccess = Firebase.setInt(fbdo, "/monitoring/vest1/humd", humidity);
    bool isDustSuccess = Firebase.setInt(fbdo, "/monitoring/vest1/dust", dust); 
    bool isMQ135Success = Firebase.setInt(fbdo, "/monitoring/vest1/mq135", mq135Val);
    bool isMQ7Success = Firebase.setInt(fbdo, "/monitoring/vest1/mq7", mq7Val);
    bool isPersonFainted = Firebase.setBool(fbdo, "/monitoring/vest1/fallDetected", isPersonFall);
    bool isWarningSuccess = Firebase.setBool(fbdo, "/monitoring/vest1/WarningStatus", warningStatus);
    bool isEmergencySuccess = Firebase.setBool(fbdo, "/monitoring/vest1/EmergencyStatus", emergencyStatus);


    if (!isTempSuccess || !isDustSuccess || !isMQ135Success || !isMQ7Success || !isPersonFainted || !isWarningSuccess || !isEmergencySuccess || !isHumdSuccess)
      Serial.println("Failed to send firebase data");

    prevTime = millis();
  }
}