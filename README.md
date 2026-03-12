# AtmosSense

Real-time indoor air quality monitor built on ESP32. Continuously monitors temperature, humidity, and air quality, raises multi-level alerts on an OLED display, RGB LEDs, and a self-hosted web dashboard — with actionable advice for every alert state.

No cloud. No app. No subscription. Open a browser on the same network and you have a full live dashboard.

---

## Hardware

ESP32 dev board paired with an SHT30 temperature and humidity sensor and an MQ-135 gas sensor for CO2-equivalent air quality. A 128×64 SSD1306 OLED provides a five-page physical interface navigated by a push button. Three LEDs give instant ambient status — green, yellow, red.

## Software

Firmware written in C++ on the Arduino framework using PlatformIO, structured into nine single-responsibility modules. The web dashboard is served directly from ESP32 SPIFFS — two tabs, Overview and Alerts, with live gauges, scrolling charts, active alert cards, and a timestamped event history.

## Alerts

Three-zone thresholds for each parameter — Normal, Warning, Danger. When danger is detected, the OLED flashes an inverted alert screen, the dashboard pins a red banner, and the browser fires a notification. Every interface reads from the same logic.

---

## What Makes AtmosSense Different

- **Actionable advice, not just numbers** — every alert state maps to a specific instruction, not just a colour or a value
- **Five simultaneous alert channels** — LED, OLED popup, dashboard panels, sticky danger banner, and browser notification, all firing from the same threshold logic
- **Fully self-hosted** — the ESP32 serves the entire dashboard from its own flash memory, no internet or external server required
- **Indoor Air Index (IAI)** — a custom air quality scale with breakpoints aligned to our alert thresholds, honestly labelled rather than misrepresented as the EPA AQI
- **Persistent room identity** — room name stored in NVS, survives reboots, shown across the OLED, dashboard, and browser tab
- **Timestamped alert history** — every warning, danger, and all-clear event is logged with a timestamp on the Alerts tab
- **Modular firmware** — nine independent modules mean adding a new sensor or feature touches nothing else

---

## Dependencies

```ini
lib_deps =
    adafruit/Adafruit SHT31 Library @ ^2.2.2
    adafruit/Adafruit SSD1306 @ ^2.5.9
    adafruit/Adafruit GFX Library @ ^1.11.9
```

---

## Team

**Team Name:** Team SENSEible

**Team Members:**
- Barnik Chakraborty
- Soumya Adhikari
- Ranit Pramanik
- Soumik Samanta
  
---

