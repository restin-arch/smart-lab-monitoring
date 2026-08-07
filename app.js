/**
 * Cyberpunk Sci-Fi Telemetry Controller & Gauge Animation Engine
 */

let sciFiChart = null;
let pollTimer = null;
let rawRecordsCache = [];

const GAUGE_CIRCUMFERENCE = 754; // 2 * PI * 120 (r=120)

// Initialize Chart.js with Cyan & Purple Glowing Streams
function initSciFiChart() {
  const ctx = document.getElementById('sciFiChart').getContext('2d');

  const cyanGrad = ctx.createLinearGradient(0, 0, 0, 300);
  cyanGrad.addColorStop(0, 'rgba(0, 242, 254, 0.4)');
  cyanGrad.addColorStop(1, 'rgba(0, 242, 254, 0.0)');

  const purpleGrad = ctx.createLinearGradient(0, 0, 0, 300);
  purpleGrad.addColorStop(0, 'rgba(155, 81, 224, 0.3)');
  purpleGrad.addColorStop(1, 'rgba(155, 81, 224, 0.0)');

  sciFiChart = new Chart(ctx, {
    type: 'line',
    data: {
      labels: [],
      datasets: [
        {
          label: 'Temperature (°C)',
          borderColor: '#00f2fe',
          borderWidth: 3,
          backgroundColor: cyanGrad,
          data: [],
          fill: true,
          tension: 0.4,
          pointRadius: 4,
          pointBackgroundColor: '#00f2fe'
        },
        {
          label: 'Humidity (%)',
          borderColor: '#9b51e0',
          borderWidth: 2,
          borderDash: [5, 5],
          backgroundColor: purpleGrad,
          data: [],
          fill: true,
          tension: 0.4,
          pointRadius: 3,
          pointBackgroundColor: '#9b51e0'
        }
      ]
    },
    options: {
      responsive: true,
      maintainAspectRatio: false,
      animation: { duration: 400 },
      scales: {
        x: {
          grid: { color: 'rgba(0, 242, 254, 0.06)' },
          ticks: { color: '#6b7c96', font: { family: 'JetBrains Mono', size: 10 } }
        },
        y: {
          grid: { color: 'rgba(0, 242, 254, 0.06)' },
          ticks: { color: '#6b7c96', font: { family: 'JetBrains Mono', size: 10 } }
        }
      },
      plugins: {
        legend: {
          labels: { color: '#ffffff', font: { family: 'Outfit', size: 11 }, usePointStyle: true }
        }
      }
    }
  });
}

// Fetch Sensor Telemetry from REST Endpoint
async function fetchTelemetry() {
  try {
    const res = await fetch('/api/sensor-data');
    if (!res.ok) throw new Error(`HTTP Error ${res.status}`);

    const result = await res.json();
    const records = result.data || (Array.isArray(result) ? result : [result]);

    if (!records || records.length === 0) {
      updateConnectionStatus(false, 'WAITING FOR DATA...');
      return;
    }

    updateConnectionStatus(true, 'LIVE CONNECTED');
    rawRecordsCache = records;
    processTelemetryRecords(records);

  } catch (err) {
    console.error('Fetch error:', err);
    updateConnectionStatus(false, 'DISCONNECTED / OFFLINE');
  }
}

// Process Records & Animate Sci-Fi Elements
function processTelemetryRecords(records) {
  const latest = records[0];

  const temp = latest.temperature !== undefined ? latest.temperature : 0;
  const hum  = latest.humidity !== undefined ? latest.humidity : 0;
  const gasPercent = latest.gasPercent !== undefined ? latest.gasPercent : 0;
  const gasRaw = latest.gasRaw !== undefined ? latest.gasRaw : 0;
  const distance = latest.distanceCm !== undefined ? latest.distanceCm : 0;
  const motion = latest.motion || false;
  const tamper = latest.tamper || false;
  const rssi = latest.wifiRssi !== undefined ? latest.wifiRssi : -55;

  // 1. Center Radial Arc Gauge (Temp 0 - 60 °C)
  document.getElementById('gaugeTempValue').innerText = Number(temp).toFixed(1);
  document.getElementById('gaugeHumVal').innerText = `${Number(hum).toFixed(1)}% HUM`;

  const tempClamped = Math.min(60, Math.max(0, temp));
  const fillRatio = tempClamped / 60.0;
  const strokeOffset = GAUGE_CIRCUMFERENCE - (GAUGE_CIRCUMFERENCE * 0.75 * fillRatio);
  document.getElementById('gaugeArc').style.strokeDashoffset = strokeOffset;

  // 2. Left Cards (RSSI & Gas)
  document.getElementById('kpiRssi').innerHTML = `${rssi} <span class="metric-unit">dBm</span>`;
  document.getElementById('rssiBar').style.width = `${Math.min(100, Math.max(0, (100 + rssi) * 2))}%`;

  document.getElementById('kpiGasPercent').innerHTML = `${Number(gasPercent).toFixed(1)}<span class="metric-unit">%</span>`;
  document.getElementById('kpiGasRaw').innerText = gasRaw;

  const gasStatusBox = document.getElementById('gasStatusBox');
  if (latest.gasDetected || gasRaw > 350) {
    gasStatusBox.innerText = '⚠️ GAS LEAK DETECTED!';
    gasStatusBox.className = 'status-box status-red';
  } else {
    gasStatusBox.innerText = 'SAFE • NO SMOKE';
    gasStatusBox.className = 'status-box box-safe';
  }

  // 3. Wellness Analytics (Sparklines)
  const pirStatus = document.getElementById('pirStatusSpark');
  const pirValText = document.getElementById('pirValText');
  if (motion) {
    pirStatus.innerText = 'MOTION';
    pirStatus.className = 'spark-status status-red';
    pirValText.innerText = 'DETECTED';
  } else {
    pirStatus.innerText = 'CLEAR';
    pirStatus.className = 'spark-status status-green';
    pirValText.innerText = 'CLEAR';
  }

  const distStatus = document.getElementById('distStatusSpark');
  const distValText = document.getElementById('distValText');
  distStatus.innerText = `${Number(distance).toFixed(1)} cm`;
  if (latest.proximityAlert || (distance > 0 && distance < 30)) {
    distStatus.className = 'spark-status status-red';
    distValText.innerText = 'INTRUSION';
  } else {
    distStatus.className = 'spark-status status-orange';
    distValText.innerText = 'NORMAL';
  }

  const tamperStatus = document.getElementById('tamperStatusSpark');
  const tamperValText = document.getElementById('tamperValText');
  if (tamper) {
    tamperStatus.innerText = 'VIBRATION';
    tamperStatus.className = 'spark-status status-red';
    tamperValText.innerText = 'ALERT';
  } else {
    tamperStatus.innerText = 'STABLE';
    tamperStatus.className = 'spark-status status-green';
    tamperValText.innerText = 'STABLE';
  }

  // 4. System Health Bars (Bottom Right)
  const tempRatio = Math.min(100, (temp / 50) * 100);
  document.getElementById('barTempText').innerText = `${tempRatio.toFixed(1)}%`;
  document.getElementById('barTempFill').style.width = `${tempRatio}%`;

  document.getElementById('barHumText').innerText = `${Number(hum).toFixed(1)}%`;
  document.getElementById('barHumFill').style.width = `${Math.min(100, hum)}%`;

  document.getElementById('barGasText').innerText = `${Number(gasPercent).toFixed(1)}%`;
  document.getElementById('barGasFill').style.width = `${Math.min(100, gasPercent)}%`;

  const distRatio = Math.min(100, (distance / 200) * 100);
  document.getElementById('barDistText').innerText = `${distRatio.toFixed(1)}%`;
  document.getElementById('barDistFill').style.width = `${distRatio}%`;

  // 5. Update Sci-Fi Chart
  const chartRecords = [...records].reverse().slice(-12);
  const timestamps = chartRecords.map(r => new Date(r.receivedAt || Date.now()).toLocaleTimeString());

  sciFiChart.data.labels = timestamps;
  sciFiChart.data.datasets[0].data = chartRecords.map(r => r.temperature);
  sciFiChart.data.datasets[1].data = chartRecords.map(r => r.humidity);
  sciFiChart.update();

  // 6. Render Raw Table
  renderTable(records);
}

// Render Raw Telemetry Packets Table
function renderTable(records) {
  const tbody = document.getElementById('sensorTableBody');
  const searchVal = document.getElementById('tableSearch')?.value.toLowerCase() || '';

  const filtered = records.filter(r => {
    const text = `${r.device} ${r.temperature} ${r.humidity} ${r.distanceCm} ${r._id}`.toLowerCase();
    return text.includes(searchVal);
  });

  document.getElementById('tableCount').innerText = `${filtered.length} PACKETS`;

  if (!filtered || filtered.length === 0) {
    tbody.innerHTML = `<tr><td colspan="9" class="empty-state">No telemetry records found in MongoDB Atlas</td></tr>`;
    return;
  }

  let html = '';
  filtered.forEach(r => {
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
        <td>${r.motion ? '<span class="status-box status-red">DETECTED</span>' : '<span class="status-box box-safe">CLEAR</span>'}</td>
        <td class="mono-text">(${Number(r.ax || 0).toFixed(2)}, ${Number(r.ay || 0).toFixed(2)}, ${Number(r.az || 0).toFixed(2)})</td>
        <td><span class="${isAlert ? 'status-box status-red' : 'status-box box-safe'}">${alertsStr}</span></td>
        <td class="mono-text text-muted">${r._id || 'N/A'}</td>
      </tr>
    `;
  });

  tbody.innerHTML = html;
}

// Connection Status Badge
function updateConnectionStatus(isConnected, text) {
  const textEl = document.getElementById('statusText');
  const badge = document.getElementById('connectionStatus');
  textEl.innerText = text;

  if (isConnected) {
    badge.style.borderColor = 'rgba(0, 230, 118, 0.35)';
    badge.style.color = '#00e676';
  } else {
    badge.style.borderColor = 'rgba(255, 51, 102, 0.35)';
    badge.style.color = '#ff3366';
  }
}

// Setup Event Listeners & Auto-Polling
document.addEventListener('DOMContentLoaded', () => {
  initSciFiChart();
  fetchTelemetry();

  const searchInput = document.getElementById('tableSearch');
  if (searchInput) {
    searchInput.addEventListener('input', () => renderTable(rawRecordsCache));
  }

  setInterval(fetchTelemetry, 2000);
});
