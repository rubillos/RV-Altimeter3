#ifndef INCLUDED_RingBuff_h
#define INCLUDED_RingBuff_h

template <class T>
class RingBuff {
  public:
    RingBuff(uint16_t bufferSize) { _buffer = new T[bufferSize](); _sum = _buffer[0]; _zero = _sum; _bufferSize = bufferSize; _index=0; _sampleCount=0; };

    void addSample(T data) {
			if (_sampleCount == _bufferSize) {
				_sum -= _buffer[_index];
			}
      _sum += data;
      _buffer[_index] = data;
      _index = (_index + 1) % _bufferSize;
      if (_sampleCount < _bufferSize) _sampleCount++;
    };
    T average() {
      if (_sampleCount == 0) {
        return _buffer[0];
      }
      return _sum / _sampleCount;
    };
    T minimum(T value) {
      if (_sampleCount == 0) {
        return value;
      }
      int16_t index = _index-1;
      for (uint16_t i=0; i<_sampleCount; i++) {
        if (index < 0) {
          index += _bufferSize;
        }
        value = min(value, _buffer[index--]);
      }
      return value;
    };
    T maximum(T value) {
      if (_sampleCount == 0) {
        return value;
      }
      int16_t index = _index-1;
      for (uint16_t i=0; i<_sampleCount; i++) {
        if (index < 0) {
          index += _bufferSize;
        }
        value = max(value, _buffer[index--]);
      }
      return value;
    };
    T lookup(uint16_t itemIndex) {
      if (_sampleCount == 0) {
        return _buffer[0];
      }
      int16_t destIndex = _index-1-min((uint16_t)itemIndex, (uint16_t)(_sampleCount-1));
      while (destIndex < 0) {
        destIndex += _bufferSize;
      }
      return _buffer[destIndex];
    };
    void replaceSample(uint16_t itemIndex, T sample) {
      if (_sampleCount > 0 && itemIndex < _sampleCount) {
        int16_t destIndex = _index-1-itemIndex;
        if (destIndex < 0) {
          destIndex += _bufferSize;
        }
        _buffer[destIndex] = sample;
      }
    };
    T nextSample() {
			if (_sampleCount) {
        int16_t srcIndex = _index - _sampleCount;
        if (srcIndex < 0) {
          srcIndex += _bufferSize;
        }
        T sample = _buffer[srcIndex];
				_sum -= sample;
        _sampleCount--;
        return sample;
			}
      else {
        return _zero;
      }
    };
    uint16_t sampleCount() { return _sampleCount; };
    bool full() { return _sampleCount == _bufferSize; };
    void reset() { _sampleCount = 0; _sum = _zero; };

  private:
    T* _buffer;
    T _sum;
		T _zero;
    uint16_t _bufferSize;
    uint16_t _sampleCount;
    uint16_t _index;
};

template <class T>
class LoopBuff {
  public:
    LoopBuff(uint16_t bufferSize) { _buffer = new T[bufferSize](); _bufferSize = bufferSize; _index=0; _sampleCount=0; };

    void addSample(T data) {
      _buffer[_index] = data;
      _index = (_index + 1) % _bufferSize;
      if (_sampleCount < _bufferSize) _sampleCount++;
    };
    T lookup(uint16_t itemIndex) {
      if (_sampleCount == 0) {
        return _buffer[0];
      }
      int16_t destIndex = _index-1-min((uint16_t)itemIndex, (uint16_t)(_sampleCount-1));
      while (destIndex < 0) {
        destIndex += _bufferSize;
      }
      return _buffer[destIndex];
    };
    void replaceSample(uint16_t itemIndex, T sample) {
      if (_sampleCount > 0 && itemIndex < _sampleCount) {
        int16_t destIndex = _index-1-itemIndex;
        if (destIndex < 0) {
          destIndex += _bufferSize;
        }
        _buffer[destIndex] = sample;
      }
    };
    uint16_t sampleCount() { return _sampleCount; };
    void reset() { _sampleCount = 0; }

  private:
    T* _buffer;
    uint16_t _bufferSize;
    uint16_t _sampleCount;
    uint16_t _index;
};

#endif
