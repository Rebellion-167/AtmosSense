#ifndef WIFI_MANAGER_H
#define WIFI_MANAGER_H

// AP mode config
#define AP_SSID     "RoomSensor-Setup"
#define AP_PASSWORD "setup1234"         // min 8 chars for WPA2
#define AP_IP       "192.168.4.1"

// Preferences namespace
#define WIFI_PREF_NS  "wifi"
#define PREF_KEY_SSID "ssid"
#define PREF_KEY_PASS "pass"

// Call in setup() — blocks until WiFi is connected.
// If no credentials saved, starts AP and serves the setup portal.
// Returns only once STA connection is established.
void wifiManagerBegin();

// Returns true if credentials are stored in NVS
bool wifiHasCredentials();

// Clear saved credentials (useful for reset button)
void wifiClearCredentials();

#endif // WIFI_MANAGER_H