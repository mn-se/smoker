#include "AppAPI.h"
#include <ArduinoJson.h>
#include <LittleFS.h>
#include "../core/SystemConfig.h"
#include "AppConfig.h"

AppAPI::AppAPI(CoreWebServer& core, TempControlModule& tempControl,
               MerossControlModule& meross, LogModule& logModule)
    : _core(core), _tempControl(tempControl), _meross(meross), _logModule(logModule) {}

void AppAPI::registerRoutes() {
    auto& srv = _core.server();

    // 初期セットアップ (APモード経由)
    srv.on("/api/setup",        HTTP_POST, [this]() { handleSetup(); });
    // 制御API
    srv.on("/control/temp",     HTTP_POST, [this]() { handleTempControl(); });
    srv.on("/control/on",       HTTP_POST, [this]() { handleForceOn(); });
    srv.on("/control/monitor",  HTTP_POST, [this]() { handleMonitorStart(); });
    srv.on("/control/idle",     HTTP_POST, [this]() { handleIdle(); });
    // ステータス・設定API
    srv.on("/status",           HTTP_GET,  [this]() { handleStatus(); });
    srv.on("/config/device",    HTTP_POST, [this]() { handleSetDevice(); });
    srv.on("/config/wifi",      HTTP_POST, [this]() { handleSetWiFi(); });
    srv.on("/config/pid",       HTTP_POST, [this]() { handleSetPIDConfig(); });
    srv.on("/config",           HTTP_GET,  [this]() { handleGetConfig(); });
    // ログAPI
    srv.on("/log",              HTTP_GET,  [this]() { handleGetLog(); });
    srv.on("/log/clear",        HTTP_POST, [this]() { handleClearLog(); });
}

// --- ハンドラ実装 ---

void AppAPI::handleSetup() {
    auto& srv = _core.server();
    String newSsid = srv.arg("ssid");
    String newPass = srv.arg("pass");
    String newUuid = srv.arg("uuid");
    String newKey  = srv.arg("key");
    String newIp   = srv.arg("ip");

    String oldSsid, oldPass, oldUuid, oldKey, oldIp;
    SystemConfig::readWiFi(oldSsid, oldPass);
    AppConfig::readMerossDevice(oldUuid, oldKey, oldIp);

    // SSIDが変更されておらずパスワードが空なら、既存のパスワードを維持する
    if (newPass.length() == 0 && newSsid == oldSsid) {
        newPass = oldPass;
    }
    // UUIDが変更されておらずキーが空なら、既存のキーを維持する
    if (newKey.length() == 0 && newUuid == oldUuid) {
        newKey = oldKey;
    }

    SystemConfig::saveWiFi(newSsid, newPass);
    AppConfig::saveMerossDevice(newUuid, newKey, newIp);
    _core.sendJson(200, "{\"status\":\"saved\"}");
    delay(1000);
    ESP.restart();
}

void AppAPI::handleTempControl() {
    if (_core.server().hasArg("target")) {
        float target = _core.server().arg("target").toFloat();
        _tempControl.setTargetTemp(target);
        _tempControl.startTempControl();
        _core.sendJson(200, "{\"status\":\"temp_control_started\"}");
    } else {
        _core.sendJson(400, "{\"error\":\"target parameter required\"}");
    }
}

void AppAPI::handleForceOn() {
    _tempControl.startForceOn();
    _core.sendJson(200, "{\"status\":\"force_on_started\"}");
}

void AppAPI::handleMonitorStart() {
    _tempControl.startMonitoring();
    _core.sendJson(200, "{\"status\":\"monitoring_started\"}");
}

void AppAPI::handleIdle() {
    _tempControl.stop();
    _core.sendJson(200, "{\"status\":\"idle\"}");
}

void AppAPI::handleStatus() {
    JsonDocument doc;
    ControlState state = _tempControl.getState();
    const char* stateStr =
        (state == IDLE)         ? "idle"         :
        (state == TEMP_CONTROL) ? "temp_control" :
        (state == FORCE_ON)     ? "force_on"     :
        (state == ERROR_STATE)  ? "error"         : "monitoring";
    doc["state"]       = stateStr;
    doc["temperature"] = _tempControl.getCurrentTemp();
    doc["heater_state"] = _tempControl.getHeaterState();
    String response;
    serializeJson(doc, response);
    _core.sendJson(200, response);
}

void AppAPI::handleSetDevice() {
    auto& srv = _core.server();
    if (srv.hasArg("uuid") && srv.hasArg("key") && srv.hasArg("ip")) {
        String uuid = srv.arg("uuid");
        String key  = srv.arg("key");
        String ip   = srv.arg("ip");
        _meross.setDeviceInfo(uuid, key, ip);
        AppConfig::saveMerossDevice(uuid, key, ip);
        _core.sendJson(200, "{\"status\":\"device_info_saved\"}");
    } else {
        _core.sendJson(400, "{\"error\":\"uuid, key, ip parameters required\"}");
    }
}

void AppAPI::handleSetWiFi() {
    auto& srv = _core.server();
    if (srv.hasArg("ssid") && srv.hasArg("password")) {
        String ssid     = srv.arg("ssid");
        String password = srv.arg("password");
        SystemConfig::saveWiFi(ssid, password);
        _core.sendJson(200, "{\"status\":\"wifi_configured\",\"ssid\":\"" + ssid + "\"}");
        delay(1000);
        ESP.restart();
    } else {
        _core.sendJson(400, "{\"error\":\"ssid, password parameters required\"}");
    }
}

void AppAPI::handleGetConfig() {
    JsonDocument doc;

    String ssid, password;
    SystemConfig::readWiFi(ssid, password);
    doc["wifi"]["ssid"]         = ssid;
    doc["wifi"]["password_set"] = (password.length() > 0);

    String uuid, key, ip;
    AppConfig::readMerossDevice(uuid, key, ip);
    doc["meross"]["uuid"]    = uuid;
    doc["meross"]["key_set"] = (key.length() > 0);
    doc["meross"]["ip"]      = ip;

    doc["temperature"]["target"] = _tempControl.getTargetTemp();
    doc["pid"]["kp"]             = _tempControl.getKp();
    doc["pid"]["ki"]             = _tempControl.getKi();
    doc["pid"]["kd"]             = _tempControl.getKd();
    doc["pid"]["window_size"]    = _tempControl.getWindowSize();

    String response;
    serializeJson(doc, response);
    _core.sendJson(200, response);
}

void AppAPI::handleSetPIDConfig() {
    auto& srv = _core.server();
    if (srv.hasArg("kp") && srv.hasArg("ki") && srv.hasArg("kd") && srv.hasArg("window_size")) {
        float         kp         = srv.arg("kp").toFloat();
        float         ki         = srv.arg("ki").toFloat();
        float         kd         = srv.arg("kd").toFloat();
        unsigned long windowSize = srv.arg("window_size").toInt();
        AppConfig::savePIDConfig(kp, ki, kd, windowSize);
        _tempControl.setPIDConfig(kp, ki, kd, windowSize);
        _core.sendJson(200, "{\"status\":\"pid_config_saved\"}");
    } else {
        _core.sendJson(400, "{\"error\":\"kp, ki, kd, window_size parameters required\"}");
    }
}

void AppAPI::handleGetLog() {
    File logFile = LittleFS.open(_logModule.getLogFilePath(), "r");
    if (!logFile) {
        _core.sendJson(404, "{\"error\":\"log_file_not_found\"}");
        return;
    }
    _core.server().streamFile(logFile, "text/csv");
    logFile.close();
}

void AppAPI::handleClearLog() {
    _logModule.clearLog();
    _core.sendJson(200, "{\"status\":\"log_cleared\"}");
}
