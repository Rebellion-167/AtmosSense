#include <Arduino.h>
#include <WiFi.h>
#include <DHT.h>
#include "Secrets.h"

#define DHTPIN 4
#define DHTTYPE DHT11

DHT dht(DHTPIN, DHTTYPE);

// -------- WIFI --------
const char* ssid = WIFI_SSID;
const char* password = WIFI_PASSWORD;

WiFiServer server(80);

// =====================================================
void setup() {
    Serial.begin(115200);
    dht.begin();

    WiFi.begin(ssid, password);
    Serial.print("Connecting to WiFi");

    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }

    Serial.println("\nConnected!");
    Serial.print("IP Address: ");
    Serial.println(WiFi.localIP());

    server.begin();
}

// =====================================================
void loop() {
    WiFiClient client = server.available();
    if (!client) return;

    Serial.println("New Client Connected");

    while (client.connected() && !client.available()) {
        delay(1);
    }

    String request = client.readStringUntil('\r');
    Serial.println(request);
    client.flush();

    float temperature = dht.readTemperature();
    float humidity = dht.readHumidity();

    if (isnan(temperature) || isnan(humidity)) {
        temperature = 0;
        humidity = 0;
    }

    client.println("HTTP/1.1 200 OK");
    client.println("Content-type:text/html");
    client.println("Connection: close");
    client.println();
    client.println("<!DOCTYPE html>");
    client.println("<html><body>");
    client.println("<h1>AtmosSense</h1>");
    client.print("<p>Temperature: ");
    client.print(temperature);
    client.println("&deg;C</p>");
    client.print("<p>Humidity: ");
    client.print(humidity);
    client.println(" %</p>");
    client.println("</body></html>");
    client.println();

    delay(1);
    client.stop();
    Serial.println("Client Disconnected");
}