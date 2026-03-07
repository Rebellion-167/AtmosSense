#ifndef ROOM_CONFIG_H
#define ROOM_CONFIG_H

#define ROOM_NAME_MAX    32
#define ROOM_DEFAULT     "My Room"

// Load room name into buf (max ROOM_NAME_MAX chars)
void roomConfigBegin();
const char* roomGetName();
void roomSetName(const char* name);

#endif // ROOM_CONFIG_H