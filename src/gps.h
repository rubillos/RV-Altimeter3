#ifndef DEF_GPS_H
#define DEF_GPS_H

#include "Arduino.h"
#include <SparkFun_u-blox_GNSS_Arduino_Library.h>
#include <RTClib.h>
#include <ZoneCalc.h>
#include "elapsedMillis.h"

typedef struct {
	bool haveFix;
	int8_t zoneOffset;
	uint8_t satellites;
	uint8_t fixType;
	float latitude;
	float longitude;
	float altitude;
	float heading;
	float speed;
	DateTime gpsTimeDate;
	uint32_t movingSeconds;
	uint32_t stoppedSeconds;
} GPSData;

class GPS {
	public:
        void begin();
        bool update();

    private:
        SFE_UBLOX_GNSS _gps;
        ZoneCalc _zoneCalc;

		elapsedMillis _gpsDataTime;
		elapsedSeconds _movingSeconds;
		elapsedSeconds _stoppedSeconds;
};

extern GPSData _gpsData;
extern GPS _gps;

#endif
