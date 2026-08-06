#include <Arduino.h>
#include "max6675.h"

MAX6675Module::MAX6675Module(int cs, int clk, int so) : _cs(cs), _clk(clk), _so(so) {}

void MAX6675Module::begin() {
    pinMode(_cs, OUTPUT);
    pinMode(_clk, OUTPUT);
    pinMode(_so, INPUT);
    digitalWrite(_cs, HIGH);
    digitalWrite(_clk, LOW);
    _initialized = true;
}

uint16_t MAX6675Module::readRaw() {
    if (!_initialized) {
        begin();
    }
    uint16_t value = 0;
    digitalWrite(_cs, LOW);
    delayMicroseconds(10);
    for (int i = 15; i >= 0; i--) {
        digitalWrite(_clk, LOW);
        delayMicroseconds(1);
        value <<= 1;
        if (digitalRead(_so)) value |= 0x01;
        digitalWrite(_clk, HIGH);
        delayMicroseconds(1);
    }
    digitalWrite(_cs, HIGH);
    return value;
}

float MAX6675Module::readTempC() {
    // Bit-banged SPI occasionally glitches on a single read (e.g. transient
    // open-thermocouple flag). Sample a few times and trust the majority
    // valid reading instead of failing on the first bad sample.
    const int SAMPLES = 3;
    float validTemps[SAMPLES];
    int validCount = 0;
    float lastErrorCode = -999;

    for (int i = 0; i < SAMPLES; i++) {
        uint16_t raw = readRaw();
        if (raw & 0x0004) {
            lastErrorCode = -999; // Not connected / faulty
        } else if (raw & 0x0002) {
            lastErrorCode = -998; // Shorted to GND
        } else if (raw & 0x0001) {
            lastErrorCode = -997; // Shorted to VCC
        } else {
            float temp = (raw >> 3) * 0.25f;
            if (temp < -200 || temp > 1350) {
                lastErrorCode = -996; // Out of range
            } else {
                validTemps[validCount++] = temp;
            }
        }
        if (i < SAMPLES - 1) delayMicroseconds(200);
    }

    if (validCount >= 2) {
        float sum = 0;
        for (int i = 0; i < validCount; i++) sum += validTemps[i];
        return sum / validCount;
    }
    if (validCount == 1) {
        return validTemps[0];
    }

    if (lastErrorCode == -999) Serial.println("[MAX6675] Thermocouple not connected or faulty");
    else if (lastErrorCode == -998) Serial.println("[MAX6675] Thermocouple shorted to GND");
    else if (lastErrorCode == -997) Serial.println("[MAX6675] Thermocouple shorted to VCC");
    else Serial.println("[MAX6675] Temperature out of range");
    return lastErrorCode;
}