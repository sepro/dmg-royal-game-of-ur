/**
 * transition.c
 * Shared screen transition animation system
 *
 * Provides a flash effect when transitioning between screens.
 * Turns display OFF-ON-OFF-ON in a timed sequence for visual feedback.
 */

#include <gb/gb.h>
#include <stdint.h>
#include "game_types.h"
#include "util/transition.h"

// External reference to next_state from main.c
extern ScreenState_t next_state;

// Transition animation state
static uint8_t transition_timer = 0;
static uint8_t transition_phase = TRANSITION_IDLE;
static ScreenState_t transition_target = STATE_TITLE;
static uint8_t transition_phase_count = 0;

/**
 * Start a screen transition animation
 */
void transition_start(ScreenState_t target, uint8_t phase_count) {
    transition_phase = 0;
    transition_timer = TRANSITION_FLASH_DURATION;
    transition_target = target;
    transition_phase_count = phase_count;
    DISPLAY_OFF;
}

/**
 * Update transition animation (called every frame)
 * Returns 1 if transition is active, 0 if complete
 */
uint8_t update_transition(void) {
    if (transition_phase == TRANSITION_IDLE) {
        return 0;
    }

    if (transition_timer > 0) {
        transition_timer--;
        return 1;
    }

    transition_phase++;

    if (transition_phase >= transition_phase_count) {
        transition_phase = TRANSITION_IDLE;
        DISPLAY_ON;
        next_state = transition_target;
        return 0;
    }

    transition_timer = TRANSITION_FLASH_DURATION;

    if (transition_phase & 1) {
        DISPLAY_ON;
    } else {
        DISPLAY_OFF;
    }

    return 1;
}
