#include "weather.h"
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include "config.h"

bool geocode(const String &location, double &lat, double &lon, String &tz,
             const String &countryCode, const String &language) {
  WiFiClientSecure client;
  client.setInsecure();
  HTTPClient http;
  String url = "https://geocoding-api.open-meteo.com/v1/search?name=" + location +
               "&count=10&language=" + language + "&format=json";
  if (countryCode.length() > 0) {
    url += "&countryCode=" + countryCode;
  }
  url.replace(" ", "%20");

  if (!http.begin(client, url)) {
    return false;
  }

  int httpCode = http.GET();
  bool ok = false;
  if (httpCode == HTTP_CODE_OK) {
    String payload = http.getString();
    DynamicJsonDocument doc(JSON_BUF_GEO);
    DeserializationError error = deserializeJson(doc, payload);
    if (!error && doc.containsKey("results") && doc["results"].size() > 0) {
      // Pick the most populous candidate: searching "Roma" can otherwise match
      // tiny villages with the same name before the actual city.
      JsonArray results = doc["results"].as<JsonArray>();
      double bestPopulation = -1.0;
      for (JsonVariant value : results) {
        JsonObject candidate = value.as<JsonObject>();
        double population = candidate["population"] | 0.0;
        if (population > bestPopulation) {
          bestPopulation = population;
          lat = candidate["latitude"].as<double>();
          lon = candidate["longitude"].as<double>();
          if (candidate.containsKey("timezone")) {
            tz = candidate["timezone"].as<String>();
          }
          ok = true;
        }
      }
    }
  }
  http.end();
  return ok;
}

bool fetchWeather(double lat, double lon, int &weatherCode, float &tempMin,
                  float &tempMax, int &utcOffsetSeconds) {
  WiFiClientSecure client;
  client.setInsecure();
  HTTPClient http;
  String url = "https://api.open-meteo.com/v1/forecast?latitude=" + String(lat, 6) +
               "&longitude=" + String(lon, 6) +
               "&daily=weather_code,temperature_2m_max,temperature_2m_min" +
               "&forecast_days=1&timezone=auto";

  if (!http.begin(client, url)) {
    return false;
  }

  int httpCode = http.GET();
  bool ok = false;
  if (httpCode == HTTP_CODE_OK) {
    String payload = http.getString();
    DynamicJsonDocument doc(JSON_BUF_LARGE);
    DeserializationError error = deserializeJson(doc, payload);
    if (!error && doc.containsKey("daily")) {
      weatherCode = doc["daily"]["weather_code"][0].as<int>();
      tempMax = doc["daily"]["temperature_2m_max"][0].as<float>();
      tempMin = doc["daily"]["temperature_2m_min"][0].as<float>();
      utcOffsetSeconds = doc["utc_offset_seconds"] | 0;
      ok = true;
    }
  }
  http.end();
  return ok;
}
