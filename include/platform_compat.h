#ifndef PLATFORM_COMPAT_H
#define PLATFORM_COMPAT_H

#include <Arduino.h>

#if defined(ARDUINO_ARCH_ESP8266)
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
using SmokerWebServer = ESP8266WebServer;
#elif defined(ARDUINO_ARCH_ESP32)
#include <WiFi.h>
#include <WebServer.h>
#include <ESPmDNS.h>
using SmokerWebServer = WebServer;
#else
#error Unsupported platform. This project supports ESP8266 and ESP32 only.
#endif

#include <LittleFS.h>

#if defined(ARDUINO_ARCH_ESP8266)
constexpr int SMOKER_RANDOM_SEED_PIN = A0;
#else
constexpr int SMOKER_RANDOM_SEED_PIN = 34;
#endif

#endif