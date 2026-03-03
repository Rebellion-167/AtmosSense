#include "WebHandlers.h"
#include "SensorReader.h"
#include <SPIFFS.h>

static WebServer* _server = nullptr;

// Helper: read a file from SPIFFS and send it
static void serveFile(const char* path, const char* contentType) {
    if (!SPIFFS.exists(path)) {
        _server->send(404, "text/plain", "File not found");
        return;
    }
    File file = SPIFFS.open(path, "r");
    _server->streamFile(file, contentType);
    file.close();
}

// ── /  ────────────────────────────────────────────────────────────────────────
static void handleRoot() {
    serveFile("/dashboard.html", "text/html");
}

// ── /dashboard.css  ───────────────────────────────────────────────────────────
static void handleCss() {
    serveFile("/dashboard.css", "text/css");
}

// ── /dashboard.js  ────────────────────────────────────────────────────────────
static void handleJs() {
    serveFile("/dashboard.js", "application/javascript");
}

// ── /data  ────────────────────────────────────────────────────────────────────
static void handleData() {
    float temperature = readTemperature();
    float humidity    = readHumidity();

    String json = "{\"temperature\":" + String(temperature) +
                  ",\"humidity\":"    + String(humidity)    + "}";

    _server->send(200, "application/json", json);
}

// ── Registration  ─────────────────────────────────────────────────────────────
void registerRoutes(WebServer& server) {
    _server = &server;
    server.on("/",              handleRoot);
    server.on("/dashboard.css", handleCss);
    server.on("/dashboard.js",  handleJs);
    server.on("/data",          handleData);
    server.on("/favicon.ico",   [&server]() {server.send(204); }); // suppress browser icon request
}