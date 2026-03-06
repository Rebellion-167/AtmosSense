#include "AlertManager.h"
#include <Arduino.h>

// ── Temperature offsets from 30-day location mean ────────────────────────────
#define TEMP_SAFE_OFFSET   4.0f   // within ±4°C of mean → safe
#define TEMP_DANGER_OFFSET 8.0f   // beyond ±8°C of mean → danger zone

// ── Indoor humidity — fixed comfort ranges (outdoor baseline not applicable) ──
#define HUM_SAFE_LO    30.0f
#define HUM_SAFE_HI    65.0f
#define HUM_DANGER_LO  20.0f
#define HUM_DANGER_HI  70.0f

// ── Gas — universal (ppm CO2-equivalent) ─────────────────────────────────────
#define GAS_SAFE_PPM   1000.0f
#define GAS_DANGER_PPM 2000.0f

// ── State ─────────────────────────────────────────────────────────────────────
static AlertLevel _level  = ALERT_NONE;
static const char* _reason = "All parameters normal";
static ClimateThresholds _th;

static void setDefaults() {
    _th = {
        .tempSafeLo   = 18.0f, .tempSafeHi   = 28.0f,
        .tempDangerLo = 10.0f, .tempDangerHi = 35.0f,
    };
}

static void setLeds(bool green, bool yellow, bool red) {
    digitalWrite(LED_GREEN,  green  ? HIGH : LOW);
    digitalWrite(LED_YELLOW, yellow ? HIGH : LOW);
    digitalWrite(LED_RED,    red    ? HIGH : LOW);
}

// ── Public API ────────────────────────────────────────────────────────────────
void alertBegin() {
    pinMode(LED_GREEN,  OUTPUT);
    pinMode(LED_YELLOW, OUTPUT);
    pinMode(LED_RED,    OUTPUT);
    setLeds(false, false, false);
    setDefaults();
}

void alertSetClimate(float meanTemp, float meanHum) {
    _th.tempSafeLo   = meanTemp - TEMP_SAFE_OFFSET;
    _th.tempSafeHi   = meanTemp + TEMP_SAFE_OFFSET;
    _th.tempDangerLo = meanTemp - TEMP_DANGER_OFFSET;
    _th.tempDangerHi = meanTemp + TEMP_DANGER_OFFSET;

    Serial.printf("[Alert] Temp safe   : %.1f – %.1f deg C\n", _th.tempSafeLo,   _th.tempSafeHi);
    Serial.printf("[Alert] Temp danger : %.1f – %.1f deg C\n", _th.tempDangerLo, _th.tempDangerHi);
}

AlertLevel alertUpdate(float temp, float hum, float gas) {
    // ── Evaluate each sensor independently ───────────────────────────────────
    // Result per sensor: -1 = not connected, 0 = safe, 1 = unsafe, 2 = danger
    int tempState = -1, humState = -1, gasState = -1;

    if (temp != -999.0f) {
        if (temp < _th.tempDangerLo || temp > _th.tempDangerHi)
            tempState = 2;
        else if (temp < _th.tempSafeLo || temp > _th.tempSafeHi)
            tempState = 1;
        else
            tempState = 0;
    }

    if (hum != -999.0f) {
        if (hum < HUM_DANGER_LO || hum > HUM_DANGER_HI)
            humState = 2;
        else if (hum < HUM_SAFE_LO || hum > HUM_SAFE_HI)
            humState = 1;
        else
            humState = 0;
    }

    if (gas != -999.0f && gas > 0) {
        if (gas >= GAS_DANGER_PPM)
            gasState = 2;
        else if (gas >= GAS_SAFE_PPM)
            gasState = 1;
        else
            gasState = 0;
    }

    // ── Count connected sensors and how many are in each state ────────────────
    int connected = 0, safe = 0, unsafe = 0, danger = 0;
    int states[3] = { tempState, humState, gasState };
    const char* names[3] = { "Temperature", "Humidity", "Air quality" };

    for (int i = 0; i < 3; i++) {
        if (states[i] == -1) continue;
        connected++;
        if      (states[i] == 0) safe++;
        else if (states[i] == 1) unsafe++;
        else if (states[i] == 2) danger++;
    }

    // ── Determine LED state ───────────────────────────────────────────────────
    // Green  : all connected sensors are safe
    // Yellow : at least one is outside safe range (but not all in danger)
    // Red    : every connected sensor is in the danger zone
    static char reasonBuf[80];
    AlertLevel prevLevel = _level;

    if (connected == 0 || safe == connected) {
        // All safe (or nothing connected)
        _level  = ALERT_NONE;
        _reason = "All parameters normal";
        setLeds(true, false, false);

    } else if (danger == connected) {
        // Every connected sensor is in danger
        _level = ALERT_DANGER;
        // Report which sensors are in danger
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
        // At least one out of range but not all in danger
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

AlertLevel  alertGetLevel()  { return _level;  }
const char* alertGetReason() { return _reason; }