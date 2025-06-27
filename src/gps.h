#pragma once

#include "defs.h"
#include "Arduino.h"
#include <SparkFun_u-blox_GNSS_Arduino_Library.h>
#include <RTClib.h>
#include <ZoneCalc.h>
#include "elapsedMillis.h"
#include "RingBuff.h"

constexpr uint16_t ringCount = 400;
constexpr uint16_t accumulateCount = 1;

// const char *fixNames[] = {"No Fix", "Dead Reckoning", "2D", "3D", "GNSS + Dead reckoning", "Time only" };

typedef struct {
	bool haveFix;
	bool haveHadFix;
	int8_t zoneOffset;
	uint8_t fixType;
	float satellites;
	float latitude;
	float longitude;
	float altitude;
	float heading;
	float speed;
	bool speedIsKPH;
	float movingSeconds;
	float stoppedSeconds;
	DateTime utcTimeDate;
	DateTime localTimeDate;
} GPSData;

class GPS {
	public:
        void begin();
        bool update();

		void setPowerSaveMode(bool powerSave);

		void didShake() { _stoppedSeconds = 0; };
		uint32_t stoppedSeconds() { return _stoppedSeconds; };
		uint32_t movingSeconds() { return _movingSeconds; };

		bool currentlyMoving() { return _movingSeconds > 0; };

		void setFakeGPS(bool fakeGPS) { _fakeGPS = fakeGPS; };
		bool fakeGPS() { return _fakeGPS; };

		void setKPHMode(bool kph) { _kphMode = kph; };
		bool isKPHMode() { return _kphMode; };

		RingBuff<int16_t>* speedHistory;
		RingBuff<int16_t>* altitudeHistory;
		uint16_t historyRate() { return 60 / accumulateCount; };

    private:
        SFE_UBLOX_GNSS _gps;
        ZoneCalc _zoneCalc;

		elapsedMillis _gpsDataTime;
		elapsedSeconds _movingSeconds;
		elapsedSeconds _stoppedSeconds;

		RingBuff<int16_t>* _speedAccumulate;
		RingBuff<int16_t>* _altitudeAccumulate;
		uint16_t _accumulateIndex;

		bool _fakeGPS;
		bool _kphMode;
};

extern GPSData _gpsData;
extern GPS _gps;
