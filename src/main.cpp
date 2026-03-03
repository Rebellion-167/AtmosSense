#include <Arduino.h>
#include <WiFi.h>
#include <WebServer.h>
#include "DHT.h"
#include "Secrets.h"

#define DHTPIN 4
#define DHTTYPE DHT11

const char* ssid = WIFI_SSID;
const char* password = WIFI_PASSWORD;

DHT dht(DHTPIN, DHTTYPE);
WebServer server(80);

void handleData() {
  float temperature = dht.readTemperature();
  float humidity = dht.readHumidity();

  if (isnan(temperature) || isnan(humidity)) {
    temperature = 0;
    humidity = 0;
  }

  String json = "{\"temperature\":" + String(temperature) +
                ",\"humidity\":" + String(humidity) + "}";

  server.send(200, "application/json", json);
}

void handleRoot() {

  String html = "<!DOCTYPE html><html><head>";
  html += "<meta charset='UTF-8'>";
  html += "<title>Room Dashboard</title>";
  html += "<script src='https://cdn.jsdelivr.net/npm/chart.js'></script>";
  html += "<script src='https://cdn.jsdelivr.net/npm/raphael/raphael.min.js'></script>";
  html += "<script src='https://cdn.jsdelivr.net/npm/justgage/justgage.min.js'></script>";

  html += "<style>";
  html += "body{font-family:Arial;text-align:center;padding:20px;background:#f4f6f9;}";
  html += ".gauge-container{display:flex;justify-content:center;gap:60px;flex-wrap:wrap;}";
  html += ".gauge{width:280px;height:220px;}";
  html += ".dashboard{display:flex;justify-content:center;gap:40px;flex-wrap:wrap;margin-top:30px;}";
  html += ".chart-container{width:45%;min-width:400px;}";

  // Outside weather widget styles
  html += ".outside-weather{display:inline-flex;align-items:center;gap:32px;background:#fff;";
  html += "border-radius:14px;box-shadow:0 2px 12px rgba(0,0,0,0.10);padding:16px 40px;";
  html += "margin:18px auto 28px auto;}";
  html += ".outside-weather .label{font-size:13px;color:#888;margin-bottom:4px;text-transform:uppercase;letter-spacing:1px;}";
  html += ".outside-weather .value{font-size:28px;font-weight:bold;color:#222;}";
  html += ".outside-weather .unit{font-size:16px;color:#555;margin-left:3px;}";
  html += ".outside-weather .location{font-size:13px;color:#aaa;margin-top:3px;cursor:pointer;text-decoration:underline dotted;}";
  html += ".weather-col{text-align:center;}";
  html += ".weather-divider{width:1px;height:60px;background:#e0e0e0;}";
  html += "#weatherStatus{font-size:13px;color:#e74c3c;margin:10px 0 0 0;}";

  // Modal overlay styles
  html += "#locationModal{display:flex;position:fixed;inset:0;background:rgba(0,0,0,0.45);";
  html += "z-index:1000;align-items:center;justify-content:center;}";
  html += ".modal-box{background:#fff;border-radius:16px;box-shadow:0 8px 32px rgba(0,0,0,0.18);";
  html += "padding:36px 40px;max-width:360px;width:90%;text-align:center;}";
  html += ".modal-box h3{margin:0 0 8px 0;font-size:20px;color:#222;}";
  html += ".modal-box p{margin:0 0 20px 0;font-size:14px;color:#777;}";
  html += ".modal-box input{width:100%;box-sizing:border-box;padding:11px 14px;font-size:16px;";
  html += "border:1.5px solid #ddd;border-radius:8px;outline:none;transition:border 0.2s;}";
  html += ".modal-box input:focus{border-color:#4a90e2;}";
  html += ".modal-box button{margin-top:14px;width:100%;padding:11px;font-size:16px;font-weight:bold;";
  html += "background:#4a90e2;color:#fff;border:none;border-radius:8px;cursor:pointer;transition:background 0.2s;}";
  html += ".modal-box button:hover{background:#2573c9;}";
  html += "#modalError{font-size:13px;color:#e74c3c;margin-top:8px;min-height:18px;}";
  html += "</style></head><body>";

  // Location input modal
  html += "<div id='locationModal'>";
  html += "<div class='modal-box'>";
  html += "<h3>&#127759; Outside Weather</h3>";
  html += "<p>Enter your city to display current outside temperature and humidity.</p>";
  html += "<input type='text' id='cityInput' placeholder='e.g. London, Tokyo, New York' autocomplete='off' />";
  html += "<button onclick='submitCity()'>Show Weather</button>";
  html += "<div id='modalError'></div>";
  html += "</div></div>";

  html += "<h2>Room Environment Dashboard</h2>";

  // Outside weather widget
  html += "<div id='outsideWeatherWidget'>";
  html += "<div class='outside-weather'>";
  html += "<div class='weather-col'>";
  html += "<div class='label'>&#127777; Outside Temp</div>";
  html += "<div class='value'><span id='outsideTemp'>--</span><span class='unit'>°C</span></div>";
  html += "<div class='location' id='outsideLocation' onclick='showModal()' title='Click to change city'>--</div>";
  html += "</div>";
  html += "<div class='weather-divider'></div>";
  html += "<div class='weather-col'>";
  html += "<div class='label'>&#128167; Outside Humidity</div>";
  html += "<div class='value'><span id='outsideHum'>--</span><span class='unit'>%</span></div>";
  html += "<div class='location' id='outsideUpdated'>&nbsp;</div>";
  html += "</div>";
  html += "</div>";
  html += "<div id='weatherStatus'></div>";
  html += "</div>";

  html += "<div class='gauge-container'>";
  html += "<div id='tempGauge' class='gauge'></div>";
  html += "<div id='humGauge' class='gauge'></div>";
  html += "</div>";

  html += "<div class='dashboard'>";
  html += "<div class='chart-container'><canvas id='tempChart'></canvas></div>";
  html += "<div class='chart-container'><canvas id='humChart'></canvas></div>";
  html += "</div>";

  html += "<script>";

  // Temperature Gauge
  html += "var tempGauge=new JustGage({";
  html += "id:'tempGauge',value:0,min:0,max:50,title:'Temperature (°C)',";
  html += "customSectors:[";
  html += "{color:'#00cc00',lo:0,hi:30},";
  html += "{color:'#ffcc00',lo:30,hi:35},";
  html += "{color:'#ff0000',lo:35,hi:50}";
  html += "],animationTime:200});";

  // Humidity Gauge
  html += "var humGauge=new JustGage({";
  html += "id:'humGauge',value:0,min:0,max:100,title:'Humidity (%)',";
  html += "customSectors:[";
  html += "{color:'#3399ff',lo:0,hi:30},";
  html += "{color:'#00cc66',lo:30,hi:70},";
  html += "{color:'#ff9900',lo:70,hi:100}";
  html += "],animationTime:200});";

  // Temperature Chart
  html += "const tempChart=new Chart(document.getElementById('tempChart'),{";
  html += "type:'line',";
  html += "data:{labels:[],datasets:[{label:'Temperature (°C)',data:[],borderColor:'red',borderWidth:2,pointRadius:2,tension:0}]},";
  html += "options:{responsive:true,animation:false,scales:{y:{min:0,max:50}}}});";

  // Humidity Chart
  html += "const humChart=new Chart(document.getElementById('humChart'),{";
  html += "type:'line',";
  html += "data:{labels:[],datasets:[{label:'Humidity (%)',data:[],borderColor:'blue',borderWidth:2,pointRadius:2,tension:0}]},";
  html += "options:{responsive:true,animation:false,scales:{y:{min:0,max:100}}}});";

  // Room sensor update function
  html += "function updateData(){";
  html += "fetch('/data').then(r=>r.json()).then(data=>{";
  html += "var time=new Date().toLocaleTimeString();";

  html += "tempGauge.refresh(data.temperature);";
  html += "humGauge.refresh(data.humidity);";

  html += "tempChart.data.labels.push(time);";
  html += "tempChart.data.datasets[0].data.push(data.temperature);";
  html += "humChart.data.labels.push(time);";
  html += "humChart.data.datasets[0].data.push(data.humidity);";

  html += "if(tempChart.data.labels.length>15){";
  html += "tempChart.data.labels.shift();";
  html += "tempChart.data.datasets[0].data.shift();";
  html += "humChart.data.labels.shift();";
  html += "humChart.data.datasets[0].data.shift();}";

  html += "tempChart.update();";
  html += "humChart.update();";
  html += "});}";

  html += "setInterval(updateData,2000);";

  // Outside weather: city input -> Nominatim geocode -> Open-Meteo
  html += "var weatherInterval=null;";

  html += "function showModal(){";
  html += "document.getElementById('locationModal').style.display='flex';";
  html += "document.getElementById('modalError').textContent='';";
  html += "document.getElementById('cityInput').focus();}";

  html += "function hideModal(){document.getElementById('locationModal').style.display='none';}";

  html += "document.getElementById('cityInput').addEventListener('keydown',function(e){";
  html += "if(e.key==='Enter')submitCity();});";

  html += "function submitCity(){";
  html += "var city=document.getElementById('cityInput').value.trim();";
  html += "if(!city){document.getElementById('modalError').textContent='Please enter a city name.';return;}";
  html += "document.getElementById('modalError').textContent='Searching...';";
  // Geocode city with Nominatim
  html += "fetch('https://nominatim.openstreetmap.org/search?q='+encodeURIComponent(city)+'&format=json&limit=1')";
  html += ".then(r=>r.json()).then(function(results){";
  html += "if(!results||results.length===0){document.getElementById('modalError').textContent='City not found. Try again.';return;}";
  html += "var lat=parseFloat(results[0].lat);var lon=parseFloat(results[0].lon);";
  html += "var displayName=results[0].display_name.split(',').slice(0,2).join(', ');";
  html += "document.getElementById('outsideLocation').textContent=displayName;";
  html += "hideModal();";
  html += "fetchOutsideWeather(lat,lon);";
  html += "if(weatherInterval)clearInterval(weatherInterval);";
  html += "weatherInterval=setInterval(function(){fetchOutsideWeather(lat,lon);},120000);";
  html += "}).catch(function(){document.getElementById('modalError').textContent='Network error. Check connection.';});}";

  html += "function fetchOutsideWeather(lat,lon){";
  html += "var url='https://api.open-meteo.com/v1/forecast?latitude='+lat+'&longitude='+lon";
  html += "+'&current=temperature_2m,relative_humidity_2m&temperature_unit=celsius';";
  html += "fetch(url).then(r=>r.json()).then(function(d){";
  html += "document.getElementById('outsideTemp').textContent=d.current.temperature_2m.toFixed(1);";
  html += "document.getElementById('outsideHum').textContent=d.current.relative_humidity_2m;";
  html += "var now=new Date().toLocaleTimeString();";
  html += "document.getElementById('outsideUpdated').textContent='Updated: '+now;";
  html += "document.getElementById('weatherStatus').textContent='';";
  html += "}).catch(function(){";
  html += "document.getElementById('weatherStatus').textContent='Could not fetch weather data.';});}";

  // Show modal on page load
  html += "showModal();";

  html += "</script></body></html>";

  server.send(200, "text/html", html);
}

void setup() {
  Serial.begin(115200);
  dht.begin();

  WiFi.begin(ssid, password);
  Serial.print("Connecting");

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("\nConnected!");
  Serial.println(WiFi.localIP());

  server.on("/", handleRoot);
  server.on("/data", handleData);
  server.begin();
}

void loop() {
  server.handleClient();
}