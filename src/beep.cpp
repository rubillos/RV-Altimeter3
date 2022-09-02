#include "beep.h"
#include "pins.h"

volatile uint8_t _beepPin;
volatile uint8_t _beepState;
volatile bool _beepMute;

Beeper _beeper(BUZZER_PIN, LOW);

void ARDUINO_ISR_ATTR onTimer() {
    digitalWrite(_beepPin, (_beepState==HIGH) ? LOW:HIGH);
}

Beeper::Beeper(uint8_t pin, uint8_t state) {
    _beepPin = pin;
    _beepState = state;
    pinMode(_beepPin, OUTPUT);
    onTimer();
    _timer = timerBegin(0, 80, true);
    timerAttachInterrupt(_timer, &onTimer, false);
}

void Beeper::beep(uint32_t duration, bool ignoreMute) {
    if (!_beepMute || ignoreMute) {
        timerRestart(_timer);
        timerAlarmWrite(_timer, duration * 1000, false);
        timerAlarmEnable(_timer);
        digitalWrite(_beepPin, _beepState);
    }
}

bool Beeper::muted() {
    return _beepMute;
}

void Beeper::setMute(bool muted) {
    _beepMute = muted;
}
