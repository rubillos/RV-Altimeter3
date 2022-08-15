#include "beep.h"

volatile hw_timer_t* timer = NULL;
uint8_t beep_pin = 0;
uint8_t beep_state;

void ARDUINO_ISR_ATTR onTimer() {
    timerEnd((hw_timer_t*)timer);
    timer = NULL;
    digitalWrite(beep_pin, (beep_state==HIGH) ? LOW:HIGH);
}

void beep(uint8_t pin, uint8_t state, uint32_t duration) {
    if (timer == NULL) {
        beep_pin = pin;
        beep_state = state;
        timer = timerBegin(0, 80, true);
        timerAttachInterrupt((hw_timer_t*)timer, &onTimer, false);
        timerAlarmWrite((hw_timer_t*)timer, duration * 1000, false);
        timerAlarmEnable((hw_timer_t*)timer);
        digitalWrite(beep_pin, state);
    }
}
