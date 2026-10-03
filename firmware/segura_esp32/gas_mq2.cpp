#include <Arduino.h>
#include "config.h"
#include "gas_mq2.h"

static uint32_t t0 = 0, lastSample = 0;
static float filt = 0;
static int baseline = -1;
static bool alarm = false;

void gas_begin() {
    analogSetPinAttenuation(PIN_MQ2_AOUT, ADC_11db);
    t0 = millis();
    filt = analogRead(PIN_MQ2_AOUT);
}

void gas_poll() {
    uint32_t now = millis();
    if (now - lastSample < 100) return;
    lastSample = now;
    filt = 0.9f * filt + 0.1f * analogRead(PIN_MQ2_AOUT);
    if (now - t0 < GAS_WARMUP_MS) return;                    // heater still warming up
    if (baseline < 0) baseline = (int)filt;                  // assumes clean air at boot
    alarm = (filt - baseline) > GAS_ALERT_DELTA_RAW;
}

bool gas_alert() { return alarm; }
int  gas_raw()   { return (int)filt; }
