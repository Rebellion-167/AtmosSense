#include "SensorStats.h"
#include <Arduino.h>
#include <time.h>
#include <float.h>

static float _minTemp =  FLT_MAX;
static float _maxTemp = -FLT_MAX;
static float _minHum  =  FLT_MAX;
static float _maxHum  = -FLT_MAX;
static float _minGas  =  FLT_MAX;
static float _maxGas  = -FLT_MAX;

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
    _minGas  =  FLT_MAX;
    _maxGas  = -FLT_MAX;
}

void statsUpdate(float temp, float hum, float gas) {
    if (temp != -999.0f) {
        if (temp < _minTemp) _minTemp = temp;
        if (temp > _maxTemp) _maxTemp = temp;
    }
    if (hum != -999.0f) {
        if (hum < _minHum) _minHum = hum;
        if (hum > _maxHum) _maxHum = hum;
    }
    if (gas != -999.0f && gas > 0) {
        if (gas < _minGas) _minGas = gas;
        if (gas > _maxGas) _maxGas = gas;
    }
}

float statsMinTemp() { return _minTemp ==  FLT_MAX ? -1 : _minTemp; }
float statsMaxTemp() { return _maxTemp == -FLT_MAX ? -1 : _maxTemp; }
float statsMinHum()  { return _minHum  ==  FLT_MAX ? -1 : _minHum;  }
float statsMaxHum()  { return _maxHum  == -FLT_MAX ? -1 : _maxHum;  }
float statsMinGas()  { return _minGas  ==  FLT_MAX ? -1 : _minGas;  }
float statsMaxGas()  { return _maxGas  == -FLT_MAX ? -1 : _maxGas;  }

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