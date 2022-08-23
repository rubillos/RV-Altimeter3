#include "prefs.h"

PrefDataRec _prefData;
Prefs _prefs;

const char* _sensorKeys[] = { "id0", "id1", "id2", "id3", "id4", "id5" };

void Prefs::readPrefs(TouchScreen& touchScreen) {
	_preferences.begin("altometer", false);
	if (PREFS_VERSION != _preferences.getUInt("version")) {
		Serial.println("Prefs version does not match, clearing.");
		_preferences.clear();
	}


	for (uint8_t i=0; i<NUM_TIRES; i++) {
		_prefData.sensorIDs[i] = _preferences.getULong(_sensorKeys[i], 0);
	}
	_prefData.alarmPressureMin = _preferences.getFloat("pressMin", 80.0);
	_prefData.alarmPressureMax = _preferences.getFloat("pressMax", 115.0);
	_prefData.alarmTempMax = _preferences.getFloat("tempMax", 158.0);
	if (_preferences.getBytes("touchMatrix", &_prefData.touchCalibration, sizeof(_prefData.touchCalibration))) {
		touchScreen.setTouchMatrix(&_prefData.touchCalibration);
	}
}

void Prefs::writePrefs() {
	_preferences.putUInt("version", PREFS_VERSION);
	for (uint8_t i=0; i<NUM_TIRES; i++) {
		_preferences.putULong(_sensorKeys[i], _prefData.sensorIDs[i]);
	}
	_preferences.putFloat("pressMin", _prefData.alarmPressureMin);
	_preferences.putFloat("pressMax", _prefData.alarmPressureMax);
	_preferences.putFloat("tempMax", _prefData.alarmTempMax);
	_preferences.putBytes("touchMatrix", &_prefData.touchCalibration, sizeof(_prefData.touchCalibration));
}

