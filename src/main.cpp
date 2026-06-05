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

// --- 前方宣言 ---
void loadConfig(Config& config);
void validateConfig(Config& config);
void initModules(const Config& config);

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

    randomSeed(analogRead(SMOKER_RANDOM_SEED_PIN));

    // --- 設定読み込み ---
    Config config = DEFAULT_CONFIG;
    loadConfig(config);

    // --- WiFi接続 ---
    Serial.printf("Connecting to WiFi: %s\n", config.ssid.c_str());
    WiFi.begin(config.ssid.c_str(), config.password.c_str());

    unsigned long startTime = millis();
    while (WiFi.status() != WL_CONNECTED) {
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
    }

    // --- LittleFS (WebUI) ---
    if (!LittleFS.begin()) {
        Serial.println("[ERROR] LittleFS Mount Failed! Please 'Upload Filesystem Image'");
    } else {
        Serial.println("LittleFS Mounted Successfully.");
    }

    // --- Core モジュール初期化 ---
    logModule.begin();
    appApi.registerRoutes();   // AppAPI のルートを先に登録
    coreServer.begin(isAPMode);

    if (wifiConnected) {
        ntp.begin();
        ota.begin("smoker");   // OTA はWiFi接続時のみ有効
    }

    // --- App モジュール初期化 ---
    max6675.begin();
    initModules(config);

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

    if (isAPMode) {
        dnsServer.processNextRequest();
        coreServer.handle();
        serialCmd.processInput();
        delay(10);
        return;
    }

    ota.handle();
    tempControl.update();
    coreServer.handle();
    serialCmd.processInput();
    delay(10);
}