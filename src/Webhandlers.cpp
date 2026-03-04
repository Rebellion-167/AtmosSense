#include "WebHandlers.h"
#include "SensorReader.h"
#include "SensorStats.h"
#include <SPIFFS.h>

static WebServer* _server = nullptr;

static void serveFile(const char* path, const char* contentType) {
    if (!SPIFFS.exists(path)) {
        _server->send(404, "text/plain", "File not found");
        return;
    }
    File file = SPIFFS.open(path, "r");
    _server->streamFile(file, contentType);
    file.close();
}

static void handleRoot() {
    serveFile("/dashboard.html", "text/html");
}

static void handleCss() {
    serveFile("/dashboard.css", "text/css");
}

static void handleJs() {
    serveFile("/dashboard.js", "application/javascript");
}

static void handleData() {
    float temp = readTemperature();
    float hum  = readHumidity();

    statsUpdate(temp, hum);

    String json = "{";
    json += "\"temperature\":"  + String(temp, 1) + ",";
    json += "\"humidity\":"     + String(hum,  1) + ",";
    json += "\"minTemp\":"      + String(statsMinTemp(), 1) + ",";
    json += "\"maxTemp\":"      + String(statsMaxTemp(), 1) + ",";
    json += "\"minHum\":"       + String(statsMinHum(),  1) + ",";
    json += "\"maxHum\":"       + String(statsMaxHum(),  1);
    json += "}";

    _server->send(200, "application/json", json);
}

void registerRoutes(WebServer& server) {
    _server = &server;
    server.on("/",              handleRoot);
    server.on("/dashboard.css", handleCss);
    server.on("/dashboard.js",  handleJs);
    server.on("/data",          handleData);
    server.on("/favicon.ico",   [&server]() { server.send(204); });
}