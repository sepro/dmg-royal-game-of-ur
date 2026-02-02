/**
 * transition.h
 * Shared screen transition animation system
 */

#ifndef TRANSITION_H
#define TRANSITION_H

#include <stdint.h>
#include "game_types.h"

/**
 * Start a screen transition animation
 * @param target The target screen state to transition to
 * @param phase_count Number of flash phases (use TRANSITION_PHASE_COUNT_3 or TRANSITION_PHASE_COUNT_1)
 */
void transition_start(ScreenState_t target, uint8_t phase_count);

/**
 * Update transition animation (called every frame)
 * @return 1 if transition is active, 0 if complete
 */
uint8_t update_transition(void);

#endif // TRANSITION_H
