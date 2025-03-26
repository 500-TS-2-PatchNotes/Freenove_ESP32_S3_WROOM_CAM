/* ======================= File Includes =======================*/
#include <Arduino.h>
#include <WiFi.h>
#include <Firebase_ESP_Client.h>
#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include "camera_utils.h"
// #include <BLE2902.h>

/* ======================= BLE Prototype Defines =======================*/
#define SERVICE_UUID        "bf897488-0ed2-4425-8051-a4a26ef47c56"
#define USER_EMAIL_UUID     "7d7d38ae-19f7-47d0-8abf-0608322fd9ed"
#define USER_PASSWORD_UUID  "4c45320c-2df8-4af8-81d3-e7c034091d44"
#define WIFI_ID_UUID        "f715c633-51b8-44aa-9726-caffb54f89e9"
#define WIFI_PASSWORD_UUID  "b8c4c56e-9b7e-46f3-87f7-dba6911b0460"
#define TIMESTAMP_UUID      "ce84f75a-0aea-48cf-ba77-aec1a27a9169"

/* ======================= Firebase Prototype Defines =======================*/
#define API_KEY "AIzaSyB2NdPoW8WFrn0s1MxQmOsMkYrbpXraQNs"            // Insert Firebase project API Key
#define USER_EMAIL "nathanante36@gmail.com"                          // Insert Authorized Email and Corresponding Password
#define USER_PASSWORD "Kamikaze102198!"
#define STORAGE_BUCKET_ID "patchnotes-d3b06.firebasestorage.app"     // Insert Firebase storage bucket ID e.g bucket-name.appspot.com
#define FILE_PHOTO_PATH "/photo.jpg"                                 // Prototypes for filename in backend storage
#define BUCKET_PHOTO "/data/photo.jpg"

/* ======================= Global Variables ======================= */
const char* ssid = "SM-A536W2193";
const char* password = "kamikaze1021";
int counter = 0;

// Firebase objects
FirebaseData fbdo;
FirebaseAuth auth;
FirebaseConfig config;

// Bluetooth variables
bool bluetoothConnection = false;
String wifiID;
String wifiPassword;
String userEmail;
String userPassword;

/* ======================= BLE Callbacks ======================= */
class ServerCallbacks : public BLEServerCallbacks {
  void onConnect(BLEServer* pServer) { 
    bluetoothConnection = true;
    Serial.println("Bluetooth device connected...");
  }
  void onDisconnect(BLEServer* pServer) {
    bluetoothConnection = false;
    Serial.println("Bluetooth device disconnected...");
    BLEDevice::startAdvertising();
  }
};

class CharacteristicCallbacks : public BLECharacteristicCallbacks {
  void onWrite(BLECharacteristic* pCharacteristic) {
    String value = pCharacteristic->getValue();
    String uuid = pCharacteristic->getUUID().toString();
    
    // Make sure value isnt empty
    if (value.length() > 0) {
      Serial.print("Received value for UUID: ");
      Serial.println(uuid);

      // Store the received values in the appropriate variables
      if (uuid == USER_EMAIL_UUID) {
        userEmail = value;
        Serial.print("User email received: ");
        Serial.println(value);
      } else if (uuid == USER_PASSWORD_UUID) {
        userPassword = value;
        Serial.print("User password received: ");
        Serial.println(value);
      } else if (uuid == WIFI_ID_UUID) {
        wifiID = value;
        Serial.print("Wifi ID received: ");
        Serial.println(value);
      } else if (uuid == WIFI_PASSWORD_UUID) {
        wifiPassword = value;
        Serial.print("Wifi password received: ");
        Serial.println(value);
      }
    }
  }
};

/*
class UserEmailCallbacks : public BLECharacteristicCallbacks {
  void onWrite(BLECharacteristic* pCharacteristic) {
    
  } 
  void onRead(BLECharacteristic* pCharacteristic) {
    // add here
  } 
};

class UserPasswordCallbacks : public BLECharacteristicCallbacks {
  void onWrite(BLECharacteristic* pCharacteristic) {
    // add here
  } 
  void onRead(BLECharacteristic* pCharacteristic) {
    // add here
  } 
};

class WifiIDCallbacks : public BLECharacteristicCallbacks {
  void onWrite(BLECharacteristic* pCharacteristic) {
    // add here
  } 
  void onRead(BLECharacteristic* pCharacteristic) {
    // add here
  } 
};

class WifiPasswordCallbacks : public BLECharacteristicCallbacks {
  void onWrite(BLECharacteristic* pCharacteristic) {
    // add here
  } 
  void onRead(BLECharacteristic* pCharacteristic) {
    // add here
  } 
};
*/

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  Serial.setDebugOutput(true);
  Serial.println();

  //
  initBluetooth();

  // Initialize WiFi
  // initWiFi();

  // Initialize Camera
  initCamera();
  configureCameraSensor();

  // Firebase
  // initFirebase();

  // Initialization complete
  Serial.println();
  Serial.println("-------------------------------------------------------------------");
  Serial.println("Board initialization successful, commencing image capture");
  Serial.println("-------------------------------------------------------------------");
  Serial.println();
}

void loop() {
  // 
  // uploadPhoto();
  // Wait for a certain time interval before capturing the next image
  Serial.println(".");
  delay(1000); // 5 seconds delay
}

void initWiFi(){
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi...");
  }
  Serial.println("Connected to WiFi!!!");
}

void initFirebase() {
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
}

void initBluetooth() {
  // Initialize BLE
  BLEDevice::init("PatchNotes-Device-1");
  BLEServer *pServer = BLEDevice::createServer();
  BLEService *pService = pServer->createService(SERVICE_UUID);

  // Define characteristics
  BLECharacteristic *pWifiID = pService->createCharacteristic(
                                        WIFI_ID_UUID,
                                        BLECharacteristic::PROPERTY_READ | BLECharacteristic::PROPERTY_WRITE
                                       );

  BLECharacteristic *pWifiPassword = pService->createCharacteristic(
                                      WIFI_PASSWORD_UUID,
                                      BLECharacteristic::PROPERTY_READ | BLECharacteristic::PROPERTY_WRITE
                                      );

  BLECharacteristic *pUserEmail = pService->createCharacteristic(
                                        USER_EMAIL_UUID,
                                        BLECharacteristic::PROPERTY_READ | BLECharacteristic::PROPERTY_WRITE
                                       );

  BLECharacteristic *pUserPassword = pService->createCharacteristic(
                                        USER_PASSWORD_UUID,
                                        BLECharacteristic::PROPERTY_READ | BLECharacteristic::PROPERTY_WRITE
                                       );

  // Bind callbacks
  pServer->setCallbacks(new ServerCallbacks());
  //pCharacteristic->setCallbacks(new CharacteristicCallbacks());
  // pWifiID->setCallbacks(new WifiIDCallbacks());
  // pWifiPassword->setCallbacks(new WifiPasswordCallbacks());
  // pUserEmail->setCallbacks(new UserEmailCallbacks());
  // pUserPassword->setCallbacks(new UserPasswordCallbacks());
  pWifiID->setCallbacks(new CharacteristicCallbacks());
  pWifiPassword->setCallbacks(new CharacteristicCallbacks());
  pUserEmail->setCallbacks(new CharacteristicCallbacks());
  pUserPassword->setCallbacks(new CharacteristicCallbacks());

  // Start the service
  pService->start();

  // Start Advertising
  BLEAdvertising *pAdvertising = BLEDevice::getAdvertising();
  pAdvertising->addServiceUUID(SERVICE_UUID);
  pAdvertising->setScanResponse(true);
  pAdvertising->setMinPreferred(0x06);  // functions that help with iPhone connections issue
  pAdvertising->setMinPreferred(0x12);
  BLEDevice::startAdvertising();
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