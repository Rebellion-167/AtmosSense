#ifndef SENSOR_READER_H
#define SENSOR_READER_H

// Initialise sensors — call once in setup(), returns immediately
void sensorBegin();

// Call every loop() — handles non-blocking DHT warmup. Returns true once ready.
bool sensorTick();

// Read temperature in °C; returns -999.0 if not ready or read failed
float readTemperature();

// Read relative humidity in %; returns -999.0 if not ready or read failed
float readHumidity();

// Read MQ-135 gas concentration in ppm (CO2-equivalent)
// Returns -999.0 if sensor not connected or reading invalid
float readGas();

// True once DHT warmup is complete
bool sensorWarmedUp();

#endif // SENSOR_READER_H