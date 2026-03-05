#include "AqiConverter.h"

// EPA AQI linear interpolation formula:
// AQI = ((AQI_high - AQI_low) / (C_high - C_low)) * (C - C_low) + AQI_low
//
// Breakpoints mapped to CO2-equivalent PPM from MQ-135:
// Good            0   – 400  ppm  → AQI   0 –  50
// Moderate      400   – 1000 ppm  → AQI  51 – 100
// Unhealthy*   1000   – 1500 ppm  → AQI 101 – 150
// Unhealthy    1500   – 2000 ppm  → AQI 151 – 200
// Very Unhealthy 2000 – 3000 ppm  → AQI 201 – 300
// Hazardous    3000+  ppm         → AQI 301+

struct Breakpoint {
    float cLow, cHigh;
    int   aqiLow, aqiHigh;
};

static const Breakpoint breakpoints[] = {
    {    0,  400,   0,  50 },
    {  400, 1000,  51, 100 },
    { 1000, 1500, 101, 150 },
    { 1500, 2000, 151, 200 },
    { 2000, 3000, 201, 300 },
    { 3000, 5000, 301, 500 }
};

int ppmToAqi(float ppm) {
    if (ppm <= 0) return -1;

    for (auto& bp : breakpoints) {
        if (ppm <= bp.cHigh) {
            return (int)(((float)(bp.aqiHigh - bp.aqiLow) / (bp.cHigh - bp.cLow))
                         * (ppm - bp.cLow) + bp.aqiLow);
        }
    }
    return 500; // above hazardous ceiling
}