#include "gps.h"

GPSData _gpsData;

void GPS::begin() {
    if (!_gps.begin(Wire)) { //Connect to the u-blox module using Wire port
        Serial.println("GPS not found!");
        while (1);
    }

	_gps.setI2CpollingWait(25); 					 // Set i2cPollingWait to 25ms
	_gps.setI2COutput(COM_TYPE_UBX);                 //Set the I2C port to output UBX only (turn off NMEA noise)
	_gps.saveConfigSelective(VAL_CFG_SUBSEC_IOPORT); //Save (only) the communications port settings to flash and BBR
	if (!_gps.setDynamicModel(DYN_MODEL_AUTOMOTIVE)) {
		Serial.println("* setDynamicModel failed!");
	}

	// _gps.enableDebugging();

	delay(300);
}

constexpr float DEGREES_TO_FLOAT = 10000000;
constexpr float HEADING_TO_FLOAT = 100000;
constexpr float SPEED_TO_FLOAT_MPH = 447.04;
constexpr float DISTANCE_TO_FLOAT_FLOAT = 304.8;

constexpr float movingThreshold = 5.0;

bool GPS::update() {
	static bool haveHadFix = false;

	static elapsedMillis gpsTime = 800;
	if (gpsTime >= 1000) {
		gpsTime = 0;

		uint16_t year;
		uint8_t month;
		uint8_t day;
		uint8_t hour;
		uint8_t minute;
		uint8_t second;

		_gpsData.haveFix = _gps.getGnssFixOk();

		year = _gps.getYear();
		month = _gps.getMonth();
		day = _gps.getDay();
		hour = _gps.getHour();
		minute = _gps.getMinute();
		second = _gps.getSecond();

		_gpsData.latitude = (float)_gps.getLatitude() / DEGREES_TO_FLOAT;
		_gpsData.longitude = (float)_gps.getLongitude() / DEGREES_TO_FLOAT;
		_gpsData.altitude = _gps.getAltitudeMSL() / DISTANCE_TO_FLOAT_FLOAT;
		_gpsData.heading = (float)_gps.getHeading() / HEADING_TO_FLOAT;
		_gpsData.speed = (float)_gps.getGroundSpeed() / SPEED_TO_FLOAT_MPH;
		_gpsData.satellites = _gps.getSIV();
		_gpsData.fixType = _gps.getFixType();

	#if 1
		if (!_gpsData.haveFix) {
			static elapsedMillis upTime;

			_gpsData.latitude = 37.7775;
			_gpsData.longitude = -122.416389;
			year = 2021;
			month = 4;
			day = 25;
			uint32_t milliTime = upTime;
			uint32_t minutes = milliTime / 1000 / 60;
			hour = 12 + (minutes / 60);
			minute = minutes % 60;
			second = (milliTime / 1000) % 60;
			static int16_t headingNum = 0;
			_gpsData.heading = headingNum;
			headingNum = (headingNum + 10) % 360;
			_gpsData.speed = second * 80.0 / 60.0;
			_gpsData.altitude = 12005;
			_gpsData.haveFix = (upTime > 2000);
		}
	#endif

		static int16_t lastZoneOffset = -7;
		bool isDST = _zoneCalc.dateIsDST(year, month, day, hour, minute, lastZoneOffset);
		float zoneOffset = _zoneCalc.zoneOffsetForGPSCoord(_gpsData.latitude, _gpsData.longitude, isDST);
		int8_t zoneHour = zoneOffset;
		int8_t zoneMinute = (zoneOffset * 60) - (zoneHour * 60);

		lastZoneOffset = zoneOffset + (isDST ? 1 : 0);

		_gpsData.zoneOffset = lastZoneOffset;
		_gpsData.gpsTimeDate = DateTime(year, month, day, hour, minute, second) + TimeSpan(0, zoneHour, zoneMinute, 0);

		if (_gpsData.haveFix) {
			haveHadFix = true;
			_gpsDataTime = 0;
		}
		if (_gpsData.speed >= movingThreshold) {
			_stoppedSeconds = 0;
		}
		else {
			_movingSeconds = 0;
		}

		_gpsData.movingSeconds = _movingSeconds;
		_gpsData.stoppedSeconds = _stoppedSeconds;
	}
	return haveHadFix;
}

GPS _gps;