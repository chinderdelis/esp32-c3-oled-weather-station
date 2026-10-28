#ifndef WEATHER_H
#define WEATHER_H

#include <Arduino.h>

// Resolve a free-text location (city or province) to coordinates and timezone
// using the Open-Meteo geocoding API. Returns true on success.
bool geocode(const String &location, double &lat, double &lon, String &tz);

// Fetch the current day's weather summary (WMO code, min/max temperature in C)
// from the Open-Meteo forecast API for the given coordinates.
// Returns true on success.
bool fetchWeather(double lat, double lon, int &weatherCode, float &tempMin, float &tempMax);

#endif
