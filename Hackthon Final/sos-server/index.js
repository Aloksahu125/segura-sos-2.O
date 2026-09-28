require('dotenv').config();
const express = require('express');
const cors = require('cors');
const twilio = require('twilio');
const http = require('http');
const path = require('path');
const { WebSocketServer } = require('ws');
const https = require('https');
const mongoose = require('mongoose');
const { SosLog, Incident, Telemetry } = require('./models');

const app = express();
const server = http.createServer(app);
const PORT = process.env.PORT || 4000;

app.use(cors());
app.use(express.json({ limit: '5mb' }));
app.use(express.static(path.join(__dirname, 'public')));

// ── MongoDB Connection ─────────────────────────────────────────
const MONGODB_URI = process.env.MONGODB_URI || '';
let dbConnected = false;

if (MONGODB_URI) {
    mongoose.connect(MONGODB_URI)
        .then(() => {
            dbConnected = true;
            console.log('✅ [MongoDB] Connected to Atlas');
        })
        .catch(err => console.error('❌ [MongoDB] Connection failed:', err.message));
} else {
    console.warn('⚠️  [MongoDB] MONGODB_URI not set — falling back to in-memory arrays');
}

// ── Fallback In-Memory Stores (used if DB not connected) ──────
const _memSosLogs    = [];
const _memIncidents  = [];

// ── Twilio Configuration ───────────────────────────────────────
const TWILIO_ACCOUNT_SID        = process.env.TWILIO_ACCOUNT_SID        || '';
const TWILIO_AUTH_TOKEN         = process.env.TWILIO_AUTH_TOKEN         || '';
const TWILIO_FROM_NUMBER        = process.env.TWILIO_FROM_NUMBER        || '';
const EMERGENCY_SERVICES_NUMBER = process.env.EMERGENCY_SERVICES_NUMBER || '';

let twilioClient;
try {
    if (TWILIO_ACCOUNT_SID.startsWith('AC') && TWILIO_ACCOUNT_SID !== 'AC_dummy_account_sid') {
        twilioClient = twilio(TWILIO_ACCOUNT_SID, TWILIO_AUTH_TOKEN);
    }
} catch (error) {
    console.error('Failed to initialize Twilio client:', error.message);
}

// ── WebSocket Server (M3 → M4 real-time SOS broadcast) ────────
const wss = new WebSocketServer({ server });
const wsClients = new Set();

wss.on('connection', async (ws) => {
    wsClients.add(ws);
    console.log(`[M3 WS] Client connected (total: ${wsClients.size})`);
    ws.on('close', () => {
        wsClients.delete(ws);
        console.log(`[M3 WS] Client disconnected (total: ${wsClients.size})`);
    });
    ws.on('error', (err) => console.error('[M3 WS] Error:', err.message));

    // Send last 5 SOS on connect so M4 has initial state
    try {
        const recent = dbConnected
            ? await SosLog.find().sort({ timestamp: -1 }).limit(5).lean()
            : _memSosLogs.slice(0, 5);
        ws.send(JSON.stringify({ type: 'history', data: recent }));
    } catch (e) {
        ws.send(JSON.stringify({ type: 'history', data: [] }));
    }
});

function broadcastSOS(entry) {
    const msg = JSON.stringify({ type: 'sos', data: entry });
    wsClients.forEach(ws => {
        if (ws.readyState === ws.OPEN) ws.send(msg);
    });
}

let lastTwilioCallTime = 0;
const TWILIO_COOLDOWN_MS = 60000;

// ── Haversine Distance (km) ───────────────────────────────────
function haversineKm(lat1, lon1, lat2, lon2) {
    const R = 6371;
    const toRad = x => x * Math.PI / 180;
    const dLat = toRad(lat2 - lat1);
    const dLon = toRad(lon2 - lon1);
    const a = Math.sin(dLat/2)**2 + Math.cos(toRad(lat1)) * Math.cos(toRad(lat2)) * Math.sin(dLon/2)**2;
    return R * 2 * Math.atan2(Math.sqrt(a), Math.sqrt(1 - a));
}

// ── Nearest Hospital Finder (OpenStreetMap Overpass API) ──────
async function findNearbyHospitals(lat, lng, radiusMeters = 10000) {
    const query = `[out:json][timeout:10];(
      node["amenity"="hospital"](around:${radiusMeters},${lat},${lng});
      way["amenity"="hospital"](around:${radiusMeters},${lat},${lng});
      relation["amenity"="hospital"](around:${radiusMeters},${lat},${lng});
    );out center body;`;

    const postData = `data=${encodeURIComponent(query)}`;
    const options = {
        hostname: 'overpass-api.de',
        port: 443,
        path: '/api/interpreter',
        method: 'POST',
        timeout: 10000,
        headers: {
            'Content-Type': 'application/x-www-form-urlencoded',
            'Content-Length': Buffer.byteLength(postData),
            'User-Agent': 'SeguraSOS/1.0'
        }
    };

    return new Promise((resolve) => {
        const req = https.request(options, (res) => {
            let data = '';
            res.on('data', chunk => data += chunk);
            res.on('end', () => {
                try {
                    const parsed = JSON.parse(data);
                    const hospitals = (parsed.elements || []).slice(0, 5).map(el => {
                        const elLat = el.lat ?? el.center?.lat;
                        const elLon = el.lon ?? el.center?.lon;
                        return {
                            name:     el.tags?.name || 'Hospital',
                            type:     el.tags?.amenity || 'hospital',
                            lat:      elLat,
                            lng:      elLon,
                            distance: (elLat && elLon) ? haversineKm(lat, lng, elLat, elLon).toFixed(2) : null,
                            address:  el.tags?.['addr:full'] || el.tags?.['addr:street'] || null
                        };
                    }).filter(h => h.lat && h.lng)
                      .sort((a, b) => parseFloat(a.distance) - parseFloat(b.distance));
                    resolve(hospitals);
                } catch {
                    resolve([]);
                }
            });
        });
        req.on('error',   () => resolve([]));
        req.on('timeout', () => { req.destroy(); resolve([]); });
        req.write(postData);
        req.end();
    });
}

// ── Twilio Emergency Alert ─────────────────────────────────────
async function sendTwilioAlert(payload) {
    if (!twilioClient) return;
    const now = Date.now();
    if (now - lastTwilioCallTime < TWILIO_COOLDOWN_MS) return;
    lastTwilioCallTime = now;

    try {
        const lat = payload.gps?.lat ?? payload.latitude ?? 'unknown';
        const lng = payload.gps?.lng ?? payload.longitude ?? 'unknown';
        const severity = (payload.severity || 'severe').toUpperCase();

        const twiml = `<?xml version="1.0" encoding="UTF-8"?>
<Response>
  <Say voice="alice" language="en-IN">
    Alert! Alert! This is an automated emergency call from Segura S O S.
    A ${severity} crash has been detected.
    Location: Latitude ${lat}, Longitude ${lng}.
    A driver needs immediate assistance. Please respond immediately.
    Repeating. A ${severity} crash has been detected at Latitude ${lat}, Longitude ${lng}.
    This was an automated alert from Segura S O S.
  </Say>
</Response>`;

        await twilioClient.calls.create({
            twiml: twiml,
            from: TWILIO_FROM_NUMBER,
            to: EMERGENCY_SERVICES_NUMBER
        });
        console.log(`[Twilio CALL] Emergency call dispatched to ${EMERGENCY_SERVICES_NUMBER}`);
    } catch (error) {
        console.error('[Twilio CALL] Error placing call:', error.message);
    }
}

// ── Helper: Save SOS to DB or Memory ──────────────────────────
async function saveSosLog(entry) {
    if (dbConnected) {
        try {
            await SosLog.create(entry);
        } catch (e) {
            // Duplicate id — update instead
            if (e.code === 11000) {
                await SosLog.findOneAndUpdate({ id: entry.id }, entry, { upsert: true });
            } else {
                console.error('[MongoDB] SosLog save error:', e.message);
            }
        }
    } else {
        _memSosLogs.unshift(entry);
        if (_memSosLogs.length > 100) _memSosLogs.pop();
    }
}

// ── POST /sos  (M2 AI Engine + web/React PWA clients) ─────────
app.post('/sos', async (req, res) => {
    console.log('[SOS /sos RECEIVED]', req.body);
    const lat = req.body.gps?.lat ?? req.body.latitude;
    const lng = req.body.gps?.lng ?? req.body.longitude;

    let hospitals = [];
    if (lat && lng) hospitals = await findNearbyHospitals(lat, lng);

    const entry = {
        id:                req.body.sos_id || Date.now().toString(),
        timestamp:         new Date(),
        status:            'sent',
        source:            req.body.source || 'm2_ai_engine',
        payload:           req.body,
        hospitals:         hospitals,
        golden_hour_start: new Date()
    };

    await saveSosLog(entry);
    broadcastSOS(entry);
    sendTwilioAlert(req.body);

    res.status(200).json({ success: true, received: true, message: 'SOS dispatched to emergency responders.', hospitals });
});

// ── POST /demo/trigger-sos  (Emergency test trigger button) ──────────
app.post('/demo/trigger-sos', async (req, res) => {
    const locations = [
        { lat: 28.6139, lng: 77.2090, city: 'New Delhi' },
        { lat: 19.0760, lng: 72.8777, city: 'Mumbai' },
        { lat: 12.9716, lng: 77.5946, city: 'Bengaluru' },
        { lat: 13.0827, lng: 80.2707, city: 'Chennai' },
        { lat: 22.5726, lng: 88.3639, city: 'Kolkata' },
    ];
    const loc = locations[Math.floor(Math.random() * locations.length)];
    const sosId = `demo-${Date.now()}`;

    const demoPayload = {
        sos_id:    sosId,
        timestamp: new Date().toISOString(),
        gps:       { lat: loc.lat + (Math.random() - 0.5) * 0.05, lng: loc.lng + (Math.random() - 0.5) * 0.05, accuracy_m: 5 },
        severity:  'severe',
        speed_kmh: Math.floor(Math.random() * 40 + 70),
        impact_g:  parseFloat((Math.random() * 2 + 3.5).toFixed(2)),
        source:    'demo_trigger',
        city:      loc.city
    };

    const hospitals = await findNearbyHospitals(loc.lat, loc.lng);

    const entry = {
        id:                sosId,
        timestamp:         new Date(),
        status:            'sent',
        source:            'demo_trigger',
        payload:           demoPayload,
        hospitals:         hospitals,
        golden_hour_start: new Date()
    };

    await saveSosLog(entry);
    broadcastSOS(entry);
    sendTwilioAlert(demoPayload);

    console.log(`[DEMO SOS] Triggered: ${demoPayload.severity} crash near ${loc.city} | ${hospitals.length} hospitals found`);
    res.status(200).json({ success: true, sos_id: sosId, location: loc.city, payload: demoPayload, hospitals });
});

// ── POST /api/v1/sos  (legacy / external clients) ──────────────
app.post('/api/v1/sos', async (req, res) => {
    console.log('[SOS /api/v1/sos RECEIVED]', req.body);
    const lat = req.body.gps?.lat ?? req.body.latitude;
    const lng = req.body.gps?.lng ?? req.body.longitude;

    let hospitals = [];
    if (lat && lng) hospitals = await findNearbyHospitals(lat, lng);

    const entry = {
        id:                req.body.recordId || req.body.sos_id || Date.now().toString(),
        timestamp:         new Date(),
        status:            'sent',
        source:            'android',
        payload:           req.body,
        hospitals:         hospitals,
        golden_hour_start: new Date()
    };

    await saveSosLog(entry);
    broadcastSOS(entry);
    sendTwilioAlert(req.body);

    res.status(200).json({ success: true, received: true, message: 'SOS dispatched to emergency responders.', hospitals });
});

// ── POST /api/v1/telemetry (M1 Android Telemetry) ─────────────
app.post('/api/v1/telemetry', async (req, res) => {
    const entry = { timestamp: new Date(), data: req.body };
    if (dbConnected) {
        try { await Telemetry.create(entry); } catch (e) {}
    }
    res.status(200).json({ received: true });
});

// ── GET /sos-logs  (SOS history) ──────────────────────────────
app.get('/sos-logs', async (req, res) => {
    try {
        const logs = dbConnected
            ? await SosLog.find().sort({ timestamp: -1 }).limit(20).lean()
            : _memSosLogs.slice(0, 20);
        res.status(200).json({ status: 'success', count: logs.length, data: logs });
    } catch (e) {
        res.status(500).json({ status: 'error', message: e.message });
    }
});

// ── POST /incident  (M4 community report form) ────────────────
app.post('/incident', async (req, res) => {
    console.log('[INCIDENT REPORTED]', req.body);
    const newIncident = {
        id:        (typeof crypto !== 'undefined' && crypto.randomUUID) ? crypto.randomUUID() : Date.now().toString(),
        timestamp: new Date(),
        ...req.body
    };

    if (dbConnected) {
        try { await Incident.create(newIncident); } catch (e) {
            console.error('[MongoDB] Incident save error:', e.message);
        }
    } else {
        _memIncidents.unshift(newIncident);
        if (_memIncidents.length > 200) _memIncidents.pop();
    }

    res.status(201).json({ success: true, incident: newIncident });
});

// ── GET /incidents  (M4 map & community feed) ──────────────────
app.get('/incidents', async (req, res) => {
    try {
        const data = dbConnected
            ? await Incident.find().sort({ timestamp: -1 }).limit(100).lean()
            : _memIncidents;
        res.status(200).json({ status: 'success', count: data.length, data });
    } catch (e) {
        res.status(500).json({ status: 'error', message: e.message });
    }
});

// ── DELETE /clear-logs  (🗑 Demo reset — clears all logs & incidents) ────
app.delete('/clear-logs', async (req, res) => {
    console.log('[CLEAR LOGS] Wiping all SOS logs, incidents and telemetry...');
    try {
        if (dbConnected) {
            await Promise.all([
                SosLog.deleteMany({}),
                Incident.deleteMany({}),
                Telemetry.deleteMany({})
            ]);
        }
        // Also clear in-memory fallback
        _memSosLogs.length    = 0;
        _memIncidents.length  = 0;

        // Broadcast 'cleared' event so all connected M4 clients reset their UI
        const msg = JSON.stringify({ type: 'cleared' });
        wsClients.forEach(ws => { if (ws.readyState === ws.OPEN) ws.send(msg); });

        console.log('[CLEAR LOGS] Done — all stores wiped.');
        res.status(200).json({ success: true, message: 'All logs and incidents cleared.' });
    } catch (e) {
        console.error('[CLEAR LOGS] Error:', e.message);
        res.status(500).json({ success: false, message: e.message });
    }
});

// ── GET /api/nearby-hospitals ─────────────────────────────────
app.get('/api/nearby-hospitals', async (req, res) => {
    const { lat, lng, radius } = req.query;
    if (!lat || !lng) return res.status(400).json({ error: 'lat and lng are required' });
    const hospitals = await findNearbyHospitals(parseFloat(lat), parseFloat(lng), parseInt(radius) || 10000);
    res.status(200).json({ status: 'success', count: hospitals.length, hospitals });
});

// ── GET /health ───────────────────────────────────────────────
app.get('/health', (req, res) => {
    res.status(200).json({
        status:   'ok',
        module:   'M3 SOS Server',
        port:     PORT,
        database: dbConnected ? 'MongoDB Atlas' : 'In-Memory (no MONGODB_URI set)'
    });
});

// ── Start ─────────────────────────────────────────────────────
server.listen(PORT, () => {
    console.log(`
${'─'.repeat(60)}
  Segura SOS — M3 SOS Server  http://localhost:${PORT}
${'─'.repeat(60)}`);
    console.log(`  POST /sos             → M2 AI Engine SOS dispatch`);
    console.log(`  POST /demo/trigger-sos→ 🎯 Emergency test trigger`);
    console.log(`  GET  /sos-logs        → SOS history (last 20)`);
    console.log(`  POST /incident        → Community report`);
    console.log(`  GET  /incidents       → All incidents (last 100)`);
    console.log(`  GET  /health          → Health check`);
    console.log('─'.repeat(60));
    if (!twilioClient) {
        console.log(`  ⚠  Twilio: set env vars for live SMS/calls.`);
    } else {
        console.log(`  ✓  Twilio configured — emergency SMS/call active.`);
    }
    console.log(`  🗄  Database: ${dbConnected ? 'MongoDB Atlas ✅' : 'In-Memory ⚠️  (set MONGODB_URI)'}`);
    console.log('─'.repeat(60) + '\n');
});
