#ifndef CONFIG_H
#define CONFIG_H

#include <Arduino.h>

// Pinout
#define SDA_PIN 5
#define SCL_PIN 6
#define BOOT_PIN 9

// OLED
#define OLED_RESET U8X8_PIN_NONE

// NVS Keys
#define NVS_NAMESPACE "c3cfg"
#define NVS_KEY_MODE "mode"
#define NVS_KEY_PROVINCE "province"
#define NVS_KEY_CONFIGURED "configured"

// Connection modes
#define MODE_NONE 0
#define MODE_WIFI 1
#define MODE_BLE 2

// AP Config
#define AP_SSID_PREFIX "ESP32C3-CFG-"

// BLE Config
#define BLE_DEVICE_NAME "ESP32C3-OLED"
#define BLE_PIN "0000"

// Timing
#define BOOT_HOLD_MS 5000
#define SCREEN_ROTATE_MS 10000

// JSON Buffer sizes
#define JSON_BUF_SMALL 256
#define JSON_BUF_MED 512
#define JSON_BUF_LARGE 1024
#define JSON_BUF_GEO 8192

// Weather refresh interval (WiFi mode)
#define WEATHER_REFRESH_MS 900000UL

#endif
