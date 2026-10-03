// JSON payload for the M3 server. Pure C++ (no Arduino dependency).
#pragma once
#include <stddef.h>
#include <stdint.h>
#include "edge_model.h"

struct GpsFix {
    bool   fix;          // a fresh fix (updated within the last few seconds)
    bool   ever_fixed;   // a position has been acquired since boot
    double lat, lng;     // last known position (valid only if ever_fixed)
    float  accuracy_m;   // rough estimate from HDOP
    float  age_s;        // seconds since the position was last updated
    bool   time_valid;
    char   iso_time[24]; // "YYYY-MM-DDTHH:MM:SSZ" (GPS UTC) when time_valid
};

struct EventRecord {
    char       event_id[48];
    uint32_t   uptime_ms;
    EventClass type;
    Severity   severity;
    float      confidence;
    float      impact_g;
    float      speed_kmh;
    bool       gas_alert;
    GpsFix     gps;
};

// Writes the JSON body into buf. Returns the length, or 0 if it did not fit.
size_t build_payload_json(char* buf, size_t cap, const EventRecord& e);
