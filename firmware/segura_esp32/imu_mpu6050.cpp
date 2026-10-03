#include <Arduino.h>
#include <Wire.h>
#include "config.h"
#include "imu_mpu6050.h"

static const uint8_t ADDR = 0x68;           // AD0 low / unconnected
static const float ACC_LSB_PER_G   = 2048.0f;   // +-16 g
static const float GYR_LSB_PER_DPS = 32.8f;     // +-1000 deg/s
static float gyroBias[3] = {0, 0, 0};

static bool writeReg(uint8_t reg, uint8_t val) {
    Wire.beginTransmission(ADDR); Wire.write(reg); Wire.write(val);
    return Wire.endTransmission() == 0;
}

bool imu_begin() {
    Wire.begin(PIN_I2C_SDA, PIN_I2C_SCL);
    Wire.setClock(400000);

    Wire.beginTransmission(ADDR); Wire.write(0x75);            // WHO_AM_I
    if (Wire.endTransmission(false) != 0) return false;
    if (Wire.requestFrom((int)ADDR, 1) != 1) return false;
    uint8_t who = Wire.read();
    Serial.printf("[IMU] WHO_AM_I = 0x%02X (0x68 expected; clones may differ)\n", who);

    bool ok = writeReg(0x6B, 0x01);      // wake up, clock = gyro X PLL
    delay(50);
    ok &= writeReg(0x1A, 0x01);          // DLPF ~184 Hz (keeps short impact spikes)
    ok &= writeReg(0x19, 0x00);          // sample-rate divider 0 (we poll)
    ok &= writeReg(0x1B, 0x10);          // gyro  +-1000 deg/s
    ok &= writeReg(0x1C, 0x18);          // accel +-16 g
    return ok;
}

bool imu_read(ImuSample* s) {
    Wire.beginTransmission(ADDR); Wire.write(0x3B);            // ACCEL_XOUT_H
    if (Wire.endTransmission(false) != 0) return false;
    if (Wire.requestFrom((int)ADDR, 14) != 14) return false;
    int16_t r[7];
    for (int i = 0; i < 7; i++) { uint8_t hi = Wire.read(); uint8_t lo = Wire.read(); r[i] = (int16_t)((hi << 8) | lo); }
    s->ax = r[0] / ACC_LSB_PER_G;  s->ay = r[1] / ACC_LSB_PER_G;  s->az = r[2] / ACC_LSB_PER_G;
    // r[3] = temperature (unused)
    s->gx = r[4] / GYR_LSB_PER_DPS - gyroBias[0];
    s->gy = r[5] / GYR_LSB_PER_DPS - gyroBias[1];
    s->gz = r[6] / GYR_LSB_PER_DPS - gyroBias[2];
    return true;
}

bool imu_calibrate_gyro() {
    double sum[3] = {0, 0, 0}; int n = 0;
    for (int i = 0; i < 200; i++) {
        ImuSample s;
        if (imu_read(&s)) { sum[0] += s.gx; sum[1] += s.gy; sum[2] += s.gz; n++; }
        delay(5);
    }
    if (n < 100) return false;
    float b[3] = {(float)(sum[0] / n), (float)(sum[1] / n), (float)(sum[2] / n)};
    // reject the estimate if the vehicle was clearly moving while calibrating
    if (fabsf(b[0]) > 50 || fabsf(b[1]) > 50 || fabsf(b[2]) > 50) return false;
    for (int i = 0; i < 3; i++) gyroBias[i] += b[i];
    return true;
}
