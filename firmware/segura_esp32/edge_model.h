// On-device accident detection: model interface.
// Pure C++ (no Arduino dependency).
//
// ---------------------------------------------------------------------------
//  MODEL STATUS: PRE-RELEASE
//  The on-device accident-detection model is under development and training.
//  The model currently bundled (model_event.h / model_severity.h) is a
//  placeholder trained on synthetic data solely to validate the end-to-end
//  inference pipeline. It has NOT been validated on real crash data and must
//  not be relied upon for real-world safety decisions.
//  To integrate the validated model, regenerate model_event.h and
//  model_severity.h (see tools/train_placeholder_model.py and README.md);
//  no other firmware change is required.
// ---------------------------------------------------------------------------
#pragma once
#include "feature_extract.h"

enum EventClass { EV_NORMAL = 0, EV_HARD_BRAKE = 1, EV_CRASH = 2 };
enum Severity   { SEV_NONE = -1, SEV_MINOR = 0, SEV_MODERATE = 1, SEV_SEVERE = 2 };

struct ModelResult {
    EventClass event;       // stage 1: normal | hard_brake | crash
    float      event_conf;  // 0..1
    Severity   severity;    // stage 2 (only when event == EV_CRASH), else SEV_NONE
    float      severity_conf;
};

// Reported in boot log and in every uplink payload.
#define MODEL_STATUS   "placeholder"      // change to "validated" once the real model lands
#define MODEL_VERSION  "0.0.1-synthetic"

// Two-stage inference: stage 1 classifies the window; if it is a crash,
// stage 2 grades the severity. `features` has NUM_FEATURES floats.
void model_run(const float* features, ModelResult* out);

const char* event_name(EventClass e);      // "normal" | "hard_brake" | "crash"
const char* severity_name(Severity s);     // "minor" | "moderate" | "severe" | "none"
