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
int gUtcOffsetSeconds = 0;
unsigned long lastWeatherFetch = 0;
unsigned long lastGeoTry = 0;

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

// Resolve the stored province to coordinates. The search is constrained to
// Italy and the most populous match is used. Compound names that the API does
// not know (e.g. "Forli-Cesena") fall back to their first part.
bool doGeocode() {
  double lat = 0, lon = 0;
  String tz = "";
  String location = storage.getProvince();
  if (location == "Sud Sardegna") {
    location = "Carbonia";  // province capital; the compound name is not indexed
  }

  bool ok = geocode(location, lat, lon, tz, "IT", "it");
  if (!ok) {
    int cut = location.indexOf('-');
    if (cut < 0) {
      cut = location.indexOf(" e ");
    }
    if (cut > 0) {
      ok = geocode(location.substring(0, cut), lat, lon, tz, "IT", "it");
    }
  }

  if (ok) {
    storage.setLat(lat);
    storage.setLon(lon);
    storage.saveAll();
  }
  return ok;
}

// Refresh the cached weather data at most every WEATHER_REFRESH_MS. If the
// location is not resolved yet, retry geocoding at most once a minute.
void updateWeatherIfNeeded() {
  if (WiFi.status() != WL_CONNECTED) {
    return;
  }
  unsigned long now = millis();

  if (storage.getLat() == 0.0 && storage.getLon() == 0.0) {
    if (lastGeoTry != 0 && now - lastGeoTry < 60000UL) {
      return;
    }
    lastGeoTry = now;
    if (!doGeocode()) {
      return;
    }
  }

  if (lastWeatherFetch != 0 && now - lastWeatherFetch < WEATHER_REFRESH_MS) {
    return;
  }
  int code = 0;
  float tmin = 0, tmax = 0;
  int offset = 0;
  if (fetchWeather(storage.getLat(), storage.getLon(), code, tmin, tmax, offset)) {
    gWeatherCode = code;
    gTempMin = tmin;
    gTempMax = tmax;
    gUtcOffsetSeconds = offset;
    lastWeatherFetch = now;
  } else if (lastWeatherFetch == 0) {
    // Avoid hammering the API on repeated failures.
    lastWeatherFetch = now;
  }
}

// Start STA mode, sync the clock (UTC) and resolve the location once.
void startWifiMode() {
  WiFi.mode(WIFI_STA);
  configTime(0, 0, "pool.ntp.org", "time.nist.gov");
  lastGeoTry = millis();
  doGeocode();
}

void setup() {
  Serial.begin(115200);
  delay(100);

  displayMgr.begin();
  storage.begin();

  checkBootReset();

  if (!storage.isConfigured()) {
    // Both provisioning methods stay active until a choice is made.
    wifiPortal.begin();
    bleProv.begin();

    unsigned long startWait = millis();
    while (millis() - startWait < 120000) {
      wifiPortal.handleClient();
      if (wifiPortal.isConfigured()) {
        break;
      }
      delay(10);
    }

    uint8_t mode = storage.getMode();
    if (mode == MODE_WIFI) {
      unsigned long wifiStart = millis();
      while (WiFi.status() != WL_CONNECTED && millis() - wifiStart < 30000) {
        displayMgr.showWifiConnecting();
        delay(500);
      }
      if (WiFi.status() == WL_CONNECTED) {
        bleProv.stop();  // disable the unused interface after a successful join
        WiFi.softAPdisconnect(true);
        startWifiMode();
      } else {
        // Wrong credentials: back to the configuration portal.
        storage.clear();
        ESP.restart();
      }
    } else if (mode == MODE_BLE) {
      WiFi.mode(WIFI_OFF);  // disable the unused interface
      displayMgr.showBlePairing();
    }
  } else {
    uint8_t mode = storage.getMode();
    if (mode == MODE_WIFI) {
      bleProv.stop();  // no-op when BLE was never initialized
      startWifiMode();
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
      updateWeatherIfNeeded();

      int hour = 0, minute = 0;
      time_t now = time(nullptr);
      if (now > 1600000000) {
        struct tm utc;
        gmtime_r(&now, &utc);
        int minutes = utc.tm_hour * 60 + utc.tm_min + gUtcOffsetSeconds / 60;
        minutes = ((minutes % 1440) + 1440) % 1440;
        hour = minutes / 60;
        minute = minutes % 60;
      }

      displayMgr.updateTime(hour, minute);
      displayMgr.showNormalScreen(true, true, gWeatherCode, gTempMin, gTempMax,
                                 hour, minute);
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
