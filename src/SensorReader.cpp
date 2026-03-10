#include "SensorReader.h"
#include <Wire.h>
#include <Adafruit_SHT31.h>
#include <Arduino.h>

// ── SHT30 config ──────────────────────────────────────────────────────────────
#define SHT30_ADDR      0x44
#define SHT30_SDA       21
#define SHT30_SCL       22

// Warmup: discard this many valid readings after power-on
#define WARMUP_READINGS 3
#define WARMUP_DELAY_MS 1000    // SHT30 max measurement interval

// ── MQ-135 config ─────────────────────────────────────────────────────────────
#define MQ135_PIN       34

#define MQ135_RL        10.0f
#define MQ135_R0        76.63f
#define MQ135_PARA      116.6020682f
#define MQ135_PARB      -2.769034857f
#define MQ135_VREF      3.3f
#define MQ135_ADC_MAX   4095.0f
#define MQ135_DIVIDER_SCALE  (10.0f / (5.6f + 10.0f))
#define MQ135_MIN_RAW   100

// ── State ─────────────────────────────────────────────────────────────────────
static Adafruit_SHT31 _sht;
static bool          _ready           = false;
static int           _warmupRemaining = WARMUP_READINGS;
static unsigned long _lastReadMs      = 0;

void sensorBegin() {
    // Share the I2C bus already initialised by the OLED (Wire on SDA=21, SCL=22)
    if (!_sht.begin(SHT30_ADDR)) {
        Serial.println("[SHT30] Not found — check wiring and I2C address.");
    } else {
        Serial.println("[SHT30] Found. Warming up...");
    }
    pinMode(MQ135_PIN, INPUT);
    Serial.println("[MQ-135] Initialised on GPIO34.");
}

bool sensorTick() {
    if (_ready) return true;

    unsigned long now = millis();
    if (now - _lastReadMs < WARMUP_DELAY_MS) return false;
    _lastReadMs = now;

    float t = _sht.readTemperature();
    float h = _sht.readHumidity();

    if (!isnan(t) && !isnan(h)) {
        _warmupRemaining--;
        Serial.printf("[SHT30] Warmup discard (%d left): %.2f C  %.2f%%\n",
                      _warmupRemaining, t, h);
        if (_warmupRemaining <= 0) {
            _ready = true;
            Serial.println("[SHT30] Ready.");
        }
    } else {
        Serial.println("[SHT30] Read failed during warmup — retrying...");
    }
    return _ready;
}

float readTemperature() {
    if (!_ready) return -999.0f;
    float t = _sht.readTemperature();
    return isnan(t) ? -999.0f : t;
}

float readHumidity() {
    if (!_ready) return -999.0f;
    float h = _sht.readHumidity();
    return isnan(h) ? -999.0f : h;
}

float readGas() {
    int raw = analogRead(MQ135_PIN);
    if (raw < MQ135_MIN_RAW) return -999.0f;

    float vMeasured = (raw / MQ135_ADC_MAX) * MQ135_VREF;
    float voltage   = vMeasured / MQ135_DIVIDER_SCALE;
    if (voltage <= 0.0f) return -999.0f;

    float rs  = MQ135_RL * (MQ135_VREF - voltage) / voltage;
    if (rs <= 0.0f) return -999.0f;

    float ppm = MQ135_PARA * powf(rs / MQ135_R0, MQ135_PARB);
    if (ppm < 0.0f)     return -999.0f;
    if (ppm > 10000.0f) ppm = 10000.0f;
    return ppm;
}

bool sensorWarmedUp() { return _ready; }