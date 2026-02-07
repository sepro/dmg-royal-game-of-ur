/**
 * link_connect.c
 * Link cable connection screen implementation
 * Shows waiting animation while attempting to connect,
 * then reveals player side assignment based on master/slave role
 */

#include <gb/gb.h>
#include <stdint.h>
#include "game_types.h"
#include "link_connect.h"
#include "link.h"
#include "font.h"
#include "input.h"
#include "transition.h"
#include "coinflip.h"
#include "screen_utils.h"

// External references to generated coin assets (shared with coinflip)
extern const uint8_t light_coin_tiles[];
extern const uint8_t dark_coin_tiles[];

// External reference to next_state from main.c
extern ScreenState_t next_state;

// Local state
static uint8_t connect_phase = CONNECT_PHASE_WAITING;
static uint8_t phase_timer = 0;
static uint8_t dot_count = 0;

// White tile data (8x8 pixels, all color 0 = white on DMG)
static const uint8_t white_tile[16] = {
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};

/**
 * Draw the waiting status with animated dots
 */
static void draw_waiting_status(void) {
    // Clear status line first
    clear_text_row_inverted(CONNECT_STATUS_X, CONNECT_STATUS_Y, 16);

    // Draw base text
    draw_text_inverted(CONNECT_STATUS_X, CONNECT_STATUS_Y, "WAITING");

    // Add animated dots (0-3)
    uint8_t x = CONNECT_STATUS_X + 7;
    for (uint8_t i = 0; i < dot_count; i++) {
        draw_text_inverted(x + i, CONNECT_STATUS_Y, ".");
    }
}

/**
 * Draw a coin at the specified position
 */
static void draw_coin(uint8_t x, uint8_t y, uint8_t tile_base) {
    uint8_t row_buf[COIN_WIDTH];
    for (uint8_t row = 0; row < COIN_HEIGHT; row++) {
        for (uint8_t col = 0; col < COIN_WIDTH; col++) {
            row_buf[col] = tile_base + (row * COIN_WIDTH) + col;
        }
        set_bkg_tiles(x, y + row, COIN_WIDTH, 1, row_buf);
    }
}

/**
 * Show the side reveal screen
 */
static void show_side_reveal(void) {
    // Clear screen
    fill_screen_with_tile(CONNECT_WHITE_TILE);

    // Draw "CONNECTED!" title (one line higher than default)
    draw_text_inverted(CONNECT_TITLE_X, CONNECT_REVEAL_Y, "CONNECTED!");

    // Draw coin based on role (master=light, slave=dark)
    if (link_role == LINK_ROLE_MASTER) {
        draw_coin(CONNECT_COIN_X, CONNECT_COIN_Y, CONNECT_LIGHT_TILE_START);
        draw_text_inverted(CONNECT_SIDE_X, CONNECT_SIDE_Y, "YOU ARE LIGHT");
    } else {
        draw_coin(CONNECT_COIN_X, CONNECT_COIN_Y, CONNECT_DARK_TILE_START);
        draw_text_inverted(CONNECT_SIDE_X, CONNECT_SIDE_Y, "YOU ARE DARK");
    }
}

/**
 * Initialize link connection screen
 */
void init_link_connect(void) {
    // Disable display during VRAM writes
    DISPLAY_OFF;

    // Initialize link hardware
    link_init();

    // Load white tile at index 0
    set_bkg_data(CONNECT_WHITE_TILE, 1, white_tile);

    // Fill screen with white
    fill_screen_with_tile(CONNECT_WHITE_TILE);

    // Load coin tiles (for side reveal)
    set_bkg_data(CONNECT_LIGHT_TILE_START, CONNECT_LIGHT_TILE_COUNT, light_coin_tiles);
    set_bkg_data(CONNECT_DARK_TILE_START, CONNECT_DARK_TILE_COUNT, dark_coin_tiles);

    // Draw title
    draw_text_inverted(CONNECT_TITLE_X, CONNECT_TITLE_Y, "LINK CABLE MODE");

    // Initialize state
    connect_phase = CONNECT_PHASE_WAITING;
    phase_timer = 0;
    dot_count = 0;

    // Draw initial status
    draw_waiting_status();

    // Draw cancel hint
    draw_text_inverted(2, 15, "B:CANCEL");

    // Clear input state
    input_reset();

    // Set background palette
    BGP_REG = 0xE4;

    // Hide sprites
    HIDE_SPRITES;
    SHOW_BKG;
    DISPLAY_ON;
}

/**
 * Update link connection screen
 */
void update_link_connect(void) {
    // Update transition animation if active
    if (update_transition()) {
        return;
    }

    // Update input state
    input_update();

    switch (connect_phase) {
        case CONNECT_PHASE_WAITING:
            // Animate dots
            phase_timer++;
            if (phase_timer >= CONNECT_DOT_CYCLE) {
                phase_timer = 0;
                dot_count = (dot_count + 1) % 4;
                draw_waiting_status();
            }

            // Try to connect
            link_connect_step();

            // Check for connection
            if (link_status == LINK_CONNECTED) {
                connect_phase = CONNECT_PHASE_SIDE_REVEAL;
                phase_timer = 0;
                show_side_reveal();
            } else if (link_status == LINK_ERROR) {
                // Error: show message and return to title
                fill_screen_with_tile(CONNECT_WHITE_TILE);
                draw_text_inverted(CONNECT_TITLE_X, CONNECT_TITLE_Y, "CONNECTION FAILED");
                draw_text_inverted(2, 9, "PRESS A TO RETRY");

                // Wait for A press
                while (!input_pressed(J_A)) {
                    input_update();
                    wait_vbl_done();
                }

                // Reset and try again
                link_reset();
                connect_phase = CONNECT_PHASE_WAITING;
                phase_timer = 0;
                dot_count = 0;
                fill_screen_with_tile(CONNECT_WHITE_TILE);
                draw_text_inverted(CONNECT_TITLE_X, CONNECT_TITLE_Y, "LINK CABLE MODE");
                draw_waiting_status();
                draw_text_inverted(2, 15, "B:CANCEL");
            }

            // Cancel with B
            if (input_pressed(J_B)) {
                link_cancel();
                transition_start(STATE_TITLE, TRANSITION_PHASE_COUNT_3);
            }
            break;

        case CONNECT_PHASE_SIDE_REVEAL:
            phase_timer++;

            // Allow early skip with A, or wait for timer
            if (input_pressed(J_A) || phase_timer >= CONNECT_REVEAL_DURATION) {
                // Enter syncing phase instead of transitioning directly
                connect_phase = CONNECT_PHASE_SYNCING;
                phase_timer = 0;

                // Show syncing status (bottom-right corner)
                draw_text_inverted(CONNECT_SYNC_X, CONNECT_SYNC_Y, "SYNCING...");
            }

            // Cancel with B during side reveal
            if (input_pressed(J_B)) {
                link_cancel();
                transition_start(STATE_TITLE, TRANSITION_PHASE_COUNT_3);
            }
            break;

        case CONNECT_PHASE_SYNCING: {
            uint8_t sync_result;
            phase_timer++;

            sync_result = link_ready_sync();

            if (sync_result == 1) {
                // Both sides ready, transition to profile select
                transition_start(STATE_LINK_PROFILE_SELECT, TRANSITION_PHASE_COUNT_3);
            } else if (sync_result == 2 || phase_timer >= CONNECT_SYNC_TIMEOUT) {
                // Peer cancelled or timeout — go back to title
                link_cancel();
                transition_start(STATE_TITLE, TRANSITION_PHASE_COUNT_3);
            }

            // Cancel with B during sync
            if (input_pressed(J_B)) {
                link_cancel();
                transition_start(STATE_TITLE, TRANSITION_PHASE_COUNT_3);
            }
            break;
        }

        default:
            connect_phase = CONNECT_PHASE_WAITING;
            break;
    }
}

/**
 * Cleanup link connection screen
 */
void cleanup_link_connect(void) {
    // Ensure display is on
    DISPLAY_ON;
}
