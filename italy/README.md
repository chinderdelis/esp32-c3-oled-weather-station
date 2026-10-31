# Italy variant — ESP32-C3 OLED Weather Station

This is the **Italian** build of the firmware. During provisioning the portal
presents a dropdown of **107 Italian provinces**; the device then resolves the
selected province to coordinates and fetches the daily weather
from Open-Meteo.

## Differences from the `world/` variant

- The configuration portal offers an **Italian province** selector instead of a
  free-text city field.
- Geocoding is constrained to Italy (`countryCode=IT`) and the most populous
  match is chosen, so `Roma` resolves to Rome instead of a same-named village.
  Compound province names fall back to their first part (e.g. `Forlì-Cesena`
  → `Forlì`).
- The clock uses the UTC offset reported by the forecast for the chosen
  location; before the first successful fetch it shows UTC. IANA timezone names
  are not used because newlib support on the ESP32 is unreliable.

Everything else — display behavior, Wi-Fi behavior, factory reset, build/flash
instructions — is identical to the [`world/`](../world/README.md) variant and is
described in the [top-level README](../README.md).

## Build & flash

```bash
cd italy
pio run -e esp32-c3-devkitm-1
pio run -e esp32-c3-devkitm-1 -t upload
pio device monitor
```

### Prebuilt firmware

A ready-to-flash factory image is provided at
[`firmware/firmware.factory.bin`](firmware/firmware.factory.bin). Flash it with
esptool (adjust the serial port):

```bash
esptool.py --port /dev/ttyUSB0 --baud 921600 write_flash 0x0 firmware/firmware.factory.bin
```

## Source map

| File             | Purpose                                                     |
|------------------|-------------------------------------------------------------|
| `main.cpp`       | Boot flow, provisioning, Wi-Fi loop, weather refresh.       |
| `wifi_portal.*`  | SoftAP config portal with the Italian province selector.    |
| `provinces.h`    | The list of 107 Italian provinces.                          |
| `ble_prov.*`     | BLE prototype (work in progress, not used by the firmware). |
| `storage.*`      | NVS persistence (mode, location, lat/lon).                  |
| `weather.*`      | Open-Meteo geocoding + forecast helpers.                    |
| `display.*`      | 72×40 OLED rendering (clock, weather icons, temps, status). |
| `config.h`       | Pinout, NVS keys, timing and JSON buffer sizes.             |
