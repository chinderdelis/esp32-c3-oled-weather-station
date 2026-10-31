# ESP32-C3 Super Mini OLED — Weather Station

> ⚠️ **Status: testing / work in progress.** This firmware is under active
> testing. The display layout, provisioning flow and data sources may still
> change between revisions. Tester feedback and issue reports are very welcome.

Firmware for the **ESP32-C3 Super Mini** board with an integrated **0.42" OLED**
display. The device boots into a **Wi-Fi** provisioning portal and then shows a
clock plus weather information on the tiny display:

- Four centered screens, each drawn as large as the 72×40 display allows:
  weather **icon** (5 s), **min temperature** (5 s), **max temperature**
  (5 s) and **clock** (7 s).
- Data is fetched live from [Open-Meteo](https://open-meteo.com/) (no API key
  required).
- **Bluetooth is work in progress:** the option is hidden in the portal and the
  firmware currently runs Wi-Fi only (see below).

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

1. On first boot (no configuration stored) the device starts a Wi-Fi
   **SoftAP** configuration portal (SSID `ESP32C3-CFG-<mac>`).
2. Connect to that AP and open **http://192.168.4.1** in a browser. The OLED
   displays the AP name and the portal IP address.
3. Enter your Wi-Fi credentials and your location (province for `italy/`, any
   city for `world/`).
4. The configuration is persisted in NVS (flash). On the next boot the device
   connects directly — no portal.
5. **Factory reset:** while the device is running, hold the `BOOT` button
   (GPIO9) for **5 seconds** to wipe the configuration and restart the
   provisioning flow. The `RESET`/`EN` button cannot be read by the firmware:
   pressing it only reboots the device and keeps the configuration.

> In Wi-Fi mode the device resolves the chosen location to coordinates after
> connecting, then periodically fetches the day's weather. Local time is derived
> from the location's UTC offset reported by the forecast, so no IANA timezone
> support is required on the device. If a location cannot be resolved, the
> device keeps retrying in the background instead of wiping the configuration.

### Bluetooth (work in progress)

Bluetooth is **not enabled** in the current firmware: the portal only offers
Wi-Fi. The BLE code and an experimental Android Web Bluetooth page
([`docs/ble/`](docs/ble/)) are kept in the repository as a starting point for a
future solution. They are not part of the supported flow yet.

A phone cannot provide time or weather over BLE without software running on the
phone: a BLE peripheral cannot force Android to open a web page, and Android has
removed the old "Physical Web/Eddystone" URL notifications. The practical
options are a Web Bluetooth page (Android) or a native app (Android/iOS), which
is why this is deferred.

### Radio use

Only Wi-Fi is used. The device starts the SoftAP portal when unconfigured and
switches to station mode after configuration; the BLE stack is not started. The
only way to reopen the portal is the **5-second BOOT-button reset** (which wipes
the configuration).

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
2. Open **http://192.168.4.1** in a browser, enter your Wi-Fi credentials and a
   location (province for `italy/`, any city for `world/`).
3. The device connects, fetches the time and weather, and starts displaying.

To wipe the configuration and start over, **hold the BOOT button (GPIO9) for 5
seconds while the device is running**. This also erases the saved Wi-Fi
credentials, so the device returns to the configuration AP instead of
reconnecting to the old network. Please report any issues (wrong weather, wrong time, display
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
