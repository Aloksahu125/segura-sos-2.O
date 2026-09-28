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
  📱 <a href="https://github.com/piyushlilhare123/segura-sos-2.O#-why-segura-sos-real-world-impact">Real World Impact</a>
</p>

## 🌐 Live Cloud Deployments

Every component of Segura SOS is continuously integrated and deployed live on high-availability cloud infrastructure. You can test each module right in your browser right now:

| Module | System Component | Cloud Provider | Status | Live Production Link |
| :--- | :--- | :--- | :---: | :--- |
| **M1** | **Sensor Telemetry Cockpit** (Mobile Web PWA) | Vercel | 🟢 Online | [**Open M1 Simulator**](https://segura-m1-simulator.vercel.app) |
| **M2** | **AI Inference & Risk Engine** (FastAPI) | Render | 🟢 Online | [**Explore M2 API Docs (Swagger)**](https://segura-m2-ai-engine.onrender.com/docs) |
| **M3** | **SOS Emergency Dispatch Command** (Node.js) | Render | 🟢 Online | [**Open M3 Command Center**](https://segura-m3-sos-server.onrender.com) |
| **M4** | **Community Safety Radar & Driver HUD** (React) | Vercel | 🟢 Online | [**Launch M4 Community App**](https://segura-m4-community-app.vercel.app) |

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

---

## 🧠 System Architecture & 3D Data Pipeline

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

---

## 📦 The 4 Core Pillars

### 🚗 M1: Sensor Telemetry Cockpit
* **Dual Operational Modes:**
  * **Interactive Simulation Deck:** Allows testing high-speed impacts, hard braking, wet/icy weather conditions, and impact G-forces from any desktop browser.
  * **Real Hardware Mobile Node:** Open the URL on iOS/Android to tap into the device's native accelerometer, gyroscope, and high-accuracy GPS with an on-screen live graph.
* **Smart Camera Integration:** In the event of an impact, the phone's front/back camera captures a photo of the incident scene to assist emergency medical responders.
* **Offline IndexedDB Vault:** Automatically queues telemetry offline if passing through tunnels or dead zones, auto-syncing when signal is restored.

### 🧠 M2: Neural AI Core
* Built on **FastAPI** with asynchronous event processing.
* **Severity Classification:** Multi-variable heuristic model evaluating velocity delta ($\Delta v$), instantaneous G-force spike, and rollover thresholds.
* **Spatial Hazard Blackspot Engine:** Correlates real-time coordinates against high-incident highway zones to adjust vehicle risk scores dynamically.

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

## ⚡ Run Locally on Your Laptop in 5 Minutes

Want to run the entire Segura SOS system on your own machine? Follow this simple, foolproof guide.

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

---

## 🛠️ Technology Stack Breakdown

* **Edge & Sensory Input:** HTML5 Geolocation API, DeviceMotionEvent, DeviceOrientationEvent, MediaDevices API (Webcam).
* **Intelligence Layer:** Python 3.11, FastAPI, Uvicorn, Pydantic v2, Scikit-Learn heuristics.
* **Dispatch & Telecom:** Node.js, Express, WebSocket (`ws`), Twilio Voice/SMS API, OpenStreetMap Overpass API.
* **Client Frontend:** React 18, Vite, Leaflet.js, OpenStreetMap CartoDB Dark Matter, CSS Glassmorphism.
* **Cloud Infrastructure:** Render (Web Services), Vercel (Edge Static Hosting), Git/GitHub CI/CD.

---

## 👥 Contributors & Acknowledgements

Developed with ❤️ for real-world road safety.
* **Repository:** [https://github.com/piyushlilhare123/segura-sos-2.O](https://github.com/piyushlilhare123/segura-sos-2.O)
* **License:** Open-source under the [MIT License](LICENSE).

<div align="center">
<b>Built to protect commuters. Engineered to empower first responders.</b><br/>
<i>Because every single second matters.</i>
</div>
