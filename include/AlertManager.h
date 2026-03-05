#ifndef ALERT_MANAGER_H
#define ALERT_MANAGER_H

// LED pins — change these to match your wiring
#define LED_GREEN  25
#define LED_YELLOW 26
#define LED_RED    27

typedef enum {
    ALERT_NONE    = 0,  // All parameters within normal range → Green
    ALERT_WARNING = 1,  // One or more parameters anomalous   → Yellow
    ALERT_DANGER  = 2   // One or more parameters in danger   → Red
} AlertLevel;

// Climate-based thresholds — set via alertSetClimate() when user enters city.
// Until set, falls back to conservative universal defaults.
struct ClimateThresholds {
    // Temperature (°C)
    float tempWarnLo;    // below this → warning
    float tempWarnHi;    // above this → warning
    float tempDangerLo;  // below this → danger
    float tempDangerHi;  // above this → danger
    // Humidity (%)
    float humWarnLo;
    float humWarnHi;
    float humDangerLo;
    float humDangerHi;
};

// Call once in setup()
void alertBegin();

// Update thresholds based on location climate normals received from dashboard.
// meanTemp: monthly mean temperature for current month (°C)
// meanHum:  monthly mean relative humidity for current month (%)
void alertSetClimate(float meanTemp, float meanHum);

// Evaluate current readings and drive LEDs accordingly.
// Pass -999 for any sensor that is not connected — it is ignored.
AlertLevel alertUpdate(float temp, float hum, float gas);

// Returns the last computed alert level
AlertLevel alertGetLevel();

// Returns a human-readable description of what triggered the alert
const char* alertGetReason();

#endif // ALERT_MANAGER_H