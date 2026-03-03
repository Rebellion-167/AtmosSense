#include "SensorReader.h"
#include "DHT.h"

#define DHTPIN  4
#define DHTTYPE DHT11

static DHT dht(DHTPIN, DHTTYPE);

void sensorBegin() {
    dht.begin();
}

float readTemperature() {
    float t = dht.readTemperature();
    return isnan(t) ? 0.0f : t;
}

float readHumidity() {
    float h = dht.readHumidity();
    return isnan(h) ? 0.0f : h;
}