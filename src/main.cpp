#include <Arduino.h>
#include <WiFi.h>
#include <WebServer.h>
#include <SPIFFS.h>

#include "Secrets.h"
#include "SensorReader.h"
#include "WebHandlers.h"

WebServer server(80);

void setup() {
    Serial.begin(115200);

    // Mount SPIFFS (files uploaded from the data/ folder live here)
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

    registerRoutes(server);
    server.begin();
    Serial.println("HTTP server started");
}

void loop() {
    server.handleClient();
}