#include "wifi_portal.h"

WifiPortal wifiPortal;

WifiPortal::WifiPortal() : server(80) {
  configDone = false;
  selectedMode = MODE_NONE;
}

void WifiPortal::begin() {
  String apName = String(AP_SSID_PREFIX) + String((uint32_t)ESP.getEfuseMac(), HEX);
  apName.toUpperCase();
  
  WiFi.mode(WIFI_AP);
  WiFi.softAP(apName.c_str());
  
  IPAddress IP = WiFi.softAPIP();
  String ipStr = IP.toString();
  displayMgr.showConfigAP(apName, ipStr);
  
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
  String html = "<html><head><title>Yes this is another config page of an esp 32 \" chinderlis</title>";
  html += "<meta name=\"viewport\" content=\"width=device-width,initial-scale=1\">";
  html += "<style>body{font-family:monospace;background:#000;color:#0f0;padding:10px}input,select{width:100%;background:#111;color:#0f0;border:1px solid #0f0;padding:5px;margin:4px 0}input[type=radio]{width:auto}.btn{background:#0f0;color:#000;padding:8px;border:none}</style>";
  html += "</head><body>";
  html += "<h1>Yes this is another config page of an esp 32 \" chinderlis</h1>";
  html += "<form action=\"/config\" method=\"POST\">";
  html += "<p>CONNECTION TYPE</p>";
  html += "<label><input type=\"radio\" name=\"mode\" value=\"wifi\" onchange=\"toggle()\"> WiFi</label><br>";
  html += "<label><input type=\"radio\" name=\"mode\" value=\"ble\" onchange=\"toggle()\"> Bluetooth</label><br>";
  html += "<div id=\"bleinfo\" style=\"display:none\"><p>Modalit&agrave; Bluetooth selezionata. Accoppiare il dispositivo con PIN: 0000. Il telefono accoppiato invier&agrave; orario e previsioni del tempo alla scheda. Il Wi-Fi verr&agrave; disabilitato.</p></div>";
  html += "<div id=\"wifi\" style=\"display:none\">";
  html += "<p>WIFI NETWORK</p>";
  html += "<select name=\"ssid\">";
  int n = WiFi.scanNetworks();
  for (int i = 0; i < n; i++) {
    html += "<option value=\"" + WiFi.SSID(i) + "\">" + WiFi.SSID(i) + "</option>";
  }
  html += "</select>";
  html += "<p>PASSWORD</p><input type=\"password\" name=\"pass\">";
  html += "</div>";
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
  html += "<script>function toggle(){var m=document.getElementsByName('mode');var b=document.getElementById('bleinfo');var w=document.getElementById('wifi');if(m[0].checked){b.style.display='none';w.style.display='block';}if(m[1].checked){b.style.display='block';w.style.display='none';}}</script>";
  html += "</body></html>";
  return html;
}

void WifiPortal::handleConfig() {
  if (server.hasArg("mode")) {
    String m = server.arg("mode");
    if (m == "wifi") {
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
      return;
    } else if (m == "ble") {
      selectedMode = MODE_BLE;
      if (server.hasArg("prov")) {
        selectedProvince = server.arg("prov");
        storage.setProvince(selectedProvince);
      }
      storage.setMode(MODE_BLE);
      storage.saveAll();
      configDone = true;
      server.send(200, "text/html", "<html><body style='background:#000;color:#0f0;font-family:monospace'><h1>OK</h1><p>BLE Mode - Pair with PIN 0000</p></body></html>");
      return;
    }
  }
  server.send(400, "text/plain", "Bad Request");
}

void WifiPortal::handleClient() {
  server.handleClient();
}

bool WifiPortal::isConfigured() {
  return configDone;
}
