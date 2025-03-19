#include "esp_system.h"
#include "esp_camera.h"

#include <WiFi.h>
#include <FS.h>
#include <Firebase_ESP_Client.h>

#define CAMERA_MODEL_ESP32S3_EYE

// Insert Firebase project API Key
#define API_KEY "AIzaSyB2NdPoW8WFrn0s1MxQmOsMkYrbpXraQNs"

// Insert Authorized Email and Corresponding Password
#define USER_EMAIL "nathanante36@gmail.com"
#define USER_PASSWORD "Kamikaze102198!"

// Insert Firebase storage bucket ID e.g bucket-name.appspot.com
#define STORAGE_BUCKET_ID "patchnotes-d3b06.firebasestorage.app"
// For example:
//#define STORAGE_BUCKET_ID "esp-iot-app.appspot.com"

// Photo File Name to save in LittleFS
#define FILE_PHOTO_PATH "/photo.jpg"
#define BUCKET_PHOTO "/data/photo.jpg"

int counter = 0;

// ===================================
// Camera Pins
// ===================================
#define PWDN_GPIO_NUM -1
#define RESET_GPIO_NUM -1
#define XCLK_GPIO_NUM 15
#define SIOD_GPIO_NUM 4
#define SIOC_GPIO_NUM 5

#define Y2_GPIO_NUM 11
#define Y3_GPIO_NUM 9
#define Y4_GPIO_NUM 8
#define Y5_GPIO_NUM 10
#define Y6_GPIO_NUM 12
#define Y7_GPIO_NUM 18
#define Y8_GPIO_NUM 17
#define Y9_GPIO_NUM 16

#define VSYNC_GPIO_NUM 6
#define HREF_GPIO_NUM 7
#define PCLK_GPIO_NUM 13
// ===================================

const char* ssid = "SM-A536W2193";
const char* password = "kamikaze1021";

// Firebase objects
FirebaseData fbdo;
FirebaseAuth auth;
FirebaseConfig config;

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  Serial.setDebugOutput(true);
  Serial.println();

  // Initialize WiFi and Little File System
  initWiFi();
  // initLittleFS();

  // Initialize Camera
  initCamera();
  //configureCameraSensor();

  // Firebase
  // Assign the api key 
  config.api_key = API_KEY;
  //Assign the user sign in credentials
  auth.user.email = USER_EMAIL;
  auth.user.password = USER_PASSWORD;
  Firebase.begin(&config, &auth);
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
  // // Frame buffer pointer
  // camera_fb_t* fb = NULL;

  // // Doing it a bunch to let the auto adjusting parameters settle... (testing needed)
  // for (uint8_t i = 0; i < 25; i++) {
  //   fb = esp_camera_fb_get();
  //   esp_camera_fb_return(fb);
  //   fb = NULL;
  // }
  
  // // Capture Image
  // fb = esp_camera_fb_get();

  // if (!fb) {
  //   Serial.println("Camera capture failed");
  //   return;
  // } else {
  //   Serial.printf("Captured image size: %d bytes\n", fb->len);
  //   Serial.printf("Total PSRAM: %d bytes\n", ESP.getPsramSize());
  //   Serial.printf("Free PSRAM: %d bytes\n", ESP.getFreePsram());
  //   Serial.printf("Used PSRAM: %d bytes\n", ESP.getPsramSize() - ESP.getFreePsram());
  // }

  // Serial.println("Image Start...");

  // // Send size of buffer
  // Serial.write((uint8_t*)&fb->len, sizeof(fb->len));

  // // Send frame buffer
  // Serial.write(fb->buf, fb->len);

  // Serial.println("Image End...");

  // Return fb
  // esp_camera_fb_return(fb);

  uploadPhoto();

  // Wait for a certain time interval before capturing the next image
  delay(5000); // 5 seconds delay
}

void initCamera() {
  Serial.println("Initializing Camera");
  Serial.println("-------------------------------------------------------------------");

  // Initialize camera interface with correct pins
  static camera_config_t camera_config = {
    .pin_pwdn       = PWDN_GPIO_NUM,
    .pin_reset      = RESET_GPIO_NUM,
    .pin_xclk       = XCLK_GPIO_NUM,
    .pin_sccb_sda   = SIOD_GPIO_NUM,
    .pin_sccb_scl   = SIOC_GPIO_NUM,
    .pin_d7         = Y9_GPIO_NUM,
    .pin_d6         = Y8_GPIO_NUM,
    .pin_d5         = Y7_GPIO_NUM,
    .pin_d4         = Y6_GPIO_NUM,
    .pin_d3         = Y5_GPIO_NUM,
    .pin_d2         = Y4_GPIO_NUM,
    .pin_d1         = Y3_GPIO_NUM,
    .pin_d0         = Y2_GPIO_NUM,
    .pin_vsync      = VSYNC_GPIO_NUM,
    .pin_href       = HREF_GPIO_NUM,
    .pin_pclk       = PCLK_GPIO_NUM,

    .xclk_freq_hz   = 20000000,         // Camera input clock, experiment with this later (max 24 mhz) | Try 10Mhz and 6Mhz 
    .ledc_timer     = LEDC_TIMER_0,     // dW bout this (for leds on it)
    .ledc_channel   = LEDC_CHANNEL_0,   // dw bout this (for leds on it)

    .pixel_format   = PIXFORMAT_JPEG,   
    .frame_size     = FRAMESIZE_VGA,
    .jpeg_quality   = 10,

    .fb_count       = 1,
    .fb_location    = CAMERA_FB_IN_DRAM,
    .grab_mode      = CAMERA_GRAB_WHEN_EMPTY
  };

  // Check if PSRAM is available, if so then use higher jpeg quality and increased frame size
  #if 1
  if (psramFound()) {
    Serial.println("PSRAM Found");
    // camera_config.pixel_format = PIXFORMAT_RGB888;  // does not work, cuz camera is dooks
    camera_config.jpeg_quality = 6;
    camera_config.frame_size   = FRAMESIZE_VGA;
    camera_config.fb_location  = CAMERA_FB_IN_PSRAM;
  }
  #endif

  // Initialize camera with configuration
  esp_err_t error_status = esp_camera_init(&camera_config);
  if (error_status != ESP_OK) {
    while (1) {
      Serial.printf("Camera initialization failed with error 0x%x\n", error_status);
      Serial.println();
      delay(500);
    }
  } else {
    Serial.printf("Camera initialization was successful\n");
  }
}

void configureCameraSensor() {
  Serial.println("Configuring camera sensor parameters");
  // Main ones to adjust are aec_value, agc_gain
  sensor_t *s = esp_camera_sensor_get();
  
  // Brightness, Contrast, and, Saturation
  s->set_brightness(s, 0);     // -2 to 2
  s->set_contrast(s, 0);       // -2 to 2
  s->set_saturation(s, 0);     // -2 to 2

  // Special Effects
  s->set_special_effect(s, 0); // 0 to 6 (0 - No Effect, 1 - Negative, 2 - Grayscale, 3 - Red Tint, 4 - Green Tint, 5 - Blue Tint, 6 - Sepia)

  // White Balance
  s->set_whitebal(s, 1);       // 0 = disable , 1 = enable
  s->set_awb_gain(s, 1);       // 0 = disable , 1 = enable
  s->set_wb_mode(s, 4);        // 0 to 4 - if awb_gain enabled (0 - Auto, 1 - Sunny, 2 - Cloudy, 3 - Office, 4 - Home)

  // Exposure control
  /*
  Notes:
    - aec2: auto exposure control
    - aec value: higher values for darker environments
  */
  s->set_exposure_ctrl(s, 1);  // 0 = disable , 1 = enable
  s->set_aec2(s, 0);           // 0 = disable , 1 = enable
  s->set_ae_level(s, 0);       // -2 to 2
  s->set_aec_value(s, 100);    // 0 to 1200

  // Gain Control
  s->set_gain_ctrl(s, 1);      // 0 = disable , 1 = enable
  s->set_agc_gain(s, 0);       // 0 to 30
  s->set_gainceiling(s, (gainceiling_t)0);  // 0 to 6

  // Image processing
  /*
  Notes:
    - bpc: black pixel correction
    - wpc: white pixel correction
    - raw_gma: gamma correction
    - lenc: lens correction for distortion
  */
  s->set_bpc(s, 0);            // 0 = disable , 1 = enable
  s->set_wpc(s, 0);            // 0 = disable , 1 = enable
  s->set_raw_gma(s, 1);        // 0 = disable , 1 = enable
  s->set_lenc(s, 1);           // 0 = disable , 1 = enable

  // Mirror and Flip
  s->set_hmirror(s, 0);        // 0 = disable , 1 = enable
  s->set_vflip(s, 0);          // 0 = disable , 1 = enable

  // dcw: downscaling (apparently good for color and noise, but reduces resolution)
  s->set_dcw(s, 1);            // 0 = disable , 1 = enable
  s->set_colorbar(s, 0);       // 0 = disable , 1 = enable
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