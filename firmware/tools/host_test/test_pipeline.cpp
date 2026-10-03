// Host-side check (runs on a PC, no ESP32 needed):
//  1. C++ feature extraction matches the Python training features (parity)
//  2. the generated model headers compile and run
//  3. the payload builder produces valid, compact JSON
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "feature_extract.h"
#include "edge_model.h"
#include "payload.h"
#include "config.h"

static char line[65536];

int main(int argc, char** argv)
{
    const char* path = argc > 1 ? argv[1] : "parity_windows.csv";
    FILE* f = fopen(path, "r");
    if (!f) { fprintf(stderr, "cannot open %s\n", path); return 2; }

    int rows = 0, feat_bad = 0, cls_ok[3] = {0}, cls_n[3] = {0};
    double worst = 0;
    ModelResult lastCrash = {}; bool haveCrash = false; float crashF[NUM_FEATURES];

    while (fgets(line, sizeof line, f)) {
        static float v[3 + WINDOW_SAMPLES * 6 + NUM_FEATURES];
        int n = 0;
        for (char* t = strtok(line, ","); t && n < (int)(sizeof v / sizeof v[0]); t = strtok(NULL, ",")) v[n++] = strtof(t, NULL);
        if (n != 3 + WINDOW_SAMPLES * 6 + NUM_FEATURES) { fprintf(stderr, "bad row (%d values)\n", n); return 2; }

        int label = (int)v[0]; float speed = v[1], drop = v[2];
        static ImuSample w[WINDOW_SAMPLES];
        for (int i = 0; i < WINDOW_SAMPLES; i++) {
            const float* s = &v[3 + i * 6];
            w[i] = {s[0], s[1], s[2], s[3], s[4], s[5]};
        }
        const float* pyf = &v[3 + WINDOW_SAMPLES * 6];
        float cf[NUM_FEATURES];
        extract_features(w, WINDOW_SAMPLES, SAMPLE_RATE_HZ, speed, drop, cf);
        for (int k = 0; k < NUM_FEATURES; k++) {
            double err = fabs(cf[k] - pyf[k]) / (1.0 + fabs(pyf[k]));
            if (err > worst) worst = err;
            if (err > 1e-3) { feat_bad++; printf("row %d feature %s: C++=%g py=%g\n", rows, FEATURE_NAMES[k], cf[k], pyf[k]); }
        }
        ModelResult r; model_run(cf, &r);
        cls_n[label]++; if ((int)r.event == label) cls_ok[label]++;
        if (r.event == EV_CRASH && !haveCrash) { lastCrash = r; haveCrash = true; memcpy(crashF, cf, sizeof cf); }
        rows++;
    }
    fclose(f);

    printf("rows=%d  worst relative feature error=%.2e  feature mismatches=%d\n", rows, worst, feat_bad);
    const char* nm[3] = {"normal", "hard_brake", "crash"};
    for (int c = 0; c < 3; c++) printf("  %-10s classified as itself: %d/%d\n", nm[c], cls_ok[c], cls_n[c]);

    // payload check
    EventRecord e; memset(&e, 0, sizeof e);
    snprintf(e.event_id, sizeof e.event_id, "%s-%lu-%u", DEVICE_ID, 123456UL, 1u);
    e.uptime_ms = 123456; e.type = EV_CRASH; e.severity = haveCrash ? lastCrash.severity : SEV_SEVERE;
    e.confidence = haveCrash ? lastCrash.event_conf : 0.9f; e.impact_g = 9.37f; e.speed_kmh = 54.2f;
    e.gas_alert = false;
    e.gps.fix = true; e.gps.ever_fixed = true; e.gps.lat = 22.719568; e.gps.lng = 75.857727;
    e.gps.accuracy_m = 6.2f; e.gps.age_s = 0.4f; e.gps.time_valid = true;
    snprintf(e.gps.iso_time, sizeof e.gps.iso_time, "2026-10-01T10:20:30Z");
    char body[600];
    size_t len = build_payload_json(body, sizeof body, e);
    printf("payload (%zu bytes):\n%s\n", len, body);

    EventRecord e2 = e; e2.gps.fix = false; e2.gps.ever_fixed = false; e2.gps.time_valid = false; e2.severity = SEV_NONE; e2.type = EV_HARD_BRAKE;
    printf("no-fix payload:\n");
    len = build_payload_json(body, sizeof body, e2); printf("%s\n", body);

    return (feat_bad == 0 && len > 0) ? 0 : 1;
}
