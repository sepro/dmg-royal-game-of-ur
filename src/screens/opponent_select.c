/**
 * opponent_select.c
 * Opponent selection screen implementation
 * 2x2 grid of opponent portraits with navigation
 * White background with dark border and black text
 */

#include <gb/gb.h>
#include <stdint.h>
#include <string.h>
#include "game_types.h"
#include "screens/opponent_select.h"
#include "util/opponent_data.h"
#include "util/font.h"
#include "util/input.h"
#include "util/transition.h"
#include "util/screen_utils.h"
#include "util/portrait.h"

// External reference to border tiles and map (from border.c)
extern const uint8_t border_tiles[];
extern const unsigned char border_map[];

// External reference to next_state from main.c
extern ScreenState_t next_state;

// Selected opponent (0-3)
uint8_t selected_opponent = 0;

// Previous selection (for redrawing border)
static uint8_t prev_selection = 0;

// Portrait X positions (tile coordinates)
static const uint8_t portrait_x[OPPONENT_COUNT] = {
    PORTRAIT_0_X, PORTRAIT_1_X, PORTRAIT_2_X, PORTRAIT_3_X
};

// Portrait Y positions (tile coordinates)
static const uint8_t portrait_y[OPPONENT_COUNT] = {
    PORTRAIT_0_Y, PORTRAIT_1_Y, PORTRAIT_2_Y, PORTRAIT_3_Y
};

/**
 * Draw a portrait at the specified grid position using merged tileset
 */
static void draw_portrait_at(uint8_t idx) {
    draw_portrait_expr(idx, PORTRAIT_EXPR_NORMAL, PORTRAIT_TILE_START,
                       portrait_x[idx], portrait_y[idx], 0);
}

/**
 * Clear the selection border around a portrait
 * Clears the 7x7 frame (skipping inner 5x5 portrait area)
 */
static void clear_border(uint8_t idx) {
    uint8_t x = portrait_x[idx] - 1;  // Border is 1 tile outside portrait
    uint8_t y = portrait_y[idx] - 1;
    clear_border_frame(x, y, BORDER_WIDTH, BORDER_HEIGHT, BLANK_TILE);
}

/**
 * Draw the selection border around a portrait
 * Uses 7x7 tilemap from border.png (skipping inner 5x5 portrait area)
 */
static void draw_border(uint8_t idx) {
    uint8_t x = portrait_x[idx] - 1;  // Border is 1 tile outside portrait
    uint8_t y = portrait_y[idx] - 1;
    draw_border_frame(x, y, BORDER_WIDTH, BORDER_HEIGHT, BORDER_TILE_START, border_map);
}

/**
 * Update the description text for selected opponent
 */
static void update_description(uint8_t idx) {
    // Clear description area (2 rows) with white background
    clear_text_row_inverted(DESC_TEXT_X, DESC_TEXT_Y, DESC_TEXT_WIDTH);
    clear_text_row_inverted(DESC_TEXT_X, DESC_TEXT_Y + 1, DESC_TEXT_WIDTH);

    // Draw opponent name and description (black text on white)
    draw_text_inverted(DESC_TEXT_X, DESC_TEXT_Y, opponent_names[idx]);
    draw_text_inverted(DESC_TEXT_X, DESC_TEXT_Y + 1, opponent_descs[idx]);
}

/**
 * Get grid column from opponent index
 */
static inline uint8_t grid_col(uint8_t idx) {
    return idx & 1;  // 0 or 1
}

/**
 * Get grid row from opponent index
 */
static inline uint8_t grid_row(uint8_t idx) {
    return idx >> 1;  // 0 or 1
}

/**
 * Initialize opponent selection screen
 */
void init_opponent_select(void) {
    // Disable display during VRAM writes
    DISPLAY_OFF;

    // Load white tile at index 0 (may have been overwritten by other screens)
    set_bkg_data(BLANK_TILE, 1, white_tile);

    // Clear the entire screen with black tiles
    clear_rect(BLANK_TILE, 0, 0, 20, 18);

    // Load merged portrait tileset (all 4 characters share one tileset)
    load_portrait_tiles(PORTRAIT_TILE_START);

    // Load border tiles (normal, dark on white background)
    set_bkg_data(BORDER_TILE_START, 25, border_tiles);

    // Draw all 4 portraits from merged tileset
    draw_portrait_at(0);
    draw_portrait_at(1);
    draw_portrait_at(2);
    draw_portrait_at(3);

    // Initialize selection to first opponent
    selected_opponent = 0;
    prev_selection = 0;

    // Draw initial selection border
    draw_border(selected_opponent);

    // Draw initial description
    update_description(selected_opponent);

    // Clear input state
    input_reset();

    // Set background palette
    BGP_REG = 0xE4;

    // Enable display
    SHOW_BKG;
    DISPLAY_ON;
}

/**
 * Update opponent selection screen (called every frame)
 */
void update_opponent_select(void) {
    // Update transition animation if active
    if (update_transition()) {
        return;  // Skip input handling during transition
    }

    // Update input state
    input_update();

    uint8_t new_selection = selected_opponent;
    uint8_t col = grid_col(selected_opponent);
    uint8_t row = grid_row(selected_opponent);

    // Handle D-pad navigation
    if (input_pressed(J_LEFT)) {
        if (col > 0) {
            new_selection--;
        }
    } else if (input_pressed(J_RIGHT)) {
        if (col < 1) {
            new_selection++;
        }
    } else if (input_pressed(J_UP)) {
        if (row > 0) {
            new_selection -= 2;
        }
    } else if (input_pressed(J_DOWN)) {
        if (row < 1) {
            new_selection += 2;
        }
    }

    // Update selection if changed
    if (new_selection != selected_opponent) {
        // Clear old border
        clear_border(selected_opponent);

        // Update selection
        prev_selection = selected_opponent;
        selected_opponent = new_selection;

        // Draw new border
        draw_border(selected_opponent);

        // Update description text
        update_description(selected_opponent);
    }

    // Handle A button (confirm selection)
    if (input_pressed(J_A)) {
        transition_start(STATE_DIFFICULTY_SELECT, TRANSITION_PHASE_COUNT_3);
    }

    // Handle B button (go back to title)
    if (input_pressed(J_B)) {
        next_state = STATE_TITLE;
    }
}

/**
 * Cleanup opponent selection screen
 */
void cleanup_opponent_select(void) {
    // Ensure display is on
    DISPLAY_ON;
}
