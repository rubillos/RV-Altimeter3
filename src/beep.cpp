#include "beep.h"

volatile uint8_t beepPin;
volatile uint8_t beepState;

#define BUZZER_PIN 25
Beeper _beeper(BUZZER_PIN, LOW);

void ARDUINO_ISR_ATTR onTimer() {
    digitalWrite(beepPin, (beepState==HIGH) ? LOW:HIGH);
}

Beeper::Beeper(uint8_t pin, uint8_t state) {
    beepPin = pin;
    beepState = state;
    pinMode(beepPin, OUTPUT);
    onTimer();
    _timer = timerBegin(0, 80, true);
    timerAttachInterrupt(_timer, &onTimer, false);
}

void Beeper::beep(uint32_t duration) {
    timerRestart(_timer);
    timerAlarmWrite(_timer, duration * 1000, false);
    timerAlarmEnable(_timer);
    digitalWrite(beepPin, beepState);
}
