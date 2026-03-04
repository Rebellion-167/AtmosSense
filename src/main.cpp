#include <Arduino.h>
#include <WiFi.h>
#include <WebServer.h>
#include <SPIFFS.h>
#include <time.h>

#include "Secrets.h"
#include "SensorReader.h"
#include "SensorStats.h"
#include "WebHandlers.h"

#define NTP_SERVER     "pool.ntp.org"

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

    // NTP server is configured but timezone offset is set later via /timezone
    // when the user enters their city on the dashboard
    configTime(0, 0, NTP_SERVER);

    statsBegin();

    registerRoutes(server);
    server.begin();
    Serial.println("HTTP server started");
}

void loop() {
    server.handleClient();
    statsCheckMidnightReset();
}