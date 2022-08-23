#ifndef DEF_PACKETS_H
#define DEF_PACKETS_H
#include <RadioLib.h>
#include "elapsedMillis64.h"

typedef struct {
    uint32_t timeStamp;
    uint32_t id;
    float pressure;
    float temperature;
    bool lowBattery;
    bool fastLeak;
    bool stale;
} TPMSPacket;

class PacketBuff {
  public:
    PacketBuff(uint16_t bufferSize) { _buffer = new TPMSPacket[bufferSize](); _count = bufferSize; _index=0; _length=0; };

    void addSample(TPMSPacket& data) {
      _buffer[_index] = data;
      _index = (_index + 1) % _count;
      if (_length < _count) _length++;
      _hash++;
    };
    TPMSPacket lookup(uint16_t itemIndex) {
      if (_length == 0) {
        return _buffer[0];
      }
      int16_t destIndex = _index-1-min((uint16_t)itemIndex, (uint16_t)(_length-1));
      while (destIndex < 0) {
        destIndex += _count;
      }
      return _buffer[destIndex];
    };
    uint16_t length() { return _length; };
    void clear() { _length = 0; };
    uint32_t hash() { return _hash; };

  private:
    TPMSPacket* _buffer;
    uint16_t _count;
    uint16_t _length;
    uint16_t _index;
    uint32_t _hash;
};

class PacketMonitor {
    public:
        void begin();
        bool getPacket(TPMSPacket* packet);

        PacketBuff* packetLog() { return _packetLog; }

    private:
        PacketBuff* _packetLog;
};

extern PacketMonitor _packetMonitor;

#endif
