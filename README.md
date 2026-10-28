# ESP32-C3 Super Mini OLED — Weather Station

> ⚠️ **Status: testing / work in progress.** This firmware is under active
> testing. The display layout, provisioning flow and data sources may still
> change between revisions. Tester feedback and issue reports are very welcome.

Firmware for the **ESP32-C3 Super Mini** board with an integrated **0.42" OLED**
display. The device boots into a provisioning portal where you choose how it
connects — **Wi-Fi** or **Bluetooth** — and then shows a clock plus weather
information on the tiny display:

- A weather **icon** screen (sunny / cloudy / rainy) and a **temperature**
  screen (daily min / max) that **alternate every 10 seconds**.
- In Wi-Fi mode the data is fetched live from
  [Open-Meteo](https://open-meteo.com/) (no API key required).
- In Bluetooth mode the phone acts as the data source (sends time + weather
  over BLE with PIN `0000`).

This repository contains **two variants** that share the same code structure:

| Folder  | Target                                                            |
|---------|-------------------------------------------------------------------|
| `italy/`| Italian build: choose one of **107 Italian provinces** in the portal. |
| `world/`| World build: type **any city** in the world in the portal.          |

## Hardware

- **Board:** ESP32-C3 (ESP32C3FH4), 4 MB flash, Wi-Fi + Bluetooth, ceramic antenna.
- **Display:** 0.42" SSD1306 OLED, effective resolution **72×40**, I²C address `0x3C`.
- **Pinout:**
  - `SDA` = GPIO5, `SCL` = GPIO6 (OLED, I²C)
  - `BOOT` button = GPIO9 (used for factory reset)
  - Blue LED = GPIO8

Reference documentation for the board is collected in [`docs/`](docs/).

## How provisioning works

1. On first boot (no configuration stored) the device starts **both**:
   - a Wi-Fi **SoftAP** captive/config portal, and
   - a **BLE** server.
 2. Connect to the Wi-Fi AP (SSID `ESP32C3-CFG-<mac>`) and open the portal, or
    pair over Bluetooth (PIN `0000`). The OLED displays the AP name and the
    portal IP address (by default **`192.168.4.1`**); open
    **http://192.168.4.1** in a browser.
3. Pick the connection type, enter your Wi-Fi credentials (Wi-Fi mode) and your
   location (province for `italy/`, any city for `world/`).
4. The choice is persisted in NVS (flash). On the next boot the device connects
   directly — no portal.
5. **Factory reset:** hold the `BOOT` button for **5 seconds** to wipe the
   configuration and restart the provisioning flow.

> In Wi-Fi mode the device resolves the chosen location to coordinates +
> timezone after connecting, then periodically fetches the day's weather. If a
> location cannot be resolved, it returns to the provisioning portal.

## Build & flash

Each variant is a standalone [PlatformIO](https://platformio.org/) project.
Install PlatformIO, then from the chosen folder:

```bash
cd italy      # or: cd world
pio run -e esp32-c3-devkitm-1          # build
pio run -e esp32-c3-devkitm-1 -t upload   # flash over USB
pio device monitor                      # serial monitor (115200 baud)
```

The `board` is `esp32-c3-devkitm-1`; USB-CDC is enabled so no external
programmer is required. Dependencies (`U8g2`, `ArduinoJson`) are pulled in
automatically via `platformio.ini`.

See [`italy/README.md`](italy/README.md) and [`world/README.md`](world/README.md)
for variant-specific notes.

## For testers — flashing the prebuilt firmware

If you just want to try the firmware without installing PlatformIO, use the
prebuilt factory images committed in each variant folder:

- `italy/firmware/firmware.factory.bin`
- `world/firmware/firmware.factory.bin`

Flash with [esptool](https://docs.espressif.com/projects/esptool/) (adjust the
serial port and baud to your system):

```bash
# Italy variant
esptool.py --port /dev/ttyUSB0 --baud 921600 write_flash 0x0 italy/firmware/firmware.factory.bin

# World variant
esptool.py --port /dev/ttyUSB0 --baud 921600 write_flash 0x0 world/firmware/firmware.factory.bin
```

After flashing, the device reboots into the provisioning portal:

1. Connect to the Wi-Fi AP named `ESP32C3-CFG-<mac>` from your phone/laptop.
   The OLED shows the AP name and the portal IP (default **`192.168.4.1`**).
2. Open **http://192.168.4.1** in a browser, choose **Wi-Fi** or **Bluetooth**,
   enter your credentials and your location (province for `italy/`, any city
   for `world/`).
3. The device connects, fetches the time and weather, and starts displaying.

To wipe the configuration and start over, **hold the BOOT button for 5
seconds**. Please report any issues (wrong weather, wrong time, display
glitches) with the variant, location and connection mode used.

## Project layout

```
.
├── LICENSE
├── README.md                # this file
├── docs/                    # board documentation and reference examples
├── italy/                   # Italian-provinces variant (PlatformIO project)
│   ├── platformio.ini
│   ├── README.md
│   └── src/
└── world/                   # World-cities variant (PlatformIO project)
    ├── platformio.ini
    ├── README.md
    └── src/
```

## Data source & privacy

Weather and geocoding are provided by Open-Meteo's free API and require **no
account or API key**. The only data leaving the device is the location name you
configure, sent to Open-Meteo to resolve coordinates. No credentials or
personal data are stored in the repository.

## License

[MIT](LICENSE).
