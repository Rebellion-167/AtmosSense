#ifndef SENSOR_READER_H
#define SENSOR_READER_H

// ── SHT30 temperature & humidity sensor (I2C 0x44, shared bus SDA=21 SCL=22)
// ── MQ-135 gas sensor (analog, GPIO34, 5.6k/10k voltage divider)

// Initialise sensors — call once in setup() AFTER Wire.begin()
void sensorBegin();

// Call every loop() — handles non-blocking SHT30 warmup. Returns true once ready.
bool sensorTick();

// Read temperature in °C; returns -999.0 if not ready or read failed
float readTemperature();

// Read relative humidity in %; returns -999.0 if not ready or read failed
float readHumidity();

// Read MQ-135 gas concentration in ppm (CO2-equivalent)
// Returns -999.0 if sensor not connected or reading invalid
float readGas();

// True once warmup is complete
bool sensorWarmedUp();

float readNoise(); // dB SPL approx; returns -999.0 if not ready

#endif // SENSOR_READER_H