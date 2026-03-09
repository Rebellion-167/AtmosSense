#include "OledDisplay.h"
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <math.h>
#include <string.h>
#include <stdio.h>

#define SCREEN_W  128
#define SCREEN_H   64

static Adafruit_SSD1306 _disp(SCREEN_W, SCREEN_H, &Wire, -1);
static bool _ready = false;

// ── Data cache ────────────────────────────────────────────────────────────────
static char  _room[32]    = "Room";
static float _temp        = NAN;
static float _hum         = NAN;
static float _gas         = -999.f;
static float _feelsLike   = NAN;
static char  _comfort[20] = "--";
static int   _aqi         = 0;
static float _minTemp     = NAN,    _maxTemp = NAN;
static float _minHum      = NAN,    _maxHum  = NAN;
static float _minGas      = -999.f, _maxGas  = -999.f;
static int   _tempState   = -1;
static int   _humState    = -1;
static int   _gasState    = -1;

// ── Page + button state ───────────────────────────────────────────────────────
static uint8_t       _page       = 0;
static bool          _inStatus   = false;
static bool          _lastBtn    = HIGH;
static unsigned long _debounceMs = 0;

// ── Draw helpers ──────────────────────────────────────────────────────────────
static void hline(int y) {
    _disp.drawFastHLine(0, y, SCREEN_W, SSD1306_WHITE);
}

static void printRight(const char* s, int y, uint8_t size) {
    int charW = (size == 2) ? 12 : 6;
    int x = SCREEN_W - (int)strlen(s) * charW;
    if (x < 0) x = 0;
    _disp.setTextSize(size);
    _disp.setCursor(x, y);
    _disp.print(s);
}

static void drawDots() {
    for (int i = 0; i < OLED_PAGES; i++) {
        int cx = SCREEN_W - 4 - (OLED_PAGES - 1 - i) * 7;
        if (i == _page) _disp.fillCircle(cx, 4, 2, SSD1306_WHITE);
        else            _disp.drawCircle(cx, 4, 2, SSD1306_WHITE);
    }
}

static const char* stateStr(int state) {
    switch (state) {
        case  0: return "Normal";
        case  1: return "Warning";
        case  2: return "DANGER";
        default: return "No sensor";
    }
}

// ── Page 0 — Overview ─────────────────────────────────────────────────────────
// Room name header + all three live readings
static void drawPageOverview() {
    char buf[16];

    _disp.setTextSize(1);
    char room[18]; strncpy(room, _room, 17); room[17] = '\0';
    _disp.setCursor(0, 0);
    _disp.print(room);
    drawDots();
    hline(10);

    // Temp
    _disp.setTextSize(1);
    _disp.setCursor(0, 14);
    _disp.print("Temp");
    if (!isnan(_temp)) snprintf(buf, sizeof(buf), "%.1fC", _temp);
    else               snprintf(buf, sizeof(buf), "--");
    printRight(buf, 13, 2);
    hline(32);

    // Humidity
    _disp.setTextSize(1);
    _disp.setCursor(0, 35);
    _disp.print("Humidity");
    if (!isnan(_hum)) snprintf(buf, sizeof(buf), "%.0f%%", _hum);
    else              snprintf(buf, sizeof(buf), "--");
    printRight(buf, 34, 2);
    hline(53);

    // AQI
    _disp.setTextSize(1);
    _disp.setCursor(0, 56);
    _disp.print("Air Quality");
    if (_gas > 0) snprintf(buf, sizeof(buf), "AQI %d", _aqi);
    else          snprintf(buf, sizeof(buf), "No sensor");
    printRight(buf, 56, 1);
}

// ── Page 1 — Temperature detail ───────────────────────────────────────────────
static void drawPageTemp() {
    char buf[20];

    _disp.setTextSize(1);
    _disp.setCursor(0, 0);
    _disp.print("TEMPERATURE");
    drawDots();
    hline(10);

    // Current reading — large
    if (!isnan(_temp)) snprintf(buf, sizeof(buf), "%.1fC", _temp);
    else               snprintf(buf, sizeof(buf), "--");
    _disp.setTextSize(2);
    _disp.setCursor(0, 13);
    _disp.print(buf);

    // Status right-aligned next to reading
    _disp.setTextSize(1);
    printRight(stateStr(_tempState), 20, 1);

    hline(32);

    // Feels like
    _disp.setTextSize(1);
    _disp.setCursor(0, 35);
    _disp.print("Feels like");
    if (!isnan(_feelsLike)) snprintf(buf, sizeof(buf), "%.1fC", _feelsLike);
    else                    snprintf(buf, sizeof(buf), "--");
    printRight(buf, 35, 1);

    // Comfort label
    _disp.setCursor(0, 45);
    _disp.print(_comfort);

    hline(54);

    // Min / Max
    _disp.setCursor(0, 56);
    if (!isnan(_minTemp)) snprintf(buf, sizeof(buf), "Lo:%.1fC", _minTemp);
    else                  snprintf(buf, sizeof(buf), "Lo:--");
    _disp.print(buf);
    if (!isnan(_maxTemp)) snprintf(buf, sizeof(buf), "Hi:%.1fC", _maxTemp);
    else                  snprintf(buf, sizeof(buf), "Hi:--");
    printRight(buf, 56, 1);
}

// ── Page 2 — Humidity detail ──────────────────────────────────────────────────
static void drawPageHum() {
    char buf[20];

    _disp.setTextSize(1);
    _disp.setCursor(0, 0);
    _disp.print("HUMIDITY");
    drawDots();
    hline(10);

    // Current reading — large
    if (!isnan(_hum)) snprintf(buf, sizeof(buf), "%.1f%%", _hum);
    else              snprintf(buf, sizeof(buf), "--");
    _disp.setTextSize(2);
    _disp.setCursor(0, 13);
    _disp.print(buf);

    // Status right-aligned
    _disp.setTextSize(1);
    printRight(stateStr(_humState), 20, 1);

    hline(32);

    // Ideal range reminder
    _disp.setTextSize(1);
    _disp.setCursor(0, 35);
    _disp.print("Ideal: 30-60%");

    hline(44);

    // Min / Max
    _disp.setCursor(0, 47);
    if (!isnan(_minHum)) snprintf(buf, sizeof(buf), "Lo: %.1f%%", _minHum);
    else                 snprintf(buf, sizeof(buf), "Lo: --");
    _disp.print(buf);

    _disp.setCursor(0, 56);
    if (!isnan(_maxHum)) snprintf(buf, sizeof(buf), "Hi: %.1f%%", _maxHum);
    else                 snprintf(buf, sizeof(buf), "Hi: --");
    _disp.print(buf);
}

// ── Page 3 — Air Quality detail ───────────────────────────────────────────────
static void drawPageGas() {
    char buf[20];

    _disp.setTextSize(1);
    _disp.setCursor(0, 0);
    _disp.print("AIR QUALITY");
    drawDots();
    hline(10);

    if (_gas <= 0) {
        _disp.setTextSize(1);
        _disp.setCursor(0, 28);
        _disp.print("Sensor not");
        _disp.setCursor(0, 40);
        _disp.print("connected");
        return;
    }

    // Current PPM — large
    snprintf(buf, sizeof(buf), "%.0fppm", _gas);
    _disp.setTextSize(2);
    _disp.setCursor(0, 13);
    _disp.print(buf);

    // Status right-aligned
    _disp.setTextSize(1);
    printRight(stateStr(_gasState), 20, 1);

    hline(32);

    // AQI
    _disp.setTextSize(1);
    _disp.setCursor(0, 35);
    _disp.print("Indoor AQI");
    snprintf(buf, sizeof(buf), "%d", _aqi);
    printRight(buf, 35, 1);

    hline(44);

    // Min / Max
    _disp.setCursor(0, 47);
    if (_minGas > 0) snprintf(buf, sizeof(buf), "Lo: %.0fppm", _minGas);
    else             snprintf(buf, sizeof(buf), "Lo: --");
    _disp.print(buf);

    _disp.setCursor(0, 56);
    if (_maxGas > 0) snprintf(buf, sizeof(buf), "Hi: %.0fppm", _maxGas);
    else             snprintf(buf, sizeof(buf), "Hi: --");
    _disp.print(buf);
}

// ── Redraw current page ───────────────────────────────────────────────────────
static void redraw() {
    _disp.clearDisplay();
    _disp.setTextColor(SSD1306_WHITE);
    switch (_page) {
        case 0: drawPageOverview(); break;
        case 1: drawPageTemp();     break;
        case 2: drawPageHum();      break;
        case 3: drawPageGas();      break;
    }
    _disp.display();
}

// ── Public API ────────────────────────────────────────────────────────────────
void oledBegin() {
    Wire.begin(OLED_SDA, OLED_SCL);
    if (!_disp.begin(SSD1306_SWITCHCAPVCC, OLED_ADDR)) {
        Serial.println("[OLED] Not found — check wiring/address.");
        return;
    }
    _ready = true;

    pinMode(OLED_BTN_PIN, INPUT_PULLUP);

    _disp.clearDisplay();
    _disp.setTextColor(SSD1306_WHITE);
    _disp.setTextSize(1);
    _disp.setCursor(28, 20);
    _disp.print("AtmosSense");
    _disp.setCursor(22, 34);
    _disp.print("Starting up...");
    _disp.display();
    Serial.printf("[OLED] Ready. Page button on GPIO%d\n", OLED_BTN_PIN);
}

void oledSetData(const char* room,
                 float temp,     float hum,     float gas,
                 float feelsLike, const char* comfortLabel,
                 int   aqi,
                 float minTemp,  float maxTemp,
                 float minHum,   float maxHum,
                 float minGas,   float maxGas,
                 int   tempState, int humState, int gasState)
{
    strncpy(_room,    room ? room : "Room",               sizeof(_room)    - 1);
    strncpy(_comfort, comfortLabel ? comfortLabel : "--",  sizeof(_comfort) - 1);
    _temp = temp;  _hum = hum;  _gas = gas;
    _feelsLike = feelsLike;
    _aqi = aqi;
    _minTemp = minTemp; _maxTemp = maxTemp;
    _minHum  = minHum;  _maxHum  = maxHum;
    _minGas  = minGas;  _maxGas  = maxGas;
    _tempState = tempState; _humState = humState; _gasState = gasState;

    if (_ready) {
        _inStatus = false;  // real data has arrived — leave boot messages behind
        redraw();
    }
}

void oledTick() {
    if (!_ready) return;

    bool btn = digitalRead(OLED_BTN_PIN);

    // Falling edge — button pressed (active LOW with pull-up)
    if (_lastBtn == HIGH && btn == LOW) {
        _debounceMs = millis();
    }

    // Confirm press after debounce window
    if (_lastBtn == LOW && btn == HIGH) {
        if (millis() - _debounceMs >= BTN_DEBOUNCE_MS) {
            _inStatus = false;
            _page = (_page + 1) % OLED_PAGES;
            redraw();
            Serial.printf("[OLED] Page -> %d\n", _page);
        }
    }

    _lastBtn = btn;
}

void oledStatus(const char* line1, const char* line2) {
    if (!_ready) return;
    _inStatus = true;
    _disp.clearDisplay();
    _disp.setTextColor(SSD1306_WHITE);
    _disp.setTextSize(1);
    _disp.setCursor(0, 20);
    _disp.print(line1);
    if (line2 && strlen(line2) > 0) {
        _disp.setCursor(0, 34);
        _disp.print(line2);
    }
    _disp.display();
}