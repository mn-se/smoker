const API_BASE = ""; 

const elCurrTemp = document.getElementById('curr-temp');
const elTgtTemp = document.getElementById('tgt-temp');
const elStateBadge = document.getElementById('state-badge');
const elConnDot = document.getElementById('conn-dot');
const elConnText = document.getElementById('conn-text');

const inTarget = document.getElementById('input-target');

// PID Inputs
const inKp = document.getElementById('input-kp');
const inKi = document.getElementById('input-ki');
const inKd = document.getElementById('input-kd');
const inWin = document.getElementById('input-win');

function setConnectionState(connected) {
    if (connected) {
        elConnDot.className = 'dot connected';
        elConnText.innerText = 'ONLINE';
    } else {
        elConnDot.className = 'dot disconnected';
        elConnText.innerText = 'OFFLINE';
        elCurrTemp.innerText = '--';
        elStateBadge.innerText = 'DISCONNECTED';
        document.body.className = '';
    }
}

function formatState(stateStr) {
    const map = {
        'idle': 'IDLE',
        'temp_control': 'AUTO CONTROL',
        'force_on': 'HEATER ON (FORCE)',
        'monitoring': 'MONITORING',
        'error': 'SENSOR ERROR (LOCKED)'
    };
    return map[stateStr] || 'UNKNOWN';
}

// --- Chart Global Variables ---
const MAX_DATA_POINTS_RT = 600; // 30 min at 3s intervals
let userIsPanningHistory = false;

function formatTime(date) {
    return date.getHours().toString().padStart(2, '0') + ':' + 
           date.getMinutes().toString().padStart(2, '0') + ':' + 
           date.getSeconds().toString().padStart(2, '0');
}

function formatElapsedTime(milliseconds) {
    const totalSeconds = Math.max(0, Math.floor(milliseconds / 1000));
    const hours = Math.floor(totalSeconds / 3600);
    const minutes = Math.floor((totalSeconds % 3600) / 60);
    const seconds = totalSeconds % 60;
    return `${hours}h${minutes.toString().padStart(2, '0')}m${seconds.toString().padStart(2, '0')}s`;
}

// --- Real-time Chart ---
const ctxRT = document.getElementById('tempChart').getContext('2d');
let rtData = {
    labels: [],
    datasets: [
        {
            label: 'Current Temp',
            data: [],
            borderColor: '#ff9f0a',
            backgroundColor: 'rgba(255, 159, 10, 0.1)',
            borderWidth: 2,
            tension: 0.4,
            fill: true,
            pointRadius: 0
        },
        {
            label: 'Target Temp',
            data: [],
            borderColor: 'rgba(255, 255, 255, 0.2)',
            borderWidth: 1,
            borderDash: [5, 5],
            fill: false,
            pointRadius: 0
        }
    ]
};

const tempChart = new Chart(ctxRT, {
    type: 'line',
    data: rtData,
    options: {
        responsive: true,
        maintainAspectRatio: false,
        animation: false,
        plugins: { legend: { display: false } },
        scales: {
            x: {
                display: true, 
                grid: { color: 'rgba(255,255,255,0.05)' },
                ticks: { color: '#8b92a5', maxTicksLimit: 6, font: { size: 10 } }
            },
            y: {
                display: true,
                grid: { color: 'rgba(255,255,255,0.05)' },
                ticks: { color: '#8b92a5' },
                suggestedMin: 0,
                suggestedMax: 100
            }
        }
    }
});

// --- History Chart ---
const ctxHist = document.getElementById('historyChart').getContext('2d');
let historyData = {
    labels: [],
    datasets: [
        {
            label: 'Historical Temp',
            data: [],
            borderColor: '#00d1ff',
            backgroundColor: 'rgba(0, 209, 255, 0.1)',
            borderWidth: 2,
            tension: 0.4,
            fill: true,
            pointRadius: 0
        },
        {
            label: 'Target Temp',
            data: [],
            borderColor: 'rgba(255, 255, 255, 0.2)',
            borderWidth: 1,
            borderDash: [5, 5],
            fill: false,
            pointRadius: 0
        }
    ]
};

const historyChart = new Chart(ctxHist, {
    type: 'line',
    data: historyData,
    options: {
        responsive: true,
        maintainAspectRatio: false,
        animation: false,
        plugins: { 
            legend: { display: false },
            zoom: {
                pan: {
                    enabled: true,
                    mode: 'x',
                    threshold: 5,
                    onPanStart: () => { userIsPanningHistory = true; },
                }
            }
        },
        scales: {
            x: {
                display: true, 
                grid: { color: 'rgba(255,255,255,0.05)' },
                ticks: { color: '#8b92a5', maxTicksLimit: 8, font: { size: 10 } }
            },
            y: {
                display: true,
                grid: { color: 'rgba(255,255,255,0.05)' },
                ticks: { color: '#8b92a5' },
                suggestedMin: 0,
                suggestedMax: 100
            }
        }
    }
});

function updateRTChart(currTemp) {
    const isControl = document.body.className.includes('state-temp_control');
    const targetTemp = isControl ? parseFloat(elTgtTemp.innerText) : null;
    const now = new Date();
    
    rtData.labels.push(formatTime(now));
    rtData.datasets[0].data.push(currTemp > -50 ? currTemp : null);
    rtData.datasets[1].data.push(targetTemp);
    
    if (rtData.labels.length > MAX_DATA_POINTS_RT) {
        rtData.labels.shift();
        rtData.datasets[0].data.shift();
        rtData.datasets[1].data.shift();
    }
    tempChart.update('none');
}

async function fetchStatus() {
    try {
        const req = await fetch(`${API_BASE}/status`);
        if (!req.ok) throw new Error();
        const data = await req.json();
        
        elCurrTemp.innerText = (data.temperature > -50 && data.temperature < 1000) ? data.temperature.toFixed(1) : 'ERR';
        elStateBadge.innerText = formatState(data.state);
        document.body.className = `state-${data.state}`;
        setConnectionState(true);
        updateRTChart(data.temperature);
    } catch(err) {
        setConnectionState(false);
    }
}

async function fetchConfig() {
    try {
        const req = await fetch(`${API_BASE}/config`);
        if (!req.ok) throw new Error();
        const data = await req.json();
        const tgt = data.temperature.target || 60;
        inTarget.value = tgt;
        elTgtTemp.innerText = tgt;
        if (data.pid) {
            inKp.value = data.pid.kp;
            inKi.value = data.pid.ki;
            inKd.value = data.pid.kd;
            inWin.value = data.pid.window_size;
        }
    } catch(err) {}
}

async function postCommand(endpoint, formData = null) {
    try {
        const options = { method: 'POST' };
        if (formData) {
            const params = new URLSearchParams();
            for (const key in formData) params.append(key, formData[key]);
            options.body = params;
            options.headers = { 'Content-Type': 'application/x-www-form-urlencoded' };
        }
        await fetch(`${API_BASE}${endpoint}`, options);
        fetchStatus();
    } catch (err) { console.error('Command failed'); }
}

document.getElementById('btn-start-temp').addEventListener('click', () => {
    const target = inTarget.value;
    elTgtTemp.innerText = target;
    postCommand('/control/temp', { target: target });
});

document.getElementById('btn-save-pid').addEventListener('click', () => {
    postCommand('/config/pid', { kp: inKp.value, ki: inKi.value, kd: inKd.value, window_size: inWin.value });
    alert('PID Configuration Saved!');
});

document.getElementById('btn-force-on').addEventListener('click', () => postCommand('/control/on'));
document.getElementById('btn-monitor').addEventListener('click', () => postCommand('/control/monitor'));
document.getElementById('btn-idle').addEventListener('click', () => postCommand('/control/idle'));

function parseAndRenderHistory(csv, isImport = false) {
    const lines = csv.split('\n');
    const hData = [];
    const hTarget = [];
    const labels = [];
    const timestamps = [];
    
    if (isImport) {
        document.getElementById('info-date').value = '';
        document.getElementById('info-food').value = '';
        document.getElementById('info-location').value = '';
        document.getElementById('info-notes').value = '';
    }

    let dataStarted = false;
    lines.forEach((line) => {
        line = line.trim();
        if (!line) return;
        if (line.startsWith('#')) {
            const match = line.match(/^#\s*(.*?):\s*(.*)$/);
            if (match && isImport) {
                const key = match[1].toLowerCase();
                const val = match[2];
                if (key === 'date') document.getElementById('info-date').value = val;
                else if (key === 'food') document.getElementById('info-food').value = val;
                else if (key === 'location') document.getElementById('info-location').value = val;
                else if (key === 'notes') document.getElementById('info-notes').value = val.replace(/\\n/g, '\n');
            }
            return;
        }
        if (line.startsWith('Timestamp')) { dataStarted = true; return; }
        if (dataStarted) {
            const [ts, curr, tgt, state] = line.split(',');
            const timestamp = Number(ts);
            if (!Number.isFinite(timestamp)) return;
            hData.push(parseFloat(curr));
            hTarget.push(parseFloat(tgt));
            timestamps.push(timestamp);
        }
    });

    const timestampScale = timestamps.some(timestamp => timestamp >= 1000000000) ? 1000 : 1;
    const firstTimestamp = timestamps[0] || 0;
    timestamps.forEach(timestamp => {
        if (timestampScale === 1000) {
            labels.push(formatTime(new Date(timestamp * 1000)));
        } else {
            labels.push(formatElapsedTime((timestamp - firstTimestamp) * timestampScale));
        }
    });

    historyData.labels = labels;
    historyData.datasets[0].data = hData;
    historyData.datasets[1].data = hTarget;

    // 最新の範囲を表示するようにx軸の範囲を設定
    const total = labels.length;
    let historyIntervalMs = 60000;
    if (timestamps.length >= 2) {
        const deltas = [];
        for (let i = 1; i < timestamps.length; i++) {
            const delta = (timestamps[i] - timestamps[i - 1]) * timestampScale;
            if (delta > 0) deltas.push(delta);
        }
        deltas.sort((a, b) => a - b);
        historyIntervalMs = deltas[Math.floor(deltas.length / 2)] || historyIntervalMs;
    }
    const windowSize = Math.max(1, Math.ceil((5 * 60 * 60 * 1000) / historyIntervalMs));
    historyChart.options.scales.x.min = total > windowSize ? total - windowSize : 0;
    historyChart.options.scales.x.max = total > 0 ? total - 1 : 0;
    
    historyChart.update();
    if (isImport) alert('Historical data and notes loaded into History Viewer!');
}

document.getElementById('btn-download-log').addEventListener('click', async () => {
    try {
        const response = await fetch(`${API_BASE}/log`);
        const rawCsv = await response.text();
        const date = document.getElementById('info-date').value;
        const food = document.getElementById('info-food').value;
        const loc = document.getElementById('info-location').value;
        const notes = document.getElementById('info-notes').value.replace(/\n/g, '\\n');
        let enrichedCsv = `# Date: ${date}\n# Food: ${food}\n# Location: ${loc}\n# Notes: ${notes}\n` + rawCsv;
        const blob = new Blob([enrichedCsv], { type: 'text/csv' });
        const url = window.URL.createObjectURL(blob);
        const a = document.createElement('a');
        a.href = url;
        a.download = `smoker_${date || 'log'}_${food || 'data'}.csv`;
        a.click();
        window.URL.revokeObjectURL(url);
    } catch (err) { alert('Download failed'); }
});

async function loadCurrentLog() {
    try {
        const response = await fetch(`${API_BASE}/log`);
        if (!response.ok) return;
        const csv = await response.text();
        parseAndRenderHistory(csv, false);
    } catch (err) { console.error('Failed to load current log:', err); }
}

// --- Real-time chart recovery from /log ---
function parseLogToRT(csv) {
    const lines = csv.split('\n');
    const rows = [];
    lines.forEach(line => {
        line = line.trim();
        if (!line) return;
        if (line.startsWith('#')) return;
        if (line.startsWith('Timestamp')) return;
        const parts = line.split(',');
        if (parts.length < 3) return;
        const ts = parseInt(parts[0]); // millis since boot
        const curr = parseFloat(parts[1]);
        const tgt = parseFloat(parts[2]);
        if (!isNaN(curr)) rows.push({ ts, curr, tgt });
    });
    if (rows.length === 0) return null;

    // Estimate logging interval from the actual log timestamps.
    let intervalMs = 30000;
    if (rows.length >= 2) {
        const deltas = [];
        for (let i = 1; i < rows.length; i++) deltas.push(rows[i].ts - rows[i-1].ts);
        deltas.sort((a,b) => a-b);
        intervalMs = deltas[Math.floor(deltas.length/2)] || intervalMs;
        if (intervalMs <= 0) intervalMs = 30000;
    }

    // Number of log entries to cover ~30 minutes
    const entriesFor30min = Math.max(1, Math.floor((30*60*1000) / intervalMs));
    const slice = rows.slice(-entriesFor30min);

    const labels = [];
    const currData = [];
    const tgtData = [];
    const lastTimestamp = slice[slice.length - 1].ts;
    const isUnixSeconds = lastTimestamp >= 1000000000;
    for (let i = 0; i < slice.length; i++) {
        if (isUnixSeconds) {
            labels.push(formatTime(new Date(slice[i].ts * 1000)));
        } else {
            labels.push(formatElapsedTime((slice[i].ts - slice[0].ts) * 1));
        }
        currData.push(slice[i].curr);
        tgtData.push(isNaN(slice[i].tgt) ? null : slice[i].tgt);
    }
    return { labels, currData, tgtData };
}

async function fetchAndRebuildRT() {
    try {
        const resp = await fetch(`${API_BASE}/log`);
        if (!resp.ok) return;
        const csv = await resp.text();
        const parsed = parseLogToRT(csv);
        if (!parsed) return;

        rtData.labels = parsed.labels.slice();
        rtData.datasets[0].data = parsed.currData.slice();
        rtData.datasets[1].data = parsed.tgtData.slice();
        tempChart.update();
    } catch (err) {
        console.error('Failed to rebuild RT from log:', err);
    }
}

// Rebuild realtime chart when page becomes visible again
document.addEventListener('visibilitychange', () => {
    if (document.visibilityState === 'visible') {
        fetchAndRebuildRT();
        fetchStatus();
    }
});
window.addEventListener('focus', () => { fetchAndRebuildRT(); fetchStatus(); });

document.getElementById('btn-import-trigger').addEventListener('click', () => {
    document.getElementById('input-import-log').click();
});

document.getElementById('input-import-log').addEventListener('change', (e) => {
    const file = e.target.files[0];
    if (!file) return;
    const reader = new FileReader();
    reader.onload = (event) => parseAndRenderHistory(event.target.result, true);
    reader.readAsText(file);
});

document.getElementById('btn-clear-log').addEventListener('click', async () => {
    if (confirm('Clear all log data on device?')) {
        await postCommand('/log/clear');
        alert('Log cleared');
        location.reload();
    }
});

fetchConfig().then(() => {
    loadCurrentLog().then(() => {
        // Rebuild realtime view from persisted log, then start polling
        fetchAndRebuildRT().then(() => {
            fetchStatus();
            setInterval(fetchStatus, 3000);
            // Refresh history viewer every 5 minutes
            setInterval(loadCurrentLog, 300000);
        });
    });
});
