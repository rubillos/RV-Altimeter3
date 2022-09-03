#ifndef DEF_TIRES_H
#define DEF_TIRES_H

#include "Adafruit_RA8875.h"
#include "packets.h"
#include "elapsedMillis.h"
#include "Buffer8.h"
#include "beep.h"

constexpr uint32_t temperatureTime = 4000;
constexpr uint32_t sensorTimeout = 20 * 60 * 1000;
constexpr uint16_t numTires = 6;

constexpr float noDataValue = -1000;
constexpr float timedOutValue = -1001;

class TireHandler {
	public:
		void begin(uint16_t yOffset) {
			_yOffset = yOffset;
			for (uint16_t i=0; i<numTires; i++) {
				_sensorPackets[i].pressure = noDataValue;
				_sensorPackets[i].temperature = noDataValue;
			}
		};

		void drawTires();
		void showTemperature();
		void recordPacket(TPMSPacket& packet);
		void sensorIDChanged(uint16_t sensorIndex);

		const char* tireName(uint16_t index);
		const char* noDataString();
		const char* timedOutString();

		String pressureString(float pressure, bool addPSI=true);
		String temperatureString(float temperature, uint8_t addDegrees=1);

		uint32_t idForSensor(uint16_t index);
		float pressureForSensor(uint16_t index);
		float temperatureForSensor(uint16_t index);

	private:
		int16_t indexOfSensor(uint32_t sensorID);
		bool sensorTimedOut(uint16_t index);

		uint16_t _yOffset;
		elapsedMillis _tempTimer = temperatureTime;

		bool _alarms[numTires];
		TPMSPacket _sensorPackets[numTires];
};

extern TireHandler _tireHandler;

#endif
