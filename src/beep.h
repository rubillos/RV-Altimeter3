#ifndef DEF_BEEP_CPP
#define DEF_BEEP_CPP

#include "Arduino.h"

extern void beep(uint8_t pin, uint8_t state, uint32_t duration, uint32_t frequency=200);

#endif
