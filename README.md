
<p align='center'>
    <img src="https://capsule-render.vercel.app/api?type=rect&color=gradient&height=331&section=header&text=%F0%9F%9A%A8%20SEGURA%20SOS&textBg=false&fontColor=black&fontSize=70&animation=fadeIn&fontAlign=47&desc=Real-Time%20Crash%20Detection%20and%20Emergency%20Dispatch!&descSize=31&descAlign=51&descAlignY=65&strokeWidth=5"/>
</p>

<p align="center">
  <i><b>"Saving Lives in the Golden Hour Through Autonomous Crash Intelligence"</b></i>
</p>

<p align="center">
  An intelligent, end-to-end telemetry pipeline combining on-device sensor fusion, neural severity classification, automated telecom dispatch, and hyper-local community situational awareness.
</p>

<p align="center">
  🌐 <a href="https://github.com/piyushlilhare123/segura-sos-2.O#-live-cloud-deployments">Explore Live Deployment</a> •
  ⚡ <a href="https://github.com/piyushlilhare123/segura-sos-2.O#-run-locally-on-your-laptop-in-5-minutes">Quick Local Setup</a> •
  🧠 <a href="https://github.com/piyushlilhare123/segura-sos-2.O#-system-architecture--3d-data-pipeline">System Architecture</a> •
  🔌 <a href="https://github.com/piyushlilhare123/segura-sos-2.O#-iot-edition-esp32-edge-unit">IoT Edition</a> •
  📱 <a href="https://github.com/piyushlilhare123/segura-sos-2.O#-why-segura-sos-real-world-impact">Real World Impact</a>
</p>
<p align="center">🎥 <b>Demo Video:</b> <a href="https://youtu.be/dQHUpz9CD6Q">Watch Segura SOS in action on YouTube</a></p>

## 🔀 Two Editions, One Safety Network

Segura SOS ships in two editions that share the same dispatch and community layers (M3 and M4):

* 📱 **Mobile prototype** — a phone runs the sensor cockpit (M1) and streams telemetry to the M2 AI engine.
* 🔌 **IoT edition** — *the in-vehicle hardware version of the mobile prototype.* A dedicated ESP32 unit (MPU6050 + NEO-6M GPS + MQ-2 gas sensor) reads the sensors, runs the crash-detection model **on the device itself** (edge AI), and sends the alert over **NB-IoT** straight to M3. No phone, no Wi-Fi and no always-on server-side inference are needed in the vehicle. See [IoT Edition: ESP32 Edge Unit](#-iot-edition-esp32-edge-unit).

| | 📱 Mobile prototype | 🔌 IoT edition |
| :--- | :--- | :--- |
| **Sensing** | Phone accelerometer, gyroscope, GPS (M1 PWA) | MPU6050 + NEO-6M + MQ-2 on an ESP32 |
| **Crash detection** | M2 server (FastAPI, heuristic classifier) | On-device model on the ESP32 (crash → severity) |
| **Uplink** | WebSocket over the phone's connection | NB-IoT modem, HTTP POST (JSON) |
| **Dispatch & community** | M3 → M4 | The same M3 → M4 |
| **Cancel window** | 30-second "I'm Safe" countdown | 30-second false positive button |
| **Status** | Live on cloud | In development |

---

## 🌐 Live Cloud Deployments

Every component of Segura SOS is continuously integrated and deployed live on high-availability cloud infrastructure. You can test each module right in your browser right now:

| Module | System Component | Cloud Provider | Status | Live Production Link |
| :--- | :--- | :--- | :---: | :--- |
| **M1** | **Sensor Telemetry Cockpit** (Mobile Web PWA) | Vercel | 🟢 Online | [**Open M1 Simulator**](https://segura-m1-simulator.vercel.app) |
| **M2** | **AI Inference & Risk Engine** (FastAPI) | Render | 🟢 Online | [**Explore M2 API Docs (Swagger)**](https://segura-m2-ai-engine.onrender.com/docs) |
| **M3** | **SOS Emergency Dispatch Command** (Node.js) | Render | 🟢 Online | [**Open M3 Command Center**](https://segura-m3-sos-server.onrender.com) |
| **M4** | **Community Safety Radar & Driver HUD** (React) | Vercel | 🟢 Online | [**Launch M4 Community App**](https://segura-m4-community-app.vercel.app) |
| **M1-IoT** | **ESP32 Edge Unit** (firmware in [`firmware/`](firmware)) | On-vehicle hardware | 🛠️ In development | [**IoT Edition docs**](#-iot-edition-esp32-edge-unit) |

---

## 💡 Why Segura SOS? (Real-World Impact)

In severe vehicular accidents, the difference between survival and fatality is determined by the **"Golden Hour"** — the first 60 minutes following trauma. 

Traditional emergency response fails because:
1. **Victims are often unconscious or trapped**, unable to dial emergency services.
2. **Callers struggle to report exact GPS coordinates** in unfamiliar highways or secluded zones.
3. **Paramedics lack preliminary crash telemetry** (impact force in Gs, collision velocity, roll angles) before reaching the scene.
4. **Nearby drivers remain unaware** of sudden highway pileups ahead, triggering catastrophic secondary collisions.

### 🛡️ How Segura SOS Solves This:
* **Autonomous Zero-Touch Detection:** Using mobile or in-vehicle hardware sensors (accelerometer, gyroscope, GPS), severe crashes are detected instantly without human intervention.
* **Smart 10-Second Abort Window:** Prevents false alarms by giving conscious drivers a responsive countdown buzzer with a single-tap "I'm Safe" cancel button.
* **Instant Automated Telecom Dispatch:** Automatically places real-time telephone voice calls and SMS alerts to emergency dispatchers (via Twilio API) complete with exact coordinates, crash velocity, and impact G-force.
* **Dynamic Community Warning Grid:** Broadcasts sub-second incident alerts and hazard warnings to all nearby vehicles on the M4 Community Safety Radar, preventing secondary collisions.
* **Phone-Independent Edge Unit (IoT edition):** A dedicated in-vehicle ESP32 unit detects crashes on the device and reports over NB-IoT, so detection does not depend on a phone being present, charged or connected.

---

## 🧠 System Architecture & 3D Data Pipeline

### 📱 Mobile prototype pipeline

```
                                  ╔═══════════════════════════════════╗
                                  ║   VEHICLE / DRIVER MOBILE PHONE   ║
                                  ║    M1 Sensor Telemetry Cockpit    ║
                                  ║  • Real Hardware GPS (watchPos)   ║
                                  ║  • 6-Axis Accelerometer (G-Force) ║
                                  ║  • Gyroscopic Tilt & Roll Angles  ║
                                  ║  • Automated Scene Photo Capture  ║
                                  ╚═════════════════╦═════════════════╝
                                                    ║ 
                     Full-Duplex Telemetry Stream   ║ WebSocket (wss://)
                     (60Hz G-Force, Velocity, GPS)  ║
                                                    ▼
                                  ╔═══════════════════════════════════╗
                                  ║       M2 NEURAL AI CORE           ║
                                  ║      FastAPI / Uvicorn (Cloud)    ║
                                  ║ ───────────────────────────────── ║
                                  ║ 1. SeverityClassifier             ║
                                  ║    (Minor | Moderate | Severe)    ║
                                  ║ 2. RiskEngine (0.00 – 1.00 Score) ║
                                  ║ 3. HazardZoneEngine (Blackspots)  ║
                                  ╚═════════════════╦═════════════════╝
                                                    ║ 
                          Autonomous SOS Dispatch   ║ HTTP POST (Webhook)
                        Triggered on High Severity  ║
                                                    ▼
                                  ╔═══════════════════════════════════╗
                                  ║     M3 EMERGENCY COMMAND HUB      ║
                                  ║      Node.js / Express / WSS      ║
                                  ║ ───────────────────────────────── ║
                                  ║ • Twilio Automated Emergency Call ║
                                  ║ • Automated Incident Logging      ║
                                  ║ • Nearest Hospital Radar (OSM API)║
                                  ╚═════════════════╦═════════════════╝
                                                    ║ 
                        Real-Time Incident Beacon   ║ WebSocket Broadcast
                        (Instant Geographic Push)   ║
                                                    ▼
                                  ╔═══════════════════════════════════╗
                                  ║     M4 COMMUNITY SAFETY RADAR     ║
                                  ║        React 18 + Vite (PWA)      ║
                                  ║ ───────────────────────────────── ║
                                  ║ • Live Pulsing Beacon on Map      ║
                                  ║ • Audio Siren Synthesizer         ║
                                  ║ • Driver HUD & Speed Warning      ║
                                  ║ • Crowd-sourced Hazard Reporting  ║
                                  ╚═══════════════════════════════════╝
```

### 🔌 IoT edition pipeline

The IoT edition replaces the phone (M1) and the server-side inference (M2) with an edge unit, and joins the same M3 → M4 chain.

```
                                  ╔═══════════════════════════════════╗
                                  ║   Sensors (IN THE VEHICLE)        ║
                                  ║  • MPU6050  6-axis motion (I2C)   ║
                                  ║  • NEO-6M   GPS position (UART)   ║
                                  ║  • MQ-2     gas / smoke (ADC)     ║
                                  ╚═════════════════╦═════════════════╝
                                                    ║ 
                              200 Hz sampling       ║ I2C / UART / ADC
                                                    ▼
                                  ╔═══════════════════════════════════╗
                                  ║        ESP32 EDGE AI CORE         ║
                                  ║ ───────────────────────────────── ║
                                  ║ 1. 0.5 s window -> 10 features    ║
                                  ║ 2. Stage 1: normal | hard_brake   ║
                                  ║             | crash               ║
                                  ║ 3. Stage 2 (crash only):          ║
                                  ║    minor | moderate | severe      ║
                                  ╚═════════════════╦═════════════════╝
                                                    ║ 
                        Alert only on a confirmed   ║ NB-IoT modem
                        crash (compact JSON)        ║ HTTP POST -> /sos
                                                    ▼
                                  ╔═══════════════════════════════════╗
                                  ║     M3 EMERGENCY COMMAND HUB      ║
                                  ║   (same server as the mobile      ║
                                  ║    prototype - M2 is bypassed)    ║
                                  ╚═════════════════╦═════════════════╝
                                                    ║ 
                        Real-Time Incident Beacon   ║ WebSocket Broadcast
                                                    ▼
                                  ╔═══════════════════════════════════╗
                                  ║     M4 COMMUNITY SAFETY RADAR     ║
                                  ║  popup / beacon on driver screens ║
                                  ╚═══════════════════════════════════╝
```

---

## 📦 The 4 Core Pillars

### 🚗 M1: Sensor Telemetry Cockpit
* **Dual Operational Modes:**
  * **Interactive Simulation Deck:** Allows testing high-speed impacts, hard braking, wet/icy weather conditions, and impact G-forces from any desktop browser.
  * **Real Hardware Mobile Node:** Open the URL on iOS/Android to tap into the device's native accelerometer, gyroscope, and high-accuracy GPS with an on-screen live graph.
* **Smart Camera Integration:** In the event of an impact, the phone's front/back camera captures a photo of the incident scene to assist emergency medical responders.
* **Offline IndexedDB Vault:** Automatically queues telemetry offline if passing through tunnels or dead zones, auto-syncing when signal is restored.
* **IoT variant:** the same sensing role is performed by a dedicated ESP32 unit.

### 🧠 M2: Neural AI Core
* Built on **FastAPI** with asynchronous event processing.
* **Severity Classification:** Multi-variable heuristic model evaluating velocity delta ($\Delta v$), instantaneous G-force spike, and rollover thresholds.
* **Spatial Hazard Blackspot Engine:** Correlates real-time coordinates against high-incident highway zones to adjust vehicle risk scores dynamically.
* **IoT variant:** crash and severity classification run on the ESP32 instead, so the IoT alert goes directly to M3.

### 📡 M3: SOS Dispatch Command Center
* Central dispatch hub built with **Node.js** and **Express**.
* **Twilio Telecom Pipeline:** Instantly dials emergency contacts and first responders with synthesized voice playback announcing exact street coordinates and collision severity.
* **OpenStreetMap Overpass Integration:** Automatically queries hospitals, clinics, and trauma centers within a 10 km radius of the crash site.
* **Live Incident Console:** Provides operators with a responsive dashboard for real-time status monitoring, audio alerts, and photo inspection.

### 🗺️ M4: Community Safety Radar & HUD
* High-performance mobile-first application built with **React 18**, **Vite**, and **Leaflet**.
* **Live Incident Ping:** Connected directly to M3 via WebSockets; sirens wail and an animated pulsating emergency beacon appears the instant a nearby crash occurs.
* **Driver Heads-Up Display (HUD):** Futuristic night-mode HUD displaying real-time speed, live risk indices, and proximity warnings for blackspots.
* **Citizen Reporting:** Empowers commuters to log road hazards, floods, and potholes in two taps.

---

## 🔌 IoT Edition: ESP32 Edge Unit

**This is the IoT (hardware) version of the mobile prototype above.** The unit reads its sensors, detects a crash and grades its severity **on the ESP32 itself**, then sends a JSON alert over **NB-IoT** to the M3 server, which notifies the community app (M4).

```
MPU6050 + NEO-6M + MQ-2 -> ESP32 (200 Hz sampling -> features -> AI model) -> NB-IoT modem -> M3 -> app popup
```

> **Model status: pre-release.** The on-device accident-detection model is under
> development and training. The bundled `model_event.h` / `model_severity.h` are
> placeholders trained on synthetic data, solely to validate the pipeline. They are
> not validated on real crash data.
> Every payload carries `"model_status":"placeholder"`.

### Hardware

| Part | Role |
| :--- | :--- |
| ESP32 DevKit V1 | Runs sampling, feature extraction, the AI model and the uplink |
| MPU6050 | 6-axis accelerometer + gyroscope (I2C) |
| NEO-6M | GPS position, speed and UTC time (UART) |
| MQ-2 | Coarse gas / smoke indicator (analog) |
| NB-IoT module + SIM | Cellular uplink (SIM7020E-style AT interface) |
| Battery + 5 V regulator | Powers the unit |

### Repo layout

The firmware lives in `firmware/`, next to the existing `Hackthon Final/` folder.

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

### Wiring

| Part | Pin | ESP32 |
|---|---|---|
| MPU6050 | SDA / SCL | GPIO 21 / 22 |
| NEO-6M | TX -> / RX <- | GPIO 16 (RX2) / 17 (TX2) |
| NB-IoT modem | TXD -> / RXD <- | GPIO 26 (RX1) / 27 (TX1) |
| MQ-2 | AOUT (through a divider) | GPIO 34 (ADC1) |
| Status LED | on-board | GPIO 2 |

<img width="544" height="596" alt="segura_esp32_hardware_wiring_diagram (1)" src="https://github.com/user-attachments/assets/2ed8feb6-1648-47eb-b2e4-4374fddcaf62" />

Power: MPU6050 and GPS from 3.3 V. **Power the modem from the battery through a
regulator that handles its current bursts - not from the ESP32 3.3 V pin.** The MQ-2
runs at 5 V and its output can exceed 3.3 V: use a voltage divider. Mounting
orientation of the MPU6050 is not critical (features are orientation-independent),
but is fixed rigidly.

### Build & flash (Arduino IDE)
1. Install ESP32 board support; board = **ESP32 Dev Module**.
2. Library Manager: install **TinyGPSPlus** (Mikal Hart). Nothing else is needed.
3. Edit `firmware/segura_esp32/config.h`: `NBIOT_APN`, `SERVER_HOST`, `SERVER_PATH` (and `SERVER_PORT` /
   `SERVER_USE_HTTPS` if needed). Use the deployed M3 host (see the table above) - an
   NB-IoT module cannot reach `localhost`.
4. Keep the unit **still for the first second after boot** (gyro calibration).
5. Open the serial monitor at 115200 baud.

### How detection works
- MPU6050 is configured at +-16 g / +-1000 deg/s and sampled at 200 Hz on core 0.
  (Impacts above 16 g are clipped - reported `impact_g` saturates there.)
- Every 100 ms the last 0.5 s window is turned into 10 features
  (`FEATURE_NAMES` in `feature_extract.cpp`) and passed to the model.
- Stage 1 classifies normal / hard_brake / crash. On a crash hit the firmware waits
  300 ms, re-evaluates the full impact window, and only then raises the alert, then
  stays quiet for 15 s (`EVENT_COOLDOWN_MS`).
- Stage 2 grades severity (minor / moderate / severe) for confirmed crashes.
- Sampling/inference runs on its own core, so a slow modem never delays detection.

### Payload (HTTP POST, `application/json`, ~430 bytes)
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

### Replacing the placeholder with the real model
1. Train on the same 10 features, in the same order and units (see `features()` in
   `train_placeholder_model.py`, which mirrors `feature_extract.cpp`).
2. Export two classifiers with emlearn (`method="inline"`, `dtype="float"`), named
   `event_model` (classes 0 normal, 1 hard_brake, 2 crash) and `severity_model`
   (0 minor, 1 moderate, 2 severe), into `model_event.h` / `model_severity.h`.
3. Set `MODEL_STATUS` / `MODEL_VERSION` in `edge_model.h`.

Any change to the feature definitions must be made in **both** the firmware and the
training code; `tools/host_test` checks that they agree.

### Tests (PC, no hardware)
```bash
pip install numpy scikit-learn emlearn
python3 firmware/tools/train_placeholder_model.py      # regenerates model headers + parity data
cd firmware/tools/host_test && make                    # feature parity, model run, payload check
```
The synthetic hold-out scores printed by the training script are an artifact of the
synthetic generator and say nothing about real-world accuracy.

### IoT demonstration flow (expected)
```
Step 1: Start M3 and M4 (deployed or local) and flash the ESP32 with the M3 host in config.h.
Step 2: Power the unit; wait for "[NB] attached" in the serial monitor.
Step 3: Trigger an impact; the serial monitor prints "[EVT] crash sev=... " and "[TX] ...".
Step 4: M3 receives the POST, dispatches, and M4 shows the beacon at the crash coordinates.
```
This flow has not been field-tested on real hardware yet (see below).

### Known limitations / to verify on hardware
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

---

## ⚡ Run Locally on Your Laptop in 5 Minutes

Want to run the entire Segura SOS system on your own machine? Follow this simple, foolproof guide. *(This runs the mobile prototype stack; for the hardware unit see [IoT Edition](#-iot-edition-esp32-edge-unit).)*

### 📋 Prerequisites
Make sure you have the following installed on your machine:
* [**Node.js (v18 or higher)**](https://nodejs.org/)
* [**Python (v3.10 or higher)**](https://www.python.org/)
* [**Git**](https://git-scm.com/)

---

### 1️⃣ Clone the Repository
```bash
git clone https://github.com/piyushlilhare123/segura-sos-2.O.git
cd segura-sos-2.O
```

---

### 2️⃣ Start Module 2: AI Engine (FastAPI)
Open your **first terminal**:

```bash
# Navigate to M2 directory
cd "Hackthon Final/sensor and detection/m2_ai_engine"

# Create and activate virtual environment (optional but recommended)
python -m venv venv
# Windows:
venv\Scripts\activate
# Mac/Linux:
source venv/bin/activate

# Install dependencies
pip install -r requirements.txt

# Start M2 server
uvicorn main:app --host 0.0.0.0 --port 8000 --reload
```
> ✅ **M2 is running at:** `http://localhost:8000` (Docs: `http://localhost:8000/docs`)

---

### 3️⃣ Start Module 3: SOS Server & Dispatch (Node.js)
Open your **second terminal**:

```bash
# Navigate to M3 directory
cd "Hackthon Final/sos-server"

# Install dependencies
npm install

# Start M3 server
node index.js
```
> ✅ **M3 Command Dashboard is running at:** `http://localhost:4000`

*(Optional: If you have Twilio credentials, set `TWILIO_ACCOUNT_SID`, `TWILIO_AUTH_TOKEN`, `TWILIO_FROM_NUMBER`, and `EMERGENCY_SERVICES_NUMBER` in your environment or `.env` file).*

---

### 4️⃣ Start Module 4: Community Safety App (React + Vite)
Open your **third terminal**:

```bash
# Navigate to M4 directory
cd "Hackthon Final/sensor and detection/m4_community_app"

# Install dependencies
npm install

# Start Vite dev server
npm run dev
```
> ✅ **M4 Community App is running at:** `http://localhost:5173`

---

### 5️⃣ Launch Module 1: Sensor Simulator (Web)
Open your **fourth terminal**:

```bash
# Navigate to M1 directory
cd "Hackthon Final/sensor and detection/m1_web_simulator"

# Run the local HTTPS server (unlocks phone sensor APIs & camera)
python https_server.py
```
> ✅ **M1 Cockpit is running at:** `https://localhost:8443` *(accept the browser security certificate once)*

---

## 🎯 30-Second Live Demonstration Workflow

Experience the entire autonomous response pipeline in 4 quick steps:

```
Step 1: Open M4 Community App (http://localhost:5173) in one browser tab.
        └── You will hear the safety radar initialize with the live map.

Step 2: Open M3 Command Dashboard (http://localhost:4000) in a second tab.
        └── Notice M2 and M4 health statuses showing green "Online ✓".

Step 3: Open M1 Cockpit (https://localhost:8443 or https://segura-m1-simulator.vercel.app).
        └── Click the red button: "🚨 Fire Demo SOS".

Step 4: WATCH THE CHAIN REACTION:
        ├── M1 initiates emergency payload dispatch.
        ├── M2 classifies the crash severity and assigns a risk score.
        ├── M3 sounds the dispatch alarm and queries nearby hospitals.
        └── M4 screen instantly wails with a high-pitched siren and places 
            a pulsating red beacon directly over the crash coordinates!
```

---

## ⚙️ Environment Configuration

### 📱 Mobile prototype (services)

| Variable | Target Module | Description | Default / Example |
| :--- | :--- | :--- | :--- |
| `PORT` | M2 / M3 | Web service listening port | `8000` (M2) / `4000` (M3) |
| `M3_SOS_URL` | M2 | URL where M2 forwards severe incidents | `http://localhost:4000/sos` |
| `TWILIO_ACCOUNT_SID` | M3 | Twilio Account SID for voice calling | `ACxxxxxxxxxxxxxxxxxxxxxxxxxxxx` |
| `TWILIO_AUTH_TOKEN` | M3 | Twilio Auth Token | `xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx` |
| `TWILIO_FROM_NUMBER` | M3 | Twilio authorized outgoing phone number | `+1234567890` |
| `EMERGENCY_SERVICES_NUMBER` | M3 | Dispatch destination phone number | `+919876543210` |
| `VITE_M3_URL` | M4 | Endpoint of M3 server for incidents | `http://localhost:4000` |
| `VITE_M2_URL` | M4 | Endpoint of M2 engine for risk queries | `http://localhost:8000` |

### 🔌 IoT edition (firmware `config.h`)

| Setting | Description | Default / Example |
| :--- | :--- | :--- |
| `DEVICE_ID` | Identifier sent in every payload (any fixed string for the prototype) | `segura-car-01` |
| `NBIOT_APN` | APN of your NB-IoT SIM | `YOUR_NBIOT_APN` |
| `SERVER_HOST` | Public M3 host (NB-IoT cannot reach `localhost`) | `segura-m3-sos-server.onrender.com` |
| `SERVER_PORT` / `SERVER_USE_HTTPS` | M3 port and TLS flag | `443` / `1` |
| `SERVER_PATH` | M3 endpoint for alerts | `/sos` |
| `MODEL_CRASH_MIN_CONF` | Minimum model confidence to accept a crash | `0.60` |
| `EVENT_COOLDOWN_MS` | Quiet time after an alert | `15000` |
| `SEND_HARD_BRAKE_EVENTS` | `1` also uplinks hard-brake events | `0` |
| `DEMO_LOCATION_FALLBACK` | `1` sends fixed demo coordinates when there is no GPS fix | `0` |

---

## 🛠️ Technology Stack Breakdown

* **Edge & Sensory Input:** HTML5 Geolocation API, DeviceMotionEvent, DeviceOrientationEvent, MediaDevices API (Webcam).
* **Intelligence Layer:** Python 3.11, FastAPI, Uvicorn, Pydantic v2, Scikit-Learn heuristics.
* **Dispatch & Telecom:** Node.js, Express, WebSocket (`ws`), Twilio Voice/SMS API, OpenStreetMap Overpass API.
* **Client Frontend:** React 18, Vite, Leaflet.js, OpenStreetMap CartoDB Dark Matter, CSS Glassmorphism.
* **Cloud Infrastructure:** Render (Web Services), Vercel (Edge Static Hosting), Git/GitHub CI/CD.
* **Embedded / IoT Edge:** ESP32 (Arduino-ESP32, FreeRTOS), MPU6050, NEO-6M (TinyGPSPlus), MQ-2, NB-IoT module (SIM7020E-style AT commands, HTTP POST), scikit-learn → emlearn for on-device C inference.

---

## 👥 Contributors & Acknowledgements

Developed with ❤️ for real-world road safety.
* **Repository:** [https://github.com/piyushlilhare123/segura-sos-2.O](https://github.com/piyushlilhare123/segura-sos-2.O)
* **License:** Open-source under the [MIT License](LICENSE).

<div align="center">
<b>Built to protect commuters. Engineered to empower first responders.</b><br/>
<i>Because every single second matters.</i>
</div>
