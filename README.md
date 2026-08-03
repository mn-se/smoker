# Smoker

Smoker is a Wi-Fi connected temperature control project built for the M5Stack Core2. It turns a smart plug on and off automatically to keep a smoker at a target temperature, while showing the current temperature and heater state on the built-in display.

## Overview

The Core2 version runs a compact embedded web dashboard, controls a Meross smart plug, reads a MAX6675 thermocouple, and uses PID-based temperature regulation to maintain steady smoking conditions. The built-in screen makes it easy to monitor temperature and power status at a glance.

## Hardware focus

- **Primary platform**: M5Stack Core2 (ESP32)
- **Display**: built-in screen for live temperature and heater status
- **Sensor**: MAX6675 thermocouple
- **Actuator**: Meross smart plug control
- **Heater**: Ishizaki Denki SK-65V
- **Framework**: Arduino / PlatformIO

## Setup

1. Clone the repository.
2. Open the project in VS Code with the PlatformIO extension installed.
3. Update include/secrets.h with your Wi-Fi SSID, password, and Meross credentials.
4. Verify MAX6675 wiring in include/hardware_pins.h.
5. Build and flash the firmware for the M5Stack Core2 environment.

## Documentation

- English
  - [User Manual](doc/USER_MANUAL.en.md)
  - [Build Guide](doc/BUILD_GUIDE.en.md)
- 日本語
  - [ユーザーマニュアル](doc/USER_MANUAL.md)
  - [ビルド・保守ガイド](doc/BUILD_GUIDE.md)

## Build and upload

- M5Stack Core2: pio run -e m5stack-core2
- M5Stack Core2 upload: pio run -e m5stack-core2 -t upload

## Dependencies

- ArduinoJson
- M5Unified

## License

This project is open source and available under the [MIT License](LICENSE).
