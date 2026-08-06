#ifndef HARDWARE_PINS_H
#define HARDWARE_PINS_H

#if __has_include("secrets.h")
#include "secrets.h"
#endif

// MAX6675 pin mapping.
// ESP8266: current wiring (CS=D0, CLK=D5, SO=D6).
// M5Stack Core2 (ESP32): external wiring example pins.
#if defined(ARDUINO_ARCH_ESP8266)
constexpr int SMOKER_PIN_MAX6675_CS = D0;
constexpr int SMOKER_PIN_MAX6675_CLK = D5;
constexpr int SMOKER_PIN_MAX6675_SO = D6;
#elif defined(ARDUINO_ARCH_ESP32)
#ifndef SECRET_MAX6675_CS
#define SECRET_MAX6675_CS 26
#endif
#ifndef SECRET_MAX6675_CLK
#define SECRET_MAX6675_CLK 27
#endif
#ifndef SECRET_MAX6675_SO
#define SECRET_MAX6675_SO 19
#endif

// Keep existing defaults, but allow override from secrets.h
constexpr int SMOKER_PIN_MAX6675_CS = SECRET_MAX6675_CS;
constexpr int SMOKER_PIN_MAX6675_CLK = SECRET_MAX6675_CLK;
constexpr int SMOKER_PIN_MAX6675_SO = SECRET_MAX6675_SO;
#else
#error Unsupported platform. This project supports ESP8266 and ESP32 only.
#endif

#endif