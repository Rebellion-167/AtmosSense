#include "OledDisplay.h"
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <math.h>

#define SCREEN_WIDTH  128
#define SCREEN_HEIGHT  64
#define OLED_RESET     -1

// Font metrics (Adafruit default font)
// Size 1: 6px wide × 8px tall per char
// Size 2: 12px wide × 16px tall per char

static Adafruit_SSD1306 _display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);
static bool _ready = false;

// ── Layout constants (pixel-perfect, no overlap) ──────────────────────────────
//  Y=0..8   City header    (size 1, 8px tall)
//  Y=9      Divider line
//  Y=11..18 "INDOOR"       (size 1, 8px tall)
//  Y=19     Divider line
//  Y=21..36 Temp row       (size 2, 16px tall)
//  Y=38     Divider line
//  Y=40..55 Humidity row   (size 2, 16px tall)

static void drawLayout(const char* city, const char* tempBuf, const char* humBuf) {
    _display.clearDisplay();
    _display.setTextColor(SSD1306_WHITE);

    // City header — size 1, y=0
    _display.setTextSize(1);
    _display.setCursor(0, 0);
    _display.print(city);

    // Divider at y=9
    _display.drawFastHLine(0, 9, SCREEN_WIDTH, SSD1306_WHITE);

    // "INDOOR" label — size 1, y=11
    _display.setTextSize(1);
    _display.setCursor(0, 11);
    _display.print("INDOOR");

    // Divider at y=20
    _display.drawFastHLine(0, 20, SCREEN_WIDTH, SSD1306_WHITE);

    // Temp label — size 1, y=22
    _display.setTextSize(1);
    _display.setCursor(0, 22);
    _display.print("Temp");

    // Temp value — size 2 (16px tall), y=22 aligns baseline with label
    _display.setTextSize(2);
    int16_t tx = SCREEN_WIDTH - (strlen(tempBuf) * 12);
    _display.setCursor(tx, 22);
    _display.print(tempBuf);

    // Divider at y=40
    _display.drawFastHLine(0, 40, SCREEN_WIDTH, SSD1306_WHITE);

    // Humidity label — size 1, y=42
    _display.setTextSize(1);
    _display.setCursor(0, 42);
    _display.print("Humidity");

    // Humidity value — size 2 (16px tall), y=42
    _display.setTextSize(2);
    int16_t hx = SCREEN_WIDTH - (strlen(humBuf) * 12);
    _display.setCursor(hx, 42);
    _display.print(humBuf);

    _display.display();
}

// ── Public API ────────────────────────────────────────────────────────────────
void oledBegin() {
    Wire.begin(OLED_SDA, OLED_SCL);

    if (!_display.begin(SSD1306_SWITCHCAPVCC, OLED_ADDR)) {
        Serial.println("[OLED] Display not found — check wiring and I2C address.");
        return;
    }

    _ready = true;
    _display.clearDisplay();
    _display.setTextColor(SSD1306_WHITE);
    _display.setTextSize(1);

    // Splash — centred
    _display.setCursor(22, 24);
    _display.print("Room  Sensor");
    _display.setCursor(22, 38);
    _display.print("Starting up...");
    _display.display();
}

void oledUpdate(const char* city, float temp, float humidity) {
    if (!_ready) return;

    // City — truncate to 21 chars max (21 × 6px = 126px)
    char cityBuf[22];
    strncpy(cityBuf, (city && strlen(city) > 0) ? city : "Unknown", sizeof(cityBuf) - 1);
    cityBuf[sizeof(cityBuf) - 1] = '\0';

    char tempBuf[10];
    if (!isnan(temp)) snprintf(tempBuf, sizeof(tempBuf), "%.1fC", temp);
    else              snprintf(tempBuf, sizeof(tempBuf), "--C");

    char humBuf[8];
    if (!isnan(humidity)) snprintf(humBuf, sizeof(humBuf), "%.0f%%", humidity);
    else                  snprintf(humBuf, sizeof(humBuf), "--%");

    drawLayout(cityBuf, tempBuf, humBuf);
}

void oledStatus(const char* line1, const char* line2) {
    if (!_ready) return;
    _display.clearDisplay();
    _display.setTextColor(SSD1306_WHITE);
    _display.setTextSize(1);
    _display.setCursor(0, 20);
    _display.print(line1);
    if (line2 && strlen(line2) > 0) {
        _display.setCursor(0, 34);
        _display.print(line2);
    }
    _display.display();
}