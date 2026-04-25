#include "SensorHistory.h"
#include <Arduino.h>
#include <time.h>
#include <string.h>
#include <stdio.h>

// ── Ring buffer storage ───────────────────────────────────────────────────────
static HistoryEntry _buf[HISTORY_SLOTS];
static int          _head      = 0;    // next write index
static int          _count     = 0;    // valid entries (0–HISTORY_SLOTS)
static bool         _ntpReady  = false;
static unsigned long _lastTickMs = 0;

void historyBegin() {
    memset(_buf, 0, sizeof(_buf));
    // Mark all slots empty
    for (int i = 0; i < HISTORY_SLOTS; i++) {
        _buf[i].timestamp = 0;
        _buf[i].temp      = -999.0f;
        _buf[i].hum       = -999.0f;
        _buf[i].gas       = -999.0f;
    }
    _head     = 0;
    _count    = 0;
    _ntpReady = false;
    Serial.printf("[History] Ring buffer initialised (%d slots, %ds interval)\n",
                  HISTORY_SLOTS, HISTORY_INTERVAL_S);
}

void historyTick(float temp, float hum, float gas) {
    // Throttle: only record once per HISTORY_INTERVAL_S
    unsigned long now = millis();
    if (_lastTickMs != 0 && (now - _lastTickMs) < (unsigned long)(HISTORY_INTERVAL_S * 1000UL)) {
        return;
    }

    // Need a valid NTP timestamp
    time_t epoch = time(nullptr);
    if (epoch < 1000000000L) {
        // NTP not yet synced — skip but don't start the timer
        return;
    }

    _lastTickMs = now;
    _ntpReady   = true;

    HistoryEntry& slot = _buf[_head];
    slot.timestamp = (uint32_t)epoch;
    slot.temp      = temp;
    slot.hum       = hum;
    slot.gas       = gas;

    _head = (_head + 1) % HISTORY_SLOTS;
    if (_count < HISTORY_SLOTS) _count++;

    Serial.printf("[History] Recorded slot %d/%d — T:%.1f H:%.1f G:%.1f @ %lu\n",
                  _count, HISTORY_SLOTS, temp, hum, gas, (unsigned long)epoch);
}

int historyCount() { return _count; }

// Returns entries newest-first in `out` (up to `count`)
int historyGet(HistoryEntry* out, int count) {
    if (_count == 0 || count <= 0) return 0;
    int n = (count < _count) ? count : _count;

    // Most-recent entry is at (_head - 1) wrapping backwards
    for (int i = 0; i < n; i++) {
        int idx = (_head - 1 - i + HISTORY_SLOTS) % HISTORY_SLOTS;
        out[i] = _buf[idx];
    }
    return n;
}

size_t historyJsonMaxBytes() {
    // Each entry: {"t":1234567890,"temp":-999.0,"hum":-999.0,"gas":-999.0},
    // ~60 chars + wrapper ~20 chars
    return (size_t)HISTORY_SLOTS * 70 + 32;
}

size_t historyToJson(char* buf, size_t maxBytes) {
    if (!buf || maxBytes < 16) return 0;

    size_t pos = 0;
    pos += snprintf(buf + pos, maxBytes - pos, "{\"count\":%d,\"interval\":%d,\"history\":[",
                    _count, HISTORY_INTERVAL_S);

    bool first = true;
    // Emit oldest → newest so the chart can push them in order
    int start = (_count < HISTORY_SLOTS) ? 0 : _head; // oldest entry index
    for (int i = 0; i < _count && pos < maxBytes - 80; i++) {
        int idx = (start + i) % HISTORY_SLOTS;
        const HistoryEntry& e = _buf[idx];
        if (e.timestamp == 0) continue;

        if (!first) buf[pos++] = ',';
        first = false;

        int written = snprintf(buf + pos, maxBytes - pos,
            "{\"t\":%lu,\"temp\":%.1f,\"hum\":%.1f,\"gas\":%.1f}",
            (unsigned long)e.timestamp,
            e.temp, e.hum, e.gas);

        if (written > 0) pos += (size_t)written;
    }

    if (pos < maxBytes - 3) {
        buf[pos++] = ']';
        buf[pos++] = '}';
        buf[pos]   = '\0';
    }

    return pos;
}