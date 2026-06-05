#ifndef CONFIG_H
#define CONFIG_H

// Optional: Load private credentials if secrets.h exists
#if __has_include("secrets.h")
#include "secrets.h"
#endif

// Fallback to empty strings if not defined in secrets.h
#ifndef SECRET_WIFI_SSID
#define SECRET_WIFI_SSID ""
#endif
#ifndef SECRET_WIFI_PASS
#define SECRET_WIFI_PASS ""
#endif
#ifndef SECRET_MEROSS_UUID
#define SECRET_MEROSS_UUID ""
#endif
#ifndef SECRET_MEROSS_KEY
#define SECRET_MEROSS_KEY ""
#endif
#ifndef SECRET_MEROSS_IP
#define SECRET_MEROSS_IP ""
#endif

// --- WiFi / Network Defaults ---
constexpr const char* WIFI_DEFAULT_SSID = SECRET_WIFI_SSID;
constexpr const char* WIFI_DEFAULT_PASS = SECRET_WIFI_PASS;
constexpr unsigned long WIFI_TIMEOUT_MS = 30000;
constexpr unsigned int DNS_PORT = 53;

// --- Meross Device Defaults ---
constexpr const char* MEROSS_DEFAULT_UUID = SECRET_MEROSS_UUID;
constexpr const char* MEROSS_DEFAULT_KEY = SECRET_MEROSS_KEY;
constexpr const char* MEROSS_DEFAULT_IP = SECRET_MEROSS_IP;

// --- Temperature Control Defaults ---
constexpr float TEMP_DEFAULT_TARGET = 60.0f;
constexpr unsigned long TEMP_UPDATE_INTERVAL_MS = 5000;
constexpr float TEMP_ERROR_THRESHOLD = -500.0f;

// --- PID Control Parameters ---
constexpr float PID_KP = 6.0f;
constexpr float PID_KI = 0.02f;
constexpr float PID_KD = 8.0f;
constexpr unsigned long PWM_WINDOW_SIZE_MS = 120000; // 2 minutes

// --- NTP Configuration ---
constexpr const char* NTP_SERVER = "pool.ntp.org";
constexpr unsigned int NTP_PORT = 123;
constexpr unsigned long NTP_TIMEOUT_MS = 1000;

#endif // CONFIG_H
