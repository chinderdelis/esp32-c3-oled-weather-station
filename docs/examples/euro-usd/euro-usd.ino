// EUR -> USD exchange-rate display for the ESP32-C3 0.42" OLED.
//
// Reference example showing how to perform an HTTPS GET on this board.
// NOTE: replace the WiFi credentials below with your own before flashing.
// (The original source contained hardcoded personal credentials and has
//  been redacted on purpose.)

#include <Arduino.h>
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <U8g2lib.h>
#include <Wire.h>
#include <ArduinoJson.h>

// ---- WiFi settings (EDIT THESE) ----
const char* ssid = "YOUR_WIFI_SSID";
const char* password = "YOUR_WIFI_PASSWORD";

// ---- OLED pins ----
#define SDA_PIN 5
#define SCL_PIN 6
U8G2_SSD1306_72X40_ER_F_HW_I2C u8g2(U8G2_R0, /* reset=*/ U8X8_PIN_NONE);

// ---- HTTPS client ----
WiFiClientSecure client;

void drawOLEDMessage(const char* msg) {
  u8g2.clearBuffer();
  u8g2.setDrawColor(1);
  u8g2.setFont(u8g2_font_5x8_tr);
  u8g2.drawStr(0, 12, msg);
  u8g2.sendBuffer();
}

String getEURtoUSD() {
  client.setInsecure();

  if (!client.connect("api.frankfurter.app", 443)) {
    return "Connection failed";
  }

  client.print(
    String("GET /latest?from=EUR&to=USD HTTP/1.1\r\n") +
    "Host: api.frankfurter.app\r\n" +
    "Connection: close\r\n\r\n"
  );

  unsigned long timeout = millis();
  while (!client.available()) {
    if (millis() - timeout > 5000) {
      client.stop();
      return "Timeout";
    }
    delay(10);
  }

  while (client.available()) {
    String line = client.readStringUntil('\n');
    if (line == "\r") break;
  }

  String payload = "";
  while (client.available()) {
    payload += client.readString();
  }
  client.stop();

  DynamicJsonDocument doc(512);
  DeserializationError error = deserializeJson(doc, payload);
  if (error) {
    return "JSON parse error";
  }

  float rate = doc["rates"]["USD"];
  return "1EUR=" + String(rate, 4) + "USD";
}

void setup() {
  Wire.begin(SDA_PIN, SCL_PIN);
  u8g2.begin();

  drawOLEDMessage("Connecting WiFi...");
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(300);
  }
}

void loop() {
  drawOLEDMessage("Fetching rate...");
  String rate = getEURtoUSD();

  u8g2.clearBuffer();
  u8g2.setDrawColor(1);
  u8g2.setFont(u8g2_font_5x8_tr);
  u8g2.drawStr(0, 10, "EUR->USD Rate:");
  u8g2.drawStr(0, 30, rate.c_str());
  u8g2.sendBuffer();

  delay(3600000);
}
