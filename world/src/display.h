#ifndef DISPLAY_H
#define DISPLAY_H

#include <Arduino.h>
#include <U8g2lib.h>
#include <Wire.h>
#include "config.h"

class DisplayMgr {
public:
  DisplayMgr();
  void begin();
  void showMessage(const char* line1, const char* line2 = nullptr);
  void showConfigAP(const String &ssid, const String &ip);
  void showBlePairing();
  void showWifiConnecting();
  void showBleActive();
  void showNormalScreen(bool showIcon, bool isWifiMode, int weatherCode, float tempMin, float tempMax, int hour, int minute);
  void showOffline();
  void updateTime(int hour, int minute);

private:
  U8G2_SSD1306_72X40_ER_F_HW_I2C u8g2;
  bool normalActive;
  uint8_t screenIndex;
  unsigned long lastSwitch;
  int currentHour;
  int currentMinute;
  void drawSmiley();
  void drawWeatherIcon(int weatherCode);
  void drawTemperature(float temp, const char *label);
  void drawClockBig();
  const uint8_t *pickNumberFont(const char *text, bool withDegree);
  void drawCenteredNumber(const char *text, bool withDegree);
};

extern DisplayMgr displayMgr;

#endif
