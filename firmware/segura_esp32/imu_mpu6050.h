// Minimal MPU6050 driver (register level, no external library).
// Configured for +-16 g / +-1000 deg/s so crash impacts do not saturate the
// sensor (library defaults of +-2 g would clip anything above 2 g).
#pragma once
#include "feature_extract.h"

bool imu_begin();                 // false if the sensor does not answer
bool imu_calibrate_gyro();        // call while the vehicle is stationary
bool imu_read(ImuSample* s);      // false on I2C error
