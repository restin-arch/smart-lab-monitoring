/**
 * Vercel Serverless API Route: /api/sensor-data
 * Ingests & Retrieves Telemetry from MongoDB Atlas Cloud Database
 */

const mongoose = require('mongoose');

const MONGO_URI = process.env.MONGO_URI || "mongodb+srv://restinsunny_db_user:222610restin@restin.e5s9d1y.mongodb.net/smart_lab_monitoring?retryWrites=true&w=majority";

// Cached Database Connection for Vercel Serverless Functions
let cachedDb = null;

async function connectToDatabase() {
  if (cachedDb && mongoose.connection.readyState === 1) {
    return cachedDb;
  }
  cachedDb = await mongoose.connect(MONGO_URI, {
    bufferCommands: false,
    serverSelectionTimeoutMS: 5000
  });
  return cachedDb;
}

// Schema for smart_lab_telemetry collection
const telemetrySchema = new mongoose.Schema({
  device:         { type: String, required: true, default: "LAB-NODE-01" },
  temperature:    { type: Number, required: true },
  humidity:       { type: Number, required: true },
  gasRaw:         { type: Number, required: true },
  gasPercent:     { type: Number, required: true },
  gasDetected:    { type: Boolean, default: false },
  distanceCm:     { type: Number, required: true },
  proximityAlert: { type: Boolean, default: false },
  motion:         { type: Boolean, required: true },
  tamper:         { type: Boolean, required: true },
  ax:             { type: Number, required: true },
  ay:             { type: Number, required: true },
  az:             { type: Number, required: true },
  wifiRssi:       { type: Number },
  alerts:         [String],
  receivedAt:     { type: Date, default: Date.now }
}, { collection: 'smart_lab_telemetry' });

const TelemetryRecord = mongoose.models.TelemetryRecord || mongoose.model('TelemetryRecord', telemetrySchema);

module.exports = async (req, res) => {
  // CORS Headers
  res.setHeader('Access-Control-Allow-Credentials', true);
  res.setHeader('Access-Control-Allow-Origin', '*');
  res.setHeader('Access-Control-Allow-Methods', 'GET,OPTIONS,PATCH,DELETE,POST,PUT');
  res.setHeader(
    'Access-Control-Allow-Headers',
    'X-CSRF-Token, X-Requested-With, Accept, Accept-Version, Content-Length, Content-MD5, Content-Type, Date, X-Api-Version'
  );

  if (req.method === 'OPTIONS') {
    res.status(200).end();
    return;
  }

  try {
    await connectToDatabase();

    if (req.method === 'POST') {
      const newRecord = new TelemetryRecord(req.body);
      await newRecord.save();
      return res.status(201).json({
        status: "success",
        message: "Telemetry inserted into MongoDB Atlas",
        recordId: newRecord._id,
        timestamp: newRecord.receivedAt
      });
    }

    if (req.method === 'GET') {
      const limit = parseInt(req.query.limit) || 50;
      const records = await TelemetryRecord.find().sort({ receivedAt: -1 }).limit(limit);
      return res.status(200).json({
        database: "smart_lab_monitoring",
        collection: "smart_lab_telemetry",
        count: records.length,
        data: records
      });
    }

    res.status(405).json({ error: "Method Not Allowed" });
  } catch (error) {
    console.error("Vercel API Error:", error);
    res.status(500).json({ error: error.message });
  }
};
