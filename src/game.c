/**
 * game.c
 * Game board display and main gameplay state
 * Phase 6: Static board display with placeholder UI
 */

#include <gb/gb.h>
#include <stdint.h>
#include "game_types.h"
#include "game.h"
#include "coinflip.h"
#include "font.h"
#include "input.h"

// External references to generated board asset
extern const uint8_t board_tiles[];
extern const unsigned char board_map[];

// External reference to next_state from main.c
extern ScreenState_t next_state;

// External reference to starting_player from coinflip.c
extern uint8_t starting_player;

/**
 * Fill UI area (rows 10-17) with background tile
 */
static void fill_ui_area(void) {
    uint8_t row[BOARD_WIDTH];

    // Fill row buffer with background tile
    for (uint8_t i = 0; i < BOARD_WIDTH; i++) {
        row[i] = GAME_BG_TILE;
    }

    // Fill each row in UI area
    for (uint8_t y = UI_START_Y; y <= UI_END_Y; y++) {
        set_bkg_tiles(0, y, BOARD_WIDTH, 1, row);
    }
}

/**
 * Draw turn indicator text
 */
static void draw_turn_indicator(void) {
    // Clear the row first
    clear_text_row_inverted(GAME_TURN_X, GAME_TURN_Y, 14);

    // Draw appropriate text based on starting player
    if (starting_player == 0) {
        draw_text_inverted(GAME_TURN_X, GAME_TURN_Y, "YOUR TURN");
    } else {
        draw_text_inverted(GAME_TURN_X, GAME_TURN_Y, "CPU TURN");
    }
}

/**
 * Draw action prompt text
 */
static void draw_action_prompt(void) {
    draw_text_inverted(GAME_PROMPT_X, GAME_PROMPT_Y, "PRESS A TO ROLL");
}

/**
 * Initialize game screen
 */
void init_game(void) {
    // Disable display during VRAM writes
    DISPLAY_OFF;

    // Load board tiles into VRAM
    set_bkg_data(GAME_BOARD_TILE_START, GAME_BOARD_TILE_COUNT, board_tiles);

    // Draw the board tilemap at top of screen
    set_bkg_tiles(BOARD_X, BOARD_Y, BOARD_WIDTH, BOARD_HEIGHT, board_map);

    // Fill UI area with background color
    fill_ui_area();

    // Load inverted font for UI text (black on white/light background)
    load_font_inverted();

    // Draw UI text
    draw_turn_indicator();
    draw_action_prompt();

    // Clear input state
    input_reset();

    // Hide sprites for now (will be used for pieces later)
    HIDE_SPRITES;
    SHOW_BKG;
    DISPLAY_ON;
}

/**
 * Update game screen (called every frame)
 */
void update_game(void) {
    // Update input state
    input_update();

    // B button returns to coinflip for testing
    if (input_pressed(J_B)) {
        next_state = STATE_COINFLIP;
    }

    // A button placeholder (will trigger dice roll later)
    if (input_pressed(J_A)) {
        // For now, just toggle turn indicator for testing
        starting_player = (starting_player == 0) ? 1 : 0;
        draw_turn_indicator();
    }
}

/**
 * Cleanup game screen
 */
void cleanup_game(void) {
    // Nothing to clean up yet
    // Will need to reset game state in future phases
}
