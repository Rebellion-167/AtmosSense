#ifndef ALERT_MANAGER_H
#define ALERT_MANAGER_H

#define LED_GREEN  25
#define LED_YELLOW 26
#define LED_RED    27

// ── Fixed universal indoor comfort thresholds ────────────────────────────────
// Temperature (°C) — heat index based, raw fallback only
#define TEMP_SAFE_LO    20.0f
#define TEMP_SAFE_HI    28.0f
#define TEMP_WARN_LO    16.0f   // below this → danger
#define TEMP_WARN_HI    32.0f   // above this → danger
// Humidity (%)
#define HUM_SAFE_LO     40.0f
#define HUM_SAFE_HI     60.0f
#define HUM_WARN_LO     30.0f
#define HUM_WARN_HI     70.0f
// Gas ppm (CO2-equivalent)
#define GAS_SAFE_PPM     800.0f
#define GAS_DANGER_PPM  1200.0f
// Noise dB SPL
#define NOISE_SAFE_DB    70.0f
#define NOISE_WARN_DB    85.0f
#define NOISE_DANGER_DB  95.0f

typedef enum {
    ALERT_NONE    = 0,  // All sensors safe        → Green
    ALERT_WARNING = 1,  // Any sensor out of range → Yellow
    ALERT_DANGER  = 2   // All sensors in danger   → Red
} AlertLevel;

void        alertBegin();
AlertLevel  alertUpdate(float temp, float hum, float gas, float noise);
AlertLevel  alertGetLevel();
const char* alertGetReason();
int         alertGetTempState(); // 0=safe, 1=warning, 2=danger, -1=not connected
int         alertGetHumState();
int         alertGetGasState();
float       alertGetFeelsLike();    // Heat index feels-like temp in °C
const char* alertGetComfortLabel(); // "Comfortable", "Warm", "Hot" etc.
int alertGetNoiseState();

#endif // ALERT_MANAGER_H