#ifndef SENSOR_READER_H
#define SENSOR_READER_H

// Initialise the DHT sensor — call once in setup()
void sensorBegin();

// Read temperature in °C; returns 0.0 on sensor error
float readTemperature();

// Read relative humidity in %; returns 0.0 on sensor error
float readHumidity();

#endif // SENSOR_READER_H