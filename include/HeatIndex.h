#ifndef HEAT_INDEX_H
#define HEAT_INDEX_H

// ── Heat Index (Feels-Like) calculator ───────────────────────────────────────
// Uses NOAA Rothfusz regression for temp >= 27°C / RH >= 40%
// Falls back to Steadman simple formula for mild conditions
// All values in °C and % RH

// Thermal comfort states (ASHRAE 55 / ISO 7730 inspired)
typedef enum {
    HI_TOO_COLD   = -2,  // Feels-like < 10°C
    HI_COLD       = -1,  // Feels-like 10–18°C
    HI_COMFORTABLE = 0,  // Feels-like 18–26°C  ← target zone
    HI_WARM        = 1,  // Feels-like 26–32°C  (caution)
    HI_HOT         = 2,  // Feels-like 32–41°C  (danger — heat stress risk)
    HI_VERY_HOT    = 3   // Feels-like > 41°C   (extreme danger)
} HeatIndexZone;

// Compute feels-like temperature from dry-bulb temp and relative humidity
float   heatIndexCalc(float tempC, float rhPct);

// Classify feels-like temperature into a comfort zone
HeatIndexZone heatIndexZone(float feelsLike);

// Human-readable label for each zone
const char* heatIndexZoneLabel(HeatIndexZone zone);

// Alert state derived from heat index zone:
// Returns 0=safe, 1=warning, 2=danger
// -2 (too cold) and -1 (cold) map to warning/danger on the cold side
int heatIndexAlertState(HeatIndexZone zone);

#endif // HEAT_INDEX_H