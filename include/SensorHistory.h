#ifndef SENSOR_HISTORY_H
#define SENSOR_HISTORY_H

#include <Arduino.h>

// ── Ring Buffer — 24 hours of readings at 5-minute intervals ──────────────────
// 288 slots × 3 floats × 4 bytes = 3456 bytes — fits comfortably in DRAM
#define HISTORY_SLOTS      288   // 24h @ 5min
#define HISTORY_INTERVAL_S 300   // 5 minutes

struct HistoryEntry {
    uint32_t timestamp;  // Unix epoch (0 = empty slot)
    float    temp;       // °C,  -999 = no reading
    float    hum;        // %,   -999 = no reading
    float    gas;        // ppm, -999 = no reading
};

// Call once after NTP is synced
void historyBegin();

// Call every loop() — internally throttled to HISTORY_INTERVAL_S
void historyTick(float temp, float hum, float gas);

// Write up to `count` most-recent entries (newest first) into `out`.
// Returns actual number of entries written (may be less than count if buffer not full).
int  historyGet(HistoryEntry* out, int count);

// Total valid entries currently stored (0–HISTORY_SLOTS)
int  historyCount();

// Serialise history to JSON into caller-supplied buffer.
// Format: {"history":[{"t":unix,"temp":22.1,"hum":55.0,"gas":450.0}, ...]}
// Returns number of bytes written (excluding null terminator).
// `maxBytes` should be at least historyJsonMaxBytes().
size_t historyToJson(char* buf, size_t maxBytes);

// Safe upper bound on JSON output size — use to allocate buffer
size_t historyJsonMaxBytes();

#endif // SENSOR_HISTORY_H