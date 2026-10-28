#ifndef WIFI_PORTAL_H
#define WIFI_PORTAL_H

#include <Arduino.h>
#include <WiFi.h>
#include <WebServer.h>
#include "config.h"
#include "storage.h"
#include "display.h"

class WifiPortal {
public:
  WifiPortal();
  void begin();
  void handleClient();
  bool isConfigured();

private:
  WebServer server;
  bool configDone;
  void setupRoutes();
  void handleRoot();
  void handleConfig();
  String buildPage();
};

extern WifiPortal wifiPortal;

#endif
