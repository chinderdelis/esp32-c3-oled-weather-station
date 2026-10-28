#ifndef STORAGE_H
#define STORAGE_H

#include <Arduino.h>
#include <Preferences.h>
#include "config.h"

class ConfigStorage {
public:
  ConfigStorage();
  void begin();
  void clear();
  bool isConfigured();
  uint8_t getMode();
  void setMode(uint8_t mode);
  String getProvince();
  void setProvince(const String &province);
  double getLat();
  double getLon();
  String getTz();
  void setLat(double v);
  void setLon(double v);
  void setTz(const String &tz);
  bool loadAll();
  void saveAll();

private:
  Preferences prefs;
  bool configured;
  uint8_t connMode;
  String province;
  double latitude;
  double longitude;
  String timezone;
};

extern ConfigStorage storage;

#endif
