# World variant — ESP32-C3 OLED Weather Station

This is the **world-wide** build of the firmware. During provisioning the portal
lets you type **any city in the world**; the device resolves it to coordinates
(via Open-Meteo geocoding) and fetches the daily weather.

## Differences from the `italy/` variant

- The configuration portal uses a **free-text city** input instead of an
  Italian province dropdown.
- Geocoding returns several candidates and the most populous match is chosen,
  so a big city is preferred over tiny villages that share its name.
- The clock uses the per-city UTC offset reported by the forecast; before the
  first successful fetch it shows UTC. IANA timezone names are not used because
  newlib support on the ESP32 is unreliable.

Everything else — display behavior, Wi-Fi behavior, factory reset, build/flash
instructions — is identical to the [`italy/`](../italy/README.md) variant and is
described in the [top-level README](../README.md).

## Build & flash

```bash
cd world
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
| `wifi_portal.*`  | SoftAP config portal with the free-text city input.         |
| `ble_prov.*`     | BLE prototype (work in progress, not used by the firmware). |
| `storage.*`      | NVS persistence (mode, location, lat/lon).                  |
| `weather.*`      | Open-Meteo geocoding + forecast helpers.                    |
| `display.*`      | 72×40 OLED rendering (clock, weather icons, temps, status). |
| `config.h`       | Pinout, NVS keys, timing and JSON buffer sizes.             |
