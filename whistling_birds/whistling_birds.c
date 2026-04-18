#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/pwm.h"

// =======================
// USER PARAMETERS
// =======================
#define NUM_LEDS 12
const uint LED_PINS[NUM_LEDS] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11};

#define BUTTON_PIN 15

#define MAX_BRIGHTNESS 65535   // 0–65535
#define GLOW_BRIGHTNESS 10000  // 0-65535
#define FADE_IN_TIME_MS 500    // milliseconds
#define FADE_OUT_TIME_MS 500   // milliseconds
#define STEPS 50               // smoothness
#define DEBOUNCE_DELAY_MS 200  // milliseconds

// =======================
// GLOBALS
// =======================
uint slices[NUM_LEDS];
uint channels[NUM_LEDS];

// =======================
// FUNCTIONS
// =======================
void burst(uint slice, uint channel) {
    pwm_set_chan_level(slice, channel, MAX_BRIGHTNESS);
    sleep_ms((int)100);
    pwm_set_chan_level(slice, channel, 0);
}
void run_sequence() {
    for (int i = 0; i <= 6; i++) {
        pwm_set_chan_level(slices[i], channels[i], GLOW_BRIGHTNESS);
    }
    sleep_ms((int)200);

    for (int i = 7; i <= 10; i++) {
        pwm_set_chan_level(slices[i], channels[i], GLOW_BRIGHTNESS);
    }
    sleep_ms((int)200);

    pwm_set_chan_level(slices[11], channels[11], GLOW_BRIGHTNESS);
    sleep_ms((int)1500);

    for (int i = 0; i < NUM_LEDS; i++) {
        burst(slices[i], channels[i]);
    }
    sleep_ms((int)1000);
}

// =======================
// MAIN
// =======================
int main() {
    stdio_init_all();

    // Setup LEDs (PWM)
    for (int i = 0; i < NUM_LEDS; i++) {
        gpio_set_function(LED_PINS[i], GPIO_FUNC_PWM);

        slices[i] = pwm_gpio_to_slice_num(LED_PINS[i]);
        channels[i] = pwm_gpio_to_channel(LED_PINS[i]);

        pwm_set_wrap(slices[i], 65535);
        pwm_set_chan_level(slices[i], channels[i], 0);
        pwm_set_enabled(slices[i], true);
    }

    // Setup button
    gpio_init(BUTTON_PIN);
    gpio_set_dir(BUTTON_PIN, GPIO_IN);
    gpio_pull_down(BUTTON_PIN);

    while (true) {
        if (gpio_get(BUTTON_PIN)) {
            run_sequence();
            sleep_ms(DEBOUNCE_DELAY_MS);
        }
    }

    return 0;
}
