#include <Arduino.h>
#include <WiFi.h>
#include <WebServer.h>
#include <SPIFFS.h>
#include <time.h>

#include "Secrets.h"
#include "SensorReader.h"
#include "SensorStats.h"
#include "WebHandlers.h"

// NTP settings — adjust NTP_TZ_OFFSET to your timezone in seconds
// UTC+5:30 (India) = 19800 | UTC+0 = 0 | UTC-5 (EST) = -18000 | UTC+1 (CET) = 3600
#define NTP_SERVER     "pool.ntp.org"
#define NTP_TZ_OFFSET  19800  // UTC+5:30 (IST)
#define NTP_DST_OFFSET 0

WebServer server(80);

void setup() {
    Serial.begin(115200);

    if (!SPIFFS.begin(true)) {
        Serial.println("SPIFFS mount failed!");
        return;
    }
    Serial.println("SPIFFS mounted");

    sensorBegin();

    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
    Serial.print("Connecting to WiFi");
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }
    Serial.println("\nConnected!");
    Serial.println(WiFi.localIP());

    // Sync time over NTP
    configTime(NTP_TZ_OFFSET, NTP_DST_OFFSET, NTP_SERVER);
    Serial.print("Syncing NTP time");
    struct tm timeinfo;
    while (!getLocalTime(&timeinfo)) {
        delay(500);
        Serial.print(".");
    }
    Serial.printf("\nTime synced: %02d:%02d:%02d\n",
                  timeinfo.tm_hour, timeinfo.tm_min, timeinfo.tm_sec);

    statsBegin();

    registerRoutes(server);
    server.begin();
    Serial.println("HTTP server started");
}

void loop() {
    server.handleClient();
    statsCheckMidnightReset();
}