#ifndef DEF_BEEP_CPP
#define DEF_BEEP_CPP

#include "Arduino.h"

class Beeper {
	public:
		Beeper(uint8_t pin, uint8_t state);

		void beep(uint32_t duration, bool ignoreMute=false);

		bool muted();
		void setMute(bool muted);
	
	private:
		hw_timer_t* _timer;

};

extern Beeper _beeper;

#endif
