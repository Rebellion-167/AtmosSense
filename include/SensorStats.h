#ifndef SENSOR_STATS_H
#define SENSOR_STATS_H

// Call once in setup() after NTP is synced
void statsBegin();

// Call on every new sensor reading to update min/max
void statsUpdate(float temp, float hum);

// Getters
float statsMinTemp();
float statsMaxTemp();
float statsMinHum();
float statsMaxHum();

// Call in loop() — checks if midnight has passed and resets if so
void statsCheckMidnightReset();

#endif // SENSOR_STATS_H