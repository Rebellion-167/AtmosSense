#include <Arduino.h>
#include <WiFi.h>
#include <WebServer.h>
#include <SPIFFS.h>
#include <time.h>

#include "SensorReader.h"
#include "SensorStats.h"
#include "SensorHistory.h"
#include "WebHandlers.h"
#include "AlertManager.h"
#include "WiFiManager.h"
#include "OledDisplay.h"
#include "RoomConfig.h"
#include "AqiConverter.h"

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

    checkResetButton();

    // Init OLED first — this calls Wire.begin(21,22) which the SHT30 also needs
    oledBegin();
    oledBootScreen();

    sensorBegin();

    wifiManagerBegin();
    oledWifiConnected(WiFi.localIP().toString().c_str());
    oledSetSystem(WiFi.localIP().toString().c_str(), 0);
    delay(1500);

    configTime(0, 0, NTP_SERVER);

    statsBegin();
    historyBegin();   // ← ring buffer init (after statsBegin, before routes)
    roomConfigBegin();

    registerRoutes(server);
    server.begin();
    Serial.println("HTTP server started");
}

void performSensorUpdate() {
    float temp = readTemperature();
    float hum = readHumidity();
    float voc = readVoc();
    float vocNorm = readVocNormalized();
    float noise = readNoise();
    
    bool dhtReady = (temp != -999.0f && hum != -999.0f);
    bool vocReady = (voc != -999.0f && voc > 0);
    
    if (dhtReady) {
        statsUpdate(temp, hum, voc, noise);
        alertUpdate(temp, hum, voc, noise);
        historyTick(temp, hum, vocReady ? voc : -999.0f);
        
        int aqi = ppmToAqi(vocReady ? voc : 0);
        const char *comfortLabel = alertGetComfortLabel();
        
        oledSetData(roomGetName(), temp, hum, voc, vocNorm, noise,
                    alertGetFeelsLike(), comfortLabel ? comfortLabel : "",
                    aqi,
                    statsMinTemp(), statsMaxTemp(),
                    statsMinHum(),  statsMaxHum(),
                    statsMinVoc(),  statsMaxVoc(),
                    statsMinNoise(), statsMaxNoise(),
                    alertGetTempState(), alertGetHumState(), alertGetVocState(), alertGetNoiseState());
    }
}

void loop() {
    sensorTick();
    server.handleClient();
    statsCheckMidnightReset();
    oledTick();

    static unsigned long _lastSensorReadMs = 0;
    if (sensorWarmedUp() && millis() - _lastSensorReadMs >= 2000) {
        _lastSensorReadMs = millis();
        performSensorUpdate();
    }

    static unsigned long _lastUptimeMs = 0;
    if (millis() - _lastUptimeMs >= 1000) {
        _lastUptimeMs = millis();
        oledSetSystem(WiFi.localIP().toString().c_str(), millis() / 1000);
    }
}