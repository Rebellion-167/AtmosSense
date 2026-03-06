#include "SensorReader.h"
#include "DHT.h"

#define DHTPIN          4
#define DHTTYPE         DHT11
#define WARMUP_READINGS 5       // discard this many valid readings on startup
#define WARMUP_DELAY_MS 2000    // DHT11 minimum sampling period

static DHT  dht(DHTPIN, DHTTYPE);
static bool _ready           = false;
static int  _warmupRemaining = WARMUP_READINGS;
static unsigned long _lastReadMs = 0;

void sensorBegin() {
    dht.begin();
    Serial.println("DHT11 initialised. Warming up in background...");
}

// Called from loop() — non-blocking warmup, returns true once ready
bool sensorTick() {
    if (_ready) return true;

    unsigned long now = millis();
    if (now - _lastReadMs < WARMUP_DELAY_MS) return false;
    _lastReadMs = now;

    float t = dht.readTemperature();
    float h = dht.readHumidity();

    if (!isnan(t) && !isnan(h)) {
        _warmupRemaining--;
        Serial.printf("DHT11 warmup discarded (%d left): %.1f deg C %.1f%%\n",
                      _warmupRemaining, t, h);
        if (_warmupRemaining <= 0) {
            _ready = true;
            Serial.println("DHT11 ready.");
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

bool sensorWarmedUp() { return _ready; }