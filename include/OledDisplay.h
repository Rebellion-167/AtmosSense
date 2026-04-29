#ifndef OLED_DISPLAY_H
#define OLED_DISPLAY_H

#include <Arduino.h>

// ── I2C pins ──────────────────────────────────────────────────────────────────
#ifndef OLED_SDA
#define OLED_SDA 21
#endif
#ifndef OLED_SCL
#define OLED_SCL 22
#endif
#ifndef OLED_ADDR
#define OLED_ADDR 0x3C
#endif

// ── Page button ───────────────────────────────────────────────────────────────
// External push button — wire between GPIO19 and GND (uses internal pull-up)
#define OLED_BTN_PIN     19
#define BTN_DEBOUNCE_MS  50

// ── Pages ─────────────────────────────────────────────────────────────────────
// 0 — Overview   : room name + live temp / hum / AQI
// 1 — Temperature: current, comfort label, min/max
// 2 — Humidity   : current, ideal range, min/max
// 3 — Air Quality: ppm, AQI, status, min/max
// 4 - Noise Level
// 5 — System Info: room name, IP address, uptime, AtmosSense
#define OLED_PAGES 6

// Call once in setup()
void oledBegin();

// Update sensor data cache and redraw current page — call every poll cycle
void oledSetData(const char* room,
                 float temp,      float hum,          float gas,   float noise,
                 float feelsLike, const char* comfortLabel,
                 int   aqi,
                 float minTemp,   float maxTemp,
                 float minHum,    float maxHum,
                 float minGas,    float maxGas,
                 float minNoise,  float maxNoise,
                 int   tempState, int humState, int gasState, int noiseState);

// Update system info cache (IP, uptime) — call once after WiFi connects,
// then periodically (e.g. every minute) to keep uptime fresh
void oledSetSystem(const char* ip, unsigned long uptimeSeconds);

// Call every loop() — handles button press and page advance
void oledTick();

// Show a temporary status message (used during boot sequence)
void oledStatus(const char* line1, const char* line2 = "");

#endif // OLED_DISPLAY_H