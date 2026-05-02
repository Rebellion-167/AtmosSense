#include "AlertManager.h"
#include <Arduino.h>

// Store feels-like for external access
static float _feelsLike = -999.0f;
static const char *_comfortLabel = "Unknown";

static AlertLevel _level = ALERT_NONE;
static const char *_reason = "All parameters normal";
static int _tempState = -1;
static int _humState = -1;
static int _gasState = -1;
static int _noiseState = -1;

static void setLeds(bool green, bool yellow, bool red)
{
    digitalWrite(LED_GREEN, green ? HIGH : LOW);
    digitalWrite(LED_YELLOW, yellow ? HIGH : LOW);
    digitalWrite(LED_RED, red ? HIGH : LOW);
}

void alertBegin()
{
    pinMode(LED_GREEN, OUTPUT);
    pinMode(LED_YELLOW, OUTPUT);
    pinMode(LED_RED, OUTPUT);
    setLeds(false, false, false);
}

AlertLevel alertUpdate(float temp, float hum, float gas, float noise = -999.0f)
{
    int tempState = -1, humState = -1, gasState = -1;

    // Temperature — direct raw thresholds, no heat index
    if (temp != -999.0f)
    {
        if (temp < TEMP_WARN_LO || temp > TEMP_WARN_HI)
            tempState = 2;
        else if (temp < TEMP_SAFE_LO || temp > TEMP_SAFE_HI)
            tempState = 1;
        else
            tempState = 0;

        // Comfort label based on raw temp
        if (temp < 10.0f)
            _comfortLabel = "Too Cold";
        else if (temp < 20.0f)
            _comfortLabel = "Cold";
        else if (temp < 28.0f)
            _comfortLabel = "Comfortable";
        else if (temp < 32.0f)
            _comfortLabel = "Warm";
        else if (temp < 41.0f)
            _comfortLabel = "Hot";
        else
            _comfortLabel = "Very Hot";

        _feelsLike = temp; // no adjustment indoors
        Serial.printf("[Alert] Temp=%.1fC State=%d (%s)\n", temp, tempState, _comfortLabel);
    }

    // Humidity
    if (hum != -999.0f)
    {
        if (hum < HUM_WARN_LO || hum > HUM_WARN_HI)
            humState = 2;
        else if (hum < HUM_SAFE_LO || hum > HUM_SAFE_HI)
            humState = 1;
        else
            humState = 0;
    }

    // Gas
    if (gas != -999.0f && gas > 0)
    {
        if (gas >= GAS_DANGER_PPM)
            gasState = 2;
        else if (gas >= GAS_SAFE_PPM)
            gasState = 1;
        else
            gasState = 0;
    }

    int noiseState = -1;
    if (noise != -999.0f && noise > 0)
    {
        if (noise >= NOISE_DANGER_DB)
            noiseState = 2;
        else if (noise >= NOISE_WARN_DB)
            noiseState = 1;
        else
            noiseState = 0;
    }

    _noiseState = noiseState;
    _tempState = tempState;
    _humState = humState;
    _gasState = gasState;

    // Count connected sensors
    int connected = 0, safe = 0, danger = 0;
    int states[4] = {tempState, humState, gasState, noiseState};
    const char *names[4] = {"Temperature", "Humidity", "Air quality", "Noise"};

    for (int i = 0; i < 4; i++)
    {
        if (states[i] == -1)
            continue;
        connected++;
        if (states[i] == 0)
            safe++;
        if (states[i] == 2)
            danger++;
    }

    static char reasonBuf[80];
    AlertLevel prevLevel = _level;

    if (connected == 0 || safe == connected)
    {
        _level = ALERT_NONE;
        _reason = "All parameters normal";
        setLeds(true, false, false);
    }
    else if (danger == connected)
    {
        _level = ALERT_DANGER;
        char parts[60] = "";
        for (int i = 0; i < 3; i++)
        {
            if (states[i] == 2)
            {
                if (strlen(parts))
                    strncat(parts, ", ", sizeof(parts) - strlen(parts) - 1);
                strncat(parts, names[i], sizeof(parts) - strlen(parts) - 1);
            }
        }
        snprintf(reasonBuf, sizeof(reasonBuf), "Danger: %s", parts);
        _reason = reasonBuf;
        setLeds(false, false, true);
    }
    else
    {
        _level = ALERT_WARNING;
        char parts[60] = "";
        for (int i = 0; i < 3; i++)
        {
            if (states[i] >= 1)
            {
                if (strlen(parts))
                    strncat(parts, ", ", sizeof(parts) - strlen(parts) - 1);
                strncat(parts, names[i], sizeof(parts) - strlen(parts) - 1);
            }
        }
        snprintf(reasonBuf, sizeof(reasonBuf), "Out of range: %s", parts);
        _reason = reasonBuf;
        setLeds(false, true, false);
    }

    if (_level != prevLevel)
    {
        Serial.printf("[Alert] Level -> %d : %s\n", _level, _reason);
    }

    return _level;
}

AlertLevel alertGetLevel() { return _level; }
const char *alertGetReason() { return _reason; }
int alertGetTempState() { return _tempState; }
int alertGetHumState() { return _humState; }
int alertGetGasState() { return _gasState; }
float alertGetFeelsLike() { return _feelsLike; }
const char *alertGetComfortLabel() { return _comfortLabel; }
int alertGetNoiseState() { return _noiseState; }