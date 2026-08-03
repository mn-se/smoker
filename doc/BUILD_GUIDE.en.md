# Smoker Pro Development and Maintenance Guide

This document explains how to set up the build environment, flash the firmware, and maintain the Smoker Pro project.

---

## 1. Development environment

This project is built with PlatformIO.

- IDE: Visual Studio Code + PlatformIO IDE extension
- Framework: Arduino
- Platform: espressif8266 / espressif32
- Boards:
  - ESP-WROOM-02 (ESP8266)
  - M5Stack Core2 (ESP32)

---

## 2. Setup and management of sensitive information

For security reasons, sensitive information such as Wi-Fi passwords is stored separately in include/secrets.h. This file is excluded from Git.

### Create include/secrets.h
Before building, create include/secrets.h with the following content:

```cpp
#ifndef SECRETS_H
#define SECRETS_H

#define SECRET_WIFI_SSID "your-ssid"
#define SECRET_WIFI_PASS "your-password"
#define SECRET_MEROSS_UUID "Meross UUID"
#define SECRET_MEROSS_KEY "Meross KEY"
#define SECRET_MEROSS_IP "Meross IP"

#endif
```

If this file does not exist, empty strings will be used during compilation, and the device will start in setup mode (AP mode).

### Optional: overwrite Meross plug credentials later
If you want to update the credentials later instead of using the defaults in secrets.h, connect to Wi-Fi and run the provided script to overwrite the ESP8266 configuration stored in EEPROM.

```bash
python tools/meross_mss305.py <Meross registered email> <Meross password>
```

---

## 3. Flashing procedure

The ESP8266 / ESP32 requires two kinds of data to be written:
- firmware binary
- Web UI filesystem image

### 3.0 Select the build environment
Use the target PlatformIO environment explicitly.

- ESP8266: esp_wroom_02
- M5Stack Core2: m5stack-core2

Examples:

```bash
pio run -e esp_wroom_02
pio run -e m5stack-core2
```

### 3.1 Flash firmware (serial connection)
This is used when changing source files in src/ or for the initial flash.

- VS Code: click the PlatformIO Upload button (arrow icon).
- CLI:
  - ESP8266: pio run -e esp_wroom_02 -t upload
  - M5Stack Core2: pio run -e m5stack-core2 -t upload

### 3.2 OTA (Over-The-Air) flashing
After the firmware has been flashed once via serial, OTA updates can be used over Wi-Fi.
Enable the following lines in platformio.ini for the target environment:

```ini
; OTA upload settings
upload_protocol = espota
upload_port = smoker.local    ; or specify an IP address
```

Then run Upload for the target environment to flash over the air.

### 3.3 Upload Web UI filesystem image
When updating files in the data/ directory (HTML, CSS, JS, manifest.json, etc.), you must upload the filesystem image separately because normal firmware uploads do not update it.

- VS Code:
  1. Open PlatformIO tasks.
  2. Open the target environment (esp_wroom_02 or m5stack-core2).
  3. Run Upload Filesystem Image.
- CLI:
  - ESP8266: pio run -e esp_wroom_02 -t uploadfs
  - M5Stack Core2: pio run -e m5stack-core2 -t uploadfs

---

## 4. Partition layout

The partition configuration is set in platformio.ini as follows:

- ESP8266 (esp_wroom_02):
  - Flash size: 2MB
  - Filesystem: LittleFS (about 256KB)
  - LD script: eagle.flash.2m256.ld
- ESP32 (m5stack-core2):
  - Uses the board’s default partition layout

Be mindful of available LittleFS space when changing the size of the Web UI or log data.

---

## 5. Board-specific pin definitions

MAX6675 pin mappings are managed in include/hardware_pins.h.

- ESP8266: existing wiring (D0 / D5 / D6)
- M5Stack Core2: GPIO pins for external wiring

Adjust the Core2 constants as needed to match your hardware wiring.

---

## 6. Debugging and serial monitor

Use the serial monitor to verify operation and diagnose errors.

- Baud rate: 115200 bps
- Key items to check:
  - Wi-Fi connection status at startup
  - Meross plug communication status
  - MAX6675 sensor readings
  - PID control calculations and heater ON/OFF timing

---

## 7. Web UI cache control

Because this project uses PWA / Service Worker behavior, browser caches may retain an older version of the Web UI.

After updating files:
1. Complete Upload Filesystem Image.
2. Open http://smoker.local in your browser and refresh.
3. If the changes still do not appear, clear the site data/cache for smoker.local.
