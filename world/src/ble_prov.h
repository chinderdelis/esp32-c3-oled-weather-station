#ifndef BLE_PROV_H
#define BLE_PROV_H

#include <Arduino.h>
#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>
#include <ArduinoJson.h>
#include "config.h"
#include "storage.h"
#include "display.h"

class BLEProv {
public:
  BLEProv();
  void begin();
  void stop();
  void loop();
  bool isConnected();
  bool hasData();
  void getData(int &hour, int &minute, int &weatherCode, float &tempMin, float &tempMax);
  bool deviceConnected;
  bool dataReceived;
  int lastHour, lastMinute;
  int lastWeatherCode;
  float lastTempMin, lastTempMax;
  
private:
  BLEServer* pServer;
  BLECharacteristic* pChar;
  unsigned long lastRequest;
};

extern BLEProv bleProv;

#endif
