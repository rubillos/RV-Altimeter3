#ifndef DEF_BEEP_CPP
#define DEF_BEEP_CPP

#include "Arduino.h"

extern void beep(uint8_t pin, uint8_t state, uint32_t duration);

class Beeper {
	public:
		Beeper(uint8_t pin, uint8_t state);

		void beep(uint32_t duration);
	
	private:
		hw_timer_t* _timer;

};

#endif
