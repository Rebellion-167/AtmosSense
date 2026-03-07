#include "AlertManager.h"
#include "HeatIndex.h"
#include <Arduino.h>

// Store feels-like for external access
static float       _feelsLike    = -999.0f;
static const char* _comfortLabel = "Unknown";

static AlertLevel  _level  = ALERT_NONE;
static const char* _reason = "All parameters normal";
static int _tempState = -1;
static int _humState  = -1;
static int _gasState  = -1;

static void setLeds(bool green, bool yellow, bool red) {
    digitalWrite(LED_GREEN,  green  ? HIGH : LOW);
    digitalWrite(LED_YELLOW, yellow ? HIGH : LOW);
    digitalWrite(LED_RED,    red    ? HIGH : LOW);
}

void alertBegin() {
    pinMode(LED_GREEN,  OUTPUT);
    pinMode(LED_YELLOW, OUTPUT);
    pinMode(LED_RED,    OUTPUT);
    setLeds(false, false, false);
}

AlertLevel alertUpdate(float temp, float hum, float gas) {
    int tempState = -1, humState = -1, gasState = -1;

    // Temperature — evaluated via Heat Index (feels-like combining temp + humidity)
    // This naturally adapts to regional climates: humid 28°C ≠ dry 28°C
    if (temp != -999.0f && hum != -999.0f) {
        float feelsLike = heatIndexCalc(temp, hum);
        HeatIndexZone zone = heatIndexZone(feelsLike);
        tempState = heatIndexAlertState(zone);
        _feelsLike    = feelsLike;
        _comfortLabel = heatIndexZoneLabel(zone);
        Serial.printf("[Alert] Temp=%.1fC RH=%.1f%% FeelsLike=%.1fC Zone=%s State=%d\n",
                      temp, hum, feelsLike, heatIndexZoneLabel(zone), tempState);
    } else if (temp != -999.0f) {
        // Humidity not available — fall back to raw temperature thresholds
        if      (temp < TEMP_WARN_LO || temp > TEMP_WARN_HI) tempState = 2;
        else if (temp < TEMP_SAFE_LO || temp > TEMP_SAFE_HI) tempState = 1;
        else                                                   tempState = 0;
    }

    // Humidity
    if (hum != -999.0f) {
        if      (hum < HUM_WARN_LO || hum > HUM_WARN_HI) humState = 2;
        else if (hum < HUM_SAFE_LO || hum > HUM_SAFE_HI) humState = 1;
        else                                               humState = 0;
    }

    // Gas
    if (gas != -999.0f && gas > 0) {
        if      (gas >= GAS_DANGER_PPM) gasState = 2;
        else if (gas >= GAS_SAFE_PPM)   gasState = 1;
        else                            gasState = 0;
    }

    _tempState = tempState;
    _humState  = humState;
    _gasState  = gasState;

    // Count connected sensors
    int connected = 0, safe = 0, danger = 0;
    int states[3]       = { tempState, humState, gasState };
    const char* names[3]= { "Temperature", "Humidity", "Air quality" };

    for (int i = 0; i < 3; i++) {
        if (states[i] == -1) continue;
        connected++;
        if (states[i] == 0) safe++;
        if (states[i] == 2) danger++;
    }

    static char reasonBuf[80];
    AlertLevel prevLevel = _level;

    if (connected == 0 || safe == connected) {
        _level  = ALERT_NONE;
        _reason = "All parameters normal";
        setLeds(true, false, false);

    } else if (danger == connected) {
        _level = ALERT_DANGER;
        char parts[60] = "";
        for (int i = 0; i < 3; i++) {
            if (states[i] == 2) {
                if (strlen(parts)) strncat(parts, ", ", sizeof(parts) - strlen(parts) - 1);
                strncat(parts, names[i], sizeof(parts) - strlen(parts) - 1);
            }
        }
        snprintf(reasonBuf, sizeof(reasonBuf), "Danger: %s", parts);
        _reason = reasonBuf;
        setLeds(false, false, true);

    } else {
        _level = ALERT_WARNING;
        char parts[60] = "";
        for (int i = 0; i < 3; i++) {
            if (states[i] >= 1) {
                if (strlen(parts)) strncat(parts, ", ", sizeof(parts) - strlen(parts) - 1);
                strncat(parts, names[i], sizeof(parts) - strlen(parts) - 1);
            }
        }
        snprintf(reasonBuf, sizeof(reasonBuf), "Out of range: %s", parts);
        _reason = reasonBuf;
        setLeds(false, true, false);
    }

    if (_level != prevLevel) {
        Serial.printf("[Alert] Level -> %d : %s\n", _level, _reason);
    }

    return _level;
}

AlertLevel  alertGetLevel()        { return _level;        }
const char* alertGetReason()       { return _reason;       }
int         alertGetTempState()    { return _tempState;    }
int         alertGetHumState()     { return _humState;     }
int         alertGetGasState()     { return _gasState;     }
float       alertGetFeelsLike()    { return _feelsLike;    }
const char* alertGetComfortLabel() { return _comfortLabel; }