// ── Canvas Gauge drawing ───────────────────────────────────────────────────────
// Replaces JustGage/Raphael entirely — no SVG overhead, pure Canvas 2D

function drawGauge(canvasId, value, min, max, sectors, unit) {
  var canvas = document.getElementById(canvasId);
  if (!canvas) return;

  // Match canvas internal resolution to its CSS display size
  var rect = canvas.getBoundingClientRect();
  canvas.width  = rect.width  || 300;
  canvas.height = rect.height || 200;

  var ctx = canvas.getContext('2d');
  var w   = canvas.width;
  var h   = canvas.height;

  // Centre horizontally, sit in the lower 75% vertically so arc has headroom
  var cx = w / 2;
  var cy = h * 0.78;
  var r  = Math.min(w * 0.42, cy * 0.88); // radius constrained to fit

  ctx.clearRect(0, 0, w, h);

  var startAngle = Math.PI * 0.75;  // bottom-left
  var endAngle   = Math.PI * 2.25;  // bottom-right
  var totalArc   = endAngle - startAngle;

  // ── Coloured sector arcs ──────────────────────────────────────────────────
  var arcWidth = r * 0.18;
  sectors.forEach(function(s) {
    var aStart = startAngle + ((s.lo - min) / (max - min)) * totalArc;
    var aEnd   = startAngle + ((s.hi - min) / (max - min)) * totalArc;
    ctx.beginPath();
    ctx.arc(cx, cy, r, aStart, aEnd);
    ctx.strokeStyle = s.color;
    ctx.lineWidth   = arcWidth;
    ctx.lineCap     = 'butt';
    ctx.stroke();
  });

  // ── Inner track (decorative ring) ─────────────────────────────────────────
  ctx.beginPath();
  ctx.arc(cx, cy, r - arcWidth * 0.75, startAngle, endAngle);
  ctx.strokeStyle = 'rgba(0,0,0,0.06)';
  ctx.lineWidth   = 2;
  ctx.stroke();

  // ── Needle ────────────────────────────────────────────────────────────────
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

  // ── Pivot dot ─────────────────────────────────────────────────────────────
  ctx.beginPath();
  ctx.arc(cx, cy, baseW * 1.8, 0, Math.PI * 2);
  ctx.fillStyle = '#444';
  ctx.fill();

  // ── Value text ────────────────────────────────────────────────────────────
  var fontSize = Math.round(r * 0.28);
  ctx.fillStyle   = '#222';
  ctx.font        = 'bold ' + fontSize + 'px Arial';
  ctx.textAlign   = 'center';
  ctx.textBaseline = 'middle';
  ctx.fillText(value != null ? value.toFixed(1) + unit : '--', cx, cy - r * 0.48);

  // ── Min / Max tick labels ─────────────────────────────────────────────────
  var labelFont = Math.round(r * 0.16) + 'px Arial';
  ctx.fillStyle  = '#999';
  ctx.font       = labelFont;
  ctx.textBaseline = 'middle';

  // Position labels at the start and end of the arc
  var labelR = r + arcWidth * 0.8;
  var minX = cx + labelR * Math.cos(startAngle);
  var minY = cy + labelR * Math.sin(startAngle);
  var maxX = cx + labelR * Math.cos(endAngle);
  var maxY = cy + labelR * Math.sin(endAngle);

  ctx.textAlign = 'right';
  ctx.fillText(min, minX, minY);
  ctx.textAlign = 'left';
  ctx.fillText(max, maxX, maxY);
}

// Sector definitions
var tempSectors = [
  { color: '#1a6faf', lo: 0,  hi: 10 },
  { color: '#74b9e7', lo: 10, hi: 18 },
  { color: '#2ecc71', lo: 18, hi: 24 },
  { color: '#f39c12', lo: 24, hi: 28 },
  { color: '#e67e22', lo: 28, hi: 35 },
  { color: '#c0392b', lo: 35, hi: 50 }
];
var humSectors = [
  { color: '#c0392b', lo: 0,  hi: 20  },
  { color: '#e67e22', lo: 20, hi: 30  },
  { color: '#2ecc71', lo: 30, hi: 60  },
  { color: '#f39c12', lo: 60, hi: 70  },
  { color: '#2980b9', lo: 70, hi: 100 }
];
var gasSectors = [
  { color: '#2ecc71', lo: 0,    hi: 400  },
  { color: '#f39c12', lo: 400,  hi: 1000 },
  { color: '#e67e22', lo: 1000, hi: 2000 }
];

// ── Live clock (based on user's geolocation timezone, not browser local time) ──
var _tzOffsetSeconds = null; // set when user submits city

function getLocationTime() {
  if (_tzOffsetSeconds === null) return '--:--:--';
  var utcMs = Date.now() + (new Date().getTimezoneOffset() * 60000); // UTC in ms
  var localMs = utcMs + (_tzOffsetSeconds * 1000);
  var d = new Date(localMs);
  return d.toLocaleTimeString('en-GB'); // HH:MM:SS 24hr format
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
  if (aqi < 0)   return '#aaa';
  if (aqi <= 50)  return '#2ecc71';
  if (aqi <= 100) return '#f39c12';
  if (aqi <= 150) return '#e67e22';
  if (aqi <= 200) return '#e74c3c';
  if (aqi <= 300) return '#8e44ad';
  return '#7f0000';
}

function aqiText(aqi) {
  if (aqi < 0)   return null;
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
  data: { labels: [], datasets: [{ label: 'Temperature (\u00b0C)', data: [], borderColor: 'red',      borderWidth: 2, pointRadius: 2, tension: 0 }] },
  options: Object.assign({}, chartOpts, { scales: { y: { min: 0, max: 50 } } })
});
const humChart = new Chart(document.getElementById('humChart'), {
  type: 'line',
  data: { labels: [], datasets: [{ label: 'Humidity (%)',          data: [], borderColor: 'blue',     borderWidth: 2, pointRadius: 2, tension: 0 }] },
  options: Object.assign({}, chartOpts, { scales: { y: { min: 0, max: 100 } } })
});
const gasChart = new Chart(document.getElementById('gasChart'), {
  type: 'line',
  data: { labels: [], datasets: [{ label: 'Air Quality (ppm)',     data: [], borderColor: '#27ae60', borderWidth: 2, pointRadius: 2, tension: 0 }] },
  options: Object.assign({}, chartOpts, { scales: { y: { min: 0, max: 2000 } } })
});

// ── Min / Max display ─────────────────────────────────────────────────────────
function updateMinMaxDisplay(data) {
  document.getElementById('minTemp').textContent = data.minTemp > 0 ? data.minTemp.toFixed(1) : '--';
  document.getElementById('maxTemp').textContent = data.maxTemp > 0 ? data.maxTemp.toFixed(1) : '--';
  document.getElementById('minHum').textContent  = data.minHum  > 0 ? data.minHum.toFixed(1)  : '--';
  document.getElementById('maxHum').textContent  = data.maxHum  > 0 ? data.maxHum.toFixed(1)  : '--';
  document.getElementById('minGas').textContent  = data.minGas  > 0 ? Math.round(data.minGas) : '--';
  document.getElementById('maxGas').textContent  = data.maxGas  > 0 ? Math.round(data.maxGas) : '--';
}

// ── Sensor polling ─────────────────────────────────────────────────────────────
var dataInterval = null;

function updateData() {
  fetch('/data').then(r => r.json()).then(data => {
    var time = new Date().toLocaleTimeString();
    var gas  = data.gas || 0;

    // Show warmup banner until sensor is ready
    var banner = document.getElementById('warmupBanner');
    if (!data.ready) {
      banner.style.display = 'flex';
      return; // don't update gauges with invalid data
    }
    banner.style.display = 'none';

    // Redraw canvas gauges
    drawGauge('tempGaugeCanvas', data.temperature, 0,   50,   tempSectors, '\u00b0C');
    drawGauge('humGaugeCanvas',  data.humidity,    0,   100,  humSectors,  '%');
    drawGauge('gasGaugeCanvas',  gas,              0,   2000, gasSectors,  'ppm');

    // Status labels
    document.getElementById('tempLabel').innerHTML = tempLabel(data.temperature);
    document.getElementById('tempDesc').innerHTML  = tempDesc(data.temperature);
    document.getElementById('humLabel').innerHTML  = humLabel(data.humidity);
    document.getElementById('humDesc').innerHTML   = humDesc(data.humidity);
    document.getElementById('gasLabel').innerHTML  = gasLabel(gas);

    // Gas desc + indoor AQI badge
    var aqi = data.aqi || -1;
    document.getElementById('gasDesc').innerHTML = gasDesc(gas);
    var badge = document.getElementById('indoorAqiBadge');
    if (aqi >= 0) {
      badge.textContent       = 'AQI ' + aqi + ' — ' + aqiText(aqi);
      badge.style.background  = aqiColor(aqi);
    } else {
      badge.textContent      = '--';
      badge.style.background = '#ccc';
    }

    updateMinMaxDisplay(data);

    // Push to charts — shared labels array
    tempChart.data.labels.push(time);
    tempChart.data.datasets[0].data.push(data.temperature);
    humChart.data.labels.push(time);
    humChart.data.datasets[0].data.push(data.humidity);
    gasChart.data.labels.push(time);
    gasChart.data.datasets[0].data.push(gas);

    if (tempChart.data.labels.length > 20) {
      tempChart.data.labels.shift(); tempChart.data.datasets[0].data.shift();
      humChart.data.labels.shift();  humChart.data.datasets[0].data.shift();
      gasChart.data.labels.shift();  gasChart.data.datasets[0].data.shift();
    }

    tempChart.update('none'); // 'none' skips Chart.js animation entirely
    humChart.update('none');
    gasChart.update('none');
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
    .then(r => r.json())
    .then(function(results) {
      if (!results || results.length === 0) {
        document.getElementById('modalError').textContent = 'City not found. Try again.';
        return;
      }
      var lat = parseFloat(results[0].lat);
      var lon = parseFloat(results[0].lon);
      var displayName = results[0].display_name.split(',').slice(0, 2).join(', ');
      document.getElementById('clockCity').innerHTML = '&#127759; ' + displayName;

      document.getElementById('modalError').textContent = 'Syncing time...';
      fetch('https://api.open-meteo.com/v1/forecast?latitude=' + lat + '&longitude=' + lon + '&current=temperature_2m&timezone=auto')
        .then(r => r.json())
        .then(function(tzData) {
          _tzOffsetSeconds = tzData.utc_offset_seconds || 0; // start location clock
          fetch('/timezone', {
            method: 'POST',
            headers: { 'Content-Type': 'application/x-www-form-urlencoded' },
            body: 'offset=' + _tzOffsetSeconds
          }).catch(function() { console.warn('Could not sync timezone to ESP32'); });

          hideModal();
          fetchOutsideWeather(lat, lon);
          if (weatherInterval) clearInterval(weatherInterval);
          weatherInterval = setInterval(function() { fetchOutsideWeather(lat, lon); }, 900000);
          if (!dataInterval) { updateData(); dataInterval = setInterval(updateData, 5000); } // 5s poll
        })
        .catch(function() {
          hideModal();
          fetchOutsideWeather(lat, lon);
          if (weatherInterval) clearInterval(weatherInterval);
          weatherInterval = setInterval(function() { fetchOutsideWeather(lat, lon); }, 900000);
          if (!dataInterval) { updateData(); dataInterval = setInterval(updateData, 5000); }
        });
    })
    .catch(function() {
      document.getElementById('modalError').textContent = 'Network error. Check connection.';
    });
}

function aqiLabel(aqi) {
  if (aqi <= 50)  return { text: 'Good',            color: '#2ecc71' };
  if (aqi <= 100) return { text: 'Moderate',         color: '#f39c12' };
  if (aqi <= 150) return { text: 'Unhealthy for Some', color: '#e67e22' };
  if (aqi <= 200) return { text: 'Unhealthy',        color: '#e74c3c' };
  if (aqi <= 300) return { text: 'Very Unhealthy',   color: '#8e44ad' };
  return           { text: 'Hazardous',              color: '#7f0000' };
}

function fetchOutsideWeather(lat, lon) {
  // Weather + AQI fetched in parallel
  var weatherUrl = 'https://api.open-meteo.com/v1/forecast?latitude=' + lat + '&longitude=' + lon
                 + '&current=temperature_2m,relative_humidity_2m&temperature_unit=celsius';
  var aqiUrl     = 'https://air-quality-api.open-meteo.com/v1/air-quality?latitude=' + lat + '&longitude=' + lon
                 + '&current=us_aqi';

  Promise.all([fetch(weatherUrl).then(r => r.json()), fetch(aqiUrl).then(r => r.json())])
    .then(function(results) {
      var weather = results[0];
      var air     = results[1];

      document.getElementById('outsideTemp').textContent = weather.current.temperature_2m.toFixed(1);
      document.getElementById('outsideHum').textContent  = weather.current.relative_humidity_2m;

      var aqi = air.current && air.current.us_aqi != null ? Math.round(air.current.us_aqi) : null;
      if (aqi !== null) {
        var label = aqiLabel(aqi);
        document.getElementById('outsideAqi').textContent      = aqi;
        document.getElementById('outsideAqi').style.color      = label.color;
        document.getElementById('outsideAqiLabel').textContent = label.text;
        document.getElementById('outsideAqiLabel').style.color = label.color;
      } else {
        document.getElementById('outsideAqi').textContent      = '--';
        document.getElementById('outsideAqiLabel').textContent = '';
      }

      document.getElementById('outsideUpdated').textContent = 'Updated: ' + new Date().toLocaleTimeString();
      document.getElementById('weatherStatus').textContent  = '';
    })
    .catch(function() {
      document.getElementById('weatherStatus').textContent = 'Could not fetch weather data.';
    });
}

showModal();