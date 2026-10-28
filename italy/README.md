# Italy variant — ESP32-C3 OLED Weather Station

This is the **Italian** build of the firmware. During provisioning the portal
presents a dropdown of **107 Italian provinces**; the device then resolves the
selected province to coordinates and timezone and fetches the daily weather
from Open-Meteo.

## Differences from the `world/` variant

- The configuration portal offers an **Italian province** selector instead of a
  free-text city field.
- Geocoding retries the province name with a `", Italy"` suffix to improve
  matching (e.g. `Roma` → `Roma, Italy`).
- The fallback timezone (used only if Open-Meteo does not return one) is set to
  Rome (`CET-1CEST,M3.5.0,M10.5.0/3`).

Everything else — display behavior, Bluetooth mode, factory reset, build/flash
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
| `main.cpp`       | Boot flow, provisioning, Wi-Fi/BLE loop, weather refresh.   |
| `wifi_portal.*`  | SoftAP config portal with the Italian province selector.    |
| `provinces.h`    | The list of 107 Italian provinces.                          |
| `ble_prov.*`     | BLE server; receives time + weather from a paired phone.    |
| `storage.*`      | NVS persistence (mode, location, lat/lon, timezone).        |
| `weather.*`      | Open-Meteo geocoding + forecast helpers.                    |
| `display.*`      | 72×40 OLED rendering (clock, weather icons, temps, status). |
| `config.h`       | Pinout, NVS keys, timing and default timezone.              |
