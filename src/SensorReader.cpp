#include "SensorReader.h"
#include <Wire.h>
#include <Adafruit_SHT31.h>
#include <Arduino.h>
#include <driver/i2s.h>

// ── SHT30 config ──────────────────────────────────────────────────────────────
#define SHT30_ADDR 0x44
#define SHT30_SDA 21
#define SHT30_SCL 22

// Warmup: discard this many valid readings after power-on
#define WARMUP_READINGS 3
#define WARMUP_DELAY_MS 1000 // SHT30 max measurement interval

// ── MQ-135 config ─────────────────────────────────────────────────────────────
#define MQ135_PIN 34

#define MQ135_RL 10.0f
#define MQ135_R0 76.63f
#define MQ135_PARA 116.6020682f
#define MQ135_PARB -2.769034857f
#define MQ135_VREF 3.3f
#define MQ135_ADC_MAX 4095.0f
#define MQ135_DIVIDER_SCALE (10.0f / (5.6f + 10.0f))
#define MQ135_MIN_RAW 100

// INMP441 Config
#define INMP441_WS 15
#define INMP441_SCK 14
#define INMP441_SD 32
#define INMP441_PORT I2S_NUM_0

#define NOISE_SAMPLES 256
#define INMP441_REF ((float)INT32_MAX) // full-scale reference for 0 dBFS
#define DB_OFFSET 120.0f               // empirical offset → approximate dB SPL

// ── State ─────────────────────────────────────────────────────────────────────
static Adafruit_SHT31 _sht;
static bool _ready = false;
static int _warmupRemaining = WARMUP_READINGS;
static unsigned long _lastReadMs = 0;
static bool _i2sReady = false;

void sensorBegin()
{
    // Share the I2C bus already initialised by the OLED (Wire on SDA=21, SCL=22)
    if (!_sht.begin(SHT30_ADDR))
    {
        Serial.println("[SHT30] Not found — check wiring and I2C address.");
    }
    else
    {
        Serial.println("[SHT30] Found. Warming up...");
    }
    pinMode(MQ135_PIN, INPUT);
    Serial.println("[MQ-135] Initialised on GPIO34.");

    // INMP441 I2S init
    i2s_config_t i2sCfg = {
        .mode                 = (i2s_mode_t)(I2S_MODE_MASTER | I2S_MODE_RX),
        .sample_rate          = 16000,
        .bits_per_sample      = I2S_BITS_PER_SAMPLE_32BIT,
        .channel_format       = I2S_CHANNEL_FMT_ONLY_LEFT,
        .communication_format = I2S_COMM_FORMAT_STAND_I2S,
        .intr_alloc_flags     = ESP_INTR_FLAG_LEVEL1,
        .dma_buf_count        = 4,
        .dma_buf_len          = 64,
        .use_apll             = false,
        .tx_desc_auto_clear   = false,
        .fixed_mclk           = 0
    };
    i2s_pin_config_t pinCfg = {
        .bck_io_num   = INMP441_SCK,
        .ws_io_num    = INMP441_WS,
        .data_out_num = I2S_PIN_NO_CHANGE,
        .data_in_num  = INMP441_SD
    };
    esp_err_t err1 = i2s_driver_install(INMP441_PORT, &i2sCfg, 0, NULL);
    esp_err_t err2 = i2s_set_pin(INMP441_PORT, &pinCfg);
    if (err1 == ESP_OK && err2 == ESP_OK) {
        i2s_zero_dma_buffer(INMP441_PORT);
        delay(200);
        _i2sReady = true;
        Serial.println("[INMP441] I2S ready.");
    } else {
        Serial.printf("[INMP441] Init failed: %d %d\n", err1, err2);
        _i2sReady = false;
    }
}

bool sensorTick()
{
    if (_ready)
        return true;

    unsigned long now = millis();
    if (now - _lastReadMs < WARMUP_DELAY_MS)
        return false;
    _lastReadMs = now;

    float t = _sht.readTemperature();
    float h = _sht.readHumidity();

    if (!isnan(t) && !isnan(h))
    {
        _warmupRemaining--;
        Serial.printf("[SHT30] Warmup discard (%d left): %.2f C  %.2f%%\n",
                      _warmupRemaining, t, h);
        if (_warmupRemaining <= 0)
        {
            _ready = true;
            Serial.println("[SHT30] Ready.");
        }
    }
    else
    {
        Serial.println("[SHT30] Read failed during warmup — retrying...");
    }
    return _ready;
}

float readTemperature()
{
    if (!_ready)
        return -999.0f;
    float t = _sht.readTemperature();
    return isnan(t) ? -999.0f : t;
}

float readHumidity()
{
    if (!_ready)
        return -999.0f;
    float h = _sht.readHumidity();
    return isnan(h) ? -999.0f : h;
}

float readGas()
{
    int raw = analogRead(MQ135_PIN);
    Serial.printf("[MQ-135] raw=%d vMeasured=%.3f\n", raw, (raw / 4095.0f) * 3.3f);
    if (raw < MQ135_MIN_RAW)
        return -999.0f;

    float vMeasured = (raw / MQ135_ADC_MAX) * MQ135_VREF;
    float voltage = vMeasured / MQ135_DIVIDER_SCALE;
    if (voltage <= 0.0f)
        return -999.0f;

    float rs = MQ135_RL * (MQ135_VREF - voltage) / voltage;
    if (rs <= 0.0f)
        return -999.0f;

    float ppm = MQ135_PARA * powf(rs / MQ135_R0, MQ135_PARB);
    if (ppm < 0.0f)
        return -999.0f;
    if (ppm > 10000.0f)
        ppm = 10000.0f;
    return ppm;
}

bool sensorWarmedUp() { return _ready; }

float readNoise() {
    if (!_i2sReady) return -999.0f;

    static int32_t buf[NOISE_SAMPLES];
    size_t bytesRead = 0;
    esp_err_t err = i2s_read(INMP441_PORT, buf, sizeof(buf), &bytesRead, pdMS_TO_TICKS(50));
    if (err != ESP_OK || bytesRead < sizeof(int32_t)) return -999.0f;

    int samples = (int)(bytesRead / sizeof(int32_t));
    double sum = 0.0;
    for (int i = 0; i < samples; i++) {
        // INMP441 data is left-justified in 32-bit word, shift down to 24-bit
        int32_t sample = buf[i] >> 8;
        double s = (double)sample / (double)0x7FFFFF;
        sum += s * s;
    }
    double rms = sqrt(sum / samples);
    if (rms < 1e-10) return 0.0f;
    float db = (float)(20.0 * log10(rms)) + DB_OFFSET;
    return db < 0.0f ? 0.0f : db;
}