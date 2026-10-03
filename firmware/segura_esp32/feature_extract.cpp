#include "feature_extract.h"
#include <math.h>

const char* const FEATURE_NAMES[NUM_FEATURES] = {
    "peak_g", "mean_g", "std_g", "peak_jerk_gps", "impulse_ms",
    "peak_gyro_dps", "mean_gyro_dps", "tilt_change_deg",
    "speed_kmh", "speed_drop_kmh"
};

void extract_features(const ImuSample* w, int n, float fs_hz,
                      float speed_kmh, float speed_drop_kmh, float* out)
{
    const double dt = 1.0 / fs_hz;
    double sum = 0, sum2 = 0, gsum = 0, imp = 0;
    double peak = 0, gpeak = 0, pjerk = 0, prev = 0;

    for (int i = 0; i < n; i++) {
        const double mag = sqrt((double)w[i].ax * w[i].ax + (double)w[i].ay * w[i].ay +
                                (double)w[i].az * w[i].az);
        const double gm  = sqrt((double)w[i].gx * w[i].gx + (double)w[i].gy * w[i].gy +
                                (double)w[i].gz * w[i].gz);
        sum += mag; sum2 += mag * mag; gsum += gm;
        if (mag > peak)  peak  = mag;
        if (gm  > gpeak) gpeak = gm;
        if (i > 0) { const double j = fabs(mag - prev) * fs_hz; if (j > pjerk) pjerk = j; }
        if (mag > 1.0) imp += (mag - 1.0) * dt * 9.81;
        prev = mag;
    }
    const double mean = sum / n;
    double var = sum2 / n - mean * mean; if (var < 0) var = 0;

    // orientation change between the start and end of the window
    int k = n / 4;
    if (k > 10) k = 10;
    if (k < 1) k = 1;
    double a0[3] = {0, 0, 0}, a1[3] = {0, 0, 0};
    for (int i = 0; i < k; i++) {
        a0[0] += w[i].ax;         a0[1] += w[i].ay;         a0[2] += w[i].az;
        a1[0] += w[n - k + i].ax; a1[1] += w[n - k + i].ay; a1[2] += w[n - k + i].az;
    }
    double n0 = sqrt(a0[0]*a0[0] + a0[1]*a0[1] + a0[2]*a0[2]);
    double n1 = sqrt(a1[0]*a1[0] + a1[1]*a1[1] + a1[2]*a1[2]);
    double tilt = 0;
    if (n0 > 1e-6 && n1 > 1e-6) {
        double c = (a0[0]*a1[0] + a0[1]*a1[1] + a0[2]*a1[2]) / (n0 * n1);
        if (c > 1) c = 1;
        if (c < -1) c = -1;
        tilt = acos(c) * 180.0 / M_PI;
    }

    out[0] = (float)peak;
    out[1] = (float)mean;
    out[2] = (float)sqrt(var);
    out[3] = (float)pjerk;
    out[4] = (float)imp;
    out[5] = (float)gpeak;
    out[6] = (float)(gsum / n);
    out[7] = (float)tilt;
    out[8] = speed_kmh;
    out[9] = speed_drop_kmh < 0 ? 0.0f : speed_drop_kmh;
}
