#include "ble_prov.h"

BLEProv bleProv;

class ServerCallbacks: public BLEServerCallbacks {
  void onConnect(BLEServer* pServer) {
    bleProv.deviceConnected = true;
  }
  void onDisconnect(BLEServer* pServer) {
    bleProv.deviceConnected = false;
    BLEDevice::startAdvertising();
  }
};

class CharCallbacks: public BLECharacteristicCallbacks {
  void onWrite(BLECharacteristic *pCharacteristic) {
    String value = pCharacteristic->getValue();
    if (value.length() > 0) {
      DynamicJsonDocument doc(JSON_BUF_MED);
      DeserializationError error = deserializeJson(doc, value.c_str());
      if (!error) {
        if (doc.containsKey("hour")) bleProv.lastHour = doc["hour"];
        if (doc.containsKey("minute")) bleProv.lastMinute = doc["minute"];
        if (doc.containsKey("weather_code")) bleProv.lastWeatherCode = doc["weather_code"];
        if (doc.containsKey("temp_min")) bleProv.lastTempMin = doc["temp_min"];
        if (doc.containsKey("temp_max")) bleProv.lastTempMax = doc["temp_max"];
        bleProv.dataReceived = true;
      }
    }
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
}

void BLEProv::begin() {
  BLEDevice::init(BLE_DEVICE_NAME);
  
  pServer = BLEDevice::createServer();
  pServer->setCallbacks(new ServerCallbacks());
  
  BLEService *pService = pServer->createService("4fafc201-1fb5-459e-8fcc-c5c9c331914b");
  pChar = pService->createCharacteristic(
    "beb5483e-36e1-4688-b7f5-ea07361b26a8",
    BLECharacteristic::PROPERTY_READ | BLECharacteristic::PROPERTY_WRITE | BLECharacteristic::PROPERTY_NOTIFY
  );
  pChar->setCallbacks(new CharCallbacks());
  pChar->addDescriptor(new BLE2902());
  
  pService->start();
  BLEAdvertising *pAdvertising = BLEDevice::getAdvertising();
  pAdvertising->addServiceUUID("4fafc201-1fb5-459e-8fcc-c5c9c331914b");
  pAdvertising->setScanResponse(true);
  pAdvertising->setMinPreferred(0x06);
  pAdvertising->setMinPreferred(0x12);
  BLEDevice::startAdvertising();
  
  displayMgr.showBlePairing();
}

void BLEProv::stop() {
  BLEDevice::deinit(true);
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
