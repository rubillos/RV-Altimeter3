#include "beep.h"

volatile uint8_t beep_pin;
volatile uint8_t beep_state;

void ARDUINO_ISR_ATTR onTimer() {
    digitalWrite(beep_pin, (beep_state==HIGH) ? LOW:HIGH);
}

Beeper::Beeper(uint8_t pin, uint8_t state) {
    beep_pin = pin;
    beep_state = state;
    pinMode(beep_pin, OUTPUT);
    onTimer();
    _timer = timerBegin(0, 80, true);
    timerAttachInterrupt(_timer, &onTimer, false);
}

void Beeper::beep(uint32_t duration) {
    timerRestart(_timer);
    timerAlarmWrite(_timer, duration * 1000, false);
    timerAlarmEnable(_timer);
    digitalWrite(beep_pin, beep_state);
}
