// ============================================================================
//  SEGURA ESP32 firmware - configuration
//  Edit the values marked  <-- EDIT  before flashing.
//  This file contains only macros so it is also usable by the host-side tests.
// ============================================================================
#pragma once

// ---- Identity (prototype: any fixed string is fine) -------------------------
#define DEVICE_ID              "segura-car-01"

// ---- Pin map (ESP32 DevKit V1, 30-pin) --------------------------------------
#define PIN_I2C_SDA            21   // MPU6050 SDA
#define PIN_I2C_SCL            22   // MPU6050 SCL
#define PIN_GPS_RX             16   // ESP32 RX2  <- NEO-6M TX
#define PIN_GPS_TX             17   // ESP32 TX2  -> NEO-6M RX
#define PIN_MODEM_RX           26   // ESP32 RX1  <- NB-IoT modem TXD
#define PIN_MODEM_TX           27   // ESP32 TX1  -> NB-IoT modem RXD
#define PIN_MODEM_PWRKEY       -1   // modem PWRKEY pin, -1 = not wired
#define PIN_MQ2_AOUT           34   // MQ-2 analog out via voltage divider (ADC1)
#define PIN_STATUS_LED         2    // on-board LED

// ---- Sampling / inference window --------------------------------------------
#define SAMPLE_RATE_HZ         200
#define SAMPLE_PERIOD_MS       (1000 / SAMPLE_RATE_HZ)
#define WINDOW_SAMPLES         100  // 0.5 s window fed to the model
#define INFER_STRIDE_SAMPLES   20   // run the model every 100 ms
#define CONFIRM_DELAY_SAMPLES  60   // re-evaluate 300 ms after a first crash hit
                                    // (must satisfy: STRIDE + CONFIRM < WINDOW)

// ---- Model decision policy ---------------------------------------------------
// Minimum model confidence (fraction of trees voting "crash") to accept a crash.
#define MODEL_CRASH_MIN_CONF   0.60f
#define EVENT_COOLDOWN_MS      15000UL   // ignore new events for this long after one
#define SEND_HARD_BRAKE_EVENTS 0         // 1 = also uplink hard-brake events

// ---- NB-IoT modem (SIM7020E-style AT interface) ------------------------------
#define MODEM_BAUD             115200
#define NBIOT_APN              "YOUR_NBIOT_APN"   // <-- EDIT (from your SIM operator)
#define NET_ATTACH_TIMEOUT_MS  180000UL
#define HTTP_TIMEOUT_MS        70000UL   // generous: free-tier hosts can cold-start
#define SEND_BACKOFF_MIN_MS    5000UL
#define SEND_BACKOFF_MAX_MS    60000UL

// ---- M3 server (HTTP POST, JSON) ---------------------------------------------
#define SERVER_HOST            "YOUR-M3-SERVER.onrender.com"   // <-- EDIT
#define SERVER_PORT            443
#define SERVER_USE_HTTPS       1
#define SERVER_PATH            "/sos"                          // <-- EDIT if M3 differs

// ---- Location fallback (demo only) -------------------------------------------
// With no GPS fix ever acquired (e.g. indoors) the payload carries null
// coordinates. For an indoor demo you can set this to 1 to send the fixed
// coordinates below instead; they are labelled "demo_fallback" in the payload.
#define DEMO_LOCATION_FALLBACK 0
#define DEMO_LAT               0.0      // <-- EDIT if fallback enabled
#define DEMO_LNG               0.0

// ---- MQ-2 gas sensor ----------------------------------------------------------
#define GAS_WARMUP_MS          60000UL  // baseline is captured at the end of warm-up
#define GAS_ALERT_DELTA_RAW    600      // ADC counts above baseline => gas_alert
