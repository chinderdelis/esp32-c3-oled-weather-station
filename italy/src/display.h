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
  void nextScreen();
  bool getCurrentScreen();

private:
  U8G2_SSD1306_72X40_ER_F_HW_I2C u8g2;
  bool screenToggle;
  unsigned long lastSwitch;
  int currentHour;
  int currentMinute;
  void drawSmiley();
  void drawIconSunnySunglasses(int x, int y);
  void drawIconCloud(int x, int y);
  void drawIconCloudRain(int x, int y);
  void drawClock(int x, int y);
};

extern DisplayMgr displayMgr;

#endif
