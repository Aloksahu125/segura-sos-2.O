import { M2_WS_URL, M2_HTTP_URL, M3_HTTP_URL, M3_WS_URL } from '../config.js';
import React, { useEffect, useRef, useState, useCallback } from 'react';
import L from 'leaflet';
import 'leaflet/dist/leaflet.css';

// ── Web Audio SOS alert (no audio file needed) ──────────────────────
let _audioCtx = null;
function getAudioCtx() {
  if (!_audioCtx) _audioCtx = new (window.AudioContext || window.webkitAudioContext)();
  if (_audioCtx.state === 'suspended') _audioCtx.resume();
  return _audioCtx;
}

function playSOSAlert(severity = 'severe') {
  try {
    const ctx  = getAudioCtx();
    const now  = ctx.currentTime;
    // Two-oscillator siren sweep: high-pitch wail + harmonic
    const freqs    = severity === 'severe' ? [880, 1320] : [660, 990];
    const duration = severity === 'severe' ? 1.8 : 1.2;
    const sweeps   = severity === 'severe' ? 3 : 2;

    freqs.forEach((baseFreq, i) => {
      const osc = ctx.createOscillator();
      const amp = ctx.createGain();
      osc.connect(amp);
      amp.connect(ctx.destination);
      osc.type = 'sawtooth';

      // Siren sweep: ramp up then down, repeated
      for (let s = 0; s < sweeps; s++) {
        const t0 = now + s * (duration / sweeps);
        const t1 = t0 + duration / sweeps / 2;
        const t2 = t0 + duration / sweeps;
        osc.frequency.setValueAtTime(baseFreq * 0.7, t0);
        osc.frequency.linearRampToValueAtTime(baseFreq * 1.3, t1);
        osc.frequency.linearRampToValueAtTime(baseFreq * 0.7, t2);
      }

      amp.gain.setValueAtTime(0, now);
      amp.gain.linearRampToValueAtTime(i === 0 ? 0.35 : 0.15, now + 0.05);
      amp.gain.setValueAtTime(i === 0 ? 0.35 : 0.15, now + duration - 0.15);
      amp.gain.linearRampToValueAtTime(0, now + duration);

      osc.start(now);
      osc.stop(now + duration + 0.1);
    });
  } catch (e) {
    console.warn('[SOSAlert]', e.message);
  }
}

// ── Brief red toast for visual+audible pairing ──────────────────────
function showSOSToast(severity = 'severe') {
  const existing = document.getElementById('sos-alert-toast');
  if (existing) existing.remove();
  const toast = document.createElement('div');
  toast.id = 'sos-alert-toast';
  toast.textContent = severity === 'severe'
    ? '🚨 SEVERE CRASH DETECTED — EMERGENCY SOS ACTIVE'
    : '⚠️ Moderate Crash Alert — SOS Dispatched';
  toast.style.cssText = [
    'position:fixed', 'top:70px', 'left:50%', 'transform:translateX(-50%)',
    'z-index:9999', 'background:' + (severity === 'severe' ? '#d71921' : '#f59e0b'),
    'color:#fff', 'font-weight:700', 'font-family:Outfit,sans-serif',
    'font-size:14px', 'letter-spacing:0.5px', 'padding:12px 24px',
    'border-radius:9999px', 'box-shadow:0 8px 32px rgba(0,0,0,0.5)',
    'animation:sos-toast-in 0.3s ease'
  ].join(';');
  // inject keyframe once
  if (!document.getElementById('sos-toast-style')) {
    const st = document.createElement('style');
    st.id = 'sos-toast-style';
    st.textContent = '@keyframes sos-toast-in{from{opacity:0;transform:translateX(-50%) translateY(-12px)}to{opacity:1;transform:translateX(-50%) translateY(0)}}';
    document.head.appendChild(st);
  }
  document.body.appendChild(toast);
  setTimeout(() => toast.remove(), 4000);
}

// Fix Leaflet default icon paths broken by bundlers
delete L.Icon.Default.prototype._getIconUrl;
L.Icon.Default.mergeOptions({
  iconRetinaUrl: 'https://unpkg.com/leaflet@1.9.4/dist/images/marker-icon-2x.png',
  iconUrl:       'https://unpkg.com/leaflet@1.9.4/dist/images/marker-icon.png',
  shadowUrl:     'https://unpkg.com/leaflet@1.9.4/dist/images/marker-shadow.png',
});

// const M2_WS_URL handled by config.js
const M2_HAZARDS_URL = (lat, lng, r) => `${M2_HTTP_URL}/hazards?lat=${lat}&lng=${lng}&radius=${r}`;
const M3_INCIDENTS   = `${M3_HTTP_URL}/incidents`;
// const M3_WS_URL handled by config.js

function createIcon(color) {
  const svg = `<svg xmlns="http://www.w3.org/2000/svg" width="26" height="42" viewBox="0 0 25 41">
    <path d="M12.5 0C5.6 0 0 5.6 0 12.5C0 22 12.5 41 12.5 41S25 22 25 12.5C25 5.6 19.4 0 12.5 0z" fill="${color}" stroke="#ffffff" stroke-width="2"/>
    <circle cx="12.5" cy="12.5" r="5" fill="#ffffff" opacity="0.9"/>
  </svg>`;
  return L.divIcon({
    html:      svg,
    iconSize:  [26, 42],
    iconAnchor:[13, 42],
    popupAnchor:[0, -36],
    className: ''
  });
}

const icons = {
  severe:   createIcon('#d71921'),
  moderate: createIcon('#ff9800'),
  minor:    createIcon('#10b981'),
};

function timeAgo(timestamp) {
  const diff = Math.floor((Date.now() - new Date(timestamp)) / 1000);
  if (diff < 60)  return `${diff}s ago`;
  if (diff < 3600) return `${Math.floor(diff/60)}m ago`;
  return `${Math.floor(diff/3600)}h ago`;
}

const sosDivIcon = L.divIcon({
  html: `<div style="width:22px;height:22px;border-radius:50%;background:#d71921;
    border:3px solid #ffffff;box-shadow:0 0 0 0 rgba(215,25,33,.8);
    animation:sosping 1.2s infinite"></div>
    <style>@keyframes sosping{0%{box-shadow:0 0 0 0 rgba(215,25,33,.8)}70%{box-shadow:0 0 0 16px rgba(215,25,33,0)}100%{box-shadow:0 0 0 0 rgba(215,25,33,0)}}</style>`,
  iconSize:   [24, 24],
  iconAnchor: [12, 12],
  popupAnchor:[0, -14],
  className: ''
});

export default function MapPage() {
  const leafletMap    = useRef(null);
  const userMarker    = useRef(null);
  const hazardLayers  = useRef([]);
  const wsRef         = useRef(null);
  const m3WsRef       = useRef(null);
  const crashMarkers  = useRef([]);  // track all crash/SOS pins for clearing
  const [incidents,   setIncidents]   = useState([]);
  const [sosAlerts,   setSosAlerts]   = useState([]);
  const [wsStatus,    setWsStatus]    = useState('Connecting…');
  const [m3WsStatus,  setM3WsStatus]  = useState('Connecting…');
  const [userPos,     setUserPos]     = useState(null);
  const [filter,      setFilter]      = useState('all');
  const [searchQuery, setSearchQuery] = useState('');
  const [clearing,    setClearing]    = useState(false);

  const pinSosMarker = useCallback((entry) => {
    const p = entry.payload || {};
    const lat = p.gps?.lat ?? p.latitude;
    const lng = p.gps?.lng ?? p.longitude;
    if (!lat || !lng || !leafletMap.current) return;
    const marker = L.marker([lat, lng], { icon: sosDivIcon })
      .addTo(leafletMap.current)
      .bindPopup(`
        <div style="font-family:'Space Grotesk',sans-serif;min-width:170px">
          <b style="color:#d71921">&#x1F6A8; SOS &mdash; ${(p.severity || 'severe').toUpperCase()}</b>
          <div style="font-size:12px;color:#888;margin-top:3px">${timeAgo(entry.timestamp)}</div>
          <hr style="margin:6px 0;border-color:rgba(255,255,255,0.1)">
          <div>&#x1F4CD; ${lat.toFixed(5)}, ${lng.toFixed(5)}</div>
          ${p.speed_kmh != null ? `<div>&#x1F697; ${Math.round(p.speed_kmh)} km/h</div>` : ''}
          ${p.impact_g  != null ? `<div>&#x26A1; ${p.impact_g.toFixed(2)} g</div>` : ''}
        </div>
      `);
    crashMarkers.current.push(marker);
  }, []);

  // Clear everything on 'cleared' WS broadcast or button click
  const clearAll = useCallback(() => {
    setIncidents([]);
    setSosAlerts([]);
    // Remove all crash/SOS markers from map
    crashMarkers.current.forEach(m => m.remove());
    crashMarkers.current = [];
  }, []);

  const clearLogs = useCallback(async () => {
    setClearing(true);
    try {
      await fetch(`${M3_HTTP_URL}/clear-logs`, { method: 'DELETE' });
      clearAll();
    } catch (e) {
      console.error('Clear logs failed:', e);
      // Still clear local state even if server fails
      clearAll();
    } finally {
      setClearing(false);
    }
  }, [clearAll]);


  useEffect(() => {
    if (leafletMap.current) return;

    const map = L.map('leaflet-map').setView([20.5937, 78.9629], 5);
    L.tileLayer('https://{s}.tile.openstreetmap.org/{z}/{x}/{y}.png', {
      attribution: '© OpenStreetMap contributors',
      maxZoom: 19
    }).addTo(map);
    leafletMap.current = map;

    if ('geolocation' in navigator) {
      navigator.geolocation.watchPosition(
        (pos) => {
          const { latitude: lat, longitude: lng } = pos.coords;
          setUserPos({ lat, lng });
          if (!userMarker.current) {
            userMarker.current = L.circleMarker([lat, lng], {
              radius: 9, fillColor: '#ffffff', color: '#d71921',
              weight: 3, fillOpacity: 1
            }).addTo(map).bindPopup('📍 Your Live Telemetry Location');
            map.setView([lat, lng], 13);
          } else {
            userMarker.current.setLatLng([lat, lng]);
          }
        },
        () => {},
        { enableHighAccuracy: true }
      );
    }

    return () => {
      if (leafletMap.current) {
        leafletMap.current.remove();
        leafletMap.current = null;
      }
    };
  }, []);

  const loadIncidents = useCallback(async () => {
    try {
      const res  = await fetch(M3_INCIDENTS);
      const data = await res.json();
      setIncidents(data.data?.slice(0, 20) || []);
      if (leafletMap.current) {
        data.data?.forEach(inc => {
          if (inc.lat && inc.lng) {
            L.marker([inc.lat, inc.lng], { icon: icons[inc.severity] || icons.minor })
              .addTo(leafletMap.current)
              .bindPopup(`<b>${inc.description || 'Incident'}</b><br>${inc.severity || ''}`);
          }
        });
      }
    } catch (_) {
      const cached = localStorage.getItem('incidents_cache');
      if (cached) setIncidents(JSON.parse(cached));
    }
  }, []);

  useEffect(() => { loadIncidents(); }, [loadIncidents]);

  useEffect(() => {
    let alive = true;
    function connect() {
      const ws = new WebSocket(M2_WS_URL);
      wsRef.current = ws;
      ws.onopen  = () => setWsStatus('Live');
      ws.onclose = () => {
        setWsStatus('Offline');
        if (alive) setTimeout(connect, 3000);
      };
      ws.onerror = () => ws.close();

      ws.onmessage = (msg) => {
        try {
          const ev = JSON.parse(msg.data);
          const sev = ev.severity || 'minor';

          // ✅ Only plot on map and alert for ACTUAL CRASHES — ignore normal telemetry
          if (sev !== 'severe' && sev !== 'moderate') return;
          if (!ev.lat || !ev.lng || !leafletMap.current) return;

          const icon = icons[sev] || icons.minor;
          const time = new Date().toISOString();

          const marker = L.marker([ev.lat, ev.lng], { icon })
            .addTo(leafletMap.current)
            .bindPopup(`
              <div style="font-family:'Space Grotesk',sans-serif;min-width:160px">
                <b style="text-transform:uppercase;color:${sev==='severe'?'#d71921':'#ff9800'}">${sev}</b>
                <div style="margin-top:4px;font-size:12px;color:#888">${timeAgo(time)}</div>
                <hr style="margin:6px 0;border-color:rgba(255,255,255,0.1)">
                <div>&#x1F697; ${ev.speed_kmh?.toFixed(0)} km/h</div>
                <div>&#x26A1; ${ev.impact_g?.toFixed(2)} g</div>
              </div>
            `);
          crashMarkers.current.push(marker);

          setIncidents(prev => [{
            id:        ev.event_id || Date.now(),
            timestamp: time,
            severity:  sev,
            speed_kmh: ev.speed_kmh,
            impact_g:  ev.impact_g,
            lat: ev.lat, lng: ev.lng,
            description: ev.alert_message || 'AI-detected crash event'
          }, ...prev].slice(0, 20));

          // 🔊 Alert community users for crash events
          playSOSAlert(sev);
          showSOSToast(sev);

        } catch (_) {}
      };
    }
    connect();
    return () => { alive = false; wsRef.current?.close(); };
  }, []);

  useEffect(() => {
    let alive = true;
    function connectM3() {
      const ws = new WebSocket(M3_WS_URL);
      m3WsRef.current = ws;
      ws.onopen  = () => setM3WsStatus('Live');
      ws.onclose = () => {
        setM3WsStatus('Offline');
        if (alive) setTimeout(connectM3, 3000);
      };
      ws.onerror = () => ws.close();

      ws.onmessage = (msg) => {
        try {
          const packet = JSON.parse(msg.data);
          if (packet.type === 'cleared') {
            clearAll(); // another tab triggered a clear
            return;
          }
          if (packet.type === 'history') {
            const items = packet.data || [];
            items.forEach(entry => pinSosMarker(entry));
            setSosAlerts(prev => [...items, ...prev].slice(0, 20));
          } else if (packet.type === 'sos') {
            const entry = packet.data;
            pinSosMarker(entry);
            setSosAlerts(prev => [entry, ...prev].slice(0, 20));
            // 🔊 Alert community users of incoming SOS
            const sev = (entry.payload?.severity || entry.severity || 'severe');
            playSOSAlert(sev);
            showSOSToast(sev);
          }
        } catch (_) {}
      };
    }
    connectM3();
    return () => { alive = false; m3WsRef.current?.close(); };
  }, [pinSosMarker, clearAll]);

  const filteredIncidents = incidents.filter(inc => {
    if (filter === 'severe') return inc.severity === 'severe';
    if (filter === 'moderate') return inc.severity === 'moderate';
    if (searchQuery.trim()) {
      return (inc.description || '').toLowerCase().includes(searchQuery.toLowerCase());
    }
    return true;
  });

  return (
    <div className="page">
      <div className="page-header">
        <h1>🗺 Community Safety Map</h1>
        <div style={{ display: 'flex', alignItems: 'center', gap: 12 }}>
          <span className={`badge ${m3WsStatus === 'Live' ? 'badge-danger' : 'badge-warning'}`}>
            SOS: {m3WsStatus}
          </span>
          <span className={`badge ${wsStatus === 'Live' ? 'badge-success' : 'badge-warning'}`}>
            M2: {wsStatus}
          </span>
          <button
            id="clear-logs-btn"
            onClick={clearLogs}
            disabled={clearing}
            title="Clear all alerts & incidents (demo reset)"
            style={{
              background: clearing ? 'rgba(255,50,50,0.15)' : 'rgba(215,25,33,0.18)',
              border: '1px solid rgba(215,25,33,0.45)',
              color: '#ff6b6b',
              borderRadius: '8px',
              padding: '6px 14px',
              cursor: clearing ? 'not-allowed' : 'pointer',
              fontSize: '13px',
              fontWeight: '600',
              display: 'flex',
              alignItems: 'center',
              gap: '6px',
              transition: 'all 0.2s',
              fontFamily: 'inherit',
            }}
          >
            {clearing ? '⏳ Clearing…' : '🗑 Clear Logs'}
          </button>
        </div>
      </div>

      <div style={{ padding: '16px 24px 8px', display: 'flex', flexDirection: 'column', gap: '12px' }}>
        <div className="search-box">
          <svg width="18" height="18" viewBox="0 0 24 24" fill="none" stroke="currentColor" strokeWidth="2.5">
            <circle cx="11" cy="11" r="8"></circle>
            <line x1="21" y1="21" x2="16.65" y2="16.65"></line>
          </svg>
          <input
            type="text"
            placeholder="Search hazards, road conditions, location..."
            value={searchQuery}
            onChange={(e) => setSearchQuery(e.target.value)}
          />
        </div>

        <div className="pills-row">
          <button className={`pill ${filter === 'all' ? 'active' : ''}`} onClick={() => setFilter('all')}>
            All Events
          </button>
          <button className={`pill danger ${filter === 'severe' ? 'active' : ''}`} onClick={() => setFilter('severe')}>
            🚨 Severe Crashes
          </button>
          <button className={`pill ${filter === 'moderate' ? 'active' : ''}`} onClick={() => setFilter('moderate')}>
            ⚠ Moderate Hazards
          </button>
        </div>
      </div>

      <div style={{ padding: '0 24px' }}>
        <div className="map-container card" style={{ padding: 0, overflow: 'hidden' }}>
          <div id="leaflet-map" style={{ width: '100%', height: '100%' }} />
        </div>
      </div>

      <div style={{ padding: '24px' }}>
        {sosAlerts.length > 0 && (
          <div style={{ marginBottom: '24px' }}>
            <h3 style={{ color: 'var(--secondary-red)', marginBottom: '12px', display: 'flex', alignItems: 'center', gap: '8px' }}>
              🚨 Active SOS Emergency Feeds ({sosAlerts.length})
            </h3>
            <div style={{ display: 'grid', gap: '10px' }}>
              {sosAlerts.slice(0, 3).map(s => {
                const p = s.payload || {};
                return (
                  <div key={s.id} className="card card-alert" style={{ display: 'flex', justifyContent: 'space-between', alignItems: 'center' }}>
                    <div>
                      <span className="badge badge-danger">
                        🚨 {(p.severity || 'severe').toUpperCase()}
                      </span>
                      <div style={{ fontSize: '13px', color: 'var(--text-muted)', marginTop: '4px' }}>
                        Source: {s.source || p.source || 'Mobile Sensors'}
                      </div>
                    </div>
                    <div style={{ textAlign: 'right' }}>
                      <div className="mono" style={{ fontSize: '15px', fontWeight: '700' }}>
                        {p.speed_kmh != null ? `${Math.round(p.speed_kmh)} km/h` : '0 km/h'}
                      </div>
                      <div style={{ fontSize: '12px', color: 'var(--text-muted)' }}>{timeAgo(s.timestamp)}</div>
                    </div>
                  </div>
                );
              })}
            </div>
          </div>
        )}

        <h3 style={{ marginBottom: '12px' }}>Live Incident Telemetry Feed</h3>
        <div style={{ display: 'grid', gap: '10px' }}>
          {filteredIncidents.length === 0 ? (
            <div className="card" style={{ textAlign: 'center', color: 'var(--text-muted)' }}>
              No incidents match current filter.
            </div>
          ) : (
            filteredIncidents.slice(0, 5).map(inc => (
              <div key={inc.id} className="card" style={{ display: 'flex', justifyContent: 'space-between', alignItems: 'center' }}>
                <div>
                  <span className={`badge ${inc.severity === 'severe' ? 'badge-danger' : inc.severity === 'moderate' ? 'badge-warning' : 'badge-success'}`}>
                    {inc.severity || 'minor'}
                  </span>
                  <div style={{ fontSize: '14px', marginTop: '4px', fontWeight: '500' }}>
                    {inc.description || 'Auto-detected Road Event'}
                  </div>
                </div>
                <div style={{ textAlign: 'right' }}>
                  <div className="mono" style={{ fontSize: '14px', fontWeight: '700' }}>
                    {inc.speed_kmh != null ? `${Math.round(inc.speed_kmh)} km/h` : ''}
                  </div>
                  <div style={{ fontSize: '12px', color: 'var(--text-muted)' }}>{timeAgo(inc.timestamp)}</div>
                </div>
              </div>
            ))
          )}
        </div>
      </div>
    </div>
  );
}
