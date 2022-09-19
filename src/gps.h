#ifndef DEF_GPS_H
#define DEF_GPS_H

#include "defs.h"
#include "Arduino.h"
#include <SparkFun_u-blox_GNSS_Arduino_Library.h>
#include <RTClib.h>
#include <ZoneCalc.h>
#include "elapsedMillis.h"
#include "RingBuff.h"

constexpr uint16_t ringCount = 400;
constexpr uint16_t accumulateCount = 1;

typedef struct {
	bool haveFix;
	int8_t zoneOffset;
	uint8_t fixType;
	float satellites;
	float latitude;
	float longitude;
	float altitude;
	float heading;
	float speed;
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
};

extern GPSData _gpsData;
extern GPS _gps;

#endif
