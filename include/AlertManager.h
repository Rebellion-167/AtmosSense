#ifndef ALERT_MANAGER_H
#define ALERT_MANAGER_H

#define LED_GREEN  25
#define LED_YELLOW 26
#define LED_RED    27

typedef enum {
    ALERT_NONE    = 0,  // All sensors safe        → Green
    ALERT_WARNING = 1,  // Any sensor out of range → Yellow
    ALERT_DANGER  = 2   // All sensors in danger   → Red
} AlertLevel;

struct ClimateThresholds {
    float tempSafeLo, tempSafeHi;     // within this → safe
    float tempDangerLo, tempDangerHi; // outside this → danger
};

void alertBegin();

// Call with 30-day mean temperature for the user's location.
// Humidity and gas use fixed universal indoor thresholds.
void alertSetClimate(float meanTemp, float meanHum);

// Evaluate readings and drive LEDs. Pass -999 for unconnected sensors.
AlertLevel alertUpdate(float temp, float hum, float gas);

AlertLevel  alertGetLevel();
const char* alertGetReason();
int         alertGetTempState(); // 0=safe, 1=warning, 2=danger, -1=not connected
int         alertGetHumState();
int         alertGetGasState();

#endif // ALERT_MANAGER_H