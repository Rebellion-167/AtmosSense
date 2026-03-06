#include "WebHandlers.h"
#include "SensorReader.h"
#include "SensorStats.h"
#include "AqiConverter.h"
#include "AlertManager.h"
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
    float gas  = -999.0f;

    bool warmedUp = sensorWarmedUp();
    bool dhtReady = (temp != -999.0f && hum != -999.0f);
    bool gasReady = (gas  != -999.0f && gas > 0);

    if (dhtReady) {
        statsUpdate(temp, hum, gas);
        alertUpdate(temp, hum, gas);
    }

    float tDisplay  = dhtReady ? temp : 0.0f;
    float hDisplay  = dhtReady ? hum  : 0.0f;
    float gDisplay  = gasReady ? gas  : 0.0f;
    int   aqi       = ppmToAqi(gasReady ? gas : 0);
    int   alertLvl  = (int)alertGetLevel();

    // Plain ASCII only — no special chars that could corrupt JSON
    const char* reason = alertGetReason();

    char json[512];
    snprintf(json, sizeof(json),
        "{"
        "\"ready\":%s,"
        "\"dhtConnected\":%s,"
        "\"gasConnected\":%s,"
        "\"temperature\":%.1f,"
        "\"humidity\":%.1f,"
        "\"gas\":%.1f,"
        "\"aqi\":%d,"
        "\"alertLevel\":%d,"
        "\"alertReason\":\"%s\","
        "\"alertTempState\":%d,"
        "\"alertHumState\":%d,"
        "\"alertGasState\":%d,"
        "\"minTemp\":%.1f,"
        "\"maxTemp\":%.1f,"
        "\"minHum\":%.1f,"
        "\"maxHum\":%.1f,"
        "\"minGas\":%.1f,"
        "\"maxGas\":%.1f"
        "}",
        warmedUp ? "true" : "false",
        dhtReady ? "true" : "false",
        gasReady ? "true" : "false",
        tDisplay, hDisplay, gDisplay,
        aqi, alertLvl, reason,
        alertGetTempState(), alertGetHumState(), alertGetGasState(),
        statsMinTemp(), statsMaxTemp(),
        statsMinHum(),  statsMaxHum(),
        statsMinGas(),  statsMaxGas()
    );

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

static void handleClimate() {
    if (!_server->hasArg("meanTemp") || !_server->hasArg("meanHum")) {
        _server->send(400, "text/plain", "Missing meanTemp or meanHum");
        return;
    }
    float meanTemp = _server->arg("meanTemp").toFloat();
    float meanHum  = _server->arg("meanHum").toFloat();
    alertSetClimate(meanTemp, meanHum);
    _server->send(200, "text/plain", "OK");
}

void registerRoutes(WebServer& server) {
    _server = &server;
    server.on("/",              handleRoot);
    server.on("/dashboard.css", handleCss);
    server.on("/dashboard.js",  handleJs);
    server.on("/data",          handleData);
    server.on("/climate",       HTTP_POST, handleClimate);
    server.on("/timezone",      HTTP_POST, handleTimezone);
    server.on("/favicon.ico",   [&server]() { server.send(204); });
    server.onNotFound([&server]() {
        Serial.printf("[404] %s %s\n",
            server.method() == HTTP_GET ? "GET" : "POST",
            server.uri().c_str());
        server.send(404, "text/plain", "Not found");
    });
}