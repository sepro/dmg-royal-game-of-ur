#include "input.h"

// Internal state
static uint8_t current_input = 0;
static uint8_t prev_input = 0;

void input_update(void) {
    prev_input = current_input;
    current_input = joypad();
}

uint8_t input_pressed(uint8_t button_mask) {
    // Edge detection: newly pressed = current AND NOT previous
    return (current_input & button_mask) & ~prev_input;
}

void input_reset(void) {
    current_input = 0;
    prev_input = 0;
}

uint8_t input_current(void) {
    return current_input;
}
