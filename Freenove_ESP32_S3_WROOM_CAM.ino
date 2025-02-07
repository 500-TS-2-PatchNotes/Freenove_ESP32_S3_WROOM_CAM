#include "esp_system.h"
#include "esp_camera.h"


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

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  Serial.setDebugOutput(true);
  Serial.printf("Initializing Camera\n");
  Serial.printf("-------------------------------------------------------------------\n");
  Serial.println();

  // Camera configuration
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
    .frame_size     = FRAMESIZE_SVGA,
    .jpeg_quality   = 12,

    .fb_count       = 1,
    .fb_location    = CAMERA_FB_IN_DRAM,
    .grab_mode      = CAMERA_GRAB_WHEN_EMPTY
  };

  // Check if PSRAM is available, if so then use higher jpeg quality and increased frame size
  #if 1
  if (psramFound()) {
    camera_config.jpeg_quality = 5;
    camera_config.frame_size   = FRAMESIZE_UXGA;
    camera_config.fb_location  = CAMERA_FB_IN_PSRAM;
  }
  #endif

  // Initialize camera with configuration
  esp_err_t error_status = esp_camera_init(&camera_config);
  if (error_status != ESP_OK) {
    Serial.printf("Camera initialization failed with error 0x%x\n", error_status);
    Serial.println();
    Serial.printf("-------------------------------------------------------------------\n");
    return;
  }

  // Configure sensor if needed (TODO)
  sensor_t* camera_sensor = esp_camera_sensor_get();
  camera_sensor->set_vflip(camera_sensor, 1); // flip it back
  camera_sensor->set_brightness(camera_sensor, 1); // up the brightness just a bit
  camera_sensor->set_saturation(camera_sensor, 0); // lower the saturation

  // Initialization complete
  Serial.println();
  Serial.printf("-------------------------------------------------------------------");
  Serial.printf("Camera initialization successful, commencing image capture");
  Serial.printf("-------------------------------------------------------------------");
  Serial.println();
}

void loop() {
  // Capture an image
  camera_fb_t* fb = NULL;
  fb = esp_camera_fb_get();
  if (!fb) {
    Serial.println("Camera capture failed");
    return;
  } else {
    Serial.printf("Captured image size: %d bytes\n", fb->len);
    Serial.printf("Total PSRAM: %d bytes\n", ESP.getPsramSize());
    Serial.printf("Free PSRAM: %d bytes\n", ESP.getFreePsram());
    Serial.printf("Used PSRAM: %d bytes\n", ESP.getPsramSize() - ESP.getFreePsram());
  }

  Serial.println("Image Start...");

  // Send size of buffer
  Serial.write((uint8_t*)&fb->len, sizeof(fb->len));

  // Send frame buffer
  Serial.write(fb->buf, fb->len);

  Serial.println("Image End...");

  // Return fb
  esp_camera_fb_return(fb);

  // Wait for a certain time interval before capturing the next image
  delay(3000); // 5 seconds delay

}
