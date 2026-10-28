#include "display.h"

DisplayMgr displayMgr;

DisplayMgr::DisplayMgr() : u8g2(U8G2_R0, /* reset=*/ U8X8_PIN_NONE) {
  screenToggle = true;
  lastSwitch = millis();
  currentHour = 0;
  currentMinute = 0;
}

void DisplayMgr::begin() {
  Wire.begin(SDA_PIN, SCL_PIN);
  u8g2.begin();
  u8g2.setContrast(255);
}

void DisplayMgr::showMessage(const char* line1, const char* line2) {
  u8g2.clearBuffer();
  u8g2.setFont(u8g2_font_5x7_tf);
  u8g2.drawStr(0, 10, line1);
  if (line2) {
    u8g2.drawStr(0, 22, line2);
  }
  u8g2.sendBuffer();
}

void DisplayMgr::showConfigAP(const String &ssid, const String &ip) {
  u8g2.clearBuffer();
  u8g2.setFont(u8g2_font_4x6_tf);
  u8g2.drawStr(0, 8, "CFG AP");
  u8g2.drawStr(0, 18, ssid.c_str());
  u8g2.drawStr(0, 28, ip.c_str());
  u8g2.sendBuffer();
}

void DisplayMgr::showBlePairing() {
  u8g2.clearBuffer();
  u8g2.setFont(u8g2_font_4x6_tf);
  u8g2.drawStr(0, 10, "BLE PAIR");
  u8g2.drawStr(0, 20, "PIN:0000");
  u8g2.sendBuffer();
}

void DisplayMgr::showWifiConnecting() {
  showMessage("WIFI", "CONNECT...");
}

void DisplayMgr::showBleActive() {
  showMessage("BLE", "ACTIVE");
}

void DisplayMgr::drawSmiley() {
  u8g2.drawCircle(36, 20, 12);
  u8g2.drawPixel(32, 18);
  u8g2.drawPixel(40, 18);
  u8g2.drawCircle(36, 24, 6, U8G2_DRAW_UPPER_RIGHT);
  u8g2.drawCircle(36, 24, 6, U8G2_DRAW_UPPER_LEFT);
}

void DisplayMgr::drawIconSunnySunglasses(int x, int y) {
  u8g2.drawCircle(x + 8, y + 8, 6);
  u8g2.drawLine(x + 8, y, x + 8, y + 2);
  u8g2.drawLine(x + 8, y + 14, x + 8, y + 16);
  u8g2.drawLine(x, y + 8, x + 2, y + 8);
  u8g2.drawLine(x + 14, y + 8, x + 16, y + 8);
  u8g2.drawBox(x + 2, y + 6, 5, 4);
  u8g2.drawBox(x + 9, y + 6, 5, 4);
  u8g2.drawLine(x + 7, y + 8, x + 9, y + 8);
}

void DisplayMgr::drawIconCloud(int x, int y) {
  u8g2.drawCircle(x + 4, y + 10, 4);
  u8g2.drawCircle(x + 10, y + 8, 5);
  u8g2.drawCircle(x + 16, y + 10, 4);
  u8g2.drawBox(x + 4, y + 10, 12, 4);
}

void DisplayMgr::drawIconCloudRain(int x, int y) {
  drawIconCloud(x, y - 2);
  u8g2.drawLine(x + 6, y + 12, x + 6, y + 16);
  u8g2.drawLine(x + 10, y + 12, x + 10, y + 16);
  u8g2.drawLine(x + 14, y + 12, x + 14, y + 16);
}

void DisplayMgr::drawClock(int x, int y) {
  u8g2.setFont(u8g2_font_5x7_tf);
  char buf[6];
  sprintf(buf, "%02d:%02d", currentHour, currentMinute);
  u8g2.drawStr(x, y, buf);
}

void DisplayMgr::updateTime(int hour, int minute) {
  currentHour = hour;
  currentMinute = minute;
}

void DisplayMgr::showNormalScreen(bool showIcon, bool isWifiMode, int weatherCode, float tempMin, float tempMax, int hour, int minute) {
  currentHour = hour;
  currentMinute = minute;
  
  unsigned long now = millis();
  if (now - lastSwitch >= SCREEN_ROTATE_MS) {
    screenToggle = !screenToggle;
    lastSwitch = now;
  }
  
  u8g2.clearBuffer();
  
  drawClock(48, 8);
  
  if (screenToggle) {
    if (showIcon) {
      if (weatherCode == 0 || weatherCode == 1) {
        drawIconSunnySunglasses(4, 12);
      } else if (weatherCode >= 2 && weatherCode <= 3) {
        drawIconCloud(4, 10);
      } else {
        drawIconCloudRain(4, 10);
      }
    }
  } else {
    u8g2.setFont(u8g2_font_5x8_tf);
    char buf1[12];
    char buf2[12];
    sprintf(buf1, "MIN:%dC", (int)round(tempMin));
    sprintf(buf2, "MAX:%dC", (int)round(tempMax));
    u8g2.drawStr(4, 24, buf1);
    u8g2.drawStr(4, 36, buf2);
  }
  
  u8g2.sendBuffer();
}

void DisplayMgr::showOffline() {
  u8g2.clearBuffer();
  drawSmiley();
  u8g2.setFont(u8g2_font_4x6_tf);
  u8g2.drawStr(8, 38, "nowifi? nodata");
  u8g2.sendBuffer();
}
