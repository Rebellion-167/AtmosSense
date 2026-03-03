// ── Gauges ────────────────────────────────────────────────────────────────────
var tempGauge = new JustGage({
  id: 'tempGauge', value: 0, min: 0, max: 50, title: '',
  width: 300, height: 240,
  customSectors: [
    { color: '#1a6faf', lo: 0,  hi: 10 },
    { color: '#74b9e7', lo: 10, hi: 18 },
    { color: '#2ecc71', lo: 18, hi: 24 },
    { color: '#f39c12', lo: 24, hi: 28 },
    { color: '#e67e22', lo: 28, hi: 35 },
    { color: '#c0392b', lo: 35, hi: 50 }
  ],
  animationTime: 300
});

var humGauge = new JustGage({
  id: 'humGauge', value: 0, min: 0, max: 100, title: '',
  width: 300, height: 240,
  customSectors: [
    { color: '#c0392b', lo: 0,  hi: 20  },
    { color: '#e67e22', lo: 20, hi: 30  },
    { color: '#2ecc71', lo: 30, hi: 60  },
    { color: '#f39c12', lo: 60, hi: 70  },
    { color: '#2980b9', lo: 70, hi: 100 }
  ],
  animationTime: 300
});

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

// ── Charts ─────────────────────────────────────────────────────────────────────
const tempChart = new Chart(document.getElementById('tempChart'), {
  type: 'line',
  data: {
    labels: [],
    datasets: [{ label: 'Temperature (\u00b0C)', data: [], borderColor: 'red', borderWidth: 2, pointRadius: 2, tension: 0 }]
  },
  options: { responsive: true, maintainAspectRatio: false, animation: false, scales: { y: { min: 0, max: 50 } } }
});

const humChart = new Chart(document.getElementById('humChart'), {
  type: 'line',
  data: {
    labels: [],
    datasets: [{ label: 'Humidity (%)', data: [], borderColor: 'blue', borderWidth: 2, pointRadius: 2, tension: 0 }]
  },
  options: { responsive: true, maintainAspectRatio: false, animation: false, scales: { y: { min: 0, max: 100 } } }
});

// ── Sensor polling ─────────────────────────────────────────────────────────────
var dataInterval = null;

function updateData() {
  fetch('/data').then(r => r.json()).then(data => {
    var time = new Date().toLocaleTimeString();

    tempGauge.refresh(data.temperature);
    humGauge.refresh(data.humidity);

    document.getElementById('tempLabel').innerHTML = tempLabel(data.temperature);
    document.getElementById('tempDesc').innerHTML  = tempDesc(data.temperature);
    document.getElementById('humLabel').innerHTML  = humLabel(data.humidity);
    document.getElementById('humDesc').innerHTML   = humDesc(data.humidity);

    tempChart.data.labels.push(time);
    tempChart.data.datasets[0].data.push(data.temperature);
    humChart.data.labels.push(time);
    humChart.data.datasets[0].data.push(data.humidity);

    if (tempChart.data.labels.length > 15) {
      tempChart.data.labels.shift();
      tempChart.data.datasets[0].data.shift();
      humChart.data.labels.shift();
      humChart.data.datasets[0].data.shift();
    }

    tempChart.update();
    humChart.update();
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
      document.getElementById('outsideLocation').textContent = displayName;
      hideModal();
      fetchOutsideWeather(lat, lon);
      if (weatherInterval) clearInterval(weatherInterval);
      weatherInterval = setInterval(function() { fetchOutsideWeather(lat, lon); }, 120000);
      if (!dataInterval) { updateData(); dataInterval = setInterval(updateData, 2000); }
    })
    .catch(function() {
      document.getElementById('modalError').textContent = 'Network error. Check connection.';
    });
}

function fetchOutsideWeather(lat, lon) {
  var url = 'https://api.open-meteo.com/v1/forecast?latitude=' + lat + '&longitude=' + lon
          + '&current=temperature_2m,relative_humidity_2m&temperature_unit=celsius';
  fetch(url)
    .then(r => r.json())
    .then(function(d) {
      document.getElementById('outsideTemp').textContent    = d.current.temperature_2m.toFixed(1);
      document.getElementById('outsideHum').textContent     = d.current.relative_humidity_2m;
      document.getElementById('outsideUpdated').textContent = 'Updated: ' + new Date().toLocaleTimeString();
      document.getElementById('weatherStatus').textContent  = '';
    })
    .catch(function() {
      document.getElementById('weatherStatus').textContent = 'Could not fetch weather data.';
    });
}

// Show modal immediately on page load
showModal();