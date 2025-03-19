/* ======================= File Includes =======================*/
#include "camera_utils.h"
#include <WiFi.h>
#include <Firebase_ESP_Client.h>

#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>

/* ======================= Firebase Prototype Defines =======================*/
#define API_KEY "AIzaSyB2NdPoW8WFrn0s1MxQmOsMkYrbpXraQNs"            // Insert Firebase project API Key
#define USER_EMAIL "nathanante36@gmail.com"                          // Insert Authorized Email and Corresponding Password
#define USER_PASSWORD "Kamikaze102198!"
#define STORAGE_BUCKET_ID "patchnotes-d3b06.firebasestorage.app"     // Insert Firebase storage bucket ID e.g bucket-name.appspot.com
#define FILE_PHOTO_PATH "/photo.jpg"                                 // Prototypes for filename in backend storage
#define BUCKET_PHOTO "/data/photo.jpg"

/* ======================= Global Variables =======================*/
const char* ssid = "SM-A536W2193";
const char* password = "kamikaze1021";
int counter = 0;

// Firebase objects
FirebaseData fbdo;
FirebaseAuth auth;
FirebaseConfig config;

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  Serial.setDebugOutput(true);
  Serial.println();

  // Initialize WiFi
  initWiFi();

  // Initialize Camera
  initCamera();
  configureCameraSensor();

  // Firebase
  config.api_key = API_KEY;           // Assign the api key 
  auth.user.email = USER_EMAIL;       // Assign the user sign in credentials
  auth.user.password = USER_PASSWORD;
  Firebase.begin(&config, &auth);     // Start firebase connection
  Firebase.reconnectWiFi(true);

  Serial.println("Connecting to Firebase...");
  unsigned long timeout = millis() + 10000; // 10 seconds timeout
  while (!Firebase.ready()) {
    Serial.print(".");
    delay(500);
    if (millis() > timeout) {
      Serial.println("\nFailed to connect to Firebase!");
      return;
    }
  }

  Serial.println("\nConnected to Firebase!");

  // Initialization complete
  Serial.println();
  Serial.println("-------------------------------------------------------------------");
  Serial.println("Board initialization successful, commencing image capture");
  Serial.println("-------------------------------------------------------------------");
  Serial.println();
}

void loop() {
  // 
  uploadPhoto();
  // Wait for a certain time interval before capturing the next image
  delay(5000); // 5 seconds delay
}

void initWiFi(){
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi...");
  }
  Serial.println("Connected to WiFi!!!");
}

void uploadPhoto() {
  Serial.printf("Capturing an image: %d\n", counter);

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

  // Upload to Firebase
  String path = "/testImages/" + String(counter) + ".jpg";
  if (Firebase.Storage.upload(&fbdo, STORAGE_BUCKET_ID, fb->buf, fb->len, path.c_str(), "image/jpeg")) {
    Serial.println("Upload success!");
  } else {
    Serial.println("Upload failed: " + fbdo.errorReason());
  }

  esp_camera_fb_return(fb);
  counter++;
}