#include "SensorStats.h"
#include <Arduino.h>
#include <time.h>
#include <float.h>

static float _minTemp =  FLT_MAX;
static float _maxTemp = -FLT_MAX;
static float _minHum  =  FLT_MAX;
static float _maxHum  = -FLT_MAX;
static float _minVoc  =  FLT_MAX;
static float _maxVoc  = -FLT_MAX;
static float _minNoise =  FLT_MAX;
static float _maxNoise = -FLT_MAX;

static int _lastResetDay = -1; // day-of-month when we last reset

void statsBegin() {
    // Record today so we don't immediately reset on first loop()
    struct tm timeinfo;
    if (getLocalTime(&timeinfo)) {
        _lastResetDay = timeinfo.tm_mday;
    }
}

static void resetStats() {
    _minTemp =  FLT_MAX;
    _maxTemp = -FLT_MAX;
    _minHum  =  FLT_MAX;
    _maxHum  = -FLT_MAX;
    _minVoc  =  FLT_MAX;
    _maxVoc  = -FLT_MAX;
    _minNoise =  FLT_MAX;
    _maxNoise = -FLT_MAX;
}

void statsUpdate(float temp, float hum, float voc, float noise) {
    if (temp != -999.0f) {
        if (temp < _minTemp) _minTemp = temp;
        if (temp > _maxTemp) _maxTemp = temp;
    }
    if (hum != -999.0f) {
        if (hum < _minHum) _minHum = hum;
        if (hum > _maxHum) _maxHum = hum;
    }
    if (voc != -999.0f && voc > 0) {
        if (voc < _minVoc) _minVoc = voc;
        if (voc > _maxVoc) _maxVoc = voc;
    }
    if (noise != -999.0f && noise > 0) {
        if (noise < _minNoise) _minNoise = noise;
        if (noise > _maxNoise) _maxNoise = noise;
    }
}

float statsMinTemp() { return _minTemp ==  FLT_MAX ? -1 : _minTemp; }
float statsMaxTemp() { return _maxTemp == -FLT_MAX ? -1 : _maxTemp; }
float statsMinHum()  { return _minHum  ==  FLT_MAX ? -1 : _minHum;  }
float statsMaxHum()  { return _maxHum  == -FLT_MAX ? -1 : _maxHum;  }
float statsMinVoc()  { return _minVoc  ==  FLT_MAX ? -1 : _minVoc;  }
float statsMaxVoc()  { return _maxVoc  == -FLT_MAX ? -1 : _maxVoc;  }
float statsMinNoise() { return _minNoise ==  FLT_MAX ? -1 : _minNoise; }
float statsMaxNoise() { return _maxNoise == -FLT_MAX ? -1 : _maxNoise; }

void statsCheckMidnightReset() {
    struct tm timeinfo;
    if (!getLocalTime(&timeinfo)) return; // NTP not ready yet

    if (_lastResetDay == -1) {
        // NTP just became available for the first time
        _lastResetDay = timeinfo.tm_mday;
        return;
    }

    if (timeinfo.tm_mday != _lastResetDay) {
        resetStats();
        _lastResetDay = timeinfo.tm_mday;
        Serial.println("Midnight reset: min/max stats cleared");
    }
}