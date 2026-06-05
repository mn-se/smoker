#include "CoreWebServer.h"

CoreWebServer::CoreWebServer() : _server(80), _isCaptivePortal(false) {}

String CoreWebServer::contentTypeFromPath(const String& path) const {
    if (path.endsWith(".html")) return "text/html";
    if (path.endsWith(".css")) return "text/css";
    if (path.endsWith(".js")) return "application/javascript";
    if (path.endsWith(".json")) return "application/json";
    if (path.endsWith(".png")) return "image/png";
    if (path.endsWith(".jpg") || path.endsWith(".jpeg")) return "image/jpeg";
    if (path.endsWith(".svg")) return "image/svg+xml";
    if (path.endsWith(".ico")) return "image/x-icon";
    if (path.endsWith(".txt")) return "text/plain";
    if (path.endsWith(".webmanifest")) return "application/manifest+json";
    return "application/octet-stream";
}

bool CoreWebServer::tryServeFile(const String& rawPath) {
    String path = rawPath;
    if (path.length() == 0 || path == "/") {
        path = "/index.html";
    }

    if (!LittleFS.exists(path)) {
        return false;
    }

    File file = LittleFS.open(path, "r");
    if (!file) {
        return false;
    }

    _server.streamFile(file, contentTypeFromPath(path));
    file.close();
    return true;
}

void CoreWebServer::begin(bool captivePortalMode) {
    _isCaptivePortal = captivePortalMode;

    _server.on("/", HTTP_GET, [this]() {
        if (!tryServeFile("/index.html")) {
            _server.send(404, "application/json", "{\"error\":\"index_not_found\"}");
        }
    });

    // 未登録URLのハンドリング
    _server.onNotFound([this]() {
        if (_server.method() == HTTP_OPTIONS) {
            // CORS preflight
            _server.sendHeader("Access-Control-Allow-Origin", "*");
            _server.sendHeader("Access-Control-Allow-Methods", "POST, GET, OPTIONS");
            _server.sendHeader("Access-Control-Allow-Headers", "Content-Type");
            _server.send(204);
        } else if (_isCaptivePortal) {
            // APモード: setup.htmlへリダイレクト
            _server.sendHeader("Location", "http://192.168.4.1/setup.html", true);
            _server.send(302, "text/plain", "Redirecting to setup");
        } else if (tryServeFile(_server.uri())) {
            return;
        } else {
            _server.sendHeader("Access-Control-Allow-Origin", "*");
            _server.send(404, "application/json", "{\"error\":\"not_found\"}");
        }
    });

    _server.begin();
    Serial.println("[CoreWebServer] HTTP server started on port 80");
}

void CoreWebServer::handle() {
    _server.handleClient();
}

void CoreWebServer::sendJson(int code, const String& json) {
    _server.sendHeader("Access-Control-Allow-Origin", "*");
    _server.send(code, "application/json", json);
}
