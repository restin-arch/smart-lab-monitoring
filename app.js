/**
 * Smart Laboratory Real-Time Analytics Application
 * Polling Engine, Chart.js Integration & MongoDB Table Renderer
 */

let envChart = null;
let gasDistChart = null;
let accelChart = null;
let pollTimer = null;
let totalPacketsReceived = 0;

// Chart Color Tokens
const COLORS = {
  cyan: '#38bdf8',
  blue: '#60a5fa',
  orange: '#fb923c',
  purple: '#a855f7',
  red: '#f87171',
  green: '#34d399',
  gridLine: 'rgba(255, 255, 255, 0.05)'
};

// Initialize Charts
function initCharts() {
  const commonOptions = {
    responsive: true,
    maintainAspectRatio: false,
    animation: { duration: 300 },
    scales: {
      x: {
        grid: { color: COLORS.gridLine },
        ticks: { color: '#94a3b8', font: { family: 'Outfit', size: 11 } }
      },
      y: {
        grid: { color: COLORS.gridLine },
        ticks: { color: '#94a3b8', font: { family: 'Outfit', size: 11 } }
      }
    },
    plugins: {
      legend: {
        labels: { color: '#f8fafc', font: { family: 'Outfit', size: 12 }, usePointStyle: true }
      }
    }
  };

  // 1. Temperature & Humidity Chart
  const envCtx = document.getElementById('envChart').getContext('2d');
  envChart = new Chart(envCtx, {
    type: 'line',
    data: {
      labels: [],
      datasets: [
        { label: 'Temp (°C)', borderColor: COLORS.cyan, backgroundColor: 'rgba(56, 189, 248, 0.1)', data: [], fill: true, tension: 0.35, pointRadius: 3 },
        { label: 'Humidity (%)', borderColor: COLORS.blue, backgroundColor: 'rgba(96, 165, 250, 0.1)', data: [], fill: true, tension: 0.35, pointRadius: 3 }
      ]
    },
    options: commonOptions
  });

  // 2. Gas & Distance Chart
  const gasDistCtx = document.getElementById('gasDistChart').getContext('2d');
  gasDistChart = new Chart(gasDistCtx, {
    type: 'line',
    data: {
      labels: [],
      datasets: [
        { label: 'Gas Conc (%)', borderColor: COLORS.orange, backgroundColor: 'rgba(251, 146, 60, 0.1)', data: [], fill: true, tension: 0.35, pointRadius: 3 },
        { label: 'Distance (cm)', borderColor: COLORS.cyan, backgroundColor: 'rgba(56, 189, 248, 0.1)', data: [], fill: true, tension: 0.35, pointRadius: 3 }
      ]
    },
    options: commonOptions
  });

  // 3. 3-Axis Acceleration Chart
  const accelCtx = document.getElementById('accelChart').getContext('2d');
  accelChart = new Chart(accelCtx, {
    type: 'line',
    data: {
      labels: [],
      datasets: [
        { label: 'Accel X (m/s²)', borderColor: COLORS.red, data: [], tension: 0.2, pointRadius: 2 },
        { label: 'Accel Y (m/s²)', borderColor: COLORS.green, data: [], tension: 0.2, pointRadius: 2 },
        { label: 'Accel Z (m/s²)', borderColor: COLORS.blue, data: [], tension: 0.2, pointRadius: 2 }
      ]
    },
    options: commonOptions
  });
}

// Fetch Sensor Data from REST API (/api/sensor-data)
async function fetchTelemetry() {
  try {
    const res = await fetch('/api/sensor-data');
    if (!res.ok) throw new Error(`HTTP Error ${res.status}`);

    const result = await res.json();
    const records = result.data || (Array.isArray(result) ? result : [result]);

    if (!records || records.length === 0) {
      updateConnectionStatus(false, 'Waiting for Telemetry...');
      return;
    }

    updateConnectionStatus(true, 'Live Connected');
    processTelemetryRecords(records);

  } catch (err) {
    console.error('Fetch error:', err);
    updateConnectionStatus(false, 'Disconnected / Offline');
  }
}

// Process Incoming Array of Telemetry Records
function processTelemetryRecords(records) {
  const latest = records[0]; // Latest record is first (sorted by receivedAt desc)
  
  totalPacketsReceived = records.length;
  document.getElementById('kpiTotalPackets').innerText = totalPacketsReceived;

  // Update Top KPI Cards
  const temp = latest.temperature !== undefined ? latest.temperature : 0;
  const hum  = latest.humidity !== undefined ? latest.humidity : 0;
  const gasPercent = latest.gasPercent !== undefined ? latest.gasPercent : 0;
  const gasRaw = latest.gasRaw !== undefined ? latest.gasRaw : 0;
  const distance = latest.distanceCm !== undefined ? latest.distanceCm : 0;
  const motion = latest.motion || false;
  const tamper = latest.tamper || false;
  const ax = latest.ax !== undefined ? latest.ax : 0;
  const ay = latest.ay !== undefined ? latest.ay : 0;
  const az = latest.az !== undefined ? latest.az : 0;

  // Temperature & Humidity
  document.getElementById('kpiTemp').innerText = Number(temp).toFixed(1);
  document.getElementById('kpiHum').innerText = Number(hum).toFixed(1);
  document.getElementById('tempBar').style.width = `${Math.min(100, (temp / 50) * 100)}%`;
  
  const tempTag = document.getElementById('tempStatus');
  if (temp > 35.0) {
    tempTag.innerText = 'High Temp!';
    tempTag.className = 'tag tag-alarm';
  } else {
    tempTag.innerText = 'Normal';
    tempTag.className = 'tag tag-normal';
  }

  // Gas & Smoke
  document.getElementById('kpiGasPercent').innerText = Number(gasPercent).toFixed(1);
  document.getElementById('kpiGasRaw').innerText = gasRaw;
  document.getElementById('gasBar').style.width = `${Math.min(100, gasPercent)}%`;

  const gasTag = document.getElementById('gasStatusTag');
  if (latest.gasDetected || gasRaw > 350) {
    gasTag.innerText = '⚠️ GAS LEAK!';
    gasTag.className = 'tag tag-alarm';
  } else {
    gasTag.innerText = 'Safe';
    gasTag.className = 'tag tag-normal';
  }

  // Distance
  document.getElementById('kpiDistance').innerText = Number(distance).toFixed(1);
  document.getElementById('distanceBar').style.width = `${Math.min(100, (distance / 200) * 100)}%`;

  const distTag = document.getElementById('distanceStatus');
  if (latest.proximityAlert || (distance > 0 && distance < 30)) {
    distTag.innerText = '🚨 Intrusion!';
    distTag.className = 'tag tag-alarm';
  } else {
    distTag.innerText = 'Normal';
    distTag.className = 'tag tag-normal';
  }

  // PIR Motion
  const pirEl = document.getElementById('kpiPir');
  const motionContainer = document.getElementById('motionContainer');
  const motionText = document.getElementById('motionStateText');
  const lastMotionTime = document.getElementById('lastMotionTime');

  if (motion) {
    pirEl.innerText = 'MOTION!';
    pirEl.style.color = '#fb923c';
    motionContainer.className = 'motion-status-container motion-active';
    motionText.innerText = 'Activity Detected';
    lastMotionTime.innerText = new Date(latest.receivedAt || Date.now()).toLocaleTimeString();
  } else {
    pirEl.innerText = 'Clear';
    pirEl.style.color = '#34d399';
    motionContainer.className = 'motion-status-container';
    motionText.innerText = 'Zone Clear';
  }

  // MPU6050
  document.getElementById('kpiAx').innerText = Number(ax).toFixed(2);
  document.getElementById('kpiAy').innerText = Number(ay).toFixed(2);
  document.getElementById('kpiAz').innerText = Number(az).toFixed(2);

  const tamperTag = document.getElementById('tamperStatusTag');
  if (tamper) {
    tamperTag.innerText = '⚠️ Vibration!';
    tamperTag.className = 'tag tag-alarm';
  } else {
    tamperTag.innerText = 'Stable';
    tamperTag.className = 'tag purple-tag';
  }

  // Ingest Time
  document.getElementById('lastIngestTime').innerText = new Date(latest.receivedAt || Date.now()).toLocaleTimeString();

  // Update Charts (reverse array so oldest is left, newest right)
  const chartRecords = [...records].reverse().slice(-15);
  const timestamps = chartRecords.map(r => new Date(r.receivedAt || Date.now()).toLocaleTimeString());

  // 1. Env Chart
  envChart.data.labels = timestamps;
  envChart.data.datasets[0].data = chartRecords.map(r => r.temperature);
  envChart.data.datasets[1].data = chartRecords.map(r => r.humidity);
  envChart.update();

  // 2. Gas & Distance Chart
  gasDistChart.data.labels = timestamps;
  gasDistChart.data.datasets[0].data = chartRecords.map(r => r.gasPercent);
  gasDistChart.data.datasets[1].data = chartRecords.map(r => r.distanceCm);
  gasDistChart.update();

  // 3. Accel Chart
  accelChart.data.labels = timestamps;
  accelChart.data.datasets[0].data = chartRecords.map(r => r.ax);
  accelChart.data.datasets[1].data = chartRecords.map(r => r.ay);
  accelChart.data.datasets[2].data = chartRecords.map(r => r.az);
  accelChart.update();

  // Render Table
  renderTable(records);
}

// Render MongoDB Telemetry Records Table
function renderTable(records) {
  const tbody = document.getElementById('sensorTableBody');
  document.getElementById('tableCount').innerText = `${records.length} Packets Received`;

  if (!records || records.length === 0) {
    tbody.innerHTML = `<tr><td colspan="9" class="empty-state">No telemetry records found in MongoDB Atlas</td></tr>`;
    return;
  }

  let html = '';
  records.forEach(r => {
    const timeStr = new Date(r.receivedAt || Date.now()).toLocaleString();
    const alertsStr = (r.alerts && r.alerts.length > 0) ? r.alerts.join(', ') : 'NORMAL';
    const isAlert = alertsStr !== 'NORMAL' && alertsStr !== 'SYSTEM_NORMAL';

    html += `
      <tr>
        <td class="mono-text">${timeStr}</td>
        <td><strong>${Number(r.temperature || 0).toFixed(1)} °C</strong></td>
        <td>${Number(r.humidity || 0).toFixed(1)} %</td>
        <td>${Number(r.gasPercent || 0).toFixed(1)}% (${r.gasRaw || 0})</td>
        <td>${Number(r.distanceCm || 0).toFixed(1)} cm</td>
        <td>${r.motion ? '<span class="tag tag-alarm">DETECTED</span>' : '<span class="tag tag-normal">CLEAR</span>'}</td>
        <td class="mono-text">(${Number(r.ax || 0).toFixed(2)}, ${Number(r.ay || 0).toFixed(2)}, ${Number(r.az || 0).toFixed(2)})</td>
        <td><span class="${isAlert ? 'tag tag-alarm' : 'tag tag-normal'}">${alertsStr}</span></td>
        <td class="mono-text text-muted">${r._id || 'N/A'}</td>
      </tr>
    `;
  });

  tbody.innerHTML = html;
}

// Connection Status Badge Manager
function updateConnectionStatus(isConnected, text) {
  const statusEl = document.getElementById('connectionStatus');
  const textEl = document.getElementById('statusText');

  textEl.innerText = text;

  if (isConnected) {
    statusEl.className = 'status-badge';
    statusEl.style.borderColor = 'rgba(52, 211, 153, 0.3)';
    statusEl.style.color = '#34d399';
  } else {
    statusEl.className = 'status-badge';
    statusEl.style.borderColor = 'rgba(248, 113, 113, 0.3)';
    statusEl.style.color = '#f87171';
  }
}

// Event Listeners & Auto-Polling
document.addEventListener('DOMContentLoaded', () => {
  initCharts();
  fetchTelemetry();

  const pollSelect = document.getElementById('pollInterval');
  const refreshBtn = document.getElementById('refreshBtn');

  function startPolling() {
    if (pollTimer) clearInterval(pollTimer);
    const ms = parseInt(pollSelect.value);
    if (ms > 0) {
      pollTimer = setInterval(fetchTelemetry, ms);
    }
  }

  pollSelect.addEventListener('change', startPolling);
  refreshBtn.addEventListener('click', fetchTelemetry);

  startPolling();
});
