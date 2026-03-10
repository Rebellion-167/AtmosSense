#include "AqiConverter.h"

// Indoor Air Index — linear interpolation aligned to AtmosSense alert thresholds:
//
// Good             0  –  800 ppm  → AQI   0 –  50   (Normal)
// Moderate       800  – 1000 ppm  → AQI  51 – 100   (Warning low)
// Unhealthy*    1000  – 1200 ppm  → AQI 101 – 150   (Warning high)
// Unhealthy     1200  – 2000 ppm  → AQI 151 – 200   (Danger)
// Very Unhealthy 2000 – 3000 ppm  → AQI 201 – 300   (Danger severe)
// Hazardous     3000+ ppm         → AQI 301 – 500   (Evacuate)

struct Breakpoint {
    float cLow, cHigh;
    int   aqiLow, aqiHigh;
};

static const Breakpoint breakpoints[] = {
    {    0,  800,   0,  50 },
    {  800, 1000,  51, 100 },
    { 1000, 1200, 101, 150 },
    { 1200, 2000, 151, 200 },
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
    return 500;
}