#include "SensorReader.h"
#include "DHT.h"

#define DHTPIN   4
#define DHTTYPE  DHT11
#define WARMUP_READINGS 5      // discard this many readings on startup
#define WARMUP_DELAY_MS 2000   // DHT11 minimum sampling period is 2s

static DHT  dht(DHTPIN, DHTTYPE);
static bool _ready           = false;
static int  _warmupRemaining = WARMUP_READINGS;

void sensorBegin() {
    dht.begin();
    // Allow the sensor to power up before the first read attempt
    delay(WARMUP_DELAY_MS);
    Serial.println("DHT11 warming up...");

    // Flush the first WARMUP_READINGS readings
    while (_warmupRemaining > 0) {
        float t = dht.readTemperature();
        float h = dht.readHumidity();
        if (!isnan(t) && !isnan(h)) {
            _warmupRemaining--;
            Serial.printf("Warmup reading discarded (%d left): %.1f°C %.1f%%\n",
                          _warmupRemaining, t, h);
        }
        delay(WARMUP_DELAY_MS); // DHT11 needs 2s between samples
    }
    _ready = true;
    Serial.println("DHT11 ready.");
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