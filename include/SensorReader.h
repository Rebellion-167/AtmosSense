#ifndef SENSOR_READER_H
#define SENSOR_READER_H

// Initialise the DHT sensor — call once in setup(), returns immediately
void sensorBegin();

// Call every loop() — handles non-blocking warmup. Returns true once ready.
bool sensorTick();

// Read temperature in °C; returns -999.0 if not ready or read failed
float readTemperature();

// Read relative humidity in %; returns -999.0 if not ready or read failed
float readHumidity();

// True once warmup is complete — stays true even if sensor is later unplugged
bool sensorWarmedUp();

#endif // SENSOR_READER_H