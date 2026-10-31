#include "display.h"
#include <math.h>

DisplayMgr displayMgr;

static const uint8_t *const kNumberFonts[] = {
  u8g2_font_logisoso38_tn,
  u8g2_font_logisoso34_tn,
  u8g2_font_logisoso30_tn,
  u8g2_font_logisoso28_tn,
  u8g2_font_logisoso26_tn,
  u8g2_font_logisoso24_tn,
  u8g2_font_logisoso20_tn,
  u8g2_font_logisoso16_tn
};

static const unsigned long kScreenMs[] = {
  SCREEN_ICON_MS,
  SCREEN_MIN_MS,
  SCREEN_MAX_MS,
  SCREEN_CLOCK_MS
};

DisplayMgr::DisplayMgr() : u8g2(U8G2_R0, /* reset=*/ U8X8_PIN_NONE) {
  normalActive = false;
  screenIndex = 0;
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
  normalActive = false;
  u8g2.clearBuffer();
  u8g2.setFont(u8g2_font_5x7_tf);
  u8g2.drawStr(0, 10, line1);
  if (line2) {
    u8g2.drawStr(0, 22, line2);
  }
  u8g2.sendBuffer();
}

void DisplayMgr::showConfigAP(const String &ssid, const String &ip) {
  normalActive = false;
  u8g2.clearBuffer();
  u8g2.setFont(u8g2_font_4x6_tf);
  u8g2.drawStr(0, 8, "CFG AP");
  u8g2.drawStr(0, 18, ssid.c_str());
  u8g2.drawStr(0, 28, ip.c_str());
  u8g2.sendBuffer();
}

void DisplayMgr::showBlePairing() {
  normalActive = false;
  u8g2.clearBuffer();
  u8g2.setFont(u8g2_font_4x6_tf);
  u8g2.drawStr(0, 10, "BLE PAIR");
  u8g2.drawStr(0, 20, "PIN:000000");
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

void DisplayMgr::drawWeatherIcon(int weatherCode) {
  if (weatherCode == 0 || weatherCode == 1) {
    const int cx = 36;
    const int cy = 20;
    u8g2.drawDisc(cx, cy, 11, U8G2_DRAW_ALL);
    static const int8_t dx[8] = {1, 1, 0, -1, -1, -1, 0, 1};
    static const int8_t dy[8] = {0, 1, 1, 1, 0, -1, -1, -1};
    for (uint8_t i = 0; i < 8; i++) {
      u8g2.drawLine(cx + dx[i] * 14, cy + dy[i] * 14,
                    cx + dx[i] * 18, cy + dy[i] * 18);
    }
    return;
  }

  bool rain = !(weatherCode >= 2 && weatherCode <= 3);
  int cy = rain ? 16 : 20;
  u8g2.drawDisc(23, cy + 4, 8, U8G2_DRAW_ALL);
  u8g2.drawDisc(34, cy - 1, 10, U8G2_DRAW_ALL);
  u8g2.drawDisc(47, cy + 4, 8, U8G2_DRAW_ALL);
  u8g2.drawBox(23, cy + 4, 25, 8);

  if (rain) {
    u8g2.drawLine(26, 30, 23, 37);
    u8g2.drawLine(36, 30, 33, 37);
    u8g2.drawLine(46, 30, 43, 37);
  }
}

const uint8_t *DisplayMgr::pickNumberFont(const char *text, bool withDegree) {
  for (uint8_t i = 0; i < sizeof(kNumberFonts) / sizeof(kNumberFonts[0]); i++) {
    u8g2.setFont(kNumberFonts[i]);
    int h = u8g2.getAscent() - u8g2.getDescent();
    if (h > 40) {
      continue;
    }
    int r = h / 12;
    if (r < 2) {
      r = 2;
    }
    int extra = withDegree ? (3 * r) : 0;
    if ((int)u8g2.getStrWidth(text) + extra <= 72) {
      return kNumberFonts[i];
    }
  }
  return u8g2_font_logisoso16_tn;
}

void DisplayMgr::drawCenteredNumber(const char *text, bool withDegree) {
  u8g2.setFont(pickNumberFont(text, withDegree));
  int8_t ascent = u8g2.getAscent();
  int8_t descent = u8g2.getDescent();
  int h = ascent - descent;
  int r = withDegree ? (h / 12) : 0;
  if (withDegree && r < 2) {
    r = 2;
  }
  int w = u8g2.getStrWidth(text);
  int extra = withDegree ? (3 * r) : 0;
  int x = (72 - (w + extra)) / 2;
  int y = (40 + ascent + descent) / 2;

  u8g2.drawStr(x, y, text);

  if (withDegree) {
    int cx = x + w + r;
    int cy = y - ascent + r;
    u8g2.drawDisc(cx, cy, r, U8G2_DRAW_ALL);
  }
}

void DisplayMgr::drawTemperature(float temp, const char *label) {
  char buf[8];
  snprintf(buf, sizeof(buf), "%d", (int)roundf(temp));
  drawCenteredNumber(buf, true);
  u8g2.setFont(u8g2_font_4x6_tf);
  u8g2.drawStr(72 - u8g2.getStrWidth(label), 39, label);
}

void DisplayMgr::drawClockBig() {
  char buf[6];
  snprintf(buf, sizeof(buf), "%02d:%02d", currentHour, currentMinute);
  drawCenteredNumber(buf, false);
}

void DisplayMgr::updateTime(int hour, int minute) {
  currentHour = hour;
  currentMinute = minute;
}

void DisplayMgr::showNormalScreen(bool showIcon, bool isWifiMode, int weatherCode, float tempMin, float tempMax, int hour, int minute) {
  (void)showIcon;
  (void)isWifiMode;
  currentHour = hour;
  currentMinute = minute;

  unsigned long now = millis();
  if (!normalActive) {
    normalActive = true;
    screenIndex = 0;
    lastSwitch = now;
  } else if (now - lastSwitch >= kScreenMs[screenIndex]) {
    screenIndex = (screenIndex + 1) % 4;
    lastSwitch = now;
  }

  u8g2.clearBuffer();

  switch (screenIndex) {
    case 0:
      drawWeatherIcon(weatherCode);
      break;
    case 1:
      drawTemperature(tempMin, "MIN");
      break;
    case 2:
      drawTemperature(tempMax, "MAX");
      break;
    default:
      drawClockBig();
      break;
  }

  u8g2.sendBuffer();
}

void DisplayMgr::showOffline() {
  normalActive = false;
  u8g2.clearBuffer();
  drawSmiley();
  u8g2.setFont(u8g2_font_4x6_tf);
  u8g2.drawStr(8, 38, "nowifi? nodata");
  u8g2.sendBuffer();
}
