#include "accel.h"

constexpr uint32_t accelTime = 100;

void Accelerometer::begin() {
    if (!_lis.begin(0x18)) {   // change this to 0x19 for alternative i2c address
        Serial.println("Accelerometer not found!");
        while (1);
    }
    _lis.setRange(LIS3DH_RANGE_4_G);
    _lis.setDataRate(LIS3DH_DATARATE_100_HZ);

   	_xShakeBuff = new RingBuff<float>(shakeRingSize);
   	_yShakeBuff = new RingBuff<float>(shakeRingSize);
   	_zShakeBuff = new RingBuff<float>(shakeRingSize);

    _sensorTime = accelTime + 1;

    _shakeWait = 0;
};

bool Accelerometer::update() {
    bool result = false;

    if (_sensorTime > accelTime) {
        _sensorTime = 0;
        // uint16_t count = 0;
        while (_lis.haveNewData()) {
            _lis.read();
            _lis.getEvent(&_lastEvent);

            _xShakeBuff->addSample(_lastEvent.acceleration.x);
            _yShakeBuff->addSample(_lastEvent.acceleration.y);
            _zShakeBuff->addSample(_lastEvent.acceleration.z);

            _newShakeData = true;
            result = true;
            // count++;
        }
        // if (count > 0) {
        //     Serial.printf("Accel: read %d samples.\n", count);
        // }
    }
    return result;
}

void printRing(const char* name, RingBuff<float>* ring) {
    uint16_t count = ring->sampleCount();
    Serial.printf("%s: ", name);
    for (uint16_t i=0; i<count; i++) {
        Serial.printf("%0.2f", ring->lookup(i));
        if (i<count-1) {
            Serial.print(", ");
        }
    }
    Serial.println();
}

bool Accelerometer::didShake(float& x, float& y, float& z) {
    bool result = false;

    update();
    if (_newShakeData && _shakeWait>2000) {
        float xShake = abs(_xShakeBuff->average() - _lastEvent.acceleration.x);
        float yShake = abs(_yShakeBuff->average() - _lastEvent.acceleration.y);
        float zShake = abs(_zShakeBuff->average() - _lastEvent.acceleration.z);

        if (xShake>_xShakeLimit || yShake>_yShakeLimit || zShake>_zShakeLimit) {
            // Serial.printf("Shake: x=%f, y=%f, z=%f\n", xShake, yShake, zShake);
            // printRing("x", _xShakeBuff);
            // printRing("y", _yShakeBuff);
            // printRing("z", _zShakeBuff);
            x = xShake;
            y = yShake;
            z = zShake;
            _newShakeData = false;
            result = true;
        }
    }
    return result;
}

bool Accelerometer::didShake() {
    float x, y, z;
    return didShake(x, y, z);
}

bool Accelerometer::getEvent(sensors_event_t& event) {
    bool result = update();
    event = _lastEvent;
    return result;
}

Accelerometer _accel;
