#include "prefs.h"
#include "touchscreen.h"
#include "beep.h"
#include "gps.h"
#include "packets.h"

PrefDataRec _prefData;
Prefs _prefs;

const char* _sensorKeys[] = { "id0", "id1", "id2", "id3", "id4", "id5" };

void Prefs::readPrefs() {
	_preferences.begin("altometer", false);
	if (PREFS_VERSION != _preferences.getUInt("version")) {
		Serial.println("Prefs version does not match, clearing.");
		_preferences.clear();
	}


	for (uint8_t i=0; i<NUM_TIRES; i++) {
		_prefData.sensorIDs[i] = _preferences.getULong(_sensorKeys[i], 0);
	}
	_prefData.alarmPressureMin = _preferences.getFloat("pressMin", 80.0);
	_prefData.alarmPressureMax = _preferences.getFloat("pressMax", 120.0);
	_prefData.alarmTempMax = _preferences.getFloat("tempMax", 140.0);
	if (_preferences.getBytes("touchMatrix", &_prefData.touchCalibration, sizeof(_prefData.touchCalibration))) {
		_touchScreen.setTouchMatrix(&_prefData.touchCalibration);
	}
	_beeper.setMute(_preferences.getBool("mute", false));
	_gps.setFakeGPS(_preferences.getBool("fakeGPS", false));
	_packetMonitor.setFakePackets(_preferences.getBool("fakePackets", false));
	setScreenCanDim(_preferences.getBool("screenCanDim", true));
}

void Prefs::writePrefs() {
	Serial.println("Writing Prefs.");
	
	_preferences.putUInt("version", PREFS_VERSION);
	for (uint8_t i=0; i<NUM_TIRES; i++) {
		_preferences.putULong(_sensorKeys[i], _prefData.sensorIDs[i]);
	}
	_preferences.putFloat("pressMin", _prefData.alarmPressureMin);
	_preferences.putFloat("pressMax", _prefData.alarmPressureMax);
	_preferences.putFloat("tempMax", _prefData.alarmTempMax);
	_preferences.putBytes("touchMatrix", &_prefData.touchCalibration, sizeof(_prefData.touchCalibration));
	_preferences.putBool("mute", _beeper.muted());
	_preferences.putBool("fakeGPS", _gps.fakeGPS());
	_preferences.putBool("fakePackets", _packetMonitor.fakePackets());
	_preferences.putBool("screenCanDim", screenCanDim());

}

Preferences _preferences;
