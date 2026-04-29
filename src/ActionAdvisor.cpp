#include "ActionAdvisor.h"
#include <string.h>

RoomAdvice adviceForTemp(const char* comfortLabel) {
    if (strcmp(comfortLabel, "Too Cold") == 0) return {
        "Dangerously Cold",
        "Add layers, use a heater, avoid prolonged exposure.",
        "Risk of hypothermia. Core body temperature may drop.",
        2
    };
    if (strcmp(comfortLabel, "Cold") == 0) return {
        "Cold Room",
        "Wear warm clothing or turn on a heater.",
        "Prolonged exposure below 20°C can cause discomfort and reduce immunity.",
        1
    };
    if (strcmp(comfortLabel, "Warm") == 0) return {
        "Warm — Stay Hydrated",
        "Drink water regularly and ensure airflow.",
        "Feels-like temperature is approaching the alert threshold.",
        1
    };
    if (strcmp(comfortLabel, "Hot") == 0) return {
        "Heat Stress Risk",
        "Turn on AC or fan immediately. Move to a cooler area. Drink water.",
        "Conditions above 32°C can cause heat exhaustion within hours.",
        2
    };
    if (strcmp(comfortLabel, "Very Hot") == 0) return {
        "Extreme Heat — Act Now",
        "Leave the room. Turn on AC. Cool down with cold water on skin.",
        "Heat stroke risk. This is a medical emergency if symptoms appear.",
        2
    };
    return {
        "Comfortable",
        "No action needed. Conditions are ideal.",
        "Temperature is within the healthy indoor range of 20–28°C.",
        0
    };
}

RoomAdvice adviceForHumidity(float humidity) {
    if (humidity < 0.0f) return {
        "Sensor Offline",
        "Check DHT sensor connection.",
        "Unable to monitor humidity.",
        0
    };
    if (humidity < 20.0f) return {
        "Critically Dry Air",
        "Use a humidifier immediately. Drink more water.",
        "Severe dryness damages respiratory tract and skin.",
        2
    };
    if (humidity < 30.0f) return {
        "Dry Air",
        "Use a humidifier or place water containers near heat sources.",
        "Below 30% irritates airways and increases static electricity.",
        1
    };
    if (humidity > 70.0f) return {
        "Critically Humid",
        "Open windows, run a dehumidifier or exhaust fan immediately.",
        "Above 70% accelerates mould growth and risks respiratory illness.",
        2
    };
    if (humidity > 60.0f) return {
        "High Humidity",
        "Improve ventilation. Open a window or run a fan.",
        "Above 60% promotes mould and dust mites.",
        1
    };
    return {
        "Humidity OK",
        "No action needed.",
        "Humidity is within the healthy indoor range of 40–60%.",
        0
    };
}

RoomAdvice adviceForGas(float ppm) {
    if (ppm < 0.0f) return {
        "Sensor Offline",
        "Check gas sensor connection.",
        "Unable to monitor air quality.",
        0
    };
    if (ppm >= 5000.0f) return {
        "Evacuate Immediately",
        "Leave the room now. Open all doors and windows. Do not re-enter.",
        "Extremely dangerous air quality. Risk of unconsciousness.",
        2
    };
    if (ppm >= 1200.0f) return {
        "Dangerous Air Quality",
        "Open all windows and doors immediately. Identify the source.",
        "Above 1200 ppm causes headaches, dizziness and breathing difficulty.",
        2
    };
    if (ppm >= 800.0f) return {
        "Poor Air Quality",
        "Open a window or turn on ventilation. Reduce indoor sources.",
        "Above 800 ppm reduces focus and causes fatigue over time.",
        1
    };
    if (ppm >= 600.0f) return {
        "Air Quality Declining",
        "Consider opening a window soon.",
        "Slightly elevated CO2. Common in closed rooms with multiple people.",
        1
    };
    return {
        "Air Quality Good",
        "No action needed.",
        "CO2 levels are within the healthy range below 800 ppm.",
        0
    };
}

RoomAdvice adviceForNoise(float db) {
    if (db < 0.0f) return {
        "Sensor Offline", "Check INMP441 connection.", "Unable to monitor noise.", 0
    };
    if (db >= 95.0f) return {
        "Dangerous Noise",
        "Leave the area or use ear protection immediately.",
        "Above 95 dB causes permanent hearing damage within minutes.",
        2
    };
    if (db >= 85.0f) return {
        "High Noise Level",
        "Reduce noise sources or limit exposure time.",
        "Prolonged exposure above 85 dB risks hearing damage.",
        1
    };
    if (db >= 70.0f) return {
        "Elevated Noise",
        "Consider reducing background noise.",
        "Above 70 dB is distracting and tiring over time.",
        1
    };
    return {
        "Noise OK", "No action needed.", "Noise level is within comfortable range.", 0
    };
}