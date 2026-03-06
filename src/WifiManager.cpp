#include "WiFiManager.h"
#include <Arduino.h>
#include <WiFi.h>
#include <WebServer.h>
#include <Preferences.h>
#include <DNSServer.h>

static Preferences  _prefs;
static WebServer    _apServer(80);
static DNSServer    _dns;
static bool         _credentialsSaved = false;

// ── Credential storage ────────────────────────────────────────────────────────
bool wifiHasCredentials() {
    _prefs.begin(WIFI_PREF_NS, true);
    bool has = _prefs.isKey(PREF_KEY_SSID) && _prefs.getString(PREF_KEY_SSID, "").length() > 0;
    _prefs.end();
    return has;
}

static void saveCredentials(const String& ssid, const String& pass) {
    _prefs.begin(WIFI_PREF_NS, false);
    _prefs.putString(PREF_KEY_SSID, ssid);
    _prefs.putString(PREF_KEY_PASS, pass);
    _prefs.end();
}

static void loadCredentials(String& ssid, String& pass) {
    _prefs.begin(WIFI_PREF_NS, true);
    ssid = _prefs.getString(PREF_KEY_SSID, "");
    pass = _prefs.getString(PREF_KEY_PASS, "");
    _prefs.end();
}

void wifiClearCredentials() {
    _prefs.begin(WIFI_PREF_NS, false);
    _prefs.clear();
    _prefs.end();
    Serial.println("[WiFi] Credentials cleared.");
}

// ── AP setup portal HTML ──────────────────────────────────────────────────────
static String buildSetupPage(bool showError = false) {
    // Scan networks
    int n = WiFi.scanNetworks();

    String options = "";
    for (int i = 0; i < n; i++) {
        String lock = (WiFi.encryptionType(i) == WIFI_AUTH_OPEN) ? "" : " &#128274;";
        options += "<option value=\"" + WiFi.SSID(i) + "\">"
                 + WiFi.SSID(i) + lock
                 + " (" + String(WiFi.RSSI(i)) + " dBm)</option>\n";
    }

    String error = showError
        ? "<div class='err'>Could not connect. Check password and try again.</div>"
        : "";

    return R"(<!DOCTYPE html><html><head>
<meta charset='UTF-8'>
<meta name='viewport' content='width=device-width,initial-scale=1'>
<title>Room Sensor Setup</title>
<style>
  *{box-sizing:border-box;margin:0;padding:0;}
  body{font-family:Arial,sans-serif;background:#f4f6f9;display:flex;align-items:center;justify-content:center;min-height:100vh;padding:20px;}
  .card{background:#fff;border-radius:16px;box-shadow:0 4px 24px rgba(0,0,0,0.10);padding:32px 28px;width:100%;max-width:400px;}
  h2{font-size:20px;color:#222;margin-bottom:6px;}
  p{font-size:13px;color:#888;margin-bottom:24px;}
  label{font-size:13px;font-weight:bold;color:#555;display:block;margin-bottom:6px;}
  select,input{width:100%;padding:10px 12px;border:1.5px solid #ddd;border-radius:8px;font-size:14px;margin-bottom:16px;outline:none;}
  select:focus,input:focus{border-color:#3498db;}
  button{width:100%;padding:12px;background:#3498db;color:#fff;border:none;border-radius:8px;font-size:15px;font-weight:bold;cursor:pointer;}
  button:hover{background:#2980b9;}
  .err{background:#fdedec;color:#c0392b;border:1px solid #f5b7b1;border-radius:8px;padding:10px 14px;font-size:13px;margin-bottom:16px;}
  .spinner{display:none;text-align:center;margin-top:16px;font-size:13px;color:#888;}
</style></head><body>
<div class='card'>
  <h2>&#127968; Room Sensor Setup</h2>
  <p>Select your WiFi network and enter the password to connect.</p>
  )" + error + R"(
  <form method='POST' action='/connect' onsubmit='document.getElementById("sp").style.display="block"'>
    <label>WiFi Network</label>
    <select name='ssid'>)" + options + R"(</select>
    <label>Password</label>
    <input type='password' name='pass' placeholder='Enter WiFi password' autocomplete='off'>
    <button type='submit'>Connect</button>
  </form>
  <div class='spinner' id='sp'>&#9889; Connecting, please wait...</div>
</div>
</body></html>)";
}

static String buildSuccessPage(const String& ssid) {
    return R"(<!DOCTYPE html><html><head>
<meta charset='UTF-8'>
<meta name='viewport' content='width=device-width,initial-scale=1'>
<title>Connected</title>
<style>
  *{box-sizing:border-box;margin:0;padding:0;}
  body{font-family:Arial,sans-serif;background:#f4f6f9;display:flex;align-items:center;justify-content:center;min-height:100vh;padding:20px;}
  .card{background:#fff;border-radius:16px;box-shadow:0 4px 24px rgba(0,0,0,0.10);padding:32px 28px;width:100%;max-width:400px;text-align:center;}
  h2{color:#1e8449;margin-bottom:12px;}
  p{font-size:14px;color:#555;line-height:1.6;}
</style></head><body>
<div class='card'>
  <h2>&#10003; Connected!</h2>
  <p>Successfully joined <strong>)" + ssid + R"(</strong>.<br><br>
  The sensor is restarting. Connect back to your home WiFi and open the sensor's IP address in your browser.</p>
</div>
</body></html>)";
}

// ── AP portal request handlers ────────────────────────────────────────────────
static void handleSetupRoot() {
    _apServer.send(200, "text/html", buildSetupPage());
}

static void handleConnect() {
    if (!_apServer.hasArg("ssid") || !_apServer.hasArg("pass")) {
        _apServer.send(400, "text/plain", "Missing fields");
        return;
    }

    String ssid = _apServer.arg("ssid");
    String pass = _apServer.arg("pass");

    Serial.printf("[WiFi] Trying to connect to: %s\n", ssid.c_str());

    WiFi.mode(WIFI_AP_STA);
    WiFi.begin(ssid.c_str(), pass.c_str());

    unsigned long start = millis();
    while (WiFi.status() != WL_CONNECTED && millis() - start < 15000) {
        _dns.processNextRequest();
        _apServer.handleClient();
        delay(10);
    }

    if (WiFi.status() == WL_CONNECTED) {
        Serial.printf("[WiFi] Connected! IP: %s\n", WiFi.localIP().toString().c_str());
        saveCredentials(ssid, pass);
        _apServer.send(200, "text/html", buildSuccessPage(ssid));
        delay(2000);
        ESP.restart();
    } else {
        Serial.println("[WiFi] Connection failed.");
        WiFi.disconnect();
        _apServer.send(200, "text/html", buildSetupPage(true));
    }
}

// ── Start AP portal and block until credentials are saved ─────────────────────
static void startPortal() {
    Serial.println("[WiFi] Starting setup portal...");

    WiFi.mode(WIFI_AP);
    WiFi.softAP(AP_SSID, AP_PASSWORD);

    Serial.printf("[WiFi] AP started — SSID: %s  IP: %s\n", AP_SSID, AP_IP);

    // DNS server redirects all domains to the portal (captive portal behaviour)
    _dns.start(53, "*", WiFi.softAPIP());

    _apServer.on("/",                      HTTP_GET,  handleSetupRoot);
    _apServer.on("/connect",               HTTP_POST, handleConnect);
    // Captive portal probes from Android, iOS, Windows
    _apServer.on("/generate_204",          HTTP_GET,  handleSetupRoot);
    _apServer.on("/fwlink",                HTTP_GET,  handleSetupRoot);
    _apServer.on("/hotspot-detect.html",   HTTP_GET,  handleSetupRoot);
    _apServer.on("/connecttest.txt",       HTTP_GET,  handleSetupRoot);
    _apServer.on("/ncsi.txt",              HTTP_GET,  handleSetupRoot);
    _apServer.onNotFound([]() { _apServer.sendHeader("Location", "/"); _apServer.send(302); });
    _apServer.begin();

    Serial.println("[WiFi] Portal running. Connect to AP and open http://" AP_IP);

    // Block here — loop DNS and server until credentials saved and ESP restarts
    while (true) {
        _dns.processNextRequest();
        _apServer.handleClient();
    }
}

// ── Public entry point ────────────────────────────────────────────────────────
void wifiManagerBegin() {
    String ssid, pass;

    if (wifiHasCredentials()) {
        loadCredentials(ssid, pass);
        Serial.printf("[WiFi] Saved credentials found. Connecting to: %s\n", ssid.c_str());

        WiFi.mode(WIFI_STA);
        WiFi.begin(ssid.c_str(), pass.c_str());

        unsigned long start = millis();
        while (WiFi.status() != WL_CONNECTED && millis() - start < 15000) {
            delay(10);
            Serial.print(".");
        }
        Serial.println();

        if (WiFi.status() == WL_CONNECTED) {
            Serial.printf("[WiFi] Connected! IP: %s\n", WiFi.localIP().toString().c_str());
            return; // all good, continue normal boot
        }

        // Saved credentials failed — clear them and fall through to portal
        Serial.println("[WiFi] Saved credentials failed. Starting portal...");
        wifiClearCredentials();
    }

    // No credentials or failed — start the setup portal
    startPortal(); // blocks until restart
}