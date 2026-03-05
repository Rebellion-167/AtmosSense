#ifndef SENSOR_READER_H
#define SENSOR_READER_H

// Initialise the DHT sensor — blocks during warmup (~10s) to discard unstable readings
void sensorBegin();

// Read temperature in °C; returns -999.0 if sensor not ready or read failed
float readTemperature();

// Read relative humidity in %; returns -999.0 if sensor not ready or read failed
float readHumidity();

#endif // SENSOR_READER_H