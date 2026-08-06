#include "meross_control.h"

MerossControlModule::MerossControlModule(NTPModule& ntp, MD5Module& md5) : _ntp(ntp), _md5(md5) {}

void MerossControlModule::setDeviceInfo(const String& uuid, const String& deviceKey, const String& ip) {
    _uuid = uuid;
    _deviceKey = deviceKey;
    _targetIp = ip;
}

String MerossControlModule::generateMsgId() {
    String res = "";
    const char* hex = "0123456789abcdef";
    for (int i = 0; i < 32; i++) res += hex[random(16)];
    return res;
}

bool MerossControlModule::toggle(bool onoff) {
    unsigned long ts = _ntp.getTime();
    if (ts == 0) {
        Serial.println("[MEROSS] NTP time sync failed");
        return false;
    }

    if (_targetIp.length() == 0) {
        Serial.println("[MEROSS] Target IP is empty. Configure /config/device first.");
        return false;
    }

    WiFiClient client;
    client.setTimeout(3000);
    bool connected = false;
    for (int attempt = 1; attempt <= 3; attempt++) {
        if (client.connect(_targetIp.c_str(), 80)) {
            connected = true;
            break;
        }
        Serial.printf("[MEROSS] Connection attempt %d/3 to %s failed\n", attempt, _targetIp.c_str());
        client.stop();
        delay(150);
    }

    if (!connected) {
        Serial.printf("[MEROSS] Connection to %s failed. Check Meross local IP and network route.\n", _targetIp.c_str());
        if (_targetIp == "192.168.0.2") {
            Serial.println("[MEROSS] Hint: device IP may have changed by DHCP. Update /config/device.");
        }
        return false;
    }

    String msgId = generateMsgId();
    String sign = _md5.calculateSign(msgId, _deviceKey, ts);
    String body = "{\"header\":{\"from\":\"http://" + _targetIp + "/config\",\"messageId\":\"" + msgId +
                  "\",\"method\":\"SET\",\"namespace\":\"Appliance.Control.ToggleX\",\"payloadVersion\":1,\"sign\":\"" + sign +
                  "\",\"timestamp\":" + String(ts) + ",\"triggerSrc\":\"AndroidLocal\",\"uuid\":\"" + _uuid +
                  "\"},\"payload\":{\"togglex\":{\"channel\":0,\"onoff\":" + String(onoff ? 1 : 0) + "}}}";

    client.print(String("POST /config HTTP/1.1\r\nHost: ") + _targetIp + "\r\nContent-Type: application/json; charset=UTF-8\r\n" +
                 "Content-Length: " + body.length() + "\r\nConnection: close\r\nUser-Agent: okhttp/5.0.0-alpha.14\r\n\r\n" + body);

    unsigned long t = millis();
    String response = "";
    while (millis() - t < 3500) {
        while (client.available()) {
            response += (char)client.read();
            t = millis();
        }
        if (!client.connected()) {
            break;
        }
        delay(2);
    }

    bool okHttp = response.indexOf("200 OK") > -1;
    bool okAck = response.indexOf("SETACK") > -1;
    bool okCode = response.indexOf("\"code\":0") > -1 || response.indexOf("\"code\": 0") > -1;
    bool success = okHttp && (okAck || okCode);

    Serial.printf("[MEROSS] Toggle %s: %s\n", onoff ? "ON" : "OFF", success ? "SUCCESS" : "FAILED");
    if (!success) {
        String head = response.substring(0, 220);
        head.replace("\r", " ");
        head.replace("\n", " ");
        Serial.printf("[MEROSS] Response(head): %s\n", head.c_str());
    }

    client.stop();
    return success;
}