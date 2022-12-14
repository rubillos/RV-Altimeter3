#ifndef DEF_PACKETS_H
#define DEF_PACKETS_H

#include <RadioLib.h>

#include "elapsedMillis.h"

// negative pressure or temperature indicates last value before stopping
constexpr float noDataValue = -1000;		// no data yet
constexpr float timedOutValue = -1001;		// sensor is offline
constexpr float sensorNotPaired = -1002;

typedef struct {
		uint32_t timeStamp;
		uint32_t id;
		float pressure;
		float temperature;
		bool lowBattery;
		bool fastLeak;
		int16_t rssi;
		uint16_t duplicateCount;
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
		void replaceSample(uint16_t itemIndex, TPMSPacket& data) {
			if (_count > 0 && itemIndex < _count) {
				int16_t destIndex = _index-1-itemIndex;
				if (destIndex < 0) {
					destIndex += _length;
				}
				_buffer[destIndex] = data;
			}
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

				void setFakePackets(bool doFakes);
				bool fakePackets() { return _doFakePackets; };

				PacketBuff* packetLog() { return _packetLog; }

		private:
				void queueNextFake();
				void makeFakePacket(TPMSPacket* packet);

				PacketBuff* _packetLog;

				bool _doFakePackets;
				elapsedMillis _fakePacketTime;
				uint32_t _fakePacketDelay;
				TPMSPacket _fakePacket;
};

extern PacketMonitor _packetMonitor;

#endif
