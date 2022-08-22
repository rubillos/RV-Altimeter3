#ifndef DEF_GPS_H
#define DEF_GPS_H

#include "Arduino.h"
#include <SparkFun_u-blox_GNSS_Arduino_Library.h>
#include <RTClib.h>
#include <ZoneCalc.h>

typedef struct {
	bool haveFix;
	uint16_t year;
	uint16_t month;
	uint16_t day;
	uint16_t hour;
	uint16_t minute;
	uint16_t second;
	float latitude;
	float longitude;
	float altitude;
	float heading;
	float speed;
	uint8_t zoneOffset;
	uint8_t satellites;
	uint8_t fixType;
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
