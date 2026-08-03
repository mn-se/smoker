# Smoker Pro Development and Maintenance Guide

This guide covers the build setup, flashing procedure, and maintenance workflow for the Smoker Pro project running on the M5Stack Core2.

---

## 1. Development environment

This project is built with PlatformIO and targets the M5Stack Core2 hardware.

- IDE: Visual Studio Code + PlatformIO IDE extension
- Framework: Arduino
- Platform: Espressif 32
- Board: M5Stack Core2

---

## 2. Setup and management of sensitive information

For security reasons, sensitive values such as Wi-Fi credentials are stored in include/secrets.h and are not tracked by Git.

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

If this file is missing, empty values are used and the device starts in setup mode.

### Optional: overwrite Meross credentials later
If you want to update the credentials later, connect the device to the network and run the provided script to rewrite the stored configuration.

```bash
python tools/meross_mss305.py <Meross registered email> <Meross password>
```

---

## 3. Flashing procedure

The firmware and the Web UI filesystem must both be written to the device.

### 3.1 Build the firmware
Use the Core2 environment explicitly:

```bash
pio run -e m5stack-core2
```

### 3.2 Flash firmware (serial connection)
This is used for the initial flash or when changing source files in src/.

- VS Code: click the PlatformIO Upload button.
- CLI:
  - M5Stack Core2: pio run -e m5stack-core2 -t upload

### 3.3 OTA (Over-The-Air) flashing
After the firmware has been flashed once, OTA updates can be used over Wi-Fi.
Enable the following lines in platformio.ini for the Core2 environment:

```ini
upload_protocol = espota
upload_port = smoker.local    ; or specify an IP address
```

Then run Upload for the target environment to flash over the air.

### 3.4 Upload the Web UI filesystem image
When updating files in the data/ directory, upload the filesystem image separately because normal firmware uploads do not update it.

- VS Code:
  1. Open PlatformIO tasks.
  2. Open the m5stack-core2 environment.
  3. Run Upload Filesystem Image.
- CLI:
  - M5Stack Core2: pio run -e m5stack-core2 -t uploadfs

---

## 4. Partition layout

The Core2 environment uses the board’s default ESP32 partition layout. The Web UI and stored logs rely on LittleFS space, so keep the data size reasonable.

---

## 5. Board-specific pin definitions

MAX6675 pin mappings are managed in include/hardware_pins.h. Adjust the Core2 pin constants as needed to match the actual wiring.

---

## 6. Debugging and serial monitor

Use the serial monitor to verify operation and diagnose issues.

- Baud rate: 115200 bps
- Key items to check:
  - Wi-Fi connection status at startup
  - Meross plug communication status
  - MAX6675 sensor readings
  - PID control calculations and heater ON/OFF timing

---

## 7. Web UI cache control

Because the project uses a browser-based UI, cached files may remain visible after updates.

After updating files:
1. Run Upload Filesystem Image.
2. Open http://smoker.local in your browser and refresh.
3. If the changes are still not visible, clear the site data or cache for smoker.local.
