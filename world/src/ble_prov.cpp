#include "ble_prov.h"
#include "BLESecurity.h"

BLEProv bleProv;

class ServerCallbacks: public BLEServerCallbacks {
  void onConnect(BLEServer* pServer) {
    bleProv.deviceConnected = true;
    bleProv.dataReceived = false;
    Serial.println("[ble] connected");
  }
  void onDisconnect(BLEServer* pServer) {
    bleProv.deviceConnected = false;
    bleProv.dataReceived = false;
    Serial.println("[ble] disconnected");
    BLEDevice::startAdvertising();
  }
};

class SecurityCallbacks : public BLESecurityCallbacks {
  uint32_t onPassKeyRequest() { return BLE_PASSKEY; }
  void onPassKeyNotify(uint32_t pass_key) { Serial.printf("[ble] passkey %06u\n", pass_key); }
  bool onSecurityRequest() { return true; }
  bool onConfirmPIN(uint32_t pass_key) { return true; }
};

class CharCallbacks: public BLECharacteristicCallbacks {
  void onWrite(BLECharacteristic *pCharacteristic) {
    String value = pCharacteristic->getValue().c_str();
    if (value.length() == 0) return;
    Serial.printf("[ble] rx %u: %s\n", value.length(), value.c_str());
    DynamicJsonDocument doc(JSON_BUF_MED);
    DeserializationError error = deserializeJson(doc, value.c_str());
    if (error) {
      Serial.printf("[ble] json error: %s\n", error.c_str());
      return;
    }
    if (doc.containsKey("hour")) bleProv.lastHour = doc["hour"];
    if (doc.containsKey("minute")) bleProv.lastMinute = doc["minute"];
    if (doc.containsKey("weather_code")) bleProv.lastWeatherCode = doc["weather_code"];
    if (doc.containsKey("temp_min")) bleProv.lastTempMin = doc["temp_min"];
    if (doc.containsKey("temp_max")) bleProv.lastTempMax = doc["temp_max"];
    bleProv.dataReceived = true;
    Serial.println("[ble] data ok");
  }
  void onRead(BLECharacteristic *pCharacteristic) {
    Serial.println("[ble] read request");
  }
};

BLEProv::BLEProv() {
  pServer = nullptr;
  pChar = nullptr;
  deviceConnected = false;
  dataReceived = false;
  lastHour = 0; lastMinute = 0;
  lastWeatherCode = 0;
  lastTempMin = 0; lastTempMax = 0;
  lastRequest = 0;
}

void BLEProv::begin() {
  BLEDevice::init(BLE_DEVICE_NAME);

  // Fixed passkey (000000) with MITM protection so the phone must pair.
  BLEDevice::setSecurityCallbacks(new SecurityCallbacks());
  BLESecurity *pSecurity = new BLESecurity();
  pSecurity->setPassKey(true, BLE_PASSKEY);
  pSecurity->setCapability(ESP_IO_CAP_OUT);
  pSecurity->setAuthenticationMode(true, true, true);

  pServer = BLEDevice::createServer();
  pServer->setCallbacks(new ServerCallbacks());
  
  BLEService *pService = pServer->createService("4fafc201-1fb5-459e-8fcc-c5c9c331914b");
  pChar = pService->createCharacteristic(
    "beb5483e-36e1-4688-b7f5-ea07361b26a8",
    BLECharacteristic::PROPERTY_READ | BLECharacteristic::PROPERTY_WRITE |
    BLECharacteristic::PROPERTY_WRITE_NR | BLECharacteristic::PROPERTY_NOTIFY
  );
  pChar->setAccessPermissions(ESP_GATT_PERM_READ_ENC_MITM | ESP_GATT_PERM_WRITE_ENC_MITM);
  pChar->setCallbacks(new CharCallbacks());
  pChar->addDescriptor(new BLE2902());
  
  pService->start();
  BLEAdvertising *pAdvertising = BLEDevice::getAdvertising();
  pAdvertising->addServiceUUID("4fafc201-1fb5-459e-8fcc-c5c9c331914b");
  pAdvertising->setScanResponse(true);
  pAdvertising->setMinPreferred(0x06);
  pAdvertising->setMinPreferred(0x12);
  BLEDevice::startAdvertising();
}

void BLEProv::stop() {
  BLEDevice::deinit(true);
}

// Ask the phone for data: notify "GET" until the first payload arrives.
void BLEProv::loop() {
  if (!deviceConnected || pChar == nullptr || dataReceived) return;
  unsigned long now = millis();
  if (lastRequest == 0 || now - lastRequest >= 5000) {
    lastRequest = now;
    pChar->setValue("GET");
    pChar->notify();
    Serial.println("[ble] request data");
  }
}

bool BLEProv::isConnected() {
  return deviceConnected;
}

bool BLEProv::hasData() {
  return dataReceived;
}

void BLEProv::getData(int &hour, int &minute, int &weatherCode, float &tempMin, float &tempMax) {
  hour = lastHour;
  minute = lastMinute;
  weatherCode = lastWeatherCode;
  tempMin = lastTempMin;
  tempMax = lastTempMax;
}
