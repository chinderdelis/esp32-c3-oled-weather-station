#include <Arduino.h>
#include <WiFi.h>
#include <time.h>
#include "config.h"
#include "storage.h"
#include "display.h"
#include "wifi_portal.h"
#include "ble_prov.h"
#include "weather.h"

// Cached weather values (updated periodically over WiFi)
int gWeatherCode = 0;
float gTempMin = 0;
float gTempMax = 0;
unsigned long lastWeatherFetch = 0;

void checkBootReset() {
  pinMode(BOOT_PIN, INPUT_PULLUP);
  if (digitalRead(BOOT_PIN) == LOW) {
    unsigned long start = millis();
    displayMgr.showMessage("RESET", "HOLD 5S");
    while (digitalRead(BOOT_PIN) == LOW) {
      if (millis() - start >= BOOT_HOLD_MS) {
        displayMgr.showMessage("RESETTING", "");
        storage.clear();
        delay(1000);
        ESP.restart();
      }
      delay(10);
    }
  }
}

// Resolve the stored city to coordinates/timezone (once) and configure TZ.
void configureLocation() {
  if (storage.getLat() == 0.0 && storage.getLon() == 0.0) {
    double lat = 0, lon = 0;
    String tz = "";
    if (geocode(storage.getProvince(), lat, lon, tz)) {
      storage.setLat(lat);
      storage.setLon(lon);
      if (tz.length()) {
        storage.setTz(tz);
      }
      storage.saveAll();
    } else {
      // City could not be resolved: return to the configuration portal.
      storage.clear();
      ESP.restart();
    }
  }

  if (storage.getTz().length()) {
    setenv("TZ", storage.getTz().c_str(), 1);
  } else {
    setenv("TZ", TZ_INFO, 1);
  }
  tzset();
}

// Refresh the cached weather data at most every WEATHER_REFRESH_MS.
void updateWeatherIfNeeded() {
  if (WiFi.status() != WL_CONNECTED) {
    return;
  }
  unsigned long now = millis();
  if (lastWeatherFetch != 0 && now - lastWeatherFetch < WEATHER_REFRESH_MS) {
    return;
  }
  int code = 0;
  float tmin = 0, tmax = 0;
  if (fetchWeather(storage.getLat(), storage.getLon(), code, tmin, tmax)) {
    gWeatherCode = code;
    gTempMin = tmin;
    gTempMax = tmax;
    lastWeatherFetch = now;
  } else if (lastWeatherFetch == 0) {
    // Avoid hammering the API on repeated failures.
    lastWeatherFetch = now;
  }
}

void setup() {
  Serial.begin(115200);
  delay(100);

  displayMgr.begin();
  storage.begin();

  checkBootReset();

  if (!storage.isConfigured()) {
    wifiPortal.begin();
    bleProv.begin();

    unsigned long startWait = millis();
    while (millis() - startWait < 180000) {
      wifiPortal.handleClient();
      if (wifiPortal.isConfigured()) {
        break;
      }
      delay(10);
    }

    uint8_t mode = storage.getMode();
    if (mode == MODE_WIFI) {
      bleProv.stop();
      unsigned long wifiStart = millis();
      while (WiFi.status() != WL_CONNECTED && millis() - wifiStart < 30000) {
        displayMgr.showWifiConnecting();
        delay(500);
      }
      if (WiFi.status() == WL_CONNECTED) {
        WiFi.softAPdisconnect(true);
        WiFi.mode(WIFI_STA);
        configTime(0, 0, "pool.ntp.org", "time.nist.gov");
        configureLocation();
      } else {
        storage.clear();
        ESP.restart();
      }
    } else if (mode == MODE_BLE) {
      WiFi.mode(WIFI_OFF);
      displayMgr.showBlePairing();
    }
  } else {
    uint8_t mode = storage.getMode();
    if (mode == MODE_WIFI) {
      bleProv.stop();
      WiFi.mode(WIFI_STA);
      configTime(0, 0, "pool.ntp.org", "time.nist.gov");
      configureLocation();
    } else if (mode == MODE_BLE) {
      WiFi.mode(WIFI_OFF);
      bleProv.begin();
      displayMgr.showBlePairing();
    }
  }
}

void loop() {
  uint8_t mode = storage.getMode();

  if (mode == MODE_WIFI) {
    if (WiFi.status() == WL_CONNECTED) {
      struct tm timeinfo = {0};
      if (getLocalTime(&timeinfo)) {
        displayMgr.updateTime(timeinfo.tm_hour, timeinfo.tm_min);
      }
      updateWeatherIfNeeded();
      displayMgr.showNormalScreen(true, true, gWeatherCode, gTempMin, gTempMax,
                                 timeinfo.tm_hour, timeinfo.tm_min);
    } else {
      displayMgr.showOffline();
    }
  } else if (mode == MODE_BLE) {
    if (bleProv.hasData()) {
      int hour, minute, weatherCode;
      float tempMin, tempMax;
      bleProv.getData(hour, minute, weatherCode, tempMin, tempMax);
      displayMgr.updateTime(hour, minute);
      displayMgr.showNormalScreen(true, false, weatherCode, tempMin, tempMax, hour, minute);
    } else {
      displayMgr.showOffline();
    }
  }

  delay(100);
}
