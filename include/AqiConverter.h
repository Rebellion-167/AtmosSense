#ifndef AQI_CONVERTER_H
#define AQI_CONVERTER_H

// Converts MQ-135 CO2-equivalent PPM to US EPA AQI (CO2 breakpoints).
// Returns -1 if ppm is invalid (<= 0).
int ppmToAqi(float ppm);

#endif // AQI_CONVERTER_H