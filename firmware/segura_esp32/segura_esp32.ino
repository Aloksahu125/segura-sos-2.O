// ============================================================================
//  SEGURA - ESP32 edge crash-detection unit
//
//  sensors (MPU6050 + NEO-6M + MQ-2)
//     -> ESP32: 200 Hz sampling -> windowed features
//     -> on-device two-stage model:  crash?  ->  severity (minor/moderate/severe)
//     -> JSON alert over NB-IoT (HTTP POST) to the M3 server
//
//  MODEL STATUS: PRE-RELEASE - the bundled model is a placeholder trained on
//  synthetic data while the validated model is under development/training.
//
//  Core 0: sensor task (sampling + inference, never blocked by the modem)
//  Core 1: loop()      (GPS, gas sensor, NB-IoT uplink)
// ============================================================================
#include <Arduino.h>
#include "config.h"
#include "feature_extract.h"
#include "edge_model.h"
#include "payload.h"
#include "imu_mpu6050.h"
#include "gps_neo6m.h"
#include "gas_mq2.h"
#include "nbiot_sim7020.h"

static_assert(INFER_STRIDE_SAMPLES + CONFIRM_DELAY_SAMPLES < WINDOW_SAMPLES,
              "the impact must still be inside the window when it is re-evaluated");

// ---------------------------------------------------------------- shared state
struct DetectedEvent {
    uint32_t   t_ms;
    EventClass type;
    float      impact_g;
    float      speed_kmh;
    Severity   severity;
    float      confidence;
};

static QueueHandle_t eventQueue;
static volatile float g_speed_kmh = 0.0f;       // written by loop(), read by sensor task
static volatile uint32_t g_imuFaults = 0;
static volatile uint32_t g_lastEventMs = 0;     // for the status LED

// ------------------------------------------------------------- sensor task (core 0)
static ImuSample ring[WINDOW_SAMPLES];

static void runModel(uint32_t n, float speedNow, float speedDrop, ModelResult* r, float* peakG) {
    static ImuSample win[WINDOW_SAMPLES];
    const uint32_t start = n % WINDOW_SAMPLES;                 // oldest sample
    for (int i = 0; i < WINDOW_SAMPLES; i++) win[i] = ring[(start + i) % WINDOW_SAMPLES];
    float f[NUM_FEATURES];
    extract_features(win, WINDOW_SAMPLES, SAMPLE_RATE_HZ, speedNow, speedDrop, f);
    model_run(f, r);
    *peakG = f[0];
}

static void emit(EventClass type, const ModelResult& r, float peakG, float speedNow) {
    DetectedEvent ev = { millis(), type, peakG, speedNow, r.severity, r.event_conf };
    xQueueSend(eventQueue, &ev, 0);
    g_lastEventMs = ev.t_ms;
    Serial.printf("[EVT] %s sev=%s conf=%.2f peak=%.2fg speed=%.1f\n",
                  event_name(type), severity_name(r.severity), r.event_conf, peakG, speedNow);
}

static void sensorTask(void*) {
    // vTaskDelayUntil with a 5 ms period assumes the default 1 kHz FreeRTOS tick (Arduino-ESP32).
    TickType_t last = xTaskGetTickCount();
    uint32_t n = 0, nextInfer = WINDOW_SAMPLES, confirmAt = 0, cooldownUntil = 0;
    bool confirming = false;
    float speedHist[3] = {0, 0, 0};                            // speed 1 s, 2 s, 3 s ago
    uint32_t lastSpeedTick = millis();

    for (;;) {
        vTaskDelayUntil(&last, pdMS_TO_TICKS(SAMPLE_PERIOD_MS));

        ImuSample s;
        if (!imu_read(&s)) { g_imuFaults++; continue; }
        ring[n % WINDOW_SAMPLES] = s;
        n++;

        const float speedNow = g_speed_kmh;
        if (millis() - lastSpeedTick >= 1000) {
            lastSpeedTick += 1000;
            speedHist[2] = speedHist[1]; speedHist[1] = speedHist[0]; speedHist[0] = speedNow;
        }
        float maxPast = speedHist[0];
        for (int i = 1; i < 3; i++) if (speedHist[i] > maxPast) maxPast = speedHist[i];
        const float speedDrop = maxPast > speedNow ? maxPast - speedNow : 0.0f;

        if (n < WINDOW_SAMPLES) continue;                       // window not filled yet
        const bool cooling = (int32_t)(millis() - cooldownUntil) < 0;

        if (!confirming && n >= nextInfer) {
            nextInfer = n + INFER_STRIDE_SAMPLES;
            if (cooling) continue;
            ModelResult r; float peak;
            runModel(n, speedNow, speedDrop, &r, &peak);
            if (r.event == EV_CRASH) {
                confirming = true; confirmAt = n + CONFIRM_DELAY_SAMPLES;   // wait for the full impact
            }
#if SEND_HARD_BRAKE_EVENTS
            else if (r.event == EV_HARD_BRAKE) { emit(EV_HARD_BRAKE, r, peak, speedNow); cooldownUntil = millis() + EVENT_COOLDOWN_MS; }
#endif
        } else if (confirming && n >= confirmAt) {
            confirming = false;
            nextInfer = n + INFER_STRIDE_SAMPLES;
            ModelResult r; float peak;
            runModel(n, speedNow, speedDrop, &r, &peak);       // final look at the whole impact
            if (r.event == EV_CRASH) {
                emit(EV_CRASH, r, peak, speedNow);
                cooldownUntil = millis() + EVENT_COOLDOWN_MS;
            }
        }
    }
}

// ------------------------------------------------------------ uplink (core 1 / loop)
static NbIotModem modem;
static EventRecord pending[4];
static int pendingCount = 0;
static uint16_t eventCounter = 0;
static bool modemReady = false;
static uint32_t nextModemTry = 0, nextSendTry = 0, backoffMs = SEND_BACKOFF_MIN_MS, nextAttachCheck = 0;

static void idleHook() { gps_poll(); }

static void enqueuePending(const DetectedEvent& ev) {
    if (pendingCount == 4) {                                   // drop the oldest if the uplink is stuck
        memmove(&pending[0], &pending[1], sizeof(EventRecord) * 3);
        pendingCount--;
    }
    EventRecord& e = pending[pendingCount++];
    memset(&e, 0, sizeof e);
    snprintf(e.event_id, sizeof e.event_id, "%s-%lu-%u", DEVICE_ID, (unsigned long)ev.t_ms, (unsigned)++eventCounter);
    e.uptime_ms = ev.t_ms;
    e.type = ev.type;
    e.severity = ev.severity;
    e.confidence = ev.confidence;
    e.impact_g = ev.impact_g;
    e.speed_kmh = ev.speed_kmh;
    e.gas_alert = gas_alert();
    gps_snapshot(&e.gps);                                      // location at the moment of the event
    nextSendTry = 0;                                           // send immediately
}

static void serviceModem() {
    const uint32_t now = millis();
    if (!modemReady) {
        if ((int32_t)(now - nextModemTry) < 0) return;
        modemReady = modem.init() && modem.waitAttached(NET_ATTACH_TIMEOUT_MS);
        if (!modemReady) { nextModemTry = millis() + 15000; Serial.println("[NB] not ready, retrying"); }
        return;
    }
    if (pendingCount == 0 && (int32_t)(now - nextAttachCheck) >= 0) {      // keep an eye on registration
        nextAttachCheck = now + 30000;
        if (!modem.attached()) { Serial.println("[NB] registration lost"); modemReady = false; }
    }
}

static void serviceUplink() {
    if (pendingCount == 0 || !modemReady) return;
    if ((int32_t)(millis() - nextSendTry) < 0) return;

    char body[480];
    if (build_payload_json(body, sizeof body, pending[0]) == 0) { Serial.println("[TX] payload too large, dropped"); pendingCount--; memmove(&pending[0], &pending[1], sizeof(EventRecord) * pendingCount); return; }
    Serial.printf("[TX] %s\n", body);

    const int status = modem.postJson(SERVER_HOST, SERVER_PORT, SERVER_USE_HTTPS, SERVER_PATH, body);
    if (status >= 200 && status < 300) {
        Serial.printf("[TX] delivered, HTTP %d\n", status);
        pendingCount--; memmove(&pending[0], &pending[1], sizeof(EventRecord) * pendingCount);
        backoffMs = SEND_BACKOFF_MIN_MS;
    } else {
        // Same event_id is re-sent on retry, so M3 can de-duplicate on it.
        Serial.printf("[TX] failed (%d), retry in %lu ms\n", status, (unsigned long)backoffMs);
        nextSendTry = millis() + backoffMs;
        backoffMs = min<uint32_t>(backoffMs * 2, SEND_BACKOFF_MAX_MS);
        if (status == -1 || status == -3) modemReady = modem.attached();   // re-check registration
    }
}

// ------------------------------------------------------------------ Arduino entry
void setup() {
    Serial.begin(115200);
    delay(300);
    pinMode(PIN_STATUS_LED, OUTPUT);
    Serial.println("\n=== SEGURA edge unit ===");
    Serial.printf("Model: %s (%s)  -  PRE-RELEASE: under development/training; placeholder trained on\n"
                  "synthetic data, for pipeline validation only. Not for real-world safety decisions.\n",
                  MODEL_STATUS, MODEL_VERSION);

    if (!imu_begin()) { Serial.println("[IMU] MPU6050 not found - check wiring (SDA=21, SCL=22)"); }
    else if (!imu_calibrate_gyro()) Serial.println("[IMU] gyro calibration skipped (vehicle moving?)");
    gps_begin();
    gas_begin();
    modem.begin(idleHook);

    eventQueue = xQueueCreate(4, sizeof(DetectedEvent));
    xTaskCreatePinnedToCore(sensorTask, "sensor", 8192, nullptr, 3, nullptr, 0);
}

void loop() {
    gps_poll();
    g_speed_kmh = gps_speed_kmh();
    gas_poll();

    DetectedEvent ev;
    while (xQueueReceive(eventQueue, &ev, 0) == pdTRUE) enqueuePending(ev);

    serviceModem();
    serviceUplink();

    // status LED: solid for 5 s after an event, slow blink while the modem is not ready, else off
    const uint32_t now = millis();
    if (g_lastEventMs && now - g_lastEventMs < 5000) digitalWrite(PIN_STATUS_LED, HIGH);
    else digitalWrite(PIN_STATUS_LED, (!modemReady && (now / 500) % 2) ? HIGH : LOW);

    delay(2);
}
