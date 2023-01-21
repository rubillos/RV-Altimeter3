#include "beep.h"
#include "pins.h"

#include "driver/ledc.h"
#include "esp_err.h"

#define LEDC_TIMER              LEDC_TIMER_0
#define LEDC_MODE               LEDC_LOW_SPEED_MODE
#define LEDC_CHANNEL            LEDC_CHANNEL_0
#define LEDC_DUTY_RES           LEDC_TIMER_13_BIT
#define LEDC_MAX_DUTY           8191
#define LEDC_DUTY               (LEDC_MAX_DUTY-200)
#define LEDC_FREQUENCY          (250)

volatile uint8_t _beepPin;
volatile uint8_t _beepState;
volatile bool _beepMute;

Beeper _beeper(BUZZER_PIN, LOW);

void ARDUINO_ISR_ATTR onTimer() {
    digitalWrite(_beepPin, (_beepState==HIGH) ? LOW:HIGH);
    // ledc_stop(LEDC_MODE, LEDC_CHANNEL, 0xFFFFF);
}

Beeper::Beeper(uint8_t pin, uint8_t state) {
    _beepPin = pin;
    _beepState = state;

    pinMode(_beepPin, OUTPUT);
    digitalWrite(_beepPin, (_beepState==HIGH) ? LOW:HIGH);
    onTimer();
    _timer = timerBegin(0, 80, true);
    timerAttachInterrupt(_timer, &onTimer, false);

    // ledc_timer_config_t ledc_timer = {
    //     .speed_mode       = LEDC_MODE,
    //     .duty_resolution  = LEDC_DUTY_RES,
    //     .timer_num        = LEDC_TIMER,
    //     .freq_hz          = LEDC_FREQUENCY,
    //     .clk_cfg          = LEDC_AUTO_CLK
    // };
    // // ESP_ERROR_CHECK(ledc_timer_config(&ledc_timer));

    // // Prepare and then apply the LEDC PWM channel configuration
    // ledc_channel_config_t ledc_channel = {
    //     .gpio_num       = _beepPin,
    //     .speed_mode     = LEDC_MODE,
    //     .channel        = LEDC_CHANNEL,
    //     .intr_type      = LEDC_INTR_DISABLE,
    //     .timer_sel      = LEDC_TIMER,
    //     .duty           = LEDC_MAX_DUTY,
    //     .hpoint         = 0,
    //     .flags          = {
    //         .output_invert  = true
    //     }
    // };
    // ESP_ERROR_CHECK(ledc_channel_config(&ledc_channel));
}

void Beeper::beep(uint32_t duration, bool ignoreMute) {
    if (!_beepMute || ignoreMute) {
        timerRestart(_timer);
        timerAlarmWrite(_timer, duration * 1000, false);
        timerAlarmEnable(_timer);
        digitalWrite(_beepPin, _beepState);

        // ESP_ERROR_CHECK(ledc_set_duty_and_update(LEDC_MODE, LEDC_CHANNEL, LEDC_DUTY, 0));
    }
}

bool Beeper::muted() {
    return _beepMute;
}

void Beeper::setMute(bool muted) {
    _beepMute = muted;
}
