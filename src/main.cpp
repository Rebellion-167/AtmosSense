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
  html += "body{font-family:Arial;text-align:center;padding:20px;}";
  html += ".gauge-container{display:flex;justify-content:center;gap:60px;flex-wrap:wrap;}";
  html += ".gauge{width:280px;height:220px;}";
  html += ".dashboard{display:flex;justify-content:center;gap:40px;flex-wrap:wrap;margin-top:30px;}";
  html += ".chart-container{width:45%;min-width:400px;}";
  html += "</style></head><body>";

  html += "<h2>Room Environment Dashboard</h2>";

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

  // Update Function
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