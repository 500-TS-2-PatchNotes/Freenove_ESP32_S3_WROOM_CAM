#include "camera_utils.h"
#include <Arduino.h>

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
    .frame_size     = FRAMESIZE_HVGA,
    .jpeg_quality   = 15,

    .fb_count       = 1,
    .fb_location    = CAMERA_FB_IN_DRAM,
    .grab_mode      = CAMERA_GRAB_WHEN_EMPTY
  };

  // Check if PSRAM is available, if so then use higher jpeg quality and increased frame size
  #if 1
  if (psramFound()) {
    Serial.println("PSRAM Found");
    // camera_config.pixel_format = PIXFORMAT_RGB888;  // does not work, cuz camera is dooks
    camera_config.jpeg_quality = 5;
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

