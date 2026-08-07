<!DOCTYPE html>
<html lang="en">
<head>
  <meta charset="UTF-8">
  <meta name="viewport" content="width=device-width, initial-scale=1.0">
  <title>SMART LAB MONITORING | Safety & Security Telemetry</title>
  <!-- Google Fonts: Outfit & JetBrains Mono -->
  <link rel="preconnect" href="https://fonts.googleapis.com">
  <link rel="preconnect" href="https://fonts.gstatic.com" crossorigin>
  <link href="https://fonts.googleapis.com/css2?family=Outfit:wght@300;400;500;600;700;800;900&family=JetBrains+Mono:wght@400;500;600;700&display=swap" rel="stylesheet">
  <!-- Chart.js -->
  <script src="https://cdn.jsdelivr.net/npm/chart.js"></script>
  <link rel="stylesheet" href="styles.css">
</head>
<body>

  <!-- Ambient Background Glow & Sci-Fi Grids -->
  <div class="bg-glow bg-glow-cyan"></div>
  <div class="bg-glow bg-glow-purple"></div>
  <div class="grid-overlay"></div>

  <div class="futuristic-container">
    
    <!-- Top Header Navigation Bar -->
    <header class="cyber-header">
      <div class="header-left">
        <div class="crown-icon">⚡</div>
        <div>
          <h1 class="header-title">SMART LAB TELEMETRY HUB</h1>
          <div class="breadcrumb">LAB-NODE-01 / ESP8266 NODEMCU / MONGODB ATLAS</div>
        </div>
      </div>

      <div class="header-right">
        <div class="status-badge-glow" id="connectionStatus">
          <span class="pulse-dot"></span>
          <span id="statusText">CONNECTING...</span>
        </div>
        <div class="pill-group">
          <button class="pill-btn active">LIVE TELEMETRY</button>
          <button class="pill-btn">SENSOR STATUS</button>
        </div>
      </div>
    </header>

    <!-- Main Dashboard Sci-Fi Grid Layout -->
    <main class="sci-fi-grid">
      
      <!-- LEFT COLUMN: System Metrics & Gas Module -->
      <div class="grid-col col-left">
        
        <!-- Card 1: Node Health & RSSI -->
        <div class="cyber-card card-metric">
          <div class="cyber-badge-tag">ESP8266 NODEMCU</div>
          <h2 class="card-headline">WIFI SIGNAL & HEALTH</h2>
          <div class="big-metric-value" id="kpiRssi">-55 <span class="metric-unit">dBm</span></div>
          <div class="metric-progress-bg">
            <div class="metric-progress-fill" id="rssiBar" style="width: 85%"></div>
          </div>
          <div class="card-footer-info">
            <span>24/7 Telemetry Sync</span>
            <span class="cyan-text" id="kpiUptime">UPTIME: ONLINE</span>
          </div>
        </div>

        <!-- Card 2: MQ-2 Gas & Smoke Detector -->
        <div class="cyber-card card-metric">
          <div class="cyber-badge-tag badge-purple">MQ-2 SENSOR</div>
          <h2 class="card-headline">GAS & SMOKE LEAK</h2>
          <div class="dual-metric-row">
            <div>
              <div class="big-metric-value purple-text" id="kpiGasPercent">18.0<span class="metric-unit">%</span></div>
              <div class="card-sublabel">GAS CONCENTRATION</div>
            </div>
            <div class="text-right">
              <div class="small-metric-value" id="kpiGasRaw">185</div>
              <div class="card-sublabel">ADC RAW (0-1023)</div>
            </div>
          </div>
          <div class="card-mini-footer">
            <div class="status-box box-safe" id="gasStatusBox">SAFE • NO GAS LEAK</div>
          </div>
        </div>

      </div>

      <!-- CENTER COLUMN: Large Radial Arc Gauge Module (DHT11 Temperature) -->
      <div class="grid-col col-center">
        <div class="cyber-card card-gauge-center">
          <div class="gauge-header">
            <div>
              <h2 class="gauge-title">ENVIRONMENT GAUGE</h2>
              <div class="gauge-subtitle">DHT11 TEMPERATURE MONITOR</div>
            </div>
            <span class="cyan-badge">DHT11 SENSOR</span>
          </div>

          <!-- Radial Circular Arc Gauge -->
          <div class="radial-gauge-wrapper">
            <svg class="gauge-svg" viewBox="0 0 300 300">
              <!-- SVG Gradient Definition -->
              <defs>
                <linearGradient id="gaugeGradient" x1="0%" y1="0%" x2="100%" y2="100%">
                  <stop offset="0%" stop-color="#00f2fe" />
                  <stop offset="100%" stop-color="#9b51e0" />
                </linearGradient>
              </defs>
              <!-- Radial Ticks & Outer Ring -->
              <circle class="gauge-track-bg" cx="150" cy="150" r="120" />
              <circle class="gauge-arc-fill" id="gaugeArc" cx="150" cy="150" r="120" />
            </svg>
            
            <!-- Gauge Ticks Markers (0°C to 60°C) -->
            <div class="gauge-tick t-0">0°C</div>
            <div class="gauge-tick t-25">15°C</div>
            <div class="gauge-tick t-50">30°C</div>
            <div class="gauge-tick t-75">45°C</div>
            <div class="gauge-tick t-100">60°C</div>

            <!-- Gauge Center Value -->
            <div class="gauge-center-content">
              <div class="gauge-big-number" id="gaugeTempValue">27.4</div>
              <div class="gauge-unit-label">°C</div>
              <div class="gauge-caption">ROOM TEMPERATURE</div>
            </div>
          </div>

          <!-- Dual Gauge Footers -->
          <div class="gauge-dual-chips">
            <div class="chip-item">
              <div class="chip-label">RELATIVE HUMIDITY</div>
              <div class="chip-val" id="gaugeHumVal">58.2% HUM</div>
            </div>
            <div class="chip-item">
              <div class="chip-label">DHT11 STATUS</div>
              <div class="chip-val" id="gaugeFreqVal">OPTIMAL</div>
            </div>
          </div>
        </div>
      </div>

      <!-- RIGHT COLUMN: Main Analytics Chart -->
      <div class="grid-col col-right">
        <div class="cyber-card card-chart-main">
          <div class="chart-header-row">
            <div>
              <h2 class="gauge-title">ANALYTICS DASHBOARD</h2>
              <div class="gauge-subtitle">REAL-TIME TEMPERATURE & HUMIDITY STREAM</div>
            </div>
            <div class="time-filter-pills">
              <button class="time-pill">DAY</button>
              <button class="time-pill">WEEK</button>
              <button class="time-pill active">LIVE</button>
              <div class="pill-number-badge" id="chartPacketBadge">12 PACKETS</div>
            </div>
          </div>

          <div class="main-chart-container">
            <canvas id="sciFiChart"></canvas>
          </div>
        </div>
      </div>

    </main>

    <!-- Bottom Row Layout -->
    <section class="bottom-sci-fi-grid">
      
      <!-- Bottom Left: Sensor Status Waveforms (PIR, Distance, MPU6050) -->
      <div class="cyber-card col-bottom-left">
        <div class="card-headline-sm">SECURITY & MOTION TRACKING</div>
        <div class="gauge-subtitle mb-15">REAL-TIME HARDWARE SENSOR MONITOR</div>

        <div class="spark-table">
          <!-- Row 1: Motion -->
          <div class="spark-row">
            <span class="spark-label">PIR MOTION</span>
            <span class="spark-status status-green" id="pirStatusSpark">CLEAR</span>
            <div class="spark-line-canvas">
              <svg class="spark-svg" viewBox="0 0 100 20">
                <path d="M0,15 L20,15 L25,3 L30,17 L35,15 L100,15" class="spark-path green-path"/>
              </svg>
            </div>
            <span class="spark-val" id="pirValText">NO MOTION</span>
          </div>

          <!-- Row 2: Distance -->
          <div class="spark-row">
            <span class="spark-label">HC-SR04 DISTANCE</span>
            <span class="spark-status status-orange" id="distStatusSpark">142.5 cm</span>
            <div class="spark-line-canvas">
              <svg class="spark-svg" viewBox="0 0 100 20">
                <path d="M0,10 L30,10 L35,18 L40,4 L45,10 L100,10" class="spark-path orange-path"/>
              </svg>
            </div>
            <span class="spark-val" id="distValText">SAFE RANGE</span>
          </div>

          <!-- Row 3: Vibration / MPU6050 -->
          <div class="spark-row">
            <span class="spark-label">MPU6050 TAMPER</span>
            <span class="spark-status status-green" id="tamperStatusSpark">STABLE</span>
            <div class="spark-line-canvas">
              <svg class="spark-svg" viewBox="0 0 100 20">
                <path d="M0,12 L40,12 L45,8 L50,15 L55,12 L100,12" class="spark-path green-path"/>
              </svg>
            </div>
            <span class="spark-val" id="tamperValText">0.02, 0.11, 9.80</span>
          </div>
        </div>
      </div>

      <!-- Bottom Right: Multi-color Progress Health Bars -->
      <div class="cyber-card col-bottom-right">
        <div class="card-headline-sm">LAB ENVIRONMENT THRESHOLDS</div>
        <div class="gauge-subtitle mb-15">5-SENSOR THRESHOLD & LOAD MONITOR</div>

        <div class="health-bars-stack">
          <!-- Bar 1: Temp -->
          <div class="health-bar-item">
            <div class="health-bar-header">
              <span>TEMPERATURE LOAD (MAX 50°C)</span>
              <span class="cyan-text" id="barTempText">54.8%</span>
            </div>
            <div class="health-track">
              <div class="health-fill fill-cyan" id="barTempFill" style="width: 54.8%"></div>
            </div>
          </div>

          <!-- Bar 2: Hum -->
          <div class="health-bar-item">
            <div class="health-bar-header">
              <span>HUMIDITY LEVEL (DHT11)</span>
              <span class="purple-text" id="barHumText">58.2%</span>
            </div>
            <div class="health-track">
              <div class="health-fill fill-purple" id="barHumFill" style="width: 58.2%"></div>
            </div>
          </div>

          <!-- Bar 3: Gas -->
          <div class="health-bar-item">
            <div class="health-bar-header">
              <span>GAS & SMOKE CONCENTRATION (MQ-2)</span>
              <span class="orange-text" id="barGasText">18.0%</span>
            </div>
            <div class="health-track">
              <div class="health-fill fill-orange" id="barGasFill" style="width: 18.0%"></div>
            </div>
          </div>

          <!-- Bar 4: Distance -->
          <div class="health-bar-item">
            <div class="health-bar-header">
              <span>PERIMETER PROXIMITY (HC-SR04)</span>
              <span class="green-text" id="barDistText">71.2%</span>
              <span class="warning-icon">🛡️</span>
            </div>
            <div class="health-track">
              <div class="health-fill fill-green" id="barDistFill" style="width: 71.2%"></div>
            </div>
          </div>
        </div>

        <div class="bottom-sys-id">
          <span>SMART LAB MONITORING</span>
          <span class="mono-text text-muted">COLLECTION: smart_lab_telemetry</span>
        </div>
      </div>

    </section>

    <!-- Telemetry Packets Data Table Section -->
    <section class="cyber-card col-full-width">
      <div class="chart-header-row">
        <div>
          <h2 class="gauge-title">RAW TELEMETRY PACKETS (MONGODB ATLAS)</h2>
          <div class="gauge-subtitle">COLLECTION: smart_lab_monitoring.smart_lab_telemetry</div>
        </div>
        <div class="time-filter-pills">
          <input type="text" id="tableSearch" placeholder="🔍 Filter logs..." class="cyber-search-input">
          <div class="pill-number-badge" id="tableCount">2 PACKETS</div>
        </div>
      </div>

      <div class="cyber-table-wrapper">
        <table class="cyber-table">
          <thead>
            <tr>
              <th>TIMESTAMP</th>
              <th>TEMP (°C)</th>
              <th>HUMIDITY (%)</th>
              <th>GAS ADC (%)</th>
              <th>DISTANCE (CM)</th>
              <th>PIR MOTION</th>
              <th>ACCEL (X, Y, Z)</th>
              <th>ALERTS</th>
              <th>DOCUMENT ID</th>
            </tr>
          </thead>
          <tbody id="sensorTableBody">
            <tr>
              <td colspan="9" class="empty-state">Loading telemetry packets...</td>
            </tr>
          </tbody>
        </table>
      </div>
    </section>

  </div>

  <script src="app.js"></script>
</body>
</html>
