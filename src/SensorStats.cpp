#include "SensorStats.h"
#include <Arduino.h>
#include <time.h>
#include <float.h>

static float _minTemp =  FLT_MAX;
static float _maxTemp = -FLT_MAX;
static float _minHum  =  FLT_MAX;
static float _maxHum  = -FLT_MAX;

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
}

void statsUpdate(float temp, float hum) {
    if (temp == 0 && hum == 0) return; // ignore sensor error readings

    if (temp < _minTemp) _minTemp = temp;
    if (temp > _maxTemp) _maxTemp = temp;
    if (hum  < _minHum)  _minHum  = hum;
    if (hum  > _maxHum)  _maxHum  = hum;
}

float statsMinTemp() { return _minTemp ==  FLT_MAX ? 0 : _minTemp; }
float statsMaxTemp() { return _maxTemp == -FLT_MAX ? 0 : _maxTemp; }
float statsMinHum()  { return _minHum  ==  FLT_MAX ? 0 : _minHum;  }
float statsMaxHum()  { return _maxHum  == -FLT_MAX ? 0 : _maxHum;  }

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