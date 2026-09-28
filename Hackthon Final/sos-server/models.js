const mongoose = require('mongoose');

// ── SOS Log Schema ─────────────────────────────────────────────
const sosLogSchema = new mongoose.Schema({
    id:                 { type: String, required: true, unique: true },
    timestamp:          { type: Date, default: Date.now },
    status:             { type: String, default: 'sent' },
    source:             { type: String, default: 'm2_ai_engine' },
    payload:            { type: mongoose.Schema.Types.Mixed },
    hospitals:          { type: Array, default: [] },
    golden_hour_start:  { type: Date, default: Date.now }
}, { timestamps: true });

// ── Incident (community report) Schema ────────────────────────
const incidentSchema = new mongoose.Schema({
    id:          { type: String, required: true, unique: true },
    timestamp:   { type: Date, default: Date.now },
    type:        { type: String, default: 'community_report' },
    description: { type: String },
    severity:    { type: String, default: 'minor' },
    lat:         { type: Number },
    lng:         { type: Number },
    reporter:    { type: String, default: 'anonymous' },
    extra:       { type: mongoose.Schema.Types.Mixed }
}, { timestamps: true });

// ── Telemetry Schema (optional, for analytics) ─────────────────
const telemetrySchema = new mongoose.Schema({
    timestamp:  { type: Date, default: Date.now },
    data:       { type: mongoose.Schema.Types.Mixed }
}, { timestamps: true });

const SosLog    = mongoose.model('SosLog',    sosLogSchema);
const Incident  = mongoose.model('Incident',  incidentSchema);
const Telemetry = mongoose.model('Telemetry', telemetrySchema);

module.exports = { SosLog, Incident, Telemetry };
