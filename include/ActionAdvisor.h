#ifndef ACTION_ADVISOR_H
#define ACTION_ADVISOR_H

// ── ActionAdvisor ─────────────────────────────────────────────────────────────
// Generates human-readable, actionable advice based on current sensor readings.
// Advice is specific to the parameter and severity — not generic warnings.

struct RoomAdvice {
    const char* title;      // Short alert title  e.g. "Air Quality危险"
    const char* action;     // What to do RIGHT NOW
    const char* reason;     // Why it matters (brief)
    int         urgency;    // 0=info, 1=warning, 2=danger
};

// Generate advice for temperature based on heat index zone label
RoomAdvice adviceForTemp(const char* comfortLabel);

// Generate advice for humidity based on current RH%
RoomAdvice adviceForHumidity(float humidity);

// Generate advice for gas/air quality based on ppm
RoomAdvice adviceForGas(float ppm);

#endif // ACTION_ADVISOR_H