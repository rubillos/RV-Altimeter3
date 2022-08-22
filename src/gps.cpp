#include "gps.h"
#include "elapsedMillis.h"

void GPS::begin() {
    if (!_gps.begin(Wire)) { //Connect to the u-blox module using Wire port
        Serial.println("GPS not found!");
        while (1);
    }

	_gps.setI2CpollingWait(25); // Set i2cPollingWait to 25ms
	_gps.setI2COutput(COM_TYPE_UBX);                 //Set the I2C port to output UBX only (turn off NMEA noise)
	_gps.saveConfigSelective(VAL_CFG_SUBSEC_IOPORT); //Save (only) the communications port settings to flash and BBR
	if (!_gps.setDynamicModel(DYN_MODEL_AUTOMOTIVE)) {
		Serial.println("* setDynamicModel failed!");
	}

	// _gps.enableDebugging();

	delay(300);
}

#define DEGREES_TO_FLOAT (10000000)
#define HEADING_TO_FLOAT (100000)
#define SPEED_TO_FLOAT_MPH (447.04)
#define DISTANCE_TO_FLOAT_FLOAT (304.8)

void GPS::getData(GPS_Data& data) {
	data.haveFix = _gps.getGnssFixOk();
	data.year = _gps.getYear();
	data.month = _gps.getMonth();
	data.day = _gps.getDay();
	data.hour = _gps.getHour();
	data.minute = _gps.getMinute();
	data.second = _gps.getSecond();
	data.latitude = (float)_gps.getLatitude() / DEGREES_TO_FLOAT;
	data.longitude = (float)_gps.getLongitude() / DEGREES_TO_FLOAT;
	data.altitude = _gps.getAltitudeMSL() / DISTANCE_TO_FLOAT_FLOAT;
	data.heading = (float)_gps.getHeading() / HEADING_TO_FLOAT;
	data.speed = (float)_gps.getGroundSpeed() / SPEED_TO_FLOAT_MPH;
	data.satellites = _gps.getSIV();
	data.fixType = _gps.getFixType();

#if 1
	if (!data.haveFix) {
		static elapsedMillis upTime;

		data.latitude = 37.7775;
		data.longitude = -122.416389;
		data.year = 2021;
		data.month = 4;
		data.day = 25;
		uint32_t milliTime = upTime;
		uint32_t minutes = milliTime / 1000 / 60;
		data.hour = 12 + (minutes / 60);
		data.minute = minutes % 60;
		uint32_t seconds = (milliTime / 1000) % 60;
		static int16_t headingNum = 0;
		data.heading = headingNum;
		headingNum = (headingNum + 10) % 360;
		data.speed = seconds * 80.0 / 60.0;
		data.altitude = 12005;
		data.haveFix = (upTime > 2000);
	}
#endif

	static int16_t lastZoneOffset = -7;
	bool isDST = _zoneCalc.dateIsDST(data.year, data.month, data.day, data.hour, data.minute, lastZoneOffset);
	float zoneOffset = _zoneCalc.zoneOffsetForGPSCoord(data.latitude, data.longitude, isDST);
	int8_t zoneHour = zoneOffset;
	int8_t zoneMinute = (zoneOffset * 60) - (zoneHour * 60);

	lastZoneOffset = zoneOffset + (isDST ? 1 : 0);

	data.zoneOffset = lastZoneOffset;
	data.gpsTimeDate = DateTime(data.year, data.month, data.day, data.hour, data.minute, data.second) + TimeSpan(0, zoneHour, zoneMinute, 0);
}
