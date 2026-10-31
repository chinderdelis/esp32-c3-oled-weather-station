#ifndef WIFI_PORTAL_H
#define WIFI_PORTAL_H

#include <Arduino.h>
#include <WiFi.h>
#include <WebServer.h>
#include "config.h"
#include "provinces.h"
#include "storage.h"
#include "display.h"

class WifiPortal {
public:
  WifiPortal();
  void begin();
  void handleClient();
  bool isConfigured();
  String getSavedSSID();
  String getSavedPass();
  bool waitForConfig(unsigned long timeoutMs);
  
private:
  WebServer server;
  bool configDone;
  uint8_t selectedMode;
  String selectedProvince;
  String ssidOptions;
  void setupRoutes();
  void handleRoot();
  void handleConfig();
  String buildPage();
};

extern WifiPortal wifiPortal;

#endif
