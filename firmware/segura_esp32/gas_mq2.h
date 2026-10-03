// MQ-2 gas/smoke sensor (analog, ADC1). Coarse indicator only.
// Its analog output can exceed 3.3 V - use a voltage divider before the ESP32 pin.
#pragma once
void gas_begin();
void gas_poll();        // call regularly (cheap)
bool gas_alert();       // true when the reading is well above the warm-up baseline
int  gas_raw();
