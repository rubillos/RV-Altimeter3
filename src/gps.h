#ifndef DEF_GPS_H
#define DEF_GPS_H

#include "Arduino.h"
#include <SparkFun_u-blox_GNSS_Arduino_Library.h>
#include <RTClib.h>
#include <ZoneCalc.h>

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
} GPS_Data;

class GPS {
	public:
        void begin();
        void getData(GPS_Data& data);

    private:
        SFE_UBLOX_GNSS _gps;
        ZoneCalc _zoneCalc;
};

#endif
