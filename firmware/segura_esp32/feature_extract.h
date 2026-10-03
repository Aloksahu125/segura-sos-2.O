// Feature extraction for the on-device accident model.
// Pure C++ (no Arduino dependency) so it can be unit-tested on a PC.
// MUST stay numerically in sync with tools/train_placeholder_model.py
// (function `features`) - the model is trained on exactly these values.
#pragma once

struct ImuSample {
    float ax, ay, az;   // acceleration, g
    float gx, gy, gz;   // angular rate, deg/s
};

#define NUM_FEATURES 10
extern const char* const FEATURE_NAMES[NUM_FEATURES];

// w: window of n samples, oldest first.  out: NUM_FEATURES floats.
//  0 peak_g            max |a|
//  1 mean_g            mean |a|
//  2 std_g             std  |a|
//  3 peak_jerk_gps     max |d|a|/dt|   (g/s)
//  4 impulse_ms        sum(max(|a|-1,0))*dt*9.81   (m/s)
//  5 peak_gyro_dps     max |w|
//  6 mean_gyro_dps     mean |w|
//  7 tilt_change_deg   angle between mean accel of first and last 10 samples
//  8 speed_kmh         GPS speed (0 if unavailable)
//  9 speed_drop_kmh    max(0, speed a few seconds ago - speed now)
void extract_features(const ImuSample* w, int n, float fs_hz,
                      float speed_kmh, float speed_drop_kmh, float* out);
