/* ======================= File Includes =======================*/
#include <Arduino.h>
#include <WiFi.h>
#include <Firebase_ESP_Client.h>
#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include "camera_utils.h"
#include "connection_utils.h"
// #include <BLE2902.h>

/* ======================= Global Variables ======================= */
int counter = 0;

// Firebase objects
FirebaseData fbdo;
FirebaseAuth auth;
FirebaseConfig config;

// Bluetooth variables
BLECharacteristic *pWifiID;
BLECharacteristic *pWifiPassword;
BLECharacteristic *pUserEmail;
BLECharacteristic *pUserPassword;
BLECharacteristic *pTimestamp;

bool bluetoothConnection = false;
String wifiID;
String wifiPassword;
String userEmail;
String userPassword;
String timestamp;

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
      } else if (uuid == TIMESTAMP_UUID) {
        timestamp = value;
        Serial.print("Timestamp received: ");
        Serial.println(value);
      }
    }
  }
};

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  Serial.setDebugOutput(true);
  Serial.println();

  // Initialize Bluetooth
  initBluetooth();

  // Initialize Camera
  initCamera();
  configureCameraSensor();

  // Initialization complete
  Serial.println("\n-------------------------------------------------------------------");
  Serial.println("Board initialization successful, commencing image capture");
  Serial.println("-------------------------------------------------------------------\n");
}

void loop() {
  if (WiFi.status() != WL_CONNECTED) {              // Check for WiFi connection, if not then try to establish one

    // Notify flutter app to send WiFi credentials
    pWifiID->setValue("request");
    pWifiPassword->setValue("request");
    pWifiID->notify();
    pWifiPassword->notify();
    
    // Attempt to connect 10 times with received credentials
    Serial.print("Connecting to WiFi...");
    for (int i = 0; i < 10; i++) {
      WiFi.begin(wifiID, wifiPassword);
      delay(1000);
      if (WiFi.status() == WL_CONNECTED) {
        break;
      }
      Serial.print(".");
    }

    Serial.println();
    
  } else {
    if (!Firebase.ready()) {     
      // Notify flutter app to send WiFi credentials
      pUserEmail->setValue("request");
      pUserPassword->setValue("request");
      pUserEmail->notify();
      pUserPassword->notify();

      Serial.println("No connection to Firebase!");
      connectFirebase();
    }
    else {
      Serial.println("Got here!!!");
      // Wait for a certain time interval before capturing the next image
      // uploadPhoto();
      delay(5000); // 5 seconds delay
    }
  }
}

void initBluetooth() {
  // Initialize BLE
  BLEDevice::init("PatchNotes-Device-1");
  BLEDevice::setMTU(50);
  BLEServer *pServer = BLEDevice::createServer();
  BLEService *pService = pServer->createService(SERVICE_UUID);

  // Define characteristics
  pWifiID = pService->createCharacteristic(
                                        WIFI_ID_UUID,
                                        BLECharacteristic::PROPERTY_NOTIFY | BLECharacteristic::PROPERTY_WRITE
                                       );

  pWifiPassword = pService->createCharacteristic(
                                      WIFI_PASSWORD_UUID,
                                      BLECharacteristic::PROPERTY_NOTIFY | BLECharacteristic::PROPERTY_WRITE
                                      );

  pUserEmail = pService->createCharacteristic(
                                        USER_EMAIL_UUID,
                                        BLECharacteristic::PROPERTY_NOTIFY | BLECharacteristic::PROPERTY_WRITE
                                       );

  pUserPassword = pService->createCharacteristic(
                                        USER_PASSWORD_UUID,
                                        BLECharacteristic::PROPERTY_NOTIFY | BLECharacteristic::PROPERTY_WRITE
                                       );

  pTimestamp = pService->createCharacteristic(
                                        TIMESTAMP_UUID,
                                        BLECharacteristic::PROPERTY_NOTIFY | BLECharacteristic::PROPERTY_WRITE
                                       );

  // Bind callbacks
  pServer->setCallbacks(new ServerCallbacks());
  pWifiID->setCallbacks(new CharacteristicCallbacks());
  pWifiPassword->setCallbacks(new CharacteristicCallbacks());
  pUserEmail->setCallbacks(new CharacteristicCallbacks());
  pUserPassword->setCallbacks(new CharacteristicCallbacks());
  pTimestamp->setCallbacks(new CharacteristicCallbacks());

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

void connectFirebase() {
  config.api_key = API_KEY;           // Assign the api key 
  auth.user.email = userEmail;        // Assign the user sign in credentials
  auth.user.password = userPassword;
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
  Serial.println("\nConnected to Firebase!\n");
}

void uploadPhoto() {
  Serial.printf("Capturing an image: %d\n", counter);

  // Frame buffer pointer
  camera_fb_t* fb = NULL;

  // Doing it a bunch to let the auto adjusting parameters settle... (testing needed)
  for (uint8_t i = 0; i < 5; i++) {
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
