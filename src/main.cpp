#include <Arduino.h>
#include <WiFi.h>
#include <WebServer.h>
#include <SPIFFS.h>
#include <time.h>

#include "SensorReader.h"
#include "SensorStats.h"
#include "WebHandlers.h"
#include "AlertManager.h"
#include "WiFiManager.h"
#include "OledDisplay.h"
#include "RoomConfig.h"

#define NTP_SERVER "pool.ntp.org"

// GPIO0 (BOOT button) held low for 3s on boot clears saved WiFi credentials
#define RESET_PIN 0

WebServer server(80);

static void checkResetButton() {
    pinMode(RESET_PIN, INPUT_PULLUP);
    if (digitalRead(RESET_PIN) == LOW) {
        Serial.println("[Reset] BOOT button held — waiting 3s to confirm WiFi reset...");
        delay(3000);
        if (digitalRead(RESET_PIN) == LOW) {
            wifiClearCredentials();
            Serial.println("[Reset] WiFi credentials cleared. Restarting...");
            delay(500);
            ESP.restart();
        }
    }
}

void setup() {
    Serial.begin(115200);

    // LEDs self-test first
    alertBegin();
    digitalWrite(LED_RED,    HIGH); delay(300); digitalWrite(LED_RED,    LOW);
    digitalWrite(LED_YELLOW, HIGH); delay(300); digitalWrite(LED_YELLOW, LOW);
    digitalWrite(LED_GREEN,  HIGH); delay(300); digitalWrite(LED_GREEN,  LOW);

    if (!SPIFFS.begin(true)) {
        Serial.println("SPIFFS mount failed!");
        return;
    }
    Serial.println("SPIFFS mounted");

    // Check if user is holding BOOT to reset WiFi
    checkResetButton();

    // Non-blocking sensor warmup
    sensorBegin();

    // Init OLED early so status messages show during boot
    oledBegin();

    // Connect to WiFi — if no credentials, WiFiManager starts AP portal
    if (wifiHasCredentials()) oledStatus("Connecting...", "to WiFi");
    wifiManagerBegin();
    oledStatus("WiFi OK", WiFi.localIP().toString().c_str());
    delay(1500);
    oledStatus("Sensor warming up", "please wait...");

    configTime(0, 0, NTP_SERVER);

    statsBegin();
    roomConfigBegin();

    registerRoutes(server);
    server.begin();
    Serial.println("HTTP server started");
}

void loop() {
    sensorTick();
    server.handleClient();
    statsCheckMidnightReset();
    oledTick();
}