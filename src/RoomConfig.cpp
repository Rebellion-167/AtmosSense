#include "RoomConfig.h"
#include <Preferences.h>
#include <string.h>

#define PREF_NS       "room"
#define PREF_KEY_NAME "name"

static char _roomName[ROOM_NAME_MAX] = ROOM_DEFAULT;
static Preferences _prefs;

void roomConfigBegin() {
    _prefs.begin(PREF_NS, true);
    String saved = _prefs.getString(PREF_KEY_NAME, ROOM_DEFAULT);
    _prefs.end();
    strncpy(_roomName, saved.c_str(), ROOM_NAME_MAX - 1);
    _roomName[ROOM_NAME_MAX - 1] = '\0';
    Serial.printf("[Room] Name: %s\n", _roomName);
}

const char* roomGetName() { return _roomName; }

void roomSetName(const char* name) {
    strncpy(_roomName, name, ROOM_NAME_MAX - 1);
    _roomName[ROOM_NAME_MAX - 1] = '\0';
    _prefs.begin(PREF_NS, false);
    _prefs.putString(PREF_KEY_NAME, _roomName);
    _prefs.end();
    Serial.printf("[Room] Name saved: %s\n", _roomName);
}