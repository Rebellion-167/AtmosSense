#ifndef OLED_DISPLAY_H
#define OLED_DISPLAY_H

#include <Arduino.h>

// ── I2C pins (change to match your wiring) ────────────────────────────────────
#ifndef OLED_SDA
#define OLED_SDA 21
#endif
#ifndef OLED_SCL
#define OLED_SCL 22
#endif

// I2C address — 0x3C is standard; some modules use 0x3D
#ifndef OLED_ADDR
#define OLED_ADDR 0x3C
#endif

// Call once in setup() — initialises display and shows splash screen
void oledBegin();

// Update display with city name and current outside weather.
// Pass NAN for temp/humidity if values are not yet available.
void oledUpdate(const char* city, float temp, float humidity);

// Show a temporary status message (used during boot)
void oledStatus(const char* line1, const char* line2 = "");

#endif // OLED_DISPLAY_H