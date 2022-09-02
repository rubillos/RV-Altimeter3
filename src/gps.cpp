#include "gps.h"

GPSData _gpsData;

int16_t speeds[] = { 5, 30, 35, 30, 45, 65, 70, 65, 75, 75, 60, 65, 35, 20, 35, 5 };
int16_t altitudes[] = { 200, 300, 700, 600, 1200, 1100, 1500, 2000, 3000, 2500, 3500, 4000, 5000, 2000, 500, 100 };
uint16_t fakeIndex = 0;

constexpr uint16_t fakeCount = 16;
constexpr uint16_t interpCount = 10;

void GPS::begin() {
	_speedAccumulate = new RingBuff<int16_t>(accumulateCount);
	_altitudeAccumulate = new RingBuff<int16_t>(accumulateCount);

	speedRing = new RingBuff<int16_t>(ringCount);
	altitudeRing = new RingBuff<int16_t>(ringCount);

	_stoppedSeconds = 10000;

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

void GPS::setPowerSaveMode(bool powerSave) {
	_gps.powerSaveMode(powerSave, 1100);
}

constexpr float DEGREES_TO_FLOAT = 10000000;
constexpr float HEADING_TO_FLOAT = 100000;
constexpr float SPEED_TO_FLOAT_MPH = 447.04;
constexpr float DISTANCE_TO_FLOAT_FLOAT = 304.8;

constexpr float movingThreshold = 5.0;

float altitudeList[] = { 100, 500, 2500, 2000, 3500, 3000, 6000, 5500, 12000, 5000, 5500, 4500 };
constexpr uint16_t altitudeCount = sizeof(altitudeList) / sizeof(float);

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

		uint32_t readTime = gpsTime;

	#if 1
		if (!_gpsData.haveFix) {
			static elapsedMillis upTime;
			static float altitudeP;

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
			_gpsData.altitude = sequenceInterp(altitudeList, altitudeCount, altitudeP);
			altitudeP += 0.001;
			if (altitudeP>1.0) {
				altitudeP -= 1.0;
			}
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

		bool moving = _gpsData.speed >= movingThreshold;

		if (moving) {
			_stoppedSeconds = 0;
		}
		else {
			_movingSeconds = 0;

		}

		if (!moving && _stoppedSeconds>30) {
			if (_speedAccumulate->lookup(0)!=-1) {
				_speedAccumulate->addSample(-1);
				_altitudeAccumulate->addSample(-1);
			}
		}
		else {
			_speedAccumulate->addSample(_gpsData.speed);
			_altitudeAccumulate->addSample(_gpsData.altitude);
		}

		_accumulateIndex++;
		if (_accumulateIndex == accumulateCount) {
			int16_t speedMax = _speedAccumulate->maximum(-1);
			int16_t altitudeMax = _altitudeAccumulate->maximum(-1);
			if (speedMax!=-1 || (speedRing->sampleCount()>0 && speedRing->lookup(0)!=-1)) {
				speedRing->addSample(speedMax);
				altitudeRing->addSample(altitudeMax);
			}
			_accumulateIndex = 0;
		}

		_gpsData.movingSeconds = _movingSeconds;
		_gpsData.stoppedSeconds = _stoppedSeconds;

		uint32_t dataTime = gpsTime;
		Serial.printf("GPS Data (%d/%dmS): Alt=%0.1f, Heading=%0.2f, GMTOffset=%d, fix=%d/%d, sat#=%0.0f, lat=%0.5f, lon=%0.5f, speed=%0.1f\n",
								readTime, dataTime, _gpsData.altitude, _gpsData.heading, _gpsData.zoneOffset,
								_gpsData.haveFix, _gpsData.fixType, _gpsData.satellites,
								_gpsData.latitude,  _gpsData.longitude, _gpsData.speed);
	}

	return haveHadFix;
}

GPS _gps;