#include <stdio.h>
#include <stdlib.h>
#include "pico/stdlib.h"
#include "hardware/pwm.h"

// =======================
// USER PARAMETERS
// =======================
#define NUM_LEDS 12
const uint8_t LED_PINS[NUM_LEDS] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11};
const uint8_t sequence[] = {11, 9, 8, 10, 7, 2, 3, 4, 1, 5, 6, 0};

#define BUTTON_PIN 15

#define MAX_BRIGHTNESS 65535   // 0–65535
#define GLOW_BRIGHTNESS 8000  // 0-65535
#define DEBOUNCE_DELAY_MS 50  // milliseconds

// =======================
// GLOBALS
// =======================
uint slices[NUM_LEDS];
uint channels[NUM_LEDS];

// =======================
// FUNCTIONS
// =======================
void charge_sequence() {
    for (uint8_t i = 0; i <= 6; i++) {
        pwm_set_chan_level(slices[i], channels[i], GLOW_BRIGHTNESS);
    }
    sleep_ms(200);

    for (uint8_t i = 7; i <= 10; i++) {
        pwm_set_chan_level(slices[i], channels[i], GLOW_BRIGHTNESS);
    }
    sleep_ms(200);

    pwm_set_chan_level(slices[11], channels[11], GLOW_BRIGHTNESS);
}

void fire_sequence() {
    uint8_t delay_buffer[NUM_LEDS];
    uint8_t buffer_pos = 0;

    // init buffer
    for (uint8_t i = 0; i < NUM_LEDS; i++) {
        delay_buffer[i] = -1;
    }

    for (uint8_t i = 0; i < NUM_LEDS; i++) {

        uint8_t led = sequence[i];

        // turn ON immediately
        pwm_set_chan_level(slices[led], channels[led], MAX_BRIGHTNESS);

        // store in buffer
        delay_buffer[buffer_pos] = led;
        buffer_pos++;

        // after 2–3 newer LEDs, turn off oldest
        if (buffer_pos >= 3) {
            uint8_t off_index = buffer_pos - 3;
            uint8_t off_led = delay_buffer[off_index];

            if (off_led >= 0) {
                pwm_set_chan_level(slices[off_led], channels[off_led], 0);
            }
        }

        sleep_ms(30 + (rand() % 30));
    }

    // final cleanup (turn off remaining LEDs in buffer)
    for (uint8_t i = 10; i < 12; i++) {
        uint8_t led = delay_buffer[i];
        if (led >= 0) {
            pwm_set_chan_level(slices[led], channels[led], 0);
            sleep_ms(30 + (rand() % 30));
        }
    }
}

// =======================
// MAIN
// =======================
int main() {
    stdio_init_all();

    // Setup LEDs (PWM)
    for (uint8_t i = 0; i < NUM_LEDS; i++) {
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
    gpio_pull_up(BUTTON_PIN);
    uint8_t state = 0;

    // Weird false button presses happen without giving it time to setup
    sleep_ms(50);

    while (true) {
        if (!gpio_get(BUTTON_PIN)) {
            state += 1;
            sleep_ms(DEBOUNCE_DELAY_MS);            // Simple blocking debounce
        }

        if (state == 1){
            charge_sequence();
            state = 2;
        }

        if (state >= 3){
            fire_sequence();
            state = 0;
        }
    }

    return 0;
}
