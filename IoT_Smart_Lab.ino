/*
 * =====================================================================================
 * Project Title : IoT-Based Smart Laboratory Safety, Security & Environmental Monitoring
 * Target Hardware: ESP8266 NodeMCU (ESP-12E Module)
 * 
 * Hardware Sensors Connected:
 *   1. DHT11 Sensor        -> Temperature & Humidity (Pin D5 / GPIO14)
 *   2. PIR Motion Sensor   -> Intrusion & Security Detection (Pin D6 / GPIO12)
 *   3. MPU6050 6-Axis IMU  -> Vibration, Seismic & Tamper Detection (I2C SDA: D2/GPIO4, SCL: D1/GPIO5)
 *   4. MQ-2 Gas/Smoke      -> Combustible Gas & Smoke Leak Detection (Pin A0 / ADC0)
 *   5. HC-SR04 Ultrasonic  -> Distance & Proximity Perimeter Guard (Trig: D7/GPIO13, Echo: D8/GPIO15)
 * =====================================================================================
 */

#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <ESP8266HTTPClient.h>
#include <WiFiClient.h>
#include <ArduinoJson.h>
#include <Wire.h>
#include <DHT.h>
#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>

// =====================================================================================
// 1. HARDWARE PIN DEFINITIONS & SAFETY THRESHOLDS
// =====================================================================================
#define I2C_SDA_PIN          4   // D2 (GPIO4) - MPU6050 SDA
#define I2C_SCL_PIN          5   // D1 (GPIO5) - MPU6050 SCL

#define DHT_PIN              14  // D5 (GPIO14) - DHT11 Data Pin
#define PIR_PIN              12  // D6 (GPIO12) - PIR Digital Output
#define TRIG_PIN             13  // D7 (GPIO13) - HC-SR04 Trigger Output Pin
#define ECHO_PIN             15  // D8 (GPIO15) - HC-SR04 Echo Input Pin
#define MQ2_AO_PIN           A0  // A0 (ADC0)   - MQ-2 Gas Sensor Analog Output

#define DHT_TYPE             DHT11

// Alert Threshold Definitions
#define TEMP_THRESHOLD_HIGH  35.0f // Temperature alert threshold in °C
#define HUM_THRESHOLD_HIGH   75.0f // Humidity alert threshold in %
#define GAS_THRESHOLD_HIGH   350   // MQ-2 ADC raw threshold (0-1023) for gas/smoke alarm
#define DISTANCE_MIN_THRESH  30.0f // Proximity alert threshold in cm (< 30 cm)
#define GRAVITY_MSS          9.81f // Standard earth gravity acceleration (m/s^2)
#define TAMPER_ACCEL_DELTA   3.5f  // Deviation limit (m/s²) to flag vibration/tilt
#define TAMPER_GYRO_LIMIT    2.5f  // Angular rate limit (rad/s) to flag tamper

// Network & Central REST API Configuration
const char* WIFI_SSID        = "YOUR_WIFI_NAME";     // <--- Put your WiFi SSID here
const char* WIFI_PASSWORD    = "YOUR_WIFI_PASSWORD"; // <--- Put your WiFi Password here
const char* REST_SERVER_URL  = "http://192.168.1.100:3000/api/sensor-data"; // Central REST API URL
const char* DEVICE_ID        = "LAB-NODE-01";

// Non-blocking Timers (milliseconds)
const unsigned long SENSOR_READ_INTERVAL = 1000;   // Read sensors every 1 second
const unsigned long UPLOAD_INTERVAL      = 5000;   // POST REST telemetry every 5 seconds
const unsigned long WIFI_CHECK_INTERVAL  = 10000;  // Verify WiFi every 10 seconds

// =====================================================================================
// 2. GLOBAL OBJECTS & STATE VARIABLES
// =====================================================================================
DHT dht(DHT_PIN, DHT_TYPE);
Adafruit_MPU6050 mpu;
ESP8266WebServer webServer(80);

// Sensor Data Variables
float temperature     = 0.0f;
float humidity        = 0.0f;
bool  dhtValid        = false;

int   gasRawValue     = 0;
float gasPercentage   = 0.0f;
bool  gasDetected     = false;

float distanceCm      = 0.0f;
bool  ultrasonicValid = false;
bool  proximityAlert  = false;

bool  motionDetected  = false;
bool  tamperDetected  = false;

float ax = 0.0f, ay = 0.0f, az = 0.0f;
float gx = 0.0f, gy = 0.0f, gz = 0.0f;
bool  mpuValid        = false;

// Alert Flags
bool  alertTemp       = false;
bool  alertHum        = false;
bool  alertGas        = false;
bool  alertMotion     = false;
bool  alertProximity  = false;
bool  alertTamper     = false;
bool  hasActiveAlert  = false;
String activeAlertNames = "NONE";

// Timing Variables
unsigned long lastSensorReadTime = 0;
unsigned long lastUploadTime     = 0;
unsigned long lastWiFiCheckTime  = 0;
bool          lastUploadSuccess  = false;

// =====================================================================================
// 3. EMBEDDED DASHBOARD HTML PAGE (PROGMEM)
// =====================================================================================
const char DASHBOARD_HTML[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>Smart Lab Monitoring Dashboard</title>
    <style>
        :root {
            --bg-color: #0b0f19;
            --card-bg: rgba(22, 31, 49, 0.75);
            --border-color: rgba(255, 255, 255, 0.1);
            --accent-cyan: #06b6d4;
            --accent-green: #10b981;
            --accent-red: #ef4444;
            --accent-orange: #f97316;
            --text-main: #f8fafc;
            --text-muted: #94a3b8;
        }
        * { box-sizing: border-box; margin: 0; padding: 0; font-family: 'Segoe UI', system-ui, -apple-system, sans-serif; }
        body { background: var(--bg-color); color: var(--text-main); padding: 20px; min-height: 100vh; background-image: radial-gradient(circle at top right, #1e1b4b, #0b0f19); }
        .container { max-width: 1200px; margin: 0 auto; }
        header { display: flex; justify-content: space-between; align-items: center; padding: 20px 0; border-bottom: 1px solid var(--border-color); margin-bottom: 24px; flex-wrap: wrap; gap: 15px; }
        h1 { font-size: 1.8rem; font-weight: 700; background: linear-gradient(135deg, #38bdf8, #818cf8); -webkit-background-clip: text; -webkit-text-fill-color: transparent; }
        .status-badge { padding: 8px 16px; border-radius: 20px; font-weight: 600; font-size: 0.9rem; text-transform: uppercase; letter-spacing: 0.5px; }
        .status-normal { background: rgba(16, 185, 129, 0.2); color: #34d399; border: 1px solid rgba(16, 185, 129, 0.4); }
        .status-alarm { background: rgba(239, 68, 68, 0.25); color: #f87171; border: 1px solid rgba(239, 68, 68, 0.5); animation: pulse 1.2s infinite; }
        @keyframes pulse { 0%, 100% { opacity: 1; } 50% { opacity: 0.5; } }
        
        .grid { display: grid; grid-template-columns: repeat(auto-fit, minmax(280px, 1fr)); gap: 20px; }
        .card { background: var(--card-bg); backdrop-filter: blur(12px); border: 1px solid var(--border-color); border-radius: 16px; padding: 24px; transition: transform 0.2s, border-color 0.2s; }
        .card:hover { transform: translateY(-3px); border-color: rgba(255, 255, 255, 0.2); }
        .card-header { display: flex; justify-content: space-between; align-items: center; margin-bottom: 16px; color: var(--text-muted); font-size: 0.95rem; font-weight: 500; }
        .card-value { font-size: 2.2rem; font-weight: 700; color: var(--text-main); margin-bottom: 8px; }
        .card-subtext { font-size: 0.85rem; color: var(--text-muted); }
        
        .alert-card { grid-column: 1 / -1; background: rgba(239, 68, 68, 0.1); border: 1px solid rgba(239, 68, 68, 0.3); display: none; }
        .alert-card.active { display: block; }
        .alert-title { color: #f87171; font-weight: 700; font-size: 1.1rem; margin-bottom: 8px; }
        
        .vector-grid { display: grid; grid-template-columns: repeat(3, 1fr); gap: 10px; margin-top: 10px; text-align: center; }
        .vector-box { background: rgba(0,0,0,0.2); padding: 8px; border-radius: 8px; border: 1px solid var(--border-color); }
        .vector-box label { font-size: 0.75rem; color: var(--text-muted); display: block; }
        .vector-box span { font-weight: 600; font-size: 1rem; color: var(--accent-cyan); }
    </style>
</head>
<body>
    <div class="container">
        <header>
            <div>
                <h1>Smart Lab Safety & Security System</h1>
                <p style="color: var(--text-muted); font-size: 0.9rem; margin-top: 4px;">NodeMCU ESP8266 Multi-Sensor Telemetry Hub</p>
            </div>
            <div id="sysStatus" class="status-badge status-normal">SYSTEM NORMAL</div>
        </header>

        <div id="alertBanner" class="card alert-card">
            <div class="alert-title">CRITICAL SAFETY / SECURITY ALARM DETECTED</div>
            <div id="alertList" style="color: #fca5a5; font-size: 0.95rem;">None</div>
        </div>

        <div class="grid">
            <!-- Temperature -->
            <div class="card">
                <div class="card-header">TEMPERATURE (DHT11)</div>
                <div class="card-value"><span id="tempVal">--</span> °C</div>
                <div class="card-subtext">Threshold: &lt; 35.0 °C</div>
            </div>

            <!-- Humidity -->
            <div class="card">
                <div class="card-header">HUMIDITY (DHT11)</div>
                <div class="card-value"><span id="humVal">--</span> %</div>
                <div class="card-subtext">Threshold: &lt; 75.0 %</div>
            </div>

            <!-- MQ-2 Gas & Smoke -->
            <div class="card">
                <div class="card-header">GAS / SMOKE SENSOR (MQ-2)</div>
                <div class="card-value" id="gasStatus" style="color: var(--accent-green);">SAFE</div>
                <div class="card-subtext">Raw ADC: <span id="gasRawVal" style="color:var(--text-main); font-weight:600;">--</span> (<span id="gasPercentVal">--</span>%)</div>
            </div>

            <!-- HC-SR04 Ultrasonic Distance -->
            <div class="card">
                <div class="card-header">ULTRASONIC DISTANCE (HC-SR04)</div>
                <div class="card-value"><span id="distVal">--</span> cm</div>
                <div class="card-subtext">Status: <span id="proxStatus" style="color: var(--accent-green);">SAFE</span> (Alert &lt; 30 cm)</div>
            </div>

            <!-- PIR Motion -->
            <div class="card">
                <div class="card-header">MOTION DETECTOR (PIR)</div>
                <div class="card-value" id="motionVal" style="color: var(--accent-green);">CLEAR</div>
                <div class="card-subtext">Intruder & Activity Presence Detection</div>
            </div>

            <!-- MPU6050 Accelerometer -->
            <div class="card">
                <div class="card-header">ACCELERATION (MPU6050)</div>
                <div class="vector-grid">
                    <div class="vector-box"><label>X (m/s²)</label><span id="axVal">0.00</span></div>
                    <div class="vector-box"><label>Y (m/s²)</label><span id="ayVal">0.00</span></div>
                    <div class="vector-box"><label>Z (m/s²)</label><span id="azVal">0.00</span></div>
                </div>
                <div class="card-subtext" style="margin-top:10px;">Status: <span id="tamperVal" style="color: var(--accent-green);">STABLE</span></div>
            </div>

            <!-- MPU6050 Gyroscope -->
            <div class="card">
                <div class="card-header">GYROSCOPE / TILT (MPU6050)</div>
                <div class="vector-grid">
                    <div class="vector-box"><label>X (rad/s)</label><span id="gxVal">0.00</span></div>
                    <div class="vector-box"><label>Y (rad/s)</label><span id="gyVal">0.00</span></div>
                    <div class="vector-box"><label>Z (rad/s)</label><span id="gzVal">0.00</span></div>
                </div>
                <div class="card-subtext" style="margin-top:10px;">6-DOF Angular Motion Tracking</div>
            </div>

            <!-- Network Info -->
            <div class="card">
                <div class="card-header">SYSTEM & NETWORK</div>
                <div style="font-size: 1.1rem; margin-bottom: 6px;">Signal: <span id="rssiVal" style="color:var(--accent-cyan);">--</span> dBm</div>
                <div class="card-subtext">Uptime: <span id="uptimeVal">--</span></div>
            </div>
        </div>
    </div>

    <script>
        async function updateDashboard() {
            try {
                const res = await fetch('/api/data');
                const d = await res.json();

                document.getElementById('tempVal').innerText = d.temperature.toFixed(1);
                document.getElementById('humVal').innerText = d.humidity.toFixed(1);

                // MQ-2 Gas
                const gasStatusEl = document.getElementById('gasStatus');
                gasStatusEl.innerText = d.gasDetected ? '⚠️ GAS/SMOKE LEAK!' : 'SAFE';
                gasStatusEl.style.color = d.gasDetected ? '#ef4444' : '#10b981';
                document.getElementById('gasRawVal').innerText = d.gasRaw;
                document.getElementById('gasPercentVal').innerText = d.gasPercent.toFixed(1);

                // HC-SR04 Ultrasonic
                document.getElementById('distVal').innerText = d.distanceCm.toFixed(1);
                const proxEl = document.getElementById('proxStatus');
                proxEl.innerText = d.proximityAlert ? '🚨 PROXIMITY INTRUSION!' : 'SAFE';
                proxEl.style.color = d.proximityAlert ? '#ef4444' : '#10b981';

                // Motion
                const motionEl = document.getElementById('motionVal');
                motionEl.innerText = d.motion ? '🚨 MOTION DETECTED' : 'CLEAR';
                motionEl.style.color = d.motion ? '#f97316' : '#10b981';

                // IMU
                document.getElementById('axVal').innerText = d.ax.toFixed(2);
                document.getElementById('ayVal').innerText = d.ay.toFixed(2);
                document.getElementById('azVal').innerText = d.az.toFixed(2);
                document.getElementById('gxVal').innerText = d.gx.toFixed(2);
                document.getElementById('gyVal').innerText = d.gy.toFixed(2);
                document.getElementById('gzVal').innerText = d.gz.toFixed(2);

                const tamperEl = document.getElementById('tamperVal');
                tamperEl.innerText = d.tamper ? '⚠️ VIBRATION/TAMPER' : 'STABLE';
                tamperEl.style.color = d.tamper ? '#ef4444' : '#10b981';

                // System Status & Alert Banner
                const sysStatus = document.getElementById('sysStatus');
                const alertBanner = document.getElementById('alertBanner');
                const alertList = document.getElementById('alertList');

                if (d.hasAlert) {
                    sysStatus.innerText = 'ALARM ACTIVE';
                    sysStatus.className = 'status-badge status-alarm';
                    alertBanner.classList.add('active');
                    alertList.innerText = d.alerts.join(' | ');
                } else {
                    sysStatus.innerText = 'SYSTEM NORMAL';
                    sysStatus.className = 'status-badge status-normal';
                    alertBanner.classList.remove('active');
                }
                
                document.getElementById('rssiVal').innerText = d.wifiRssi;
                document.getElementById('uptimeVal').innerText = Math.floor(d.uptime) + 's';

            } catch (err) {
                console.error('Fetch error:', err);
            }
        }

        setInterval(updateDashboard, 1000);
        updateDashboard();
    </script>
</body>
</html>
)rawliteral";

// =====================================================================================
// 4. FUNCTION PROTOTYPES
// =====================================================================================
void connectWiFi();
void readDHT();
void readMQ2();
void readUltrasonic();
void readPIR();
void readMPU();
void checkAlerts();
void sendToServer();
void printSerial();
void handleRoot();
void handleApiData();

// =====================================================================================
// 5. SETUP FUNCTION
// =====================================================================================
void setup() {
  Serial.begin(115200);
  while (!Serial && millis() < 3000);

  Serial.println();
  Serial.println(F("========================================================="));
  Serial.println(F("   IoT SMART LAB SAFETY & ENVIRONMENTAL MONITORING SYSTEM "));
  Serial.println(F("   Board: ESP8266 NodeMCU | Integrated Web Server         "));
  Serial.println(F("========================================================="));

  // GPIO Setup
  pinMode(PIR_PIN, INPUT);
  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);
  digitalWrite(TRIG_PIN, LOW);

  // Initialize I2C Wire for MPU6050
  Wire.begin(I2C_SDA_PIN, I2C_SCL_PIN);

  // Initialize DHT11
  Serial.print(F("[INIT] Initializing DHT11 Sensor... "));
  dht.begin();
  Serial.println(F("OK"));

  // Initialize MPU6050
  Serial.print(F("[INIT] Initializing MPU6050 IMU... "));
  if (mpu.begin()) {
    mpu.setAccelerometerRange(MPU6050_RANGE_8_G);
    mpu.setGyroRange(MPU6050_RANGE_500_DEG);
    mpu.setFilterBandwidth(MPU6050_BAND_21_HZ);
    mpuValid = true;
    Serial.println(F("OK"));
  } else {
    mpuValid = false;
    Serial.println(F("FAILED! Check D1/D2 wiring."));
  }

  // Connect to WiFi Network
  connectWiFi();

  // Configure Local Web Server Routes
  webServer.on("/", HTTP_GET, handleRoot);
  webServer.on("/api/data", HTTP_GET, handleApiData);
  webServer.begin();
  Serial.println(F("[HTTP] Local Web Dashboard Server started on port 80"));

  Serial.println(F("[INIT] System setup complete. Entering main execution loop...\n"));
}

// =====================================================================================
// 6. MAIN LOOP
// =====================================================================================
void loop() {
  unsigned long currentMillis = millis();

  // Handle incoming web dashboard browser requests
  webServer.handleClient();

  // Task 1: WiFi Re-connection Watchdog (every 10 seconds)
  if (currentMillis - lastWiFiCheckTime >= WIFI_CHECK_INTERVAL) {
    lastWiFiCheckTime = currentMillis;
    connectWiFi();
  }

  // Task 2: Sample all 5 Hardware Sensors (every 1 second)
  if (currentMillis - lastSensorReadTime >= SENSOR_READ_INTERVAL) {
    lastSensorReadTime = currentMillis;
    readDHT();
    readMQ2();
    readUltrasonic();
    readPIR();
    readMPU();
    checkAlerts();
  }

  // Task 3: POST Telemetry to Central REST Server & Output to Serial (every 5 seconds)
  if (currentMillis - lastUploadTime >= UPLOAD_INTERVAL) {
    lastUploadTime = currentMillis;
    sendToServer();
    printSerial();
  }
}

// =====================================================================================
// 7. WEB SERVER ROUTE HANDLERS
// =====================================================================================
void handleRoot() {
  webServer.send_P(200, "text/html", DASHBOARD_HTML);
}

void handleApiData() {
  StaticJsonDocument<512> doc;

  doc["temperature"]    = dhtValid ? temperature : 0.0f;
  doc["humidity"]       = dhtValid ? humidity : 0.0f;
  doc["gasRaw"]         = gasRawValue;
  doc["gasPercent"]     = gasPercentage;
  doc["gasDetected"]    = gasDetected;
  doc["distanceCm"]     = ultrasonicValid ? distanceCm : 0.0f;
  doc["proximityAlert"] = proximityAlert;
  doc["motion"]         = motionDetected;
  doc["tamper"]         = tamperDetected;
  doc["ax"]             = ax;
  doc["ay"]             = ay;
  doc["az"]             = az;
  doc["gx"]             = gx;
  doc["gy"]             = gy;
  doc["gz"]             = gz;
  doc["hasAlert"]       = hasActiveAlert;
  doc["wifiRssi"]       = WiFi.RSSI();
  doc["uptime"]         = millis() / 1000;

  JsonArray alertArray = doc.createNestedArray("alerts");
  if (alertGas)       alertArray.add("COMBUSTIBLE GAS / SMOKE LEAK");
  if (alertProximity) alertArray.add("PROXIMITY INTRUSION (<30cm)");
  if (alertTemp)      alertArray.add("HIGH TEMPERATURE WARNING");
  if (alertHum)       alertArray.add("HIGH HUMIDITY WARNING");
  if (alertMotion)    alertArray.add("INTRUDER MOTION DETECTED");
  if (alertTamper)    alertArray.add("SEISMIC / TAMPER VIBRATION DETECTED");

  String jsonStr;
  serializeJson(doc, jsonStr);
  webServer.send(200, "application/json", jsonStr);
}

// =====================================================================================
// 8. SENSOR & HARDWARE DRIVER FUNCTIONS
// =====================================================================================
void connectWiFi() {
  if (WiFi.status() == WL_CONNECTED) return;

  Serial.print(F("[WIFI] Connecting to SSID: "));
  Serial.println(WIFI_SSID);

  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

  unsigned long startAttempt = millis();
  while (WiFi.status() != WL_CONNECTED && millis() - startAttempt < 5000) {
    delay(100);
    Serial.print(F("."));
  }
  Serial.println();

  if (WiFi.status() == WL_CONNECTED) {
    Serial.print(F("[WIFI] Connected! IP Address: http://"));
    Serial.println(WiFi.localIP());
  } else {
    Serial.println(F("[WIFI] Connection pending/failed. Retrying in background..."));
  }
}

void readDHT() {
  float h = dht.readHumidity();
  float t = dht.readTemperature();
  if (!isnan(h) && !isnan(t)) {
    humidity = h;
    temperature = t;
    dhtValid = true;
  } else {
    dhtValid = false;
  }
}

void readMQ2() {
  gasRawValue = analogRead(MQ2_AO_PIN); // Reads 0 to 1023 from A0
  gasPercentage = (gasRawValue / 1023.0f) * 100.0f;
  gasDetected = (gasRawValue > GAS_THRESHOLD_HIGH);
}

void readUltrasonic() {
  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);

  long duration = pulseIn(ECHO_PIN, HIGH, 30000); // 30ms max duration timeout
  if (duration > 0) {
    distanceCm = duration * 0.034f / 2.0f;
    ultrasonicValid = true;
    proximityAlert = (distanceCm > 0.0f && distanceCm < DISTANCE_MIN_THRESH);
  } else {
    ultrasonicValid = false;
    proximityAlert = false;
  }
}

void readPIR() {
  motionDetected = (digitalRead(PIR_PIN) == HIGH);
}

void readMPU() {
  if (!mpuValid) {
    if (mpu.begin()) mpuValid = true;
    else return;
  }

  sensors_event_t a, g, temp;
  if (mpu.getEvent(&a, &g, &temp)) {
    ax = a.acceleration.x;
    ay = a.acceleration.y;
    az = a.acceleration.z;
    gx = g.gyro.x;
    gy = g.gyro.y;
    gz = g.gyro.z;

    float accelMag = sqrt(ax * ax + ay * ay + az * az);
    float accelDelta = abs(accelMag - GRAVITY_MSS);
    float gyroMag = sqrt(gx * gx + gy * gy + gz * gz);

    tamperDetected = (accelDelta > TAMPER_ACCEL_DELTA || gyroMag > TAMPER_GYRO_LIMIT);
  } else {
    mpuValid = false;
  }
}

void checkAlerts() {
  alertTemp      = dhtValid && (temperature > TEMP_THRESHOLD_HIGH);
  alertHum       = dhtValid && (humidity > HUM_THRESHOLD_HIGH);
  alertGas       = gasDetected;
  alertProximity = proximityAlert;
  alertMotion    = motionDetected;
  alertTamper    = tamperDetected;

  hasActiveAlert = alertGas || alertProximity || alertMotion || alertTamper || alertTemp;

  activeAlertNames = "";
  if (alertGas)       activeAlertNames += "GAS_LEAK_ALERT ";
  if (alertProximity) activeAlertNames += "PROXIMITY_INTRUSION ";
  if (alertTemp)      activeAlertNames += "HIGH_TEMP ";
  if (alertHum)       activeAlertNames += "HIGH_HUMIDITY ";
  if (alertMotion)    activeAlertNames += "MOTION_DETECTED ";
  if (alertTamper)    activeAlertNames += "TAMPER_DETECTED ";

  activeAlertNames.trim();
  if (activeAlertNames.length() == 0) activeAlertNames = "NONE";
}

void sendToServer() {
  if (WiFi.status() != WL_CONNECTED) {
    lastUploadSuccess = false;
    return;
  }

  WiFiClient client;
  HTTPClient http;
  http.begin(client, REST_SERVER_URL);
  http.setTimeout(3000);
  http.addHeader("Content-Type", "application/json");

  StaticJsonDocument<512> doc;
  doc["device"]      = DEVICE_ID;
  doc["temperature"] = dhtValid ? temperature : 0.0f;
  doc["humidity"]    = dhtValid ? humidity : 0.0f;
  doc["gasRaw"]      = gasRawValue;
  doc["gasPercent"]  = gasPercentage;
  doc["distanceCm"]  = ultrasonicValid ? distanceCm : 0.0f;
  doc["motion"]      = motionDetected;
  doc["tamper"]      = tamperDetected;
  doc["ax"]          = ax;
  doc["ay"]          = ay;
  doc["az"]          = az;
  doc["wifi"]        = WiFi.RSSI();

  JsonArray alertArray = doc.createNestedArray("alerts");
  if (alertGas)       alertArray.add("GAS_LEAK_ALERT");
  if (alertProximity) alertArray.add("PROXIMITY_INTRUSION");
  if (alertTemp)      alertArray.add("HIGH_TEMP");
  if (alertMotion)    alertArray.add("MOTION_DETECTED");
  if (alertTamper)    alertArray.add("TAMPER_DETECTED");

  String jsonStr;
  serializeJson(doc, jsonStr);

  int httpCode = http.POST(jsonStr);
  lastUploadSuccess = (httpCode == HTTP_CODE_OK || httpCode == HTTP_CODE_CREATED);
  http.end();
}

void printSerial() {
  Serial.println(F("\n--- [ SENSOR TELEMETRY & ALERT MONITOR ] ---"));
  if (dhtValid) {
    Serial.printf("Temperature : %.1f °C %s\n", temperature, alertTemp ? "[!] OVER TEMP" : "[NORMAL]");
    Serial.printf("Humidity    : %.1f %% %s\n", humidity, alertHum ? "[!] HIGH HUMIDITY" : "[NORMAL]");
  } else {
    Serial.println(F("DHT11 Sensor: READ ERROR"));
  }

  Serial.printf("MQ-2 Gas    : ADC=%d (%.1f%%) %s\n", gasRawValue, gasPercentage, alertGas ? ">>> GAS / SMOKE LEAK DETECTED! <<<" : "[SAFE]");
  
  if (ultrasonicValid) {
    Serial.printf("Ultrasonic  : %.1f cm %s\n", distanceCm, proximityAlert ? ">>> PROXIMITY INTRUSION (<30cm)! <<<" : "[SAFE]");
  } else {
    Serial.println(F("Ultrasonic  : READ TIMEOUT / OUT OF RANGE"));
  }

  Serial.printf("PIR Motion  : %s\n", motionDetected ? ">>> MOTION DETECTED! <<<" : "Clear");

  if (mpuValid) {
    Serial.printf("Accel (m/s²): X=%.2f, Y=%.2f, Z=%.2f\n", ax, ay, az);
    Serial.printf("Gyro (rad/s): X=%.2f, Y=%.2f, Z=%.2f\n", gx, gy, gz);
    Serial.printf("Tamper Alert: %s\n", tamperDetected ? ">>> VIBRATION / TAMPER DETECTED! <<<" : "Stable");
  } else {
    Serial.println(F("MPU6050 IMU : NOT CONNECTED"));
  }

  Serial.printf("Active Alerts: %s\n", activeAlertNames.c_str());
  Serial.printf("WiFi RSSI   : %d dBm (IP: http://%s)\n", WiFi.RSSI(), WiFi.localIP().toString().c_str());
  Serial.printf("Server POST : %s\n", lastUploadSuccess ? "SUCCESS [200 OK]" : "FAILED / SKIPPED");
  Serial.println(F("---------------------------------------------"));
}
