#include "SensorReader.h"
#include "DHT.h"
#include <Arduino.h>

#define DHTPIN          4
#define DHTTYPE         DHT11
#define WARMUP_READINGS 5
#define WARMUP_DELAY_MS 2000

// ── MQ-135 config ─────────────────────────────────────────────────────────────
#define MQ135_PIN       34      // GPIO34 — analog input only

// Load resistance on the breakout board (kΩ) — typically 10kΩ
#define MQ135_RL        10.0f

// Rs/R0 ratio in clean air — calibrate by reading in fresh air for 24h
#define MQ135_R0        76.63f

// Curve constants for CO2 (from MQ-135 datasheet log-log regression)
#define MQ135_PARA      116.6020682f
#define MQ135_PARB     -2.769034857f

// ADC reference voltage and resolution
#define MQ135_VREF      3.3f
#define MQ135_ADC_MAX   4095.0f

// Voltage divider correction — restores actual sensor voltage before the divider
// Divider: R1=5.6kΩ (top), R2=10kΩ (bottom)
// Scale = R2 / (R1 + R2) = 10 / 15.6 = 0.6410
// Inverse applied to recover original sensor voltage
#define MQ135_DIVIDER_SCALE  (10.0f / (5.6f + 10.0f))

#define MQ135_MIN_RAW   100

static DHT  dht(DHTPIN, DHTTYPE);
static bool _ready           = false;
static int  _warmupRemaining = WARMUP_READINGS;
static unsigned long _lastReadMs = 0;

void sensorBegin() {
    dht.begin();
    pinMode(MQ135_PIN, INPUT);
    Serial.println("[Sensor] DHT11 initialised. Warming up...");
    Serial.println("[Sensor] MQ-135 initialised on GPIO34.");
}

bool sensorTick() {
    if (_ready) return true;
    unsigned long now = millis();
    if (now - _lastReadMs < WARMUP_DELAY_MS) return false;
    _lastReadMs = now;

    float t = dht.readTemperature();
    float h = dht.readHumidity();

    if (!isnan(t) && !isnan(h)) {
        _warmupRemaining--;
        Serial.printf("[DHT11] Warmup discard (%d left): %.1f C %.1f%%\n",
                      _warmupRemaining, t, h);
        if (_warmupRemaining <= 0) {
            _ready = true;
            Serial.println("[DHT11] Ready.");
        }
    }
    return _ready;
}

float readTemperature() {
    if (!_ready) return -999.0f;
    float t = dht.readTemperature();
    return isnan(t) ? -999.0f : t;
}

float readHumidity() {
    if (!_ready) return -999.0f;
    float h = dht.readHumidity();
    return isnan(h) ? -999.0f : h;
}

float readGas() {
    int raw = analogRead(MQ135_PIN);

    // Consider disconnected if raw value is too low
    if (raw < MQ135_MIN_RAW) {
        return -999.0f;
    }

    // Convert raw ADC → measured voltage → actual sensor voltage (undo divider)
    float vMeasured = (raw / MQ135_ADC_MAX) * MQ135_VREF;
    float voltage   = vMeasured / MQ135_DIVIDER_SCALE;  // recover pre-divider voltage
    if (voltage <= 0.0f) return -999.0f;

    // Rs = RL * (Vref - Vout) / Vout
    float rs = MQ135_RL * (MQ135_VREF - voltage) / voltage;
    if (rs <= 0.0f) return -999.0f;

    float ratio = rs / MQ135_R0;
    float ppm   = MQ135_PARA * powf(ratio, MQ135_PARB);

    // Clamp to reasonable indoor range
    if (ppm < 0.0f)    return -999.0f;
    if (ppm > 10000.f) ppm = 10000.0f;

    return ppm;
}

bool sensorWarmedUp() { return _ready; }