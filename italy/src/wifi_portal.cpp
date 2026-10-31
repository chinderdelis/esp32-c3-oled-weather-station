#include "wifi_portal.h"

WifiPortal wifiPortal;

WifiPortal::WifiPortal() : server(80) {
  configDone = false;
  selectedMode = MODE_NONE;
}

void WifiPortal::begin() {
  String apName = String(AP_SSID_PREFIX) + String((uint32_t)ESP.getEfuseMac(), HEX);
  apName.toUpperCase();
  
  Serial.print("[portal] starting AP: ");
  Serial.println(apName);
  Serial.flush();
  WiFi.mode(WIFI_AP_STA);
  bool apStarted = WiFi.softAP(apName.c_str());
  Serial.print("[portal] softAP=");
  Serial.println(apStarted ? 1 : 0);
  Serial.flush();

  IPAddress IP = WiFi.softAPIP();
  String ipStr = IP.toString();
  Serial.print("[portal] IP: ");
  Serial.println(ipStr);
  Serial.flush();
  displayMgr.showConfigAP(apName, ipStr);

  // Scan once at startup so the page can be served instantly. Scanning inside
  // the request handler can briefly drop the AP and lose the response.
  ssidOptions = "";
  int n = WiFi.scanNetworks();
  for (int i = 0; i < n; i++) {
    String ssid = WiFi.SSID(i);
    if (ssid.length() == 0) {
      continue;
    }
    ssidOptions += "<option value=\"" + ssid + "\">" + ssid + "</option>";
  }
  WiFi.scanDelete();

  setupRoutes();
  server.begin();
}

void WifiPortal::setupRoutes() {
  server.on("/", [this]() { handleRoot(); });
  server.on("/config", HTTP_POST, [this]() { handleConfig(); });
}

void WifiPortal::handleRoot() {
  server.send(200, "text/html", buildPage());
}

String WifiPortal::buildPage() {
  String html = "<html><head><title>ESP32-C3 OLED Config</title>";
  html += "<meta name=\"viewport\" content=\"width=device-width,initial-scale=1\">";
  html += "<style>body{font-family:monospace;background:#000;color:#0f0;padding:10px}input,select{width:100%;background:#111;color:#0f0;border:1px solid #0f0;padding:5px;margin:4px 0}input[type=radio]{width:auto}.btn{background:#0f0;color:#000;padding:8px;border:none}</style>";
  html += "</head><body>";
  html += "<h1>ESP32-C3 OLED Config</h1>";
  html += "<form action=\"/config\" method=\"POST\">";
  html += "<input type=\"hidden\" name=\"mode\" value=\"wifi\">";
  html += "<p>WIFI NETWORK</p>";
  html += "<select name=\"ssid\">";
  html += ssidOptions;
  html += "</select>";
  html += "<p>PASSWORD</p><input type=\"password\" name=\"pass\">";
  html += "<p>PROVINCE</p><select name=\"prov\">";
  for (int i = 0; i < NUM_PROVINCES; i++) {
    html += "<option value=\"";
    html += String((const char*)pgm_read_ptr(&ITALIAN_PROVINCES[i]));
    html += "\">";
    html += String((const char*)pgm_read_ptr(&ITALIAN_PROVINCES[i]));
    html += "</option>";
  }
  html += "</select>";
  html += "<br><br><input type=\"submit\" class=\"btn\" value=\"SAVE\">";
  html += "</form>";
  html += "</body></html>";
  return html;
}

void WifiPortal::handleConfig() {
  selectedMode = MODE_WIFI;
  if (server.hasArg("ssid")) {
    WiFi.begin(server.arg("ssid").c_str(), server.hasArg("pass") ? server.arg("pass").c_str() : "");
  }
  if (server.hasArg("prov")) {
    selectedProvince = server.arg("prov");
    storage.setProvince(selectedProvince);
  }
  storage.setMode(MODE_WIFI);
  storage.saveAll();
  configDone = true;
  server.send(200, "text/html", "<html><body style='background:#000;color:#0f0;font-family:monospace'><h1>OK</h1><p>Connecting WiFi...</p></body></html>");
}

void WifiPortal::handleClient() {
  server.handleClient();
}

bool WifiPortal::isConfigured() {
  return configDone;
}
