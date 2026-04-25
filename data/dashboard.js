// ── Canvas Gauge drawing ───────────────────────────────────────────────────────
var _gaugeCache = {};

function drawGauge(canvasId, value, min, max, sectors, unit) {
  var canvas = document.getElementById(canvasId);
  if (!canvas) return;

  if (!_gaugeCache[canvasId]) {
    var rect = canvas.getBoundingClientRect();
    canvas.width  = rect.width  || 300;
    canvas.height = rect.height || 200;
    _gaugeCache[canvasId] = { w: canvas.width, h: canvas.height, ctx: canvas.getContext('2d') };
  }
  var w   = _gaugeCache[canvasId].w;
  var h   = _gaugeCache[canvasId].h;
  var ctx = _gaugeCache[canvasId].ctx;

  var cx = w / 2;
  var cy = h * 0.78;
  var r  = Math.min(w * 0.42, cy * 0.88);

  ctx.clearRect(0, 0, w, h);

  var startAngle = Math.PI * 0.75;
  var endAngle   = Math.PI * 2.25;
  var totalArc   = endAngle - startAngle;
  var arcWidth   = r * 0.18;

  ctx.beginPath();
  ctx.arc(cx, cy, r, startAngle, endAngle);
  ctx.strokeStyle = '#e0e0e0';
  ctx.lineWidth   = arcWidth + 4;
  ctx.lineCap     = 'butt';
  ctx.stroke();

  var gapAngle = 0.018;
  sectors.forEach(function(s) {
    var aStart = startAngle + ((s.lo - min) / (max - min)) * totalArc + gapAngle / 2;
    var aEnd   = startAngle + ((s.hi - min) / (max - min)) * totalArc - gapAngle / 2;
    if (aEnd <= aStart) return;
    ctx.beginPath();
    ctx.arc(cx, cy, r, aStart, aEnd);
    ctx.strokeStyle = s.color;
    ctx.lineWidth   = arcWidth;
    ctx.lineCap     = 'butt';
    ctx.stroke();
  });

  var clampedVal  = Math.min(Math.max(value, min), max);
  var needleAngle = startAngle + ((clampedVal - min) / (max - min)) * totalArc;
  var needleLen   = r - arcWidth - 6;
  var baseW       = needleLen * 0.06;

  ctx.save();
  ctx.translate(cx, cy);
  ctx.rotate(needleAngle);
  ctx.beginPath();
  ctx.moveTo(0,  baseW);
  ctx.lineTo(needleLen, 0);
  ctx.lineTo(0, -baseW);
  ctx.closePath();
  ctx.fillStyle = '#333';
  ctx.fill();
  ctx.restore();

  ctx.beginPath();
  ctx.arc(cx, cy, baseW * 1.8, 0, Math.PI * 2);
  ctx.fillStyle = '#444';
  ctx.fill();

  var labelFont = Math.round(r * 0.16) + 'px Arial';
  ctx.fillStyle  = '#999';
  ctx.font       = labelFont;
  ctx.textBaseline = 'middle';

  var labelR = r + arcWidth * 0.8;
  var minX = cx + labelR * Math.cos(startAngle);
  var minY = cy + labelR * Math.sin(startAngle);
  var maxX = cx + labelR * Math.cos(endAngle);
  var maxY = cy + labelR * Math.sin(endAngle);

  ctx.textAlign = 'right';
  ctx.fillText(min, minX, minY);
  ctx.textAlign = 'left';
  ctx.fillText(max, maxX, maxY);

  var valEl = document.getElementById(canvasId.replace('Canvas', 'Value'));
  if (valEl) valEl.textContent = value != null ? value.toFixed(1) + ' ' + unit : '--';
}

// Sector definitions
var tempSectors = [
  { color: '#1a237e', lo: 0,  hi: 10 },
  { color: '#4fc3f7', lo: 10, hi: 18 },
  { color: '#2ecc71', lo: 18, hi: 24 },
  { color: '#f9e04b', lo: 24, hi: 28 },
  { color: '#e67e22', lo: 28, hi: 35 },
  { color: '#c0392b', lo: 35, hi: 50 }
];
var humSectors = [
  { color: '#c0392b', lo: 0,  hi: 20  },
  { color: '#e67e22', lo: 20, hi: 30  },
  { color: '#2ecc71', lo: 30, hi: 60  },
  { color: '#4fc3f7', lo: 60, hi: 70  },
  { color: '#1a237e', lo: 70, hi: 100 }
];
var gasSectors = [
  { color: '#2ecc71', lo: 0,    hi: 400  },
  { color: '#f39c12', lo: 400,  hi: 1000 },
  { color: '#e67e22', lo: 1000, hi: 2000 }
];

// ── Live clock ─────────────────────────────────────────────────────────────────
var _tzOffsetSeconds = null;
var _currentCity     = '';

function getLocationTime() {
  if (_tzOffsetSeconds === null) return '--:--:--';
  var utcMs   = Date.now() + (new Date().getTimezoneOffset() * 60000);
  var localMs = utcMs + (_tzOffsetSeconds * 1000);
  var d = new Date(localMs);
  return d.toLocaleTimeString('en-GB');
}

setInterval(function() {
  document.getElementById('clockTime').textContent = getLocationTime();
}, 1000);

// ── Status helpers ─────────────────────────────────────────────────────────────
function tempLabel(t) {
  if (t < 10) return '&#10052; Dangerously Cold';
  if (t < 18) return '&#129398; Cold';
  if (t < 24) return '&#9989; Comfortable';
  if (t < 28) return '&#127796; Warm';
  if (t < 35) return '&#128293; Hot';
  return '&#9763; Dangerously Hot';
}
function tempDesc(t) {
  if (t < 10) return 'Risk of hypothermia. Heating strongly recommended.';
  if (t < 18) return 'Below comfort range. Consider warming the room.';
  if (t < 24) return 'Ideal indoor temperature. No action needed.';
  if (t < 28) return 'Slightly above comfort. Ventilation may help.';
  if (t < 35) return 'Uncomfortable heat. Use fan or A/C.';
  return 'Dangerous heat level. Take immediate action.';
}
function humLabel(h) {
  if (h < 20) return '&#128033; Very Dry';
  if (h < 30) return '&#127797; Dry';
  if (h < 60) return '&#9989; Comfortable';
  if (h < 70) return '&#128167; Humid';
  return '&#127783; Very Humid';
}
function humDesc(h) {
  if (h < 20) return 'Skin & eye irritation likely. Static buildup risk. Use humidifier.';
  if (h < 30) return 'Below ideal range. A humidifier may improve comfort.';
  if (h < 60) return 'Ideal humidity range. No action needed.';
  if (h < 70) return 'Approaching uncomfortable levels. Ventilate if possible.';
  return 'Mold and dust-mite risk. Use dehumidifier or ventilate.';
}
function gasLabel(g) {
  if (g <= 0)   return '&#128268; Sensor not connected';
  if (g < 400)  return '&#127807; Fresh Air';
  if (g < 1000) return '&#9989; Normal';
  if (g < 2000) return '&#128168; Poor Air Quality';
  return '&#9763; Very Poor Air Quality';
}
function gasDesc(g) {
  if (g <= 0)   return 'MQ-135 sensor not detected. Connect the sensor to enable air quality monitoring.';
  if (g < 400)  return 'Excellent air quality. Better than typical indoor air.';
  if (g < 1000) return 'Normal indoor air quality. No action needed.';
  if (g < 2000) return 'Air quality is poor. Open windows and ventilate the room.';
  return 'Dangerous air quality. Ventilate immediately.';
}
function aqiColor(aqi) {
  if (aqi < 0)    return '#aaa';
  if (aqi <= 50)  return '#2ecc71';
  if (aqi <= 100) return '#f39c12';
  if (aqi <= 150) return '#e67e22';
  if (aqi <= 200) return '#e74c3c';
  if (aqi <= 300) return '#8e44ad';
  return '#7f0000';
}
function aqiText(aqi) {
  if (aqi < 0)    return null;
  if (aqi <= 50)  return 'Good';
  if (aqi <= 100) return 'Moderate';
  if (aqi <= 150) return 'Unhealthy for Some';
  if (aqi <= 200) return 'Unhealthy';
  if (aqi <= 300) return 'Very Unhealthy';
  return 'Hazardous';
}

// ── Charts ─────────────────────────────────────────────────────────────────────
var chartOpts = { responsive: true, maintainAspectRatio: false, animation: false };

const tempChart = new Chart(document.getElementById('tempChart'), {
  type: 'line',
  data: { labels: [], datasets: [{ label: 'Temperature (°C)', data: [], borderColor: 'red',      borderWidth: 2, pointRadius: 2, tension: 0.3, fill: false }] },
  options: Object.assign({}, chartOpts, { scales: { y: { min: 0, max: 50 } } })
});
const humChart = new Chart(document.getElementById('humChart'), {
  type: 'line',
  data: { labels: [], datasets: [{ label: 'Humidity (%)',     data: [], borderColor: 'blue',     borderWidth: 2, pointRadius: 2, tension: 0.3, fill: false }] },
  options: Object.assign({}, chartOpts, { scales: { y: { min: 0, max: 100 } } })
});
const gasChart = new Chart(document.getElementById('gasChart'), {
  type: 'line',
  data: { labels: [], datasets: [{ label: 'Air Quality (ppm)', data: [], borderColor: '#27ae60', borderWidth: 2, pointRadius: 2, tension: 0.3, fill: false }] },
  options: Object.assign({}, chartOpts, { scales: { y: { min: 0, max: 2000 } } })
});

// ── Ring buffer history ────────────────────────────────────────────────────────
var _historyLoaded  = false;   // true once we've fetched the full history
var _historyCount   = 0;       // last known count from /data

// Format a Unix timestamp to a short readable time label
function fmtTime(unixSec) {
  var d = new Date(unixSec * 1000);
  return d.toLocaleTimeString('en-GB', { hour: '2-digit', minute: '2-digit' });
}

// Load full history from /history and populate charts (called once on startup)
function loadHistory() {
  fetch('/history')
    .then(function(r) { return r.json(); })
    .then(function(data) {
      var entries = data.history || [];
      if (entries.length === 0) return;

      var tempLabels = [], tempVals = [];
      var humLabels  = [], humVals  = [];
      var gasLabels  = [], gasVals  = [];

      entries.forEach(function(e) {
        var lbl = fmtTime(e.t);
        if (e.temp !== -999) { tempLabels.push(lbl); tempVals.push(e.temp); }
        if (e.hum  !== -999) { humLabels.push(lbl);  humVals.push(e.hum);   }
        if (e.gas  !== -999) { gasLabels.push(lbl);  gasVals.push(e.gas);   }
      });

      tempChart.data.labels = tempLabels; tempChart.data.datasets[0].data = tempVals;
      humChart.data.labels  = humLabels;  humChart.data.datasets[0].data  = humVals;
      gasChart.data.labels  = gasLabels;  gasChart.data.datasets[0].data  = gasVals;

      tempChart.update('none');
      humChart.update('none');
      gasChart.update('none');

      _historyLoaded = true;
      _historyCount  = entries.length;

      // Update history status badge
      updateHistoryBadge(data.count, data.interval);
      console.log('[History] Loaded ' + entries.length + ' entries from ring buffer');
    })
    .catch(function(err) {
      console.warn('[History] Could not load history:', err);
      _historyLoaded = true; // don't block live updates
    });
}

// Push a single live reading to charts (called after history is loaded)
function pushLiveReading(dhtOk, gasOk, temp, hum, gas) {
  var time = new Date().toLocaleTimeString('en-GB');  // HH:MM:SS

  // Keep the chart from growing unbounded during a long session
  var MAX_LIVE = 360;

  function pushToChart(chart, ok, val) {
    chart.data.labels.push(time);
    chart.data.datasets[0].data.push(ok ? val : null);
    if (chart.data.labels.length > MAX_LIVE) {
      chart.data.labels.shift();
      chart.data.datasets[0].data.shift();
    }
  }

  pushToChart(tempChart, dhtOk, temp);
  pushToChart(humChart,  dhtOk, hum);
  pushToChart(gasChart,  gasOk, gas);

  tempChart.update('none');
  humChart.update('none');
  gasChart.update('none');
}

function updateHistoryBadge(count, intervalSec) {
  var el = document.getElementById('historyBadge');
  if (!el) return;
  var hours = Math.round((count * intervalSec) / 3600);
  el.textContent = count + ' readings' + (hours > 0 ? ' (' + hours + 'h)' : '');
  el.style.display = count > 0 ? 'inline-block' : 'none';
}

// ── Min / Max display ─────────────────────────────────────────────────────────
function updateMinMaxDisplay(data) {
  document.getElementById('minTemp').textContent = data.minTemp > 0 ? data.minTemp.toFixed(1) : '--';
  document.getElementById('maxTemp').textContent = data.maxTemp > 0 ? data.maxTemp.toFixed(1) : '--';
  document.getElementById('minHum').textContent  = data.minHum  > 0 ? data.minHum.toFixed(1)  : '--';
  document.getElementById('maxHum').textContent  = data.maxHum  > 0 ? data.maxHum.toFixed(1)  : '--';
  document.getElementById('minGas').textContent  = data.minGas  > 0 ? Math.round(data.minGas) : '--';
  document.getElementById('maxGas').textContent  = data.maxGas  > 0 ? Math.round(data.maxGas) : '--';
}

// ── Per-parameter alert boxes ──────────────────────────────────────────────────
function updateParamAlerts(data) {
  function setPanel(boxId, statusId, actionId, state, title, action) {
    var box = document.getElementById(boxId);
    var st  = document.getElementById(statusId);
    var ac  = document.getElementById(actionId);
    if (!box) return;
    box.className = 'alert-panel state-' + state;
    if (st) st.textContent = title;
    if (ac) {
      var ts = (state === 'danger' && _dangerTriggeredAt)
               ? action + ' (since ' + _dangerTriggeredAt + ')'
               : action;
      ac.textContent = ts;
    }
  }

  if (!data.dhtConnected) {
    setPanel('tempAlertBox', 'tempAlertStatus', 'tempAlertAction', 'unknown', 'No Sensor', 'Check DHT11 connection.');
  } else {
    var ta = data.tempAdvice || {};
    var tState = data.alertTempState === 2 ? 'danger' : data.alertTempState === 1 ? 'warning' : 'normal';
    setPanel('tempAlertBox', 'tempAlertStatus', 'tempAlertAction', tState, ta.title || '', ta.action || '');
  }

  if (!data.dhtConnected) {
    setPanel('humAlertBox', 'humAlertStatus', 'humAlertAction', 'unknown', 'No Sensor', 'Check DHT11 connection.');
  } else {
    var ha = data.humAdvice || {};
    var hState = data.alertHumState === 2 ? 'danger' : data.alertHumState === 1 ? 'warning' : 'normal';
    setPanel('humAlertBox', 'humAlertStatus', 'humAlertAction', hState, ha.title || '', ha.action || '');
  }

  if (!data.gasConnected) {
    setPanel('gasAlertBox', 'gasAlertStatus', 'gasAlertAction', 'unknown', 'No Sensor', 'Check MQ-135 connection.');
  } else {
    var ga = data.gasAdvice || {};
    var gState = data.alertGasState === 2 ? 'danger' : data.alertGasState === 1 ? 'warning' : 'normal';
    setPanel('gasAlertBox', 'gasAlertStatus', 'gasAlertAction', gState, ga.title || '', ga.action || '');
  }
}

// ── Sensor polling ─────────────────────────────────────────────────────────────
var dataInterval = null;

// ── Danger banner + notifications ─────────────────────────────────────────────
function switchTab(name) {
  document.getElementById('tabOverview').style.display = name === 'overview' ? 'block' : 'none';
  document.getElementById('tabAlerts').style.display   = name === 'alerts'   ? 'block' : 'none';
  document.getElementById('tabBtnOverview').classList.toggle('active', name === 'overview');
  document.getElementById('tabBtnAlerts').classList.toggle('active',   name === 'alerts');
  if (name === 'overview') {
    setTimeout(function() {
      tempChart.resize(); humChart.resize(); gasChart.resize();
    }, 50);
  }
}

// ── Alert history ──────────────────────────────────────────────────────────────
var _alertLog       = [];
var _lastAlertLevel = 0;
var _dangerTriggeredAt = null;

function addLogEntry(type, title, detail) {
  var time = new Date().toLocaleTimeString();
  _alertLog.unshift({ time: time, type: type, title: title, detail: detail });
  if (_alertLog.length > 50) _alertLog.pop();
  renderAlertLog();
}

function renderAlertLog() {
  var list = document.getElementById('alertHistoryList');
  if (!list) return;
  if (_alertLog.length === 0) {
    list.innerHTML = '<div class="no-alerts-msg">No alerts recorded yet.</div>';
    return;
  }
  list.innerHTML = _alertLog.map(function(e) {
    var icon = e.type === 'clear' ? '\u2705' : e.type === 'danger' ? '\u26a0' : '\u26a1';
    return '<div class="alert-log-entry log-' + e.type + '">'
      + '<div class="log-entry-time">' + e.time + '</div>'
      + '<div class="log-entry-icon">' + icon + '</div>'
      + '<div class="log-entry-body">'
      + '<div class="log-entry-title">' + e.title + '</div>'
      + (e.detail ? '<div class="log-entry-detail">' + e.detail + '</div>' : '')
      + '</div></div>';
  }).join('');
}

function clearAlertLog() {
  _alertLog = [];
  renderAlertLog();
}

function renderActiveAlerts(data) {
  var list = document.getElementById('activeAlertsList');
  if (!list) return;

  var cards = [];

  function makeCard(sev, icon, param, title, valueStr, action, since) {
    return '<div class="active-alert-card sev-' + sev + '">'
      + '<div class="active-alert-card-header">'
      + '<div class="active-alert-card-icon">' + icon + '</div>'
      + '<div><div class="active-alert-card-param">' + param + '</div>'
      + '<div class="active-alert-card-title">' + title + '</div></div></div>'
      + '<div class="active-alert-card-value">' + valueStr + '</div>'
      + '<div class="active-alert-card-action">' + action + '</div>'
      + (since ? '<div class="active-alert-card-since">Since ' + since + '</div>' : '')
      + '</div>';
  }

  if (data.alertTempState >= 1 && data.dhtConnected) {
    var sev = data.alertTempState === 2 ? 'danger' : 'warning';
    var adv = data.tempAdvice || {};
    cards.push(makeCard(sev, '\u{1F321}', 'Temperature',
      adv.title || '', data.temperature.toFixed(1) + '\u00b0C',
      adv.action || '', data.alertTempState === 2 ? _dangerTriggeredAt : null));
  }
  if (data.alertHumState >= 1 && data.dhtConnected) {
    var sev = data.alertHumState === 2 ? 'danger' : 'warning';
    var adv = data.humAdvice || {};
    cards.push(makeCard(sev, '\u{1F4A7}', 'Humidity',
      adv.title || '', data.humidity.toFixed(0) + '%',
      adv.action || '', data.alertHumState === 2 ? _dangerTriggeredAt : null));
  }
  if (data.alertGasState >= 1 && data.gasConnected) {
    var sev = data.alertGasState === 2 ? 'danger' : 'warning';
    var adv = data.gasAdvice || {};
    cards.push(makeCard(sev, '\u{1F33F}', 'Air Quality',
      adv.title || '', data.gas.toFixed(0) + ' ppm',
      adv.action || '', data.alertGasState === 2 ? _dangerTriggeredAt : null));
  }

  if (cards.length === 0) {
    list.innerHTML = '<div class="no-alerts-msg">\u2713 All parameters are within safe range.</div>';
  } else {
    list.innerHTML = cards.join('');
  }
}

if ('Notification' in window && Notification.permission === 'default') {
  Notification.requestPermission();
}

function fireBrowserNotification(title, body) {
  if ('Notification' in window && Notification.permission === 'granted') {
    new Notification(title, { body: body, icon: '' });
  }
}

function updateDangerBanner(data, time) {
  var banner  = document.getElementById('dangerBanner');
  var bTitle  = document.getElementById('dangerBannerTitle');
  var bDetail = document.getElementById('dangerBannerDetail');
  var bTime   = document.getElementById('dangerBannerTime');

  var dangers = [];
  if (data.alertTempState === 2 && data.dhtConnected)
    dangers.push('Temp ' + data.temperature.toFixed(1) + '\u00b0C');
  if (data.alertHumState === 2 && data.dhtConnected)
    dangers.push('Humidity ' + data.humidity.toFixed(0) + '%');
  if (data.alertGasState === 2 && data.gasConnected)
    dangers.push('Air quality ' + data.gas.toFixed(0) + 'ppm');

  var isDanger  = dangers.length > 0;
  var isWarning = !isDanger && (data.alertLevel > 0);

  var badge = document.getElementById('alertBadge');
  if (isDanger) {
    badge.style.display = 'inline-flex';
    badge.textContent = dangers.length;
  } else {
    badge.style.display = 'none';
  }

  if (isDanger) {
    banner.style.display = 'flex';
    bTitle.textContent  = '\u26a0\ufe0f ' + (dangers.length === 1 ? 'Danger Detected' : dangers.length + ' Danger Conditions Active');
    bDetail.textContent = dangers.join('  \u2022  ');

    if (_lastAlertLevel < 2) {
      _dangerTriggeredAt = time;
      var notifBody = [];
      if (data.tempAdvice  && data.alertTempState === 2) notifBody.push(data.tempAdvice.action);
      if (data.humAdvice   && data.alertHumState  === 2) notifBody.push(data.humAdvice.action);
      if (data.gasAdvice   && data.alertGasState  === 2) notifBody.push(data.gasAdvice.action);
      fireBrowserNotification('AtmosSense \u2014 Danger Alert', notifBody.join('\n'));
      addLogEntry('danger',
        (dangers.length === 1 ? 'Danger: ' : 'Multiple Dangers: ') + dangers.join(', '),
        notifBody.join(' | '));
    }
    bTime.textContent = 'Since ' + _dangerTriggeredAt;

  } else {
    banner.style.display = 'none';
    if (_lastAlertLevel === 2) {
      _dangerTriggeredAt = null;
      addLogEntry('clear', 'All Clear \u2014 Conditions back to normal', 'All parameters returned to safe range.');
    } else if (isWarning && _lastAlertLevel === 0) {
      var warns = [];
      if (data.alertTempState === 1 && data.dhtConnected) warns.push('Temperature ' + data.temperature.toFixed(1) + '\u00b0C');
      if (data.alertHumState  === 1 && data.dhtConnected) warns.push('Humidity ' + data.humidity.toFixed(0) + '%');
      if (data.alertGasState  === 1 && data.gasConnected) warns.push('Air quality ' + data.gas.toFixed(0) + 'ppm');
      if (warns.length) addLogEntry('warning', 'Warning: ' + warns.join(', '), '');
    }
  }

  _lastAlertLevel = isDanger ? 2 : (isWarning ? 1 : 0);
  renderActiveAlerts(data);
}

function updateData() {
  fetch('/data')
    .then(function(r) { return r.json(); })
    .then(function(data) {
      var time = new Date().toLocaleTimeString();
      var gas  = data.gas || 0;

      var warmupBanner = document.getElementById('warmupBanner');
      if (!data.ready) {
        warmupBanner.style.display = 'flex';
        return;
      }
      warmupBanner.style.display = 'none';

      updateParamAlerts(data);
      updateDangerBanner(data, time);

      var dhtOk = data.dhtConnected !== false;
      var gasOk = data.gasConnected === true;

      document.getElementById('tempOverlay').classList.toggle('visible', !dhtOk);
      document.getElementById('humOverlay').classList.toggle('visible',  !dhtOk);
      document.getElementById('gasOverlay').classList.toggle('visible',  !gasOk);

      if (dhtOk) {
        drawGauge('tempGaugeCanvas', data.temperature, 0, 50,  tempSectors, '\u00b0C');
        drawGauge('humGaugeCanvas',  data.humidity,    0, 100, humSectors,  '%');
        document.getElementById('tempLabel').innerHTML = tempLabel(data.temperature);
        document.getElementById('tempDesc').innerHTML  = tempDesc(data.temperature);
        document.getElementById('humLabel').innerHTML  = humLabel(data.humidity);
        document.getElementById('humDesc').innerHTML   = humDesc(data.humidity);
      }

      if (gasOk) {
        drawGauge('gasGaugeCanvas', gas, 0, 2000, gasSectors, 'ppm');
        document.getElementById('gasLabel').innerHTML = gasLabel(gas);
        document.getElementById('gasDesc').innerHTML  = gasDesc(gas);
      } else {
        document.getElementById('gasLabel').innerHTML = '&#128268; Sensor not connected';
        document.getElementById('gasDesc').innerHTML  = gasDesc(0);
      }

      var aqi   = data.aqi || -1;
      var badge = document.getElementById('indoorAqiBadge');
      if (aqi >= 0 && gasOk) {
        badge.textContent      = 'IAI ' + aqi + ' — ' + aqiText(aqi);
        badge.style.background = aqiColor(aqi);
      } else {
        badge.textContent      = '--';
        badge.style.background = '#ccc';
      }

      updateMinMaxDisplay(data);

      // ── Ring buffer: load full history once, then push every live poll ───────
      var newCount = data.historyCount || 0;
      if (!_historyLoaded) {
        // First poll — fetch full history from ring buffer, then start live updates
        loadHistory();
      } else {
        // Push every poll to the chart (5s live updates)
        pushLiveReading(dhtOk, gasOk, data.temperature, data.humidity, gas);
        // Update badge when ring buffer gains a new persisted entry (every 5 min)
        if (newCount > _historyCount) {
          _historyCount = newCount;
          updateHistoryBadge(newCount, 300);
        }
      }
    })
    .catch(function(err) {
      console.error('[updateData] fetch/parse error:', err);
    });
}

// ── Outside weather ────────────────────────────────────────────────────────────
var weatherInterval = null;

function showModal() {
  document.getElementById('locationModal').style.display = 'flex';
  document.getElementById('modalError').textContent = '';
  document.getElementById('cityInput').focus();
}
function hideModal() {
  document.getElementById('locationModal').style.display = 'none';
}

document.getElementById('cityInput').addEventListener('keydown', function(e) {
  if (e.key === 'Enter') submitCity();
});

function submitCity() {
  var city = document.getElementById('cityInput').value.trim();
  if (!city) { document.getElementById('modalError').textContent = 'Please enter a city name.'; return; }
  document.getElementById('modalError').textContent = 'Searching...';

  fetch('https://nominatim.openstreetmap.org/search?q=' + encodeURIComponent(city) + '&format=json&limit=1')
    .then(function(r) { return r.json(); })
    .then(function(results) {
      if (!results || results.length === 0) {
        document.getElementById('modalError').textContent = 'City not found. Try again.';
        return;
      }
      var lat = parseFloat(results[0].lat);
      var lon = parseFloat(results[0].lon);
      var displayName = results[0].display_name.split(',').slice(0, 2).join(', ');
      _currentCity = displayName;
      document.getElementById('clockCity').innerHTML = '&#127759; ' + displayName;

      document.getElementById('modalError').textContent = 'Syncing time...';
      fetch('https://api.open-meteo.com/v1/forecast?latitude=' + lat + '&longitude=' + lon + '&current=temperature_2m&timezone=auto')
        .then(function(r) { return r.json(); })
        .then(function(tzData) {
          _tzOffsetSeconds = tzData.utc_offset_seconds || 0;
          fetch('/timezone', {
            method: 'POST',
            headers: { 'Content-Type': 'application/x-www-form-urlencoded' },
            body: 'offset=' + _tzOffsetSeconds
          }).catch(function() { console.warn('Could not sync timezone to ESP32'); });

          hideModal();
          fetchOutsideWeather(lat, lon);
          if (weatherInterval) clearInterval(weatherInterval);
          weatherInterval = setInterval(function() { fetchOutsideWeather(lat, lon); }, 900000);
          if (!dataInterval) {
            setTimeout(function() { updateData(); dataInterval = setInterval(updateData, 5000); }, 300);
          }
        })
        .catch(function() {
          hideModal();
          fetchOutsideWeather(lat, lon);
          if (weatherInterval) clearInterval(weatherInterval);
          weatherInterval = setInterval(function() { fetchOutsideWeather(lat, lon); }, 900000);
          if (!dataInterval) {
            setTimeout(function() { updateData(); dataInterval = setInterval(updateData, 5000); }, 300);
          }
        });
    })
    .catch(function() {
      document.getElementById('modalError').textContent = 'Network error. Check connection.';
    });
}

function aqiLabel(aqi) {
  if (aqi <= 50)  return { text: 'Good',              color: '#2ecc71' };
  if (aqi <= 100) return { text: 'Moderate',           color: '#f39c12' };
  if (aqi <= 150) return { text: 'Unhealthy for Some', color: '#e67e22' };
  if (aqi <= 200) return { text: 'Unhealthy',          color: '#e74c3c' };
  if (aqi <= 300) return { text: 'Very Unhealthy',     color: '#8e44ad' };
  return                  { text: 'Hazardous',          color: '#7f0000' };
}

function fetchOutsideWeather(lat, lon) {
  var weatherUrl = 'https://api.open-meteo.com/v1/forecast?latitude=' + lat + '&longitude=' + lon
                 + '&current=temperature_2m,relative_humidity_2m&temperature_unit=celsius';
  var aqiUrl     = 'https://air-quality-api.open-meteo.com/v1/air-quality?latitude=' + lat + '&longitude=' + lon
                 + '&current=us_aqi';

  Promise.all([fetch(weatherUrl).then(function(r) { return r.json(); }), fetch(aqiUrl).then(function(r) { return r.json(); })])
    .then(function(results) {
      var weather = results[0];
      var air     = results[1];
      var el;

      var oTemp = weather.current.temperature_2m;
      var oHum  = weather.current.relative_humidity_2m;

      if ((el = document.getElementById('outsideTemp')))    el.textContent = oTemp.toFixed(1);
      if ((el = document.getElementById('outsideHum')))     el.textContent = oHum;

      var aqi = air.current && air.current.us_aqi != null ? Math.round(air.current.us_aqi) : null;
      if (aqi !== null) {
        var label = aqiLabel(aqi);
        if ((el = document.getElementById('outsideAqi'))) { el.textContent = aqi; el.style.color = label.color; }
      } else {
        if ((el = document.getElementById('outsideAqi'))) { el.textContent = '--'; el.style.color = ''; }
      }

      if ((el = document.getElementById('outsideUpdated'))) el.textContent = 'Updated: ' + new Date().toLocaleTimeString();
      if ((el = document.getElementById('weatherStatus')))  el.textContent = '';

      fetch('/climate', {
        method: 'POST',
        headers: { 'Content-Type': 'application/x-www-form-urlencoded' },
        body: 'outsideTemp=' + oTemp.toFixed(1) +
              '&outsideHum='  + Math.round(oHum) +
              '&city='        + encodeURIComponent(_currentCity)
      }).catch(function() { console.warn('Could not push weather to OLED'); });
    })
    .catch(function() {
      var el = document.getElementById('weatherStatus');
      if (el) el.textContent = 'Could not fetch weather data.';
    });
}

// ── Room name ──────────────────────────────────────────────────────────────────
function fetchRoomName() {
  fetch('/roomname')
    .then(function(r) { return r.json(); })
    .then(function(d) {
      if (d.roomName) {
        document.getElementById('roomNameDisplay').textContent = d.roomName;
        document.title = 'AtmosSense — ' + d.roomName;
      }
    })
    .catch(function() {});
}

function openRoomNameEditor() {
  var current = document.getElementById('roomNameDisplay').textContent;
  document.getElementById('roomNameInput').value = current === 'Room Environment Dashboard' ? '' : current;
  document.getElementById('roomNameError').textContent = '';
  document.getElementById('roomNameModal').style.display = 'flex';
  document.getElementById('roomNameInput').focus();
}

function saveRoomName() {
  var name = document.getElementById('roomNameInput').value.trim();
  if (!name) { document.getElementById('roomNameError').textContent = 'Please enter a room name.'; return; }
  fetch('/roomname', {
    method: 'POST',
    headers: { 'Content-Type': 'application/x-www-form-urlencoded' },
    body: 'name=' + encodeURIComponent(name)
  })
  .then(function() {
    document.getElementById('roomNameDisplay').textContent = name;
    document.title = 'AtmosSense — ' + name;
    document.getElementById('roomNameModal').style.display = 'none';
  })
  .catch(function() {
    document.getElementById('roomNameError').textContent = 'Could not save. Try again.';
  });
}

document.getElementById('roomNameModal').addEventListener('click', function(e) {
  if (e.target === this) this.style.display = 'none';
});

document.getElementById('roomNameInput').addEventListener('keydown', function(e) {
  if (e.key === 'Enter') saveRoomName();
});

fetchRoomName();
showModal();