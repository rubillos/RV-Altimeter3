#include "elapsedMillis64.h"

uint64_t millis64() {
  static uint32_t low32 = 0, high32 = 0;
  uint32_t new_low32 = millis();

  if (new_low32 < low32)
    high32++;

  low32 = new_low32;

  return (uint64_t) high32 << 32 | low32;
}

