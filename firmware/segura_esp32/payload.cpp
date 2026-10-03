#include "payload.h"
#include "config.h"
#include <stdio.h>

size_t build_payload_json(char* buf, size_t cap, const EventRecord& e)
{
    char lat[24], lng[24], acc[16], age[16], sev[16];
    const char* src;
    bool have = false; double la = 0, lo = 0;

    if (e.gps.fix)             { src = "gps";       have = true; la = e.gps.lat; lo = e.gps.lng; }
    else if (e.gps.ever_fixed) { src = "last_known"; have = true; la = e.gps.lat; lo = e.gps.lng; }
#if DEMO_LOCATION_FALLBACK
    else                       { src = "demo_fallback"; have = true; la = DEMO_LAT; lo = DEMO_LNG; }
#else
    else                       { src = "none"; }
#endif
    if (have) { snprintf(lat, sizeof lat, "%.6f", la); snprintf(lng, sizeof lng, "%.6f", lo); }
    else      { snprintf(lat, sizeof lat, "null");     snprintf(lng, sizeof lng, "null"); }

    if (e.gps.ever_fixed) { snprintf(acc, sizeof acc, "%.1f", e.gps.accuracy_m);
                            snprintf(age, sizeof age, "%.1f", e.gps.age_s); }
    else                  { snprintf(acc, sizeof acc, "null"); snprintf(age, sizeof age, "null"); }

    if (e.severity == SEV_NONE) snprintf(sev, sizeof sev, "null");
    else                        snprintf(sev, sizeof sev, "\"%s\"", severity_name(e.severity));

    int n = snprintf(buf, cap,
        "{\"event_id\":\"%s\",\"device_id\":\"%s\",\"timestamp\":\"%s\",\"uptime_ms\":%lu,"
        "\"type\":\"%s\",\"source\":\"real\","
        "\"gps\":{\"lat\":%s,\"lng\":%s,\"accuracy_m\":%s,\"fix\":%s,\"age_s\":%s,\"location_source\":\"%s\"},"
        "\"speed_kmh\":%.1f,\"impact_g\":%.2f,\"weather\":\"clear\","
        "\"severity\":%s,\"confidence\":%.2f,\"gas_alert\":%s,"
        "\"model_status\":\"%s\",\"model_version\":\"%s\"}",
        e.event_id, DEVICE_ID, e.gps.time_valid ? e.gps.iso_time : "", (unsigned long)e.uptime_ms,
        event_name(e.type),
        lat, lng, acc, e.gps.fix ? "true" : "false", age, src,
        e.speed_kmh, e.impact_g,
        sev, e.confidence, e.gas_alert ? "true" : "false",
        MODEL_STATUS, MODEL_VERSION);
    return (n > 0 && (size_t)n < cap) ? (size_t)n : 0;
}
