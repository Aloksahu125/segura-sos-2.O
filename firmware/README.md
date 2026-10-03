# SEGURA - ESP32 edge firmware

Hardware unit for the SEGURA crash-detection system. Reads the sensors, detects a
crash and grades its severity **on the ESP32 itself**, then sends a JSON alert over
**NB-IoT** to the M3 server, which notifies the community app.

```
MPU6050 + NEO-6M + MQ-2 -> ESP32 (200 Hz sampling -> features -> AI model) -> NB-IoT modem -> M3 -> app popup
```

> **Model status: pre-release.** The on-device accident-detection model is under
> development and training. The bundled `model_event.h` / `model_severity.h` are
> placeholders trained on synthetic data, solely to validate the pipeline. They are
> not validated on real crash data and must not be used for real-world safety
> decisions. Every payload carries `"model_status":"placeholder"`.

## Repo layout
```
firmware/
  segura_esp32/            Arduino sketch (open segura_esp32.ino)
    config.h               pins, thresholds, APN, server  <- edit this
    feature_extract.*      10 window features (pure C++)
    edge_model.*           two-stage model interface (crash -> severity)
    model_event.h          generated: normal | hard_brake | crash
    model_severity.h       generated: minor | moderate | severe
    payload.*              JSON builder (pure C++)
    imu_mpu6050.*  gps_neo6m.*  gas_mq2.*  nbiot_sim7020.*
  tools/
    train_placeholder_model.py   trains + exports the placeholder model
    host_test/                   PC-side test: feature parity, model, payload
```

## Wiring
| Part | Pin | ESP32 |
|---|---|---|
| MPU6050 | SDA / SCL | GPIO 21 / 22 |
| NEO-6M | TX -> / RX <- | GPIO 16 (RX2) / 17 (TX2) |
| NB-IoT modem | TXD -> / RXD <- | GPIO 26 (RX1) / 27 (TX1) |
| MQ-2 | AOUT (through a divider) | GPIO 34 (ADC1) |
| Status LED | on-board | GPIO 2 |

Power: MPU6050 and GPS from 3.3 V. **Power the modem from the battery through a
regulator that handles its current bursts - not from the ESP32 3.3 V pin.** Check
whether your modem's UART is 1.8 V or 3.3 V and level-shift if needed. The MQ-2
runs at 5 V and its output can exceed 3.3 V: use a voltage divider. Mounting
orientation of the MPU6050 is not critical (features are orientation-independent),
but fix it rigidly.

## Build & flash (Arduino IDE)
1. Install ESP32 board support; board = **ESP32 Dev Module**.
2. Library Manager: install **TinyGPSPlus** (Mikal Hart). Nothing else is needed.
3. Edit `config.h`: `NBIOT_APN`, `SERVER_HOST`, `SERVER_PATH` (and `SERVER_PORT` /
   `SERVER_USE_HTTPS` if needed).
4. Keep the unit **still for the first second after boot** (gyro calibration).
5. Open the serial monitor at 115200 baud.

## How detection works
- MPU6050 is configured at +-16 g / +-1000 deg/s and sampled at 200 Hz on core 0.
  (Impacts above 16 g are clipped - reported `impact_g` saturates there.)
- Every 100 ms the last 0.5 s window is turned into 10 features
  (`FEATURE_NAMES` in `feature_extract.cpp`) and passed to the model.
- Stage 1 classifies normal / hard_brake / crash. On a crash hit the firmware waits
  300 ms, re-evaluates the full impact window, and only then raises the alert, then
  stays quiet for 15 s (`EVENT_COOLDOWN_MS`).
- Stage 2 grades severity (minor / moderate / severe) for confirmed crashes.
- Sampling/inference runs on its own core, so a slow modem never delays detection.

## Payload (HTTP POST, `application/json`, ~430 bytes)
```json
{"event_id":"segura-car-01-123456-1","device_id":"segura-car-01","timestamp":"2026-10-01T10:20:30Z",
 "uptime_ms":123456,"type":"crash","source":"real",
 "gps":{"lat":22.719568,"lng":75.857727,"accuracy_m":6.2,"fix":true,"age_s":0.4,"location_source":"gps"},
 "speed_kmh":54.2,"impact_g":9.37,"weather":"clear","severity":"moderate","confidence":1.00,
 "gas_alert":false,"model_status":"placeholder","model_version":"0.0.1-synthetic"}
```
- Field names follow the repo's `SensorEvent` (`event_id`, `timestamp`, `type`, `gps`,
  `speed_kmh`, `impact_g`, `weather`, `source`); `device_id`, `severity`, `confidence`,
  `gas_alert`, `model_*` are additions. Adjust `payload.cpp` to match M3's `/sos` body.
- GPS: `location_source` is `gps` (fresh fix), `last_known` (fix lost), `demo_fallback`
  (only if `DEMO_LOCATION_FALLBACK` is enabled) or `none` (lat/lng are `null`).
  The firmware never invents coordinates. `timestamp` is GPS UTC, or `""` without time.
- `weather` is always `"clear"` (no weather sensor).
- Failed sends are retried with backoff using the **same `event_id`**; M3 should
  de-duplicate on it. Up to 4 unsent events are kept in RAM.

## Replacing the placeholder with the real model
1. Train on the same 10 features, in the same order and units (see `features()` in
   `train_placeholder_model.py`, which mirrors `feature_extract.cpp`).
2. Export two classifiers with emlearn (`method="inline"`, `dtype="float"`), named
   `event_model` (classes 0 normal, 1 hard_brake, 2 crash) and `severity_model`
   (0 minor, 1 moderate, 2 severe), into `model_event.h` / `model_severity.h`.
3. Set `MODEL_STATUS` / `MODEL_VERSION` in `edge_model.h`.
Any change to the feature definitions must be made in **both** the firmware and the
training code; `tools/host_test` checks that they agree.

## Tests (PC, no hardware)
```
pip install numpy scikit-learn emlearn
python3 tools/train_placeholder_model.py      # regenerates model headers + parity data
cd tools/host_test && make                    # feature parity, model run, payload check
```
The synthetic hold-out scores printed by the training script are an artifact of the
synthetic generator and say nothing about real-world accuracy.

## Known limitations / to verify on hardware
- The modem driver follows the SIM7020 AT manual and SIMCom HTTP application note but
  is **untested on a real module**. Check the lines marked `VERIFY` (APN command,
  `AT+CHTTPSEND` argument format, response URC, TLS support, command-length limit).
- If your module cannot do HTTPS, or the server only accepts HTTPS, you will need a
  small relay. Free-tier hosts can take ~1 min to wake, hence `HTTP_TIMEOUT_MS`.
- Only the PC-side parts (features, model, payload) were compiled and run; the
  Arduino-side files were syntax-checked against stub headers, not built for ESP32.
- Thresholds/scales in the synthetic data assume vehicle-like dynamics; a toy car will
  behave differently, so retrain on real recordings before trusting any result.
- A crash hit triggers the alert with no cancel step: false positives go straight to M3.
