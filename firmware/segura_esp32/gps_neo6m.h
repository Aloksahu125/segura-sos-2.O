// NEO-6M GPS reader (TinyGPSPlus, UART2).
#pragma once
#include "payload.h"

void  gps_begin();
void  gps_poll();                       // call often: drains the UART buffer
void  gps_snapshot(GpsFix* out);        // last known position + freshness
float gps_speed_kmh();                  // 0 when no fresh speed
