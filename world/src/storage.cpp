#include "storage.h"

ConfigStorage storage;

ConfigStorage::ConfigStorage() {
  configured = false;
  connMode = MODE_NONE;
  province = "";
  latitude = 0.0;
  longitude = 0.0;
  timezone = "";
}

void ConfigStorage::begin() {
  prefs.begin(NVS_NAMESPACE, false);
  loadAll();
}

void ConfigStorage::clear() {
  prefs.clear();
  configured = false;
  connMode = MODE_NONE;
  province = "";
  latitude = 0.0;
  longitude = 0.0;
  timezone = "";
}

bool ConfigStorage::isConfigured() {
  return configured;
}

uint8_t ConfigStorage::getMode() {
  return connMode;
}

void ConfigStorage::setMode(uint8_t mode) {
  connMode = mode;
  prefs.putUChar(NVS_KEY_MODE, connMode);
}

String ConfigStorage::getProvince() {
  return province;
}

void ConfigStorage::setProvince(const String &prov) {
  province = prov;
  prefs.putString(NVS_KEY_PROVINCE, province);
}

double ConfigStorage::getLat() {
  return latitude;
}

double ConfigStorage::getLon() {
  return longitude;
}

String ConfigStorage::getTz() {
  return timezone;
}

void ConfigStorage::setLat(double v) {
  latitude = v;
  prefs.putDouble("lat", v);
}

void ConfigStorage::setLon(double v) {
  longitude = v;
  prefs.putDouble("lon", v);
}

void ConfigStorage::setTz(const String &tz) {
  timezone = tz;
  prefs.putString("tz", tz);
}

bool ConfigStorage::loadAll() {
  configured = prefs.getBool(NVS_KEY_CONFIGURED, false);
  connMode = prefs.getUChar(NVS_KEY_MODE, MODE_NONE);
  province = prefs.getString(NVS_KEY_PROVINCE, "");
  latitude = prefs.getDouble("lat", 0.0);
  longitude = prefs.getDouble("lon", 0.0);
  timezone = prefs.getString("tz", "");
  return configured;
}

void ConfigStorage::saveAll() {
  prefs.putBool(NVS_KEY_CONFIGURED, true);
  prefs.putUChar(NVS_KEY_MODE, connMode);
  if (province.length() > 0) {
    prefs.putString(NVS_KEY_PROVINCE, province);
  }
  prefs.putDouble("lat", latitude);
  prefs.putDouble("lon", longitude);
  if (timezone.length() > 0) {
    prefs.putString("tz", timezone);
  }
  configured = true;
}
