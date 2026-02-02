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
#include "opponent_select.h"
#include "opponent_data.h"
#include "font.h"
#include "input.h"
#include "transition.h"

// External references to generated profile assets
extern const uint8_t profile_01_tiles[];
extern const unsigned char profile_01_map[];
extern const uint8_t profile_02_tiles[];
extern const unsigned char profile_02_map[];
extern const uint8_t profile_03_tiles[];
extern const unsigned char profile_03_map[];
extern const uint8_t profile_04_tiles[];
extern const unsigned char profile_04_map[];

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
 * Clear a rectangular area with black tiles
 */
static void clear_rect(uint8_t x, uint8_t y, uint8_t w, uint8_t h) {
    uint8_t row[20];
    for (uint8_t i = 0; i < w && i < 20; i++) {
        row[i] = BLANK_TILE;
    }
    for (uint8_t j = 0; j < h; j++) {
        set_bkg_tiles(x, y + j, w, 1, row);
    }
}

/**
 * Draw a portrait at the specified position
 * @param idx Opponent index (0-3)
 * @param tile_base VRAM tile index where this profile's tiles start
 */
static void draw_portrait(uint8_t idx, uint8_t tile_base) {
    uint8_t x = portrait_x[idx];
    uint8_t y = portrait_y[idx];
    const unsigned char *map;

    // Select the correct map
    switch (idx) {
        case 0: map = profile_01_map; break;
        case 1: map = profile_02_map; break;
        case 2: map = profile_03_map; break;
        case 3: map = profile_04_map; break;
        default: return;
    }

    // Draw the portrait tile by tile, adding tile_base offset
    uint8_t row_buf[PORTRAIT_WIDTH];
    for (uint8_t row = 0; row < PORTRAIT_HEIGHT; row++) {
        for (uint8_t col = 0; col < PORTRAIT_WIDTH; col++) {
            uint8_t map_tile = map[row * PORTRAIT_WIDTH + col];
            row_buf[col] = tile_base + map_tile;
        }
        set_bkg_tiles(x, y + row, PORTRAIT_WIDTH, 1, row_buf);
    }
}

/**
 * Clear the selection border around a portrait
 * Clears the 7x7 frame (skipping inner 5x5 portrait area)
 */
static void clear_border(uint8_t idx) {
    uint8_t x = portrait_x[idx] - 1;  // Border is 1 tile outside portrait
    uint8_t y = portrait_y[idx] - 1;

    // Clear entire 7x7 border frame (skip inner 5x5)
    for (uint8_t row = 0; row < BORDER_HEIGHT; row++) {
        for (uint8_t col = 0; col < BORDER_WIDTH; col++) {
            // Skip inner 5x5 portrait area (rows 1-5, cols 1-5)
            if (row >= 1 && row <= 5 && col >= 1 && col <= 5) continue;
            set_bkg_tile_xy(x + col, y + row, BLANK_TILE);
        }
    }
}

/**
 * Draw the selection border around a portrait
 * Uses 7x7 tilemap from border.png (skipping inner 5x5 portrait area)
 */
static void draw_border(uint8_t idx) {
    uint8_t x = portrait_x[idx] - 1;  // Border is 1 tile outside portrait
    uint8_t y = portrait_y[idx] - 1;

    // Draw 7x7 border frame using tilemap (skip inner 5x5)
    for (uint8_t row = 0; row < BORDER_HEIGHT; row++) {
        for (uint8_t col = 0; col < BORDER_WIDTH; col++) {
            // Skip inner 5x5 portrait area (rows 1-5, cols 1-5)
            if (row >= 1 && row <= 5 && col >= 1 && col <= 5) continue;
            uint8_t tile = BORDER_TILE_START + border_map[row * BORDER_WIDTH + col];
            set_bkg_tile_xy(x + col, y + row, tile);
        }
    }
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

// White tile data (8x8 pixels, all color 0 = white on DMG)
static const uint8_t white_tile[16] = {
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};

/**
 * Initialize opponent selection screen
 */
void init_opponent_select(void) {
    // Disable display during VRAM writes
    DISPLAY_OFF;

    // Load white tile at index 0 (may have been overwritten by other screens)
    set_bkg_data(BLANK_TILE, 1, white_tile);

    // Clear the entire screen with black tiles
    clear_rect(0, 0, 20, 18);

    // Load profile tiles into VRAM with proper per-profile tile counts
    // Calculate offsets based on actual tile counts
    uint8_t tile_offset = PORTRAIT_TILE_START;
    uint8_t profile_offsets[OPPONENT_COUNT];

    profile_offsets[0] = tile_offset;
    set_bkg_data(tile_offset, profile_tile_counts[0], profile_01_tiles);
    tile_offset += profile_tile_counts[0];

    profile_offsets[1] = tile_offset;
    set_bkg_data(tile_offset, profile_tile_counts[1], profile_02_tiles);
    tile_offset += profile_tile_counts[1];

    profile_offsets[2] = tile_offset;
    set_bkg_data(tile_offset, profile_tile_counts[2], profile_03_tiles);
    tile_offset += profile_tile_counts[2];

    profile_offsets[3] = tile_offset;
    set_bkg_data(tile_offset, profile_tile_counts[3], profile_04_tiles);

    // Load border tiles (normal, dark on white background)
    set_bkg_data(BORDER_TILE_START, 25, border_tiles);

    // Load inverted font tiles (black text on white background)
    load_font_inverted();

    // Draw all 4 portraits with correct tile offsets
    draw_portrait(0, profile_offsets[0]);
    draw_portrait(1, profile_offsets[1]);
    draw_portrait(2, profile_offsets[2]);
    draw_portrait(3, profile_offsets[3]);

    // Initialize selection to first opponent
    selected_opponent = 0;
    prev_selection = 0;

    // Draw initial selection border
    draw_border(selected_opponent);

    // Draw initial description
    update_description(selected_opponent);

    // Clear input state
    input_reset();

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
