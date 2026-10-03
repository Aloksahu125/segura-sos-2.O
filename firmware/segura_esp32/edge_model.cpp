#include "edge_model.h"
#include "config.h"
// Generated model code (emlearn). Defines non-static functions, so include
// these headers from this translation unit ONLY.
#include "model_event.h"      // event_model_predict_proba(...)    3 classes
#include "model_severity.h"   // severity_model_predict_proba(...) 3 classes

void model_run(const float* f, ModelResult* out)
{
    float p[3] = {0, 0, 0};
    event_model_predict_proba(f, NUM_FEATURES, p, 3);

    out->severity = SEV_NONE;
    out->severity_conf = 0.0f;

    if (p[EV_CRASH] >= MODEL_CRASH_MIN_CONF) {
        out->event = EV_CRASH;
        out->event_conf = p[EV_CRASH];

        float s[3] = {0, 0, 0};
        severity_model_predict_proba(f, NUM_FEATURES, s, 3);
        int best = 0;
        for (int i = 1; i < 3; i++) if (s[i] > s[best]) best = i;
        out->severity = (Severity)best;
        out->severity_conf = s[best];
    } else {
        // not a (confident) crash: pick the better of normal / hard_brake
        out->event = (p[EV_HARD_BRAKE] > p[EV_NORMAL]) ? EV_HARD_BRAKE : EV_NORMAL;
        out->event_conf = p[out->event];
    }
}

const char* event_name(EventClass e) {
    switch (e) { case EV_CRASH: return "crash"; case EV_HARD_BRAKE: return "hard_brake"; default: return "normal"; }
}
const char* severity_name(Severity s) {
    switch (s) { case SEV_MINOR: return "minor"; case SEV_MODERATE: return "moderate"; case SEV_SEVERE: return "severe"; default: return "none"; }
}
