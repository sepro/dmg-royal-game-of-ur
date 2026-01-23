/**
 * main.c
 * Entry point and main game loop for Royal Game of Ur
 */

#include <gb/gb.h>
#include <stdint.h>
#include "game_types.h"
#include "title.h"

// Global game state
ScreenState_t current_state = STATE_TITLE;
ScreenState_t next_state = STATE_TITLE;

/**
 * Main entry point
 */
void main(void) {
    // Initialize the title screen
    init_title();

    // Enable display with background and sprites
    DISPLAY_ON;
    SHOW_BKG;
    SHOW_SPRITES;

    // Main game loop
    while (1) {
        // Handle state transitions
        if (next_state != current_state) {
            // Cleanup current state if needed
            switch (current_state) {
                case STATE_TITLE:
                    cleanup_title();
                    break;
                default:
                    break;
            }

            // Initialize next state
            current_state = next_state;
            switch (current_state) {
                case STATE_TITLE:
                    init_title();
                    break;
                default:
                    break;
            }
        }

        // Update current state
        switch (current_state) {
            case STATE_TITLE:
                update_title();
                break;
            default:
                break;
        }

        // Wait for VBlank before next frame
        wait_vbl_done();
    }
}
