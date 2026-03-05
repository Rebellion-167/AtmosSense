#include "AlertManager.h"
#include <Arduino.h>

// ── Gas thresholds (universal — gas ppm is not location-dependent) ────────────
#define GAS_WARN_PPM   1000.0f
#define GAS_DANGER_PPM 2000.0f

// How far from the climate mean to trigger warnings/danger for temperature
#define TEMP_WARN_OFFSET   4.0f   // ±4°C from mean → warning
#define TEMP_DANGER_OFFSET 8.0f   // ±8°C from mean → danger

// ── State ─────────────────────────────────────────────────────────────────────
static AlertLevel       _level  = ALERT_NONE;
static const char*      _reason = "All parameters normal";
static ClimateThresholds _th;
static bool             _climateSet = false;

// ── Default universal thresholds (used until climate data arrives) ────────────
static void setDefaults() {
    _th = {
        .tempWarnLo   = 16.0f, .tempWarnHi   = 28.0f,
        .tempDangerLo = 10.0f, .tempDangerHi = 35.0f,
        .humWarnLo    = 25.0f, .humWarnHi    = 65.0f,
        .humDangerLo  = 15.0f, .humDangerHi  = 80.0f,
    };
}

// ── Helpers ───────────────────────────────────────────────────────────────────
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
    // Temperature thresholds derived from local climate mean
    _th.tempWarnLo   = meanTemp - TEMP_WARN_OFFSET;
    _th.tempWarnHi   = meanTemp + TEMP_WARN_OFFSET;
    _th.tempDangerLo = meanTemp - TEMP_DANGER_OFFSET;
    _th.tempDangerHi = meanTemp + TEMP_DANGER_OFFSET;

    // Humidity uses fixed indoor comfort thresholds regardless of outdoor climate.
    // Outdoor humidity is not a reliable baseline for indoor air quality.
    _th.humWarnLo   = 30.0f;
    _th.humWarnHi   = 65.0f;
    _th.humDangerLo = 20.0f;
    _th.humDangerHi = 80.0f;

    _climateSet = true;

    Serial.printf("[Alert] Climate thresholds updated:\n");
    Serial.printf("  Temp  warn  : %.1f – %.1f deg C\n", _th.tempWarnLo,  _th.tempWarnHi);
    Serial.printf("  Temp  danger: %.1f – %.1f deg C\n", _th.tempDangerLo, _th.tempDangerHi);
    Serial.printf("  Hum   warn  : %.1f – %.1f pct (fixed indoor range)\n", _th.humWarnLo, _th.humWarnHi);
    Serial.printf("  Hum   danger: %.1f – %.1f pct (fixed indoor range)\n", _th.humDangerLo, _th.humDangerHi);
}

AlertLevel alertUpdate(float temp, float hum, float gas) {
    bool danger  = false;
    bool warning = false;
    static char reasonBuf[80];

    // ── Temperature ───────────────────────────────────────────────────────────
    if (temp != -999.0f) {
        if (temp < _th.tempDangerLo || temp > _th.tempDangerHi) {
            danger = true;
            snprintf(reasonBuf, sizeof(reasonBuf),
                     "Dangerous temperature: %.1f deg C", temp);
        } else if (temp < _th.tempWarnLo || temp > _th.tempWarnHi) {
            warning = true;
            snprintf(reasonBuf, sizeof(reasonBuf),
                     "Temperature outside local normal: %.1f deg C", temp);
        }
    }

    // ── Humidity ──────────────────────────────────────────────────────────────
    if (hum != -999.0f) {
        if (hum < _th.humDangerLo || hum > _th.humDangerHi) {
            danger = true;
            snprintf(reasonBuf, sizeof(reasonBuf),
                     "Dangerous humidity: %.1f pct", hum);
        } else if (!danger && (hum < _th.humWarnLo || hum > _th.humWarnHi)) {
            warning = true;
            snprintf(reasonBuf, sizeof(reasonBuf),
                     "Humidity outside local normal: %.1f pct", hum);
        }
    }

    // ── Gas (universal thresholds — not location-dependent) ──────────────────
    if (gas != -999.0f && gas > 0) {
        if (gas >= GAS_DANGER_PPM) {
            danger = true;
            snprintf(reasonBuf, sizeof(reasonBuf),
                     "Dangerous air quality: %.0f ppm", gas);
        } else if (!danger && gas >= GAS_WARN_PPM) {
            warning = true;
            snprintf(reasonBuf, sizeof(reasonBuf),
                     "Poor air quality: %.0f ppm", gas);
        }
    }

    // ── Drive LEDs ────────────────────────────────────────────────────────────
    AlertLevel prevLevel = _level;

    if (danger) {
        _level  = ALERT_DANGER;
        _reason = reasonBuf;
        setLeds(false, false, true);
    } else if (warning) {
        _level  = ALERT_WARNING;
        _reason = reasonBuf;
        setLeds(false, true, false);
    } else {
        _level  = ALERT_NONE;
        _reason = "All parameters normal";
        setLeds(true, false, false);
    }

    // Only log when level changes — avoids Serial spam every poll cycle
    if (_level != prevLevel) {
        Serial.printf("[Alert] Level changed to %d — %s\n", _level, _reason);
    }

    return _level;
}

AlertLevel  alertGetLevel()  { return _level;  }
const char* alertGetReason() { return _reason; }