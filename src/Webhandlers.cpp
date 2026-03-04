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

static void handleTimezone() {
    if (!_server->hasArg("offset")) {
        _server->send(400, "text/plain", "Missing offset");
        return;
    }
    long offsetSeconds = _server->arg("offset").toInt();
    configTime(offsetSeconds, 0, "pool.ntp.org");

    struct tm timeinfo;
    if (getLocalTime(&timeinfo)) {
        Serial.printf("Timezone set. Local time: %02d:%02d:%02d\n",
                      timeinfo.tm_hour, timeinfo.tm_min, timeinfo.tm_sec);
    }
    _server->send(200, "text/plain", "OK");
}

void registerRoutes(WebServer& server) {
    _server = &server;
    server.on("/",              handleRoot);
    server.on("/dashboard.css", handleCss);
    server.on("/dashboard.js",  handleJs);
    server.on("/data",          handleData);
    server.on("/timezone",      HTTP_POST, handleTimezone);
    server.on("/favicon.ico",   [&server]() { server.send(204); });
}