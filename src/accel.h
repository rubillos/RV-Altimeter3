#ifndef DEF_ACCEL_H
#define DEF_ACCEL_H

#include <Adafruit_LIS3DH.h>
#include <Adafruit_Sensor.h>
#include "elapsedMillis.h"
#include "RingBuff.h"

constexpr uint16_t shakeRingSize = 100;

class Accelerometer {
	public:
        void begin();
        bool update();

        void setShakeLimits(float xLimit, float yLimit, float zLimit) {
            _xShakeLimit = xLimit;
            _yShakeLimit = yLimit;
            _zShakeLimit = zLimit;
        };
        bool didShake(float& x, float& y, float& z);
        bool didShake();

        bool getEvent(sensors_event_t& event);

    private:
        Adafruit_LIS3DH _lis;
        elapsedMillis _sensorTime;
        sensors_event_t _lastEvent;

  		RingBuff<float>* _xShakeBuff;
  		RingBuff<float>* _yShakeBuff;
  		RingBuff<float>* _zShakeBuff;

        float _xShakeLimit;
        float _yShakeLimit;
        float _zShakeLimit;

        bool _newShakeData;
        elapsedMillis _shakeWait;
};

extern Accelerometer _accel;

#endif