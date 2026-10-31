#ifndef WEATHER_H
#define WEATHER_H

#include <Arduino.h>

// Resolve a free-text location to coordinates using the Open-Meteo geocoding
// API. When countryCode is given (ISO-3166 alpha-2, e.g. "IT") results are
// constrained to that country. Among the returned candidates the most populous
// match is chosen, which avoids small villages that share a big city's name.
// Returns true on success.
bool geocode(const String &location, double &lat, double &lon, String &tz,
             const String &countryCode = "", const String &language = "en");

// Fetch the current day's weather summary (WMO code, min/max temperature in C)
// plus the location's current UTC offset in seconds. The offset is used to
// compute local time without relying on IANA timezone support in newlib.
// Returns true on success.
bool fetchWeather(double lat, double lon, int &weatherCode, float &tempMin,
                  float &tempMax, int &utcOffsetSeconds);

#endif
