#ifndef HARDWARE_PINS_H
#define HARDWARE_PINS_H

// MAX6675 pin mapping.
// ESP8266: current wiring (CS=D0, CLK=D5, SO=D6).
// M5Stack Core2 (ESP32): external wiring example pins.
#if defined(ARDUINO_ARCH_ESP8266)
constexpr int SMOKER_PIN_MAX6675_CS = D0;
constexpr int SMOKER_PIN_MAX6675_CLK = D5;
constexpr int SMOKER_PIN_MAX6675_SO = D6;
#elif defined(ARDUINO_ARCH_ESP32)
constexpr int SMOKER_PIN_MAX6675_CS = 26;
constexpr int SMOKER_PIN_MAX6675_CLK = 18;
constexpr int SMOKER_PIN_MAX6675_SO = 19;
#else
#error Unsupported platform. This project supports ESP8266 and ESP32 only.
#endif

#endif