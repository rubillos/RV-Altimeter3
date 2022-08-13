#ifndef DEF_PACKETS_H
#define DEF_PACKETS_H
#include <RadioLib.h>
#include "elapsedMillis64.h"

typedef struct {
    uint32_t id;
    float pressure;
    float temperature;
    bool low_battery;
    bool fast_leak;
    bool stale;
    elapsedSeconds timestamp;
} TPMS_Packet;

void packetsBegin();
bool readPacket(TPMS_Packet* packet);

#endif
