#ifndef WIFI_MANAGER_H
#define WIFI_MANAGER_H

// AP mode config
#define AP_SSID     "RoomSensor-Setup"
#define AP_PASSWORD "setup1234"
#define AP_IP       "192.168.4.1"

// Preferences namespace
#define WIFI_PREF_NS  "wifi"
#define PREF_KEY_SSID "ssid"
#define PREF_KEY_PASS "pass"

// Optional callback — called when AP portal starts so the caller
// can display connection instructions (e.g. on OLED) without
// WiFiManager needing to know about OledDisplay.
void wifiManagerBegin();
bool wifiHasCredentials();
void wifiClearCredentials();

#endif // WIFI_MANAGER_H