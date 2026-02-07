#ifndef INPUT_H
#define INPUT_H

#include <gb/gb.h>
#include <stdint.h>

/**
 * @file input.h
 * @brief Centralized input handling with edge detection
 *
 * Provides a simple API for reading button presses with automatic edge detection.
 * Call input_update() once per frame, then use input_pressed() to check for newly
 * pressed buttons. Use input_reset() when transitioning between screens to prevent
 * ghost presses.
 */

/**
 * @brief Update input state for current frame
 *
 * Reads joypad state and performs edge detection. Call once per frame before
 * checking button presses.
 */
void input_update(void);

/**
 * @brief Check if button(s) were newly pressed this frame
 *
 * @param button_mask Button mask (J_A, J_B, J_UP, etc.) or combination
 * @return Non-zero if any specified buttons were newly pressed
 */
uint8_t input_pressed(uint8_t button_mask);

/**
 * @brief Reset input state
 *
 * Clears internal state to prevent ghost presses when transitioning between
 * screens. Call in screen init functions.
 */
void input_reset(void);

/**
 * @brief Get current raw joypad state
 *
 * @return Current joypad state (for advanced usage)
 */
uint8_t input_current(void);

#endif // INPUT_H
