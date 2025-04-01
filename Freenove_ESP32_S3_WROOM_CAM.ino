/* ======================= File Includes =======================*/
#include <Arduino.h>
#include <SD.h>
#include <WiFi.h>
#include <Firebase_ESP_Client.h>
#include <time.h>
#include "camera_utils.h"
#include "connection_utils.h"

/* ======================= Global Variables ======================= */

// Firebase objects
FirebaseData fbdo;
FirebaseAuth auth;
FirebaseConfig config;

// Bluetooth variables
const char* wifiID = "SM-A536W2193";
const char* wifiPassword = "kamikaze1021";
String userID = "LgcajroI97fAlGVGtzHvQ7uNgiX2";

// NTP server and offsets for timestamp
const char* ntpServer = "pool.ntp.org";
const long gmtOffset_sec = -25200;  // MST (UTC-7)
const int daylightOffset_sec = 0;   // No daylight saving time (Standard Time)
String timestamp;

void setup() {
  // put your setup code here, to run once:
  Serial.begin(921600);
  Serial.setDebugOutput(true);
  Serial.println();

  // Needs pin set for LDO
  pinMode(22, OUTPUT);  // Set pin as output
  digitalWrite(22, HIGH);

  Serial.println("Connecting to WiFi...");
  WiFi.begin(wifiID, wifiPassword);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("Connected!");
  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);

  // Initialize Camera
  initCamera();

  // Initialization complete
  Serial.println("\n-------------------------------------------------------------------");
  Serial.println("Board initialization successful, commencing image capture");
  Serial.println("-------------------------------------------------------------------\n");
}

void loop() {
  if (!Firebase.ready()) {     
    Serial.println("No connection to Firebase!");
    connectFirebase();
    if (Firebase.ready()) {
      Serial.println("\nConnected to Firebase!\n");
    }
  }
  else {
    Serial.println("Got here!!!");

    // Wait for a certain time interval before capturing the next image
    uploadPhoto();
    delay(60000); // 60 seconds delay
  }
}

void connectFirebase() {
  Serial.println("Connecting to Firebase...");
  config.api_key = API_KEY;           // Assign the api key 
  auth.user.email = EMAIL_KEY;        // Assign the user sign in credentials
  auth.user.password = PASSWORD_KEY;
  Firebase.begin(&config, &auth);     // Start firebase connection
  Firebase.reconnectWiFi(true);

  unsigned long timeout = millis() + 10000; // 10 seconds timeout
  while (!Firebase.ready()) {
    Serial.print(".");
    delay(500);
    if (millis() > timeout) {
      Serial.println("\nFailed to connect to Firebase!");
      return;
    }
  }
}

void updateTimestamp() {
    struct tm timeinfo;
    if (!getLocalTime(&timeinfo)) {
        timestamp = "TimeError";
        return;
    }
    char buffer[20];  // Buffer to store formatted timestamp
    strftime(buffer, sizeof(buffer), "%Y-%m-%d_%H-%M-%S", &timeinfo);
    timestamp = String(buffer);  // Update global timestamp
}

void uploadPhoto() {
  
  // Frame buffer pointer
  camera_fb_t* fb = NULL;

  // Doing it a bunch to let the auto adjusting parameters settle... (testing needed)
  for (uint8_t i = 0; i < 10; i++) {
    fb = esp_camera_fb_get();
    esp_camera_fb_return(fb);
    fb = NULL;
  }

  // Capture Image
  fb = esp_camera_fb_get();
  Serial.printf("Captured %d bytes\n", fb->len);

  // Update timestamp
  updateTimestamp();
  
  Serial.printf("Capturing an image: " + timestamp + "\n");

  // Upload to Firebase
  String path = "/images/" + userID + "/" + timestamp  + ".jpg";
  if (Firebase.Storage.upload(&fbdo, STORAGE_BUCKET_ID, fb->buf, fb->len, path.c_str(), "image/jpeg")) {
    Serial.println("Upload success!");
  } else {
    Serial.println("Upload failed: " + fbdo.errorReason());
  }

  esp_camera_fb_return(fb);
}
