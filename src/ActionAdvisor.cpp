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
        "Prolonged exposure can cause discomfort and reduce immunity.",
        1
    };
    if (strcmp(comfortLabel, "Warm") == 0) return {
        "Warm — Stay Hydrated",
        "Drink water regularly and ensure airflow.",
        "Heat and humidity combined increase fatigue and dehydration risk.",
        1
    };
    if (strcmp(comfortLabel, "Hot") == 0) return {
        "Heat Stress Risk",
        "Turn on AC or fan immediately. Move to a cooler area. Drink water.",
        "Conditions can cause heat exhaustion within hours.",
        2
    };
    if (strcmp(comfortLabel, "Very Hot") == 0) return {
        "Extreme Heat — Act Now",
        "Leave the room. Turn on AC. Cool down with cold water on skin.",
        "Heat stroke risk. This is a medical emergency if symptoms appear.",
        2
    };
    // Comfortable
    return {
        "Comfortable",
        "No action needed. Conditions are ideal.",
        "Temperature and humidity are within healthy indoor comfort range.",
        0
    };
}

RoomAdvice adviceForHumidity(float humidity) {
    if (humidity < 20.0f) return {
        "Critically Dry Air",
        "Use a humidifier immediately. Drink more water.",
        "Severe dryness damages respiratory tract and skin. Eyes may feel irritated.",
        2
    };
    if (humidity < 30.0f) return {
        "Dry Air",
        "Use a humidifier or place water containers near heat sources.",
        "Dry air irritates airways and increases static electricity.",
        1
    };
    if (humidity > 70.0f) return {
        "Critically Humid",
        "Open windows, run a dehumidifier or exhaust fan immediately.",
        "Mould growth accelerates above 70%. Risk of respiratory illness.",
        2
    };
    if (humidity > 60.0f) return {
        "High Humidity",
        "Improve ventilation. Open a window or run a fan.",
        "High humidity promotes mould, dust mites and discomfort.",
        1
    };
    return {
        "Humidity OK",
        "No action needed.",
        "Humidity is within the healthy indoor range of 30-60%.",
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
    if (ppm >= 2000.0f) return {
        "Dangerous Air Quality",
        "Open all windows and doors immediately. Identify the source.",
        "Levels this high cause headaches, dizziness and breathing difficulty.",
        2
    };
    if (ppm >= 1000.0f) return {
        "Poor Air Quality",
        "Open a window or turn on ventilation. Reduce indoor sources.",
        "Stuffy air reduces focus and causes fatigue over time.",
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
        "CO2 levels are within healthy range.",
        0
    };
}