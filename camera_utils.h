#ifndef CAMERA_UTILS_H
#define CAMERA_UTILS_H

#include "esp_system.h"
#include "esp_camera.h"

// /* ======================= Camera Pins =======================*/
// #define PWDN_GPIO_NUM -1
// #define RESET_GPIO_NUM -1
// #define XCLK_GPIO_NUM 15
// #define SIOD_GPIO_NUM 4
// #define SIOC_GPIO_NUM 5

// #define Y2_GPIO_NUM 11
// #define Y3_GPIO_NUM 9
// #define Y4_GPIO_NUM 8
// #define Y5_GPIO_NUM 10
// #define Y6_GPIO_NUM 12
// #define Y7_GPIO_NUM 18
// #define Y8_GPIO_NUM 17
// #define Y9_GPIO_NUM 16

// #define VSYNC_GPIO_NUM 6
// #define HREF_GPIO_NUM 7
// #define PCLK_GPIO_NUM 13
// /* ===========================================================*/

/* ======================= Camera Pins =======================*/
#define PWDN_GPIO_NUM -1
#define RESET_GPIO_NUM -1
#define XCLK_GPIO_NUM 0
#define SIOD_GPIO_NUM 15
#define SIOC_GPIO_NUM 2

#define Y2_GPIO_NUM 7
#define Y3_GPIO_NUM 5
#define Y4_GPIO_NUM 19
#define Y5_GPIO_NUM 8
#define Y6_GPIO_NUM 20
#define Y7_GPIO_NUM 27
#define Y8_GPIO_NUM 26
#define Y9_GPIO_NUM 25

#define VSYNC_GPIO_NUM 32
#define HREF_GPIO_NUM 33
#define PCLK_GPIO_NUM 4
/* ===========================================================*/

void initCamera();
void configureCameraSensor();

#endif