#include "../include/platform_compat.h"
#include "../include/hardware_pins.h"
#include <DNSServer.h>

// --- Core モジュール ---
#include "core/log_module.h"
#include "core/SystemConfig.h"
#include "core/CoreWebServer.h"
#include "core/OTAModule.h"

// --- App モジュール ---
#include "app/ntp.h"
#include "app/md5.h"
#include "app/max6675.h"
#include "app/meross_control.h"
#include "app/temp_control.h"
#include "app/serial_cmd.h"
#include "app/AppConfig.h"
#include "app/AppAPI.h"

#include "../include/config.h"

#if defined(ARDUINO_ARCH_ESP32)
#include <M5Unified.h>
#endif

// --- 設定構造体 ---
struct Config {
    String ssid;
    String password;
    String merossUUID;
    String merossKey;
    String merossIP;
    float defaultTargetTemp;
};

// --- デフォルト設定 ---
const Config DEFAULT_CONFIG = {
    WIFI_DEFAULT_SSID,
    WIFI_DEFAULT_PASS,
    MEROSS_DEFAULT_UUID,
    MEROSS_DEFAULT_KEY,
    MEROSS_DEFAULT_IP,
    TEMP_DEFAULT_TARGET
};

// ============================================================
// --- グローバルインスタンス ---
// [Core]
LogModule      logModule;
CoreWebServer  coreServer;
OTAModule      ota;

// [App]
NTPModule              ntp;
MD5Module              md5;
MAX6675Module          max6675(SMOKER_PIN_MAX6675_CS, SMOKER_PIN_MAX6675_CLK, SMOKER_PIN_MAX6675_SO);
MerossControlModule    meross(ntp, md5);
TempControlModule      tempControl(max6675, meross, logModule);
SerialCmdModule        serialCmd(tempControl, meross);
AppAPI                 appApi(coreServer, tempControl, meross, logModule);
// ============================================================

DNSServer dnsServer;
bool isAPMode    = false;
bool wifiConnected = false;

#if defined(ARDUINO_ARCH_ESP32)
unsigned long lastDisplayUpdateMs = 0;
unsigned long lastBootDisplayUpdateMs = 0;
bool dashboardLayoutDrawn = false;

const char* controlStateLabel(ControlState state) {
    switch (state) {
        case IDLE:         return "IDLE";
        case TEMP_CONTROL:  return "AUTO CONTROL";
        case FORCE_ON:      return "FORCE ON";
        case MONITORING:    return "MONITORING";
        case ERROR_STATE:   return "ERROR";
        default:            return "UNKNOWN";
    }
}

uint16_t controlStateColor(ControlState state) {
    switch (state) {
        case IDLE:         return TFT_DARKGREY;
        case TEMP_CONTROL:  return TFT_ORANGE;
        case FORCE_ON:      return TFT_RED;
        case MONITORING:    return TFT_CYAN;
        case ERROR_STATE:   return TFT_RED;
        default:            return TFT_LIGHTGREY;
    }
}

const char* heaterStateLabel(int heaterState) {
    if (heaterState == 1) return "ON";
    if (heaterState == 0) return "OFF";
    return "UNKNOWN";
}

const char* sensorStatusLabel(float temp) {
    if (temp <= -999.0f) return "NO PROBE";
    if (temp <= -998.0f) return "SHORT GND";
    if (temp <= -997.0f) return "SHORT VCC";
    if (temp <= -996.0f) return "OUT OF RANGE";
    return "OK";
}

uint16_t heaterStateColor(int heaterState) {
    if (heaterState == 1) return TFT_RED;
    if (heaterState == 0) return TFT_GREEN;
    return TFT_LIGHTGREY;
}

void drawInfoCard(int x, int y, int w, int h, const char* label, const char* value, uint16_t accentColor) {
    M5.Display.fillRoundRect(x, y, w, h, 10, 0x1111);
    M5.Display.drawRoundRect(x, y, w, h, 10, accentColor);

    M5.Display.setTextColor(TFT_LIGHTGREY, 0x1111);
    M5.Display.setTextSize(1);
    M5.Display.setCursor(x + 10, y + 6);
    M5.Display.print(label);

    M5.Display.setTextColor(accentColor, 0x1111);
    M5.Display.setTextSize(2);
    M5.Display.setCursor(x + 10, y + 20);
    M5.Display.print(value);
}

void drawBootScreen(const char* title, const char* line1, const char* line2, uint16_t accentColor) {
    M5.Display.clear(BLACK);
    M5.Display.setTextColor(WHITE, BLACK);
    M5.Display.setTextSize(2);
    M5.Display.setCursor(10, 8);
    M5.Display.println("Smoker Controller");

    M5.Display.fillRoundRect(10, 44, 300, 120, 12, 0x1111);
    M5.Display.drawRoundRect(10, 44, 300, 120, 12, accentColor);

    M5.Display.setTextColor(accentColor, 0x1111);
    M5.Display.setTextSize(3);
    M5.Display.setCursor(24, 62);
    M5.Display.println(title);

    M5.Display.setTextColor(TFT_WHITE, 0x1111);
    M5.Display.setTextSize(2);
    M5.Display.setCursor(24, 98);
    M5.Display.println(line1);

    M5.Display.setTextSize(1);
    M5.Display.setTextColor(TFT_LIGHTGREY, 0x1111);
    M5.Display.setCursor(24, 130);
    M5.Display.println(line2);
}

void drawStartupDoneScreen() {
    M5.Display.clear(BLACK);
    M5.Display.fillScreen(TFT_RED);
    M5.Display.setTextColor(TFT_WHITE, TFT_RED);
    M5.Display.setTextSize(4);
    M5.Display.setCursor(18, 28);
    M5.Display.println("TEST");

    M5.Display.setTextSize(3);
    M5.Display.setCursor(18, 78);
    M5.Display.println("SCREEN");

    M5.Display.setTextSize(2);
    M5.Display.setCursor(18, 126);
    M5.Display.println("Switching now");
}

void drawStageScreen(const char* stage, const char* detail, uint16_t accentColor) {
    M5.Display.clear(BLACK);
    M5.Display.fillScreen(TFT_BLACK);
    M5.Display.setTextColor(accentColor, BLACK);
    M5.Display.setTextSize(4);
    M5.Display.setCursor(18, 24);
    M5.Display.println(stage);

    M5.Display.setTextSize(2);
    M5.Display.setTextColor(TFT_WHITE, BLACK);
    M5.Display.setCursor(18, 88);
    M5.Display.println(detail);
}

void drawDashboardLiveScreen() {
    M5.Display.clear(BLACK);
    M5.Display.fillScreen(BLACK);
    M5.Display.setTextColor(TFT_WHITE, BLACK);
    M5.Display.setTextSize(3);
    M5.Display.setCursor(12, 10);
    M5.Display.println("Smoker Dashboard");

    M5.Display.setTextColor(TFT_DARKGREY, BLACK);
    M5.Display.setTextSize(2);
    M5.Display.setCursor(12, 44);
    M5.Display.println("CURRENT TEMP");

    M5.Display.setTextColor(TFT_CYAN, BLACK);
    M5.Display.setTextSize(4);
    M5.Display.setCursor(12, 66);
    M5.Display.println("TEMP: --.- C");

    M5.Display.setTextColor(TFT_ORANGE, BLACK);
    M5.Display.setTextSize(2);
    M5.Display.setCursor(12, 116);
    M5.Display.println("TARGET: --.- C");

    M5.Display.setTextColor(TFT_WHITE, BLACK);
    M5.Display.setCursor(12, 146);
    M5.Display.println("STATE: STARTING");

    M5.Display.setCursor(12, 172);
    M5.Display.println("HEATER: UNKNOWN");

    M5.Display.setCursor(12, 198);
    M5.Display.println("WIFI: OFFLINE");

    M5.Display.setCursor(12, 224);
    M5.Display.println("SENSOR: NO SIGNAL");

    dashboardLayoutDrawn = true;
}
#endif

// --- 前方宣言 ---
void loadConfig(Config& config);
void validateConfig(Config& config);
void initModules(const Config& config);

#if defined(ARDUINO_ARCH_ESP32)
void initCore2Display();
void updateCore2Display();
#endif

#if defined(ARDUINO_ARCH_ESP32)
void initCore2Display() {
    auto cfg = M5.config();
    M5.begin(cfg);
    M5.Display.setRotation(1);
    drawBootScreen("BOOT", "Starting dashboard...", "Reading config and connecting to Wi-Fi", TFT_ORANGE);
}

void updateCore2Display() {
    const unsigned long now = millis();
    if (now - lastDisplayUpdateMs < 1000) return;

    // MAX6675 CLK shares GPIO18 with the Core2 LCD's SPI clock. Skip the
    // redraw for a short settle window right after a sensor read so the two
    // never touch the pin back-to-back in the same instant.
    if (now - tempControl.getLastSensorReadMs() < 300) return;

    lastDisplayUpdateMs = now;

    if (!dashboardLayoutDrawn) {
        drawDashboardLiveScreen();
    }

    const float currentTemp = tempControl.getCurrentTemp();
    Serial.printf("[DISPLAY] currentTemp=%.2f (same value used by /status)\n", currentTemp);

    float targetTemp = tempControl.getTargetTemp();
    ControlState state = tempControl.getState();
    int heaterState = tempControl.getHeaterState();
    bool tempValid = currentTemp > -995.0f && currentTemp < 1000.0f;
    bool wifiOk = wifiConnected;

    const char* stateLabel = controlStateLabel(state);
    const char* heaterLabel = heaterStateLabel(heaterState);
    const char* wifiLabel = isAPMode ? "AP MODE" : (wifiOk ? "ONLINE" : "OFFLINE");
    const char* sensorLabel = sensorStatusLabel(currentTemp);

    M5.Display.fillRect(12, 66, 300, 38, BLACK);
    if (tempValid) {
        M5.Display.setTextColor(TFT_CYAN, BLACK);
        M5.Display.setTextSize(4);
        M5.Display.setCursor(12, 66);
        M5.Display.printf("TEMP: %.1f C", currentTemp);
    } else {
        M5.Display.setTextColor(TFT_RED, BLACK);
        M5.Display.setTextSize(4);
        M5.Display.setCursor(12, 66);
        M5.Display.print("TEMP: --.- C");
    }

    M5.Display.setTextSize(2);
    M5.Display.fillRect(12, 116, 300, 22, BLACK);
    M5.Display.setTextColor(TFT_ORANGE, BLACK);
    M5.Display.setCursor(12, 116);
    M5.Display.printf("TARGET: %.1f C", targetTemp);

    M5.Display.fillRect(12, 146, 300, 22, BLACK);
    M5.Display.setTextColor(TFT_WHITE, BLACK);
    M5.Display.setCursor(12, 146);
    M5.Display.printf("STATE: %s", stateLabel);

    M5.Display.fillRect(12, 172, 300, 22, BLACK);
    M5.Display.setCursor(12, 172);
    M5.Display.printf("HEATER: %s", heaterLabel);

    M5.Display.fillRect(12, 198, 300, 22, BLACK);
    M5.Display.setCursor(12, 198);
    M5.Display.printf("WIFI: %s", wifiLabel);

    M5.Display.fillRect(12, 224, 300, 22, BLACK);
    M5.Display.setCursor(12, 224);
    if (tempValid) {
        M5.Display.printf("SENSOR: %s", sensorLabel);
    } else {
        M5.Display.printf("SENSOR: %s (%.0f)", sensorLabel, currentTemp);
    }

    M5.Display.setTextColor(WHITE, BLACK);
}

void updateBootDisplay(const char* title, const char* line1, const char* line2, uint16_t accentColor) {
    const unsigned long now = millis();
    if (now - lastBootDisplayUpdateMs < 500) return;
    lastBootDisplayUpdateMs = now;
    drawBootScreen(title, line1, line2, accentColor);
}

#endif

// ============================================================
void loadConfig(Config& config) {
    SystemConfig::begin();
    SystemConfig::readWiFi(config.ssid, config.password);

    String uuid, key, ip;
    AppConfig::readMerossDevice(uuid, key, ip);
    config.merossUUID = uuid;
    config.merossKey  = key;
    config.merossIP   = ip;

    // 保存値がない場合はデフォルトを使用
    if (config.ssid.length() == 0) {
        config.ssid     = DEFAULT_CONFIG.ssid;
        config.password = DEFAULT_CONFIG.password;
    }
    if (config.merossUUID.length() == 0) {
        config.merossUUID = DEFAULT_CONFIG.merossUUID;
        config.merossKey  = DEFAULT_CONFIG.merossKey;
        config.merossIP   = DEFAULT_CONFIG.merossIP;
    }

    validateConfig(config);
}

void validateConfig(Config& config) {
    if (config.ssid.length() == 0 || config.ssid.length() > 32) {
        Serial.println("[WARNING] Invalid SSID, using default");
        config.ssid = DEFAULT_CONFIG.ssid;
    }
    if (config.password.length() < 8) {
        Serial.println("[WARNING] Password too short, using default");
        config.password = DEFAULT_CONFIG.password;
    }
    if (config.merossUUID.length() != 32) {
        Serial.println("[WARNING] Invalid Meross UUID, using default");
        config.merossUUID = DEFAULT_CONFIG.merossUUID;
    }
    if (config.merossKey.length() != 32) {
        Serial.println("[WARNING] Invalid Meross key, using default");
        config.merossKey = DEFAULT_CONFIG.merossKey;
    }
    if (config.merossIP.length() == 0) {
        Serial.println("[WARNING] Invalid Meross IP, using default");
        config.merossIP = DEFAULT_CONFIG.merossIP;
    }
    if (config.defaultTargetTemp < 0 || config.defaultTargetTemp > 500) {
        Serial.println("[WARNING] Invalid target temperature, using default");
        config.defaultTargetTemp = DEFAULT_CONFIG.defaultTargetTemp;
    }
}

void initModules(const Config& config) {
    meross.setDeviceInfo(config.merossUUID, config.merossKey, config.merossIP);
    tempControl.setTargetTemp(config.defaultTargetTemp);
}

// ============================================================
void setup() {
    Serial.begin(115200);
    Serial.println("ESP8266 Smoker Controller v1.0");
    Serial.println("==============================");
    Serial.printf("MAX6675 pins -> CS:%d CLK:%d SO:%d\n", SMOKER_PIN_MAX6675_CS, SMOKER_PIN_MAX6675_CLK, SMOKER_PIN_MAX6675_SO);

#if defined(ARDUINO_ARCH_ESP32)
    initCore2Display();
#endif

    randomSeed(analogRead(SMOKER_RANDOM_SEED_PIN));

    // --- 設定読み込み ---
    Config config = DEFAULT_CONFIG;
    loadConfig(config);

    // --- WiFi接続 ---
    Serial.printf("Connecting to WiFi: %s\n", config.ssid.c_str());
    WiFi.begin(config.ssid.c_str(), config.password.c_str());

    unsigned long startTime = millis();
    while (WiFi.status() != WL_CONNECTED) {
        const unsigned long elapsedSec = (millis() - startTime) / 1000;
        char elapsedText[24];
        snprintf(elapsedText, sizeof(elapsedText), "%lu sec elapsed", elapsedSec);
#if defined(ARDUINO_ARCH_ESP32)
        updateBootDisplay("CONNECTING", config.ssid.c_str(), elapsedText, TFT_ORANGE);
#endif
        if (millis() - startTime > WIFI_TIMEOUT_MS) {
            Serial.println("\n[WARNING] WiFi connection failed! Starting AP Mode for Setup.");
            break;
        }
        delay(500);
        Serial.print(".");
    }

    if (WiFi.status() == WL_CONNECTED) {
        wifiConnected = true;
        Serial.println("\nWiFi Connected!");
        Serial.printf("IP Address: %s\n", WiFi.localIP().toString().c_str());

#if defined(ARDUINO_ARCH_ESP32)
        drawBootScreen("ONLINE", WiFi.localIP().toString().c_str(), "Dashboard will start now", TFT_GREEN);
#endif

        if (MDNS.begin("smoker")) {
            Serial.println("mDNS responder started: smoker.local");
        }
    } else {
        WiFi.mode(WIFI_AP);
        WiFi.softAP("SMOKER-SETUP");
        delay(500);
        Serial.print("\nAP IP address: ");
        Serial.println(WiFi.softAPIP());
        dnsServer.start(DNS_PORT, "*", WiFi.softAPIP());
        isAPMode = true;

#if defined(ARDUINO_ARCH_ESP32)
        drawBootScreen("AP MODE", "Connect to SMOKER-SETUP", WiFi.softAPIP().toString().c_str(), TFT_YELLOW);
#endif
    }

    // --- LittleFS (WebUI) ---
    if (!LittleFS.begin()) {
        Serial.println("[ERROR] LittleFS Mount Failed! Please 'Upload Filesystem Image'");
    } else {
        Serial.println("LittleFS Mounted Successfully.");
    }

#if defined(ARDUINO_ARCH_ESP32)
    drawStageScreen("LFS OK", "LittleFS mounted", TFT_CYAN);
    delay(800);
#endif

    // --- Core モジュール初期化 ---
    logModule.begin();

#if defined(ARDUINO_ARCH_ESP32)
    drawStageScreen("LOG OK", "Log module ready", TFT_CYAN);
    delay(800);
#endif

    appApi.registerRoutes();   // AppAPI のルートを先に登録
    coreServer.begin(isAPMode);

#if defined(ARDUINO_ARCH_ESP32)
    drawStageScreen("HTTP OK", "Web routes ready", TFT_CYAN);
    delay(800);
#endif

    if (wifiConnected) {
        ntp.begin();
        ota.begin("smoker");   // OTA はWiFi接続時のみ有効

#if defined(ARDUINO_ARCH_ESP32)
        drawStageScreen("OTA OK", "OTA ready", TFT_CYAN);
        delay(800);
#endif
    }

    // --- App モジュール初期化 ---
    initModules(config);

#if defined(ARDUINO_ARCH_ESP32)
    drawStageScreen("APP OK", "Modules configured", TFT_GREEN);
    delay(800);
#endif

#if defined(ARDUINO_ARCH_ESP32)
    drawStageScreen("SENSOR OK", "Switching to dashboard", TFT_GREEN);
    delay(800);

    drawDashboardLiveScreen();
    delay(200);
#endif

#if defined(ARDUINO_ARCH_ESP32)
    lastDisplayUpdateMs = 0;
    updateCore2Display();
#endif

    // --- 起動メッセージ ---
    Serial.println("\nSystem initialized successfully!");
    Serial.println("Serial commands: t<temp>, o, m, i, s, h");
    Serial.println("HTTP API: /control/*, /status, /config, /log");
    if (wifiConnected) {
        Serial.println("OTA: Ready (use PlatformIO 'Upload (OTA)')");
    }
}

void loop() {
#if defined(ARDUINO_ARCH_ESP8266)
    MDNS.update();
#endif

#if defined(ARDUINO_ARCH_ESP32)
    M5.update();
#endif

    if (isAPMode) {
        dnsServer.processNextRequest();
        tempControl.update();
        coreServer.handle();
        serialCmd.processInput();
#if defined(ARDUINO_ARCH_ESP32)
        updateCore2Display();
#endif
        delay(10);
        return;
    }

    ota.handle();
    tempControl.update();
    coreServer.handle();
    serialCmd.processInput();
#if defined(ARDUINO_ARCH_ESP32)
    updateCore2Display();
#endif
    delay(10);
}