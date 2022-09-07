#ifndef DEF_PREFS_H
#define DEF_PREFS_H

#include "Arduino.h"
#include "defs.h"
#include <Preferences.h>
#include "touchscreen.h"

#define PREFS_VERSION 5

typedef struct {
	uint32_t sensorIDs[NUM_TIRES];

	float alarmPressureMin;
	float alarmPressureMax;
	float alarmTempMax;

	tsMatrix_t touchCalibration;
} PrefDataRec;

class Prefs {
	public:
        void readPrefs();
        void writePrefs();

    private:
        Preferences _preferences;
};

extern PrefDataRec _prefData;
extern Prefs _prefs;

#endif
