#include "HeatIndex.h"
#include <math.h>

// ── NOAA Rothfusz regression coefficients ─────────────────────────────────────
// Valid for T >= 27°C and RH >= 40%
// HI = c1 + c2*T + c3*R + c4*T*R + c5*T² + c6*R² + c7*T²*R + c8*T*R² + c9*T²*R²
static const float C1 = -8.78469475556f;
static const float C2 =  1.61139411f;
static const float C3 =  2.33854883889f;
static const float C4 = -0.14611605f;
static const float C5 = -0.012308094f;
static const float C6 = -0.0164248277778f;
static const float C7 =  0.002211732f;
static const float C8 =  0.00072546f;
static const float C9 = -0.000003582f;

float heatIndexCalc(float T, float R) {
    if (T < -10.0f || T > 60.0f || R < 0.0f || R > 100.0f) return T;

    // Simple Steadman formula for mild conditions
    if (T < 27.0f || R < 40.0f) {
        // Steadman: HI ≈ T + 0.33*e - 0.70*ws - 4.00
        // Simplified for indoor (no wind): HI = T + 0.33*(RH/100 * 6.105 * exp(17.27*T/(237.3+T))) - 4.0
        float e = (R / 100.0f) * 6.105f * expf(17.27f * T / (237.3f + T));
        return T + 0.33f * e - 4.0f;
    }

    // Rothfusz full regression for hot-humid conditions
    float HI = C1
             + C2 * T
             + C3 * R
             + C4 * T * R
             + C5 * T * T
             + C6 * R * R
             + C7 * T * T * R
             + C8 * T * R * R
             + C9 * T * T * R * R;

    // Rothfusz adjustment for very dry hot air (RH < 13% and T 27–44°C)
    if (R < 13.0f && T >= 27.0f && T <= 44.0f) {
        HI -= ((13.0f - R) / 4.0f) * sqrtf((17.0f - fabsf(T - 35.0f)) / 17.0f);
    }
    // Adjustment for very high humidity at moderate temp
    else if (R > 85.0f && T >= 27.0f && T <= 30.0f) {
        HI += ((R - 85.0f) / 10.0f) * ((30.0f - T) / 5.0f);
    }

    return HI;
}

HeatIndexZone heatIndexZone(float feelsLike) {
    if (feelsLike < 10.0f)  return HI_TOO_COLD;
    if (feelsLike < 20.0f)  return HI_COLD;
    if (feelsLike < 28.0f)  return HI_COMFORTABLE;
    if (feelsLike < 32.0f)  return HI_WARM;
    if (feelsLike < 41.0f)  return HI_HOT;
    return                         HI_VERY_HOT;
}

const char* heatIndexZoneLabel(HeatIndexZone zone) {
    switch (zone) {
        case HI_TOO_COLD:    return "Too Cold";
        case HI_COLD:        return "Cold";
        case HI_COMFORTABLE: return "Comfortable";
        case HI_WARM:        return "Warm";
        case HI_HOT:         return "Hot";
        case HI_VERY_HOT:    return "Very Hot";
        default:             return "Unknown";
    }
}

int heatIndexAlertState(HeatIndexZone zone) {
    switch (zone) {
        case HI_COMFORTABLE: return 0; // safe
        case HI_COLD:        return 1; // warning
        case HI_WARM:        return 1; // warning
        case HI_TOO_COLD:    return 2; // danger
        case HI_HOT:         return 2; // danger
        case HI_VERY_HOT:    return 2; // danger
        default:             return 1;
    }
}