#include <Arduino.h>
#include <TinyGPSPlus.h>
#include "config.h"
#include "gps_neo6m.h"

static TinyGPSPlus gps;
static HardwareSerial gpsSerial(2);

void gps_begin() {
    gpsSerial.setRxBufferSize(1024);    // survive short stalls while the modem is busy
    gpsSerial.begin(9600, SERIAL_8N1, PIN_GPS_RX, PIN_GPS_TX);
}

void gps_poll() {
    while (gpsSerial.available() > 0) gps.encode(gpsSerial.read());
}

void gps_snapshot(GpsFix* o) {
    o->ever_fixed = gps.location.isValid();
    o->age_s      = o->ever_fixed ? gps.location.age() / 1000.0f : 0.0f;
    o->fix        = o->ever_fixed && gps.location.age() < 5000;
    o->lat        = o->ever_fixed ? gps.location.lat() : 0.0;
    o->lng        = o->ever_fixed ? gps.location.lng() : 0.0;
    // NEO-6M is typically ~2.5 m CEP; scale by HDOP as a rough accuracy estimate
    o->accuracy_m = gps.hdop.isValid() ? gps.hdop.hdop() * 2.5f : 10.0f;
    o->time_valid = gps.date.isValid() && gps.time.isValid() && gps.date.year() >= 2024;
    if (o->time_valid) {
        snprintf(o->iso_time, sizeof o->iso_time, "%04u-%02u-%02uT%02u:%02u:%02uZ",
                 gps.date.year(), gps.date.month(), gps.date.day(),
                 gps.time.hour(), gps.time.minute(), gps.time.second());
    } else {
        o->iso_time[0] = 0;
    }
}

float gps_speed_kmh() {
    return (gps.speed.isValid() && gps.speed.age() < 2500) ? (float)gps.speed.kmph() : 0.0f;
}
