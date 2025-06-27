#pragma once

#include "Adafruit_RA8875.h"
#include "packets.h"
#include "elapsedMillis.h"
#include "beep.h"

constexpr uint32_t temperatureTime = 4000;
constexpr uint32_t sensorTimeout = 20 * 60 * 1000;
constexpr uint16_t numTires = 6;

class TireHandler {
	public:
		TireHandler() {
			for (uint16_t i=0; i<numTires; i++) {
				_sensorPackets[i].pressure = noDataValue;
				_sensorPackets[i].temperature = noDataValue;
			}
		};

		void drawTires();
		void showTemperature();
		void recordPacket(TPMSPacket& packet);
		void setSensorID(uint16_t sensorIndex, uint32_t id);

		int16_t indexOfTireAtPoint(tsPoint_t pt);
		void pressTire(uint16_t index);

		void checkSensorData(bool moving);
		void restoreSavedTireData();
		void adjustForRadioReset();

		const char* tireName(uint16_t index, bool shortName=false);
		const char* noDataString();
		const char* timedOutString();

		uint16_t pressureColor(float pressure);
		uint16_t pressureColorForSensor(uint16_t index);
		uint16_t temperatureColor(float temperature);
		uint16_t temperatureColorForSensor(uint16_t index);

		uint32_t idForSensor(uint16_t index);
		int16_t indexOfSensor(uint32_t sensorID);
		float pressureForSensor(uint16_t index);
		float temperatureForSensor(uint16_t index);

		String pressureString(float pressure, bool addPSI=true);
		String pressureStringForSensor(uint16_t index, bool addPSI=true);
		String temperatureString(float temperature, uint8_t addDegrees=1);
		String temperatureStringForSensor(uint16_t index, uint8_t addDegrees=1);

		bool temperatureWarning(float temperature);
		bool temperatureAlarm(float temperature);
		bool pressureWarning(float pressure);
		bool pressureAlarm(float pressure);

	private:
		bool sensorTimedOut(uint16_t index);

		int32_t codedColor(float value);
		String codedString(float value);

		elapsedMillis _tempTimer = temperatureTime;

		bool _alarms[numTires];
		TPMSPacket _sensorPackets[numTires];

		elapsedMillis _sensorCheckTime;
};

extern TireHandler _tireHandler;
