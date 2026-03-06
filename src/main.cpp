#include <Arduino.h>
#include <WiFi.h>
#include <WebServer.h>
#include <SPIFFS.h>
#include <time.h>

#include "Secrets.h"
#include "SensorReader.h"
#include "SensorStats.h"
#include "WebHandlers.h"
#include "AlertManager.h"

#define NTP_SERVER "pool.ntp.org"

WebServer server(80);

void setup() {
    Serial.begin(115200);

    // LEDs first so self-test runs immediately
    alertBegin();
    digitalWrite(LED_RED,    HIGH); delay(300); digitalWrite(LED_RED,    LOW);
    digitalWrite(LED_YELLOW, HIGH); delay(300); digitalWrite(LED_YELLOW, LOW);
    digitalWrite(LED_GREEN,  HIGH); delay(300); digitalWrite(LED_GREEN,  LOW);

    if (!SPIFFS.begin(true)) {
        Serial.println("SPIFFS mount failed!");
        return;
    }
    Serial.println("SPIFFS mounted");

    // Non-blocking — warmup happens in loop() via sensorTick()
    sensorBegin();

    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
    Serial.print("Connecting to WiFi");
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }
    Serial.println("\nConnected!");
    Serial.println(WiFi.localIP());

    configTime(0, 0, NTP_SERVER);

    statsBegin();

    registerRoutes(server);
    server.begin();
    Serial.println("HTTP server started");
}

void loop() {
    sensorTick();              // non-blocking DHT warmup
    server.handleClient();
    statsCheckMidnightReset();
}