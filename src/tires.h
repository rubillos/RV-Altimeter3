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

class TireHandler {
	public:
		void begin(uint16_t yOffset) {
			_yOffset = yOffset;
		};

		void drawTires();
		void showTemperature();
		void recordPacket(TPMSPacket& packet);
		void sensorIDChanged(uint16_t sensorIndex);

		uint32_t* sensorIDs;

	private:
		int16_t indexOfSensor(uint32_t sensorID);

		uint16_t _yOffset;
		elapsedMillis _tempTimer = temperatureTime;

		bool _alarms[numTires];
		TPMSPacket _sensorPackets[numTires];
};

extern TireHandler _tireHandler;

#endif
