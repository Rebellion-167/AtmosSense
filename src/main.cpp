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
  html += "body{font-family:Arial;margin:0;padding:20px;background:#f4f6f9;box-sizing:border-box;}";
  html += "h2{text-align:center;margin-bottom:16px;color:#222;}";

  // Outside weather widget
  html += ".outside-weather{display:inline-flex;align-items:center;gap:32px;background:#fff;";
  html += "border-radius:14px;box-shadow:0 2px 12px rgba(0,0,0,0.10);padding:14px 36px;";
  html += "margin:0 auto 24px auto;}";
  html += ".outside-weather .label{font-size:12px;color:#888;margin-bottom:4px;text-transform:uppercase;letter-spacing:1px;}";
  html += ".outside-weather .value{font-size:26px;font-weight:bold;color:#222;}";
  html += ".outside-weather .unit{font-size:15px;color:#555;margin-left:3px;}";
  html += ".outside-weather .loc{font-size:12px;color:#aaa;margin-top:3px;cursor:pointer;text-decoration:underline dotted;}";
  html += ".weather-col{text-align:center;}";
  html += ".weather-divider{width:1px;height:56px;background:#e0e0e0;}";
  html += "#weatherStatus{font-size:13px;color:#e74c3c;text-align:center;margin-bottom:8px;}";
  html += ".outside-header{text-align:center;}";

  // Main two-column layout — grid so rows share height naturally
  html += ".main-layout{display:grid;grid-template-columns:340px 1fr;gap:24px;width:100%;box-sizing:border-box;}";

  // Left column: gauges stacked
  html += ".left-col{display:contents;}";

  // Gauge card — fills its grid row
  html += ".gauge-card{background:#fff;border-radius:14px;box-shadow:0 2px 12px rgba(0,0,0,0.08);padding:18px 16px 14px 16px;text-align:center;display:flex;flex-direction:column;}";
  html += ".gauge-card .card-title{font-size:13px;font-weight:bold;color:#555;text-transform:uppercase;letter-spacing:1px;margin-bottom:8px;}";
  html += ".gauge{width:300px;height:240px;margin:0 auto;flex-shrink:0;}";

  // Status box below each gauge
  html += ".status-box{margin-top:12px;border-radius:10px;padding:10px 14px;background:#f8f9fa;border:1.5px solid #e0e0e0;flex:1;}";
  html += ".status-box .status-label{font-size:18px;font-weight:bold;color:#333;margin-bottom:4px;}";
  html += ".status-box .status-desc{font-size:12px;color:#777;line-height:1.5;}";

  // Right column: charts stacked — also display:contents so children share grid rows
  html += ".right-col{display:contents;}";
  html += ".chart-card{background:#fff;border-radius:14px;box-shadow:0 2px 12px rgba(0,0,0,0.08);padding:14px 16px;display:flex;flex-direction:column;}";
  html += ".chart-card .card-title{font-size:13px;font-weight:bold;color:#555;text-transform:uppercase;letter-spacing:1px;margin-bottom:8px;flex-shrink:0;}";
  // Canvas wrapper fills remaining card height
  html += ".chart-card .canvas-wrap{flex:1;position:relative;min-height:0;}";
  html += ".chart-card canvas{position:absolute;inset:0;width:100% !important;height:100% !important;}";

  // Modal
  html += "#locationModal{display:flex;position:fixed;inset:0;background:rgba(0,0,0,0.45);z-index:1000;align-items:center;justify-content:center;}";
  html += ".modal-box{background:#fff;border-radius:16px;box-shadow:0 8px 32px rgba(0,0,0,0.18);padding:36px 40px;max-width:360px;width:90%;text-align:center;}";
  html += ".modal-box h3{margin:0 0 8px 0;font-size:20px;color:#222;}";
  html += ".modal-box p{margin:0 0 20px 0;font-size:14px;color:#777;}";
  html += ".modal-box input{width:100%;box-sizing:border-box;padding:11px 14px;font-size:16px;border:1.5px solid #ddd;border-radius:8px;outline:none;transition:border 0.2s;}";
  html += ".modal-box input:focus{border-color:#4a90e2;}";
  html += ".modal-box button{margin-top:14px;width:100%;padding:11px;font-size:16px;font-weight:bold;background:#4a90e2;color:#fff;border:none;border-radius:8px;cursor:pointer;transition:background 0.2s;}";
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

  // Outside weather widget (centered above layout)
  html += "<div class='outside-header'>";
  html += "<div class='outside-weather'>";
  html += "<div class='weather-col'>";
  html += "<div class='label'>&#127777; Outside Temp</div>";
  html += "<div class='value'><span id='outsideTemp'>--</span><span class='unit'>°C</span></div>";
  html += "<div class='loc' id='outsideLocation' onclick='showModal()' title='Click to change city'>--</div>";
  html += "</div>";
  html += "<div class='weather-divider'></div>";
  html += "<div class='weather-col'>";
  html += "<div class='label'>&#128167; Outside Humidity</div>";
  html += "<div class='value'><span id='outsideHum'>--</span><span class='unit'>%</span></div>";
  html += "<div class='loc' id='outsideUpdated'>&nbsp;</div>";
  html += "</div>";
  html += "</div>";
  html += "<div id='weatherStatus'></div>";
  html += "</div>";

  // Main two-column layout — direct grid children: gauge-card, chart-card, gauge-card, chart-card
  html += "<div class='main-layout'>";

  // Row 1 — Temperature
  html += "<div class='gauge-card'>";
  html += "<div class='card-title'>&#127777; Indoor Temperature</div>";
  html += "<div id='tempGauge' class='gauge'></div>";
  html += "<div class='status-box'>";
  html += "<div class='status-label' id='tempLabel'>--</div>";
  html += "<div class='status-desc' id='tempDesc'>Waiting for data...</div>";
  html += "</div></div>";

  html += "<div class='chart-card'>";
  html += "<div class='card-title'>&#128200; Temperature History (°C)</div>";
  html += "<div class='canvas-wrap'><canvas id='tempChart'></canvas></div>";
  html += "</div>";

  // Row 2 — Humidity
  html += "<div class='gauge-card'>";
  html += "<div class='card-title'>&#128167; Indoor Humidity</div>";
  html += "<div id='humGauge' class='gauge'></div>";
  html += "<div class='status-box'>";
  html += "<div class='status-label' id='humLabel'>--</div>";
  html += "<div class='status-desc' id='humDesc'>Waiting for data...</div>";
  html += "</div></div>";

  html += "<div class='chart-card'>";
  html += "<div class='card-title'>&#128201; Humidity History (%)</div>";
  html += "<div class='canvas-wrap'><canvas id='humChart'></canvas></div>";
  html += "</div>";

  html += "</div>"; // end main-layout

  html += "<script>";

  // Temperature Gauge
  // Zones based on ASHRAE / WHO indoor comfort standards:
  // <10°C  = Dangerously Cold
  // 10-18  = Cold
  // 18-24  = Comfortable (ideal indoor range)
  // 24-28  = Warm
  // 28-35  = Hot
  // >35    = Dangerously Hot
  html += "var tempGauge=new JustGage({";
  html += "id:'tempGauge',value:0,min:0,max:50,title:'',";
  html += "width:300,height:240,";
  html += "customSectors:[";
  html += "{color:'#1a6faf',lo:0,hi:10},";
  html += "{color:'#74b9e7',lo:10,hi:18},";
  html += "{color:'#2ecc71',lo:18,hi:24},";
  html += "{color:'#f39c12',lo:24,hi:28},";
  html += "{color:'#e67e22',lo:28,hi:35},";
  html += "{color:'#c0392b',lo:35,hi:50}";
  html += "],animationTime:300});";

  html += "var humGauge=new JustGage({";
  html += "id:'humGauge',value:0,min:0,max:100,title:'',";
  html += "width:300,height:240,";
  html += "customSectors:[";
  html += "{color:'#c0392b',lo:0,hi:20},";
  html += "{color:'#e67e22',lo:20,hi:30},";
  html += "{color:'#2ecc71',lo:30,hi:60},";
  html += "{color:'#f39c12',lo:60,hi:70},";
  html += "{color:'#2980b9',lo:70,hi:100}";
  html += "],animationTime:300});";

  // Temperature label + description
  html += "function tempLabel(t){";
  html += "if(t<10)return'&#10052; Dangerously Cold';";
  html += "if(t<18)return'&#129398; Cold';";
  html += "if(t<24)return'&#9989; Comfortable';";
  html += "if(t<28)return'&#127796; Warm';";
  html += "if(t<35)return'&#128293; Hot';";
  html += "return'&#9763; Dangerously Hot';}";

  html += "function tempDesc(t){";
  html += "if(t<10)return'Risk of hypothermia. Heating strongly recommended.';";
  html += "if(t<18)return'Below comfort range. Consider warming the room.';";
  html += "if(t<24)return'Ideal indoor temperature. No action needed.';";
  html += "if(t<28)return'Slightly above comfort. Ventilation may help.';";
  html += "if(t<35)return'Uncomfortable heat. Use fan or A/C.';";
  html += "return'Dangerous heat level. Take immediate action.';}";

  // Humidity label + description
  html += "function humLabel(h){";
  html += "if(h<20)return'&#128033; Very Dry';";
  html += "if(h<30)return'&#127797; Dry';";
  html += "if(h<60)return'&#9989; Comfortable';";
  html += "if(h<70)return'&#128167; Humid';";
  html += "return'&#127783; Very Humid';}";

  html += "function humDesc(h){";
  html += "if(h<20)return'Skin & eye irritation likely. Static buildup risk. Use humidifier.';";
  html += "if(h<30)return'Below ideal range. A humidifier may improve comfort.';";
  html += "if(h<60)return'Ideal humidity range. No action needed.';";
  html += "if(h<70)return'Approaching uncomfortable levels. Ventilate if possible.';";
  html += "return'Mold and dust-mite risk. Use dehumidifier or ventilate.';}";


  // Temperature Chart
  html += "const tempChart=new Chart(document.getElementById('tempChart'),{";
  html += "type:'line',";
  html += "data:{labels:[],datasets:[{label:'Temperature (°C)',data:[],borderColor:'red',borderWidth:2,pointRadius:2,tension:0}]},";
  html += "options:{responsive:true,maintainAspectRatio:false,animation:false,scales:{y:{min:0,max:50}}}});";

  // Humidity Chart
  html += "const humChart=new Chart(document.getElementById('humChart'),{";
  html += "type:'line',";
  html += "data:{labels:[],datasets:[{label:'Humidity (%)',data:[],borderColor:'blue',borderWidth:2,pointRadius:2,tension:0}]},";
  html += "options:{responsive:true,maintainAspectRatio:false,animation:false,scales:{y:{min:0,max:100}}}});";

  // Room sensor update function
  html += "function updateData(){";
  html += "fetch('/data').then(r=>r.json()).then(data=>{";
  html += "var time=new Date().toLocaleTimeString();";

  html += "tempGauge.refresh(data.temperature);";
  html += "humGauge.refresh(data.humidity);";
  html += "document.getElementById('tempLabel').innerHTML=tempLabel(data.temperature);";
  html += "document.getElementById('tempDesc').innerHTML=tempDesc(data.temperature);";
  html += "document.getElementById('humLabel').innerHTML=humLabel(data.humidity);";
  html += "document.getElementById('humDesc').innerHTML=humDesc(data.humidity);";

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

  html += "var dataInterval=null;";

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
  html += "if(!dataInterval){updateData();dataInterval=setInterval(updateData,2000);}";
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