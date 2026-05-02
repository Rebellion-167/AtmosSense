#include "WebHandlers.h"
#include "SensorReader.h"
#include "SensorStats.h"
#include "SensorHistory.h"
#include "AqiConverter.h"
#include "AlertManager.h"
#include "OledDisplay.h"
#include "RoomConfig.h"
#include "ActionAdvisor.h"
#include <SPIFFS.h>

static char _city[32] = "Unknown";
static float _outsideTemp = NAN;
static float _outsideHum = NAN;

static WebServer *_server = nullptr;

static void serveFile(const char *path, const char *contentType)
{
    if (!SPIFFS.exists(path))
    {
        _server->send(404, "text/plain", "File not found");
        return;
    }
    File file = SPIFFS.open(path, "r");
    _server->streamFile(file, contentType);
    file.close();
}

static void handleRoot()
{
    serveFile("/dashboard.html", "text/html");
}

static void handleCss()
{
    serveFile("/dashboard.css", "text/css");
}

static void handleJs()
{
    serveFile("/dashboard.js", "application/javascript");
}

static void handleData()
{
    float temp = readTemperature();
    float hum = readHumidity();
    float gas = readGas();
    float noise = readNoise();
    bool noiseReady = (noise != -999.0f);

    bool warmedUp = sensorWarmedUp();
    bool dhtReady = (temp != -999.0f && hum != -999.0f);
    bool gasReady = (gas != -999.0f && gas > 0);

    if (dhtReady)
    {
        statsUpdate(temp, hum, gas, noise);
        alertUpdate(temp, hum, gas, noise);
        historyTick(temp, dhtReady ? hum : -999.0f, gasReady ? gas : -999.0f);
    }

    float tDisplay = dhtReady ? temp : 0.0f;
    float hDisplay = dhtReady ? hum : 0.0f;
    float gDisplay = gasReady ? gas : 0.0f;
    float nDisplay = noiseReady ? noise : 0.0f;
    int aqi = ppmToAqi(gasReady ? gas : 0);
    int alertLvl = (int)alertGetLevel();

    const char *reason = alertGetReason();
    const char *comfortLabel = alertGetComfortLabel();

    RoomAdvice tAdv = adviceForTemp(comfortLabel ? comfortLabel : "Comfortable");
    RoomAdvice hAdv = adviceForHumidity(dhtReady ? hum : -1.0f);
    RoomAdvice gAdv = adviceForGas(gasReady ? gas : -1.0f);
    RoomAdvice nAdv = adviceForNoise(noiseReady ? noise : -1.0f);

    const char *tTitle  = tAdv.title  ? tAdv.title  : "";
    const char *tAction = tAdv.action ? tAdv.action : "";
    const char *tReason = tAdv.reason ? tAdv.reason : "";
    const char *hTitle  = hAdv.title  ? hAdv.title  : "";
    const char *hAction = hAdv.action ? hAdv.action : "";
    const char *hReason = hAdv.reason ? hAdv.reason : "";
    const char *gTitle  = gAdv.title  ? gAdv.title  : "";
    const char *gAction = gAdv.action ? gAdv.action : "";
    const char *gReason = gAdv.reason ? gAdv.reason : "";
    const char *nTitle  = nAdv.title  ? nAdv.title  : "";
    const char *nAction = nAdv.action ? nAdv.action : "";
    const char *nReason = nAdv.reason ? nAdv.reason : "";

    char json[2048];
    snprintf(json, sizeof(json),
             "{"
             "\"ready\":%s,"
             "\"dhtConnected\":%s,"
             "\"gasConnected\":%s,"
             "\"noiseConnected\":%s,"
             "\"temperature\":%.1f,"
             "\"humidity\":%.1f,"
             "\"gas\":%.1f,"
             "\"noise\":%.1f,"
             "\"aqi\":%d,"
             "\"feelsLike\":%.1f,"
             "\"comfortLabel\":\"%s\","
             "\"alertLevel\":%d,"
             "\"alertReason\":\"%s\","
             "\"alertTempState\":%d,"
             "\"alertHumState\":%d,"
             "\"alertGasState\":%d,"
             "\"alertNoiseState\":%d,"
             "\"tempAdvice\":{\"title\":\"%s\",\"action\":\"%s\",\"reason\":\"%s\",\"urgency\":%d},"
             "\"humAdvice\":{\"title\":\"%s\",\"action\":\"%s\",\"reason\":\"%s\",\"urgency\":%d},"
             "\"gasAdvice\":{\"title\":\"%s\",\"action\":\"%s\",\"reason\":\"%s\",\"urgency\":%d},"
             "\"noiseAdvice\":{\"title\":\"%s\",\"action\":\"%s\",\"reason\":\"%s\",\"urgency\":%d},"
             "\"minTemp\":%.1f,"
             "\"maxTemp\":%.1f,"
             "\"minHum\":%.1f,"
             "\"maxHum\":%.1f,"
             "\"minGas\":%.1f,"
             "\"maxGas\":%.1f,"
             "\"minNoise\":%.1f,"
             "\"maxNoise\":%.1f,"
             "\"historyCount\":%d"
             "}",
             warmedUp ? "true" : "false",
             dhtReady ? "true" : "false",
             gasReady ? "true" : "false",
             noiseReady ? "true" : "false",
             tDisplay, hDisplay, gDisplay, nDisplay,
             aqi,
             alertGetFeelsLike(), comfortLabel ? comfortLabel : "",
             alertLvl, reason,
             alertGetTempState(), alertGetHumState(), alertGetGasState(), alertGetNoiseState(),
             tTitle, tAction, tReason, tAdv.urgency,
             hTitle, hAction, hReason, hAdv.urgency,
             gTitle, gAction, gReason, gAdv.urgency,
             nTitle, nAction, nReason, nAdv.urgency,
             statsMinTemp(), statsMaxTemp(),
             statsMinHum(),  statsMaxHum(),
             statsMinGas(),  statsMaxGas(),
             statsMinNoise(), statsMaxNoise(),
             historyCount());

    _server->send(200, "application/json", json);

    if (dhtReady)
    {
        oledSetData(roomGetName(), temp, hum, gas, noise,
                    alertGetFeelsLike(), comfortLabel ? comfortLabel : "",
                    aqi,
                    statsMinTemp(), statsMaxTemp(),
                    statsMinHum(),  statsMaxHum(),
                    statsMinGas(),  statsMaxGas(),
                    statsMinNoise(), statsMaxNoise(),
                    alertGetTempState(), alertGetHumState(), alertGetGasState(), alertGetNoiseState());
    }
}

// ── /history — returns full ring buffer as JSON ───────────────────────────────
static void handleHistory()
{
    size_t maxBytes = historyJsonMaxBytes();
    char *buf = (char *)malloc(maxBytes);
    if (!buf)
    {
        _server->send(503, "text/plain", "Out of memory");
        return;
    }

    historyToJson(buf, maxBytes);
    _server->send(200, "application/json", buf);
    free(buf);
}

static void handleTimezone()
{
    if (!_server->hasArg("offset"))
    {
        _server->send(400, "text/plain", "Missing offset");
        return;
    }
    long offsetSeconds = _server->arg("offset").toInt();
    configTime(offsetSeconds, 0, "pool.ntp.org");

    struct tm timeinfo;
    if (getLocalTime(&timeinfo))
    {
        Serial.printf("Timezone set. Local time: %02d:%02d:%02d\n",
                      timeinfo.tm_hour, timeinfo.tm_min, timeinfo.tm_sec);
    }
    _server->send(200, "text/plain", "OK");
}

static void handleClimate()
{
    if (_server->hasArg("city") && _server->arg("city").length() > 0)
    {
        strncpy(_city, _server->arg("city").c_str(), sizeof(_city) - 1);
        _city[sizeof(_city) - 1] = '\0';
        Serial.printf("[OLED] City updated: %s\n", _city);
    }

    if (_server->hasArg("outsideTemp"))
        _outsideTemp = _server->arg("outsideTemp").toFloat();
    if (_server->hasArg("outsideHum"))
        _outsideHum = _server->arg("outsideHum").toFloat();

    float t = readTemperature();
    float h = readHumidity();
    float _g = readGas();
    float _n = readNoise();
    if (t != -999.0f && h != -999.0f)
    {
        oledSetData(roomGetName(), t, h, _g, _n,
                    alertGetFeelsLike(), alertGetComfortLabel(), ppmToAqi(_g > 0 ? _g : 0),
                    statsMinTemp(), statsMaxTemp(),
                    statsMinHum(), statsMaxHum(),
                    statsMinGas(), statsMaxGas(),
                    statsMinNoise(), statsMaxNoise(),
                    alertGetTempState(), alertGetHumState(), alertGetGasState(), alertGetNoiseState());
    }

    _server->send(200, "text/plain", "OK");
}

static void handleRoomNameGet()
{
    char json[64];
    snprintf(json, sizeof(json), "{\"roomName\":\"%s\"}", roomGetName());
    _server->send(200, "application/json", json);
}

static void handleRoomNamePost()
{
    if (!_server->hasArg("name") || _server->arg("name").length() == 0)
    {
        _server->send(400, "text/plain", "Missing name");
        return;
    }
    String name = _server->arg("name");
    name.trim();
    roomSetName(name.c_str());

    float t = readTemperature();
    float h = readHumidity();
    float _g = readGas();
    float _n = readNoise();
    if (t != -999.0f && h != -999.0f)
    {
        oledSetData(roomGetName(), t, h, _g, _n,
                    alertGetFeelsLike(), alertGetComfortLabel(), ppmToAqi(_g > 0 ? _g : 0),
                    statsMinTemp(), statsMaxTemp(),
                    statsMinHum(), statsMaxHum(),
                    statsMinGas(), statsMaxGas(),
                    statsMinNoise(), statsMaxNoise(),
                    alertGetTempState(), alertGetHumState(), alertGetGasState(), alertGetNoiseState());
    }

    _server->send(200, "text/plain", "OK");
}

static void handleExportCsv()
{
    char csv[512];
    snprintf(csv, sizeof(csv),
             "Parameter,Current,Min,Max\r\n"
             "Temperature (C),%.1f,%.1f,%.1f\r\n"
             "Humidity (%%),%.1f,%.1f,%.1f\r\n"
             "Air Quality (ppm),%.1f,%.1f,%.1f\r\n",
             readTemperature(), statsMinTemp(), statsMaxTemp(),
             readHumidity(), statsMinHum(), statsMaxHum(),
             readGas(), statsMinGas(), statsMaxGas());
    _server->sendHeader("Content-Disposition", "attachment; filename=\"atmossense.csv\"");
    _server->send(200, "text/csv", csv);
}

void registerRoutes(WebServer &server)
{
    _server = &server;
    server.on("/", handleRoot);
    server.on("/dashboard.css", handleCss);
    server.on("/dashboard.js", handleJs);
    server.on("/data", handleData);
    server.on("/history", HTTP_GET, handleHistory);
    server.on("/climate", HTTP_POST, handleClimate);
    server.on("/timezone", HTTP_POST, handleTimezone);
    server.on("/roomname", HTTP_GET, handleRoomNameGet);
    server.on("/roomname", HTTP_POST, handleRoomNamePost);
    server.on("/export", HTTP_GET, handleExportCsv);
    server.on("/favicon.ico", [&server]()
              { server.send(204); });
    server.onNotFound([&server]()
                      {
        Serial.printf("[404] %s %s\n",
            server.method() == HTTP_GET ? "GET" : "POST",
            server.uri().c_str());
        server.send(404, "text/plain", "Not found"); });
}