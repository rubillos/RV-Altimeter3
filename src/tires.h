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
        void begin(Adafruit_RA8875& tft, Buffer8& buffer, Beeper& beeper,
                        uint16_t yOffset, uint32_t* sensorIDs,
                        float *pressureMin, float* pressureMax, float* temperatureMax) {
            _tft = &tft;
            _buffer = &buffer;
            _beeper = &beeper;
            _yOffset = yOffset;
            _sensorIDs = sensorIDs;
            _pressureMax = pressureMax;
            _pressureMin = pressureMin;
            _temperatureMax = temperatureMax;
        };

        void drawTires();
        void showTemperature();
        void recordPacket(TPMS_Packet& packet);
        void sensorIDChanged(uint16_t sensorIndex);

    private:
        int16_t indexOfSensor(uint32_t sensor_id);

        Adafruit_RA8875* _tft;
        Buffer8* _buffer;
        Beeper* _beeper;
        uint16_t _yOffset;
        uint32_t* _sensorIDs;
        elapsedMillis _tempTimer = temperatureTime;

        bool _alarms[numTires];

        float* _pressureMin;
        float* _pressureMax;
        float* _temperatureMax;

        TPMS_Packet _sensorPackets[numTires];
};

#endif
