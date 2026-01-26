/**
 * difficulty_select.c
 * Difficulty selection screen implementation
 * White background with dark text, shows selected opponent portrait
 */

#include <gb/gb.h>
#include <stdint.h>
#include "game_types.h"
#include "difficulty_select.h"
#include "opponent_select.h"
#include "opponent_data.h"
#include "font.h"

// External references to generated profile assets
extern const uint8_t profile_01_tiles[];
extern const unsigned char profile_01_map[];
extern const uint8_t profile_02_tiles[];
extern const unsigned char profile_02_map[];
extern const uint8_t profile_03_tiles[];
extern const unsigned char profile_03_map[];
extern const uint8_t profile_04_tiles[];
extern const unsigned char profile_04_map[];

// External reference to arrow sprite tiles
extern const uint8_t arrow_tiles[];

// External reference to next_state from main.c
extern ScreenState_t next_state;

// Global: selected difficulty
uint8_t selected_difficulty = DIFFICULTY_MEDIUM;

// Local state
static uint8_t current_selection = DIFFICULTY_MEDIUM;
static uint8_t prev_input = 0;
static const uint8_t arrow_sprite_index = 0;

// Transition animation state
static uint8_t transition_timer = 0;
static uint8_t transition_phase = TRANSITION_IDLE;
static ScreenState_t transition_target = STATE_DIFFICULTY_SELECT;
static uint8_t transition_phase_count = 0;

// White tile data (8x8 pixels, all color 0 = white on DMG)
static const uint8_t white_tile[16] = {
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};

// Difficulty option labels
static const char *difficulty_labels[DIFFICULTY_COUNT] = {
    "EASY",
    "MEDIUM",
    "HARD"
};

/**
 * Start a screen transition animation
 */
static void transition_start(ScreenState_t target, uint8_t phase_count) {
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
static uint8_t update_transition(void) {
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

/**
 * Fill screen with white tiles
 */
static void fill_screen_white(void) {
    uint8_t row[20];
    for (uint8_t i = 0; i < 20; i++) {
        row[i] = WHITE_TILE;
    }
    for (uint8_t y = 0; y < 18; y++) {
        set_bkg_tiles(0, y, 20, 1, row);
    }
}

/**
 * Draw the selected opponent's portrait
 */
static void draw_selected_portrait(void) {
    const uint8_t *tiles;
    const unsigned char *map;
    uint8_t tile_count;

    switch (selected_opponent) {
        case 0:
            tiles = profile_01_tiles;
            map = profile_01_map;
            tile_count = profile_tile_counts[0];
            break;
        case 1:
            tiles = profile_02_tiles;
            map = profile_02_map;
            tile_count = profile_tile_counts[1];
            break;
        case 2:
            tiles = profile_03_tiles;
            map = profile_03_map;
            tile_count = profile_tile_counts[2];
            break;
        case 3:
            tiles = profile_04_tiles;
            map = profile_04_map;
            tile_count = profile_tile_counts[3];
            break;
        default:
            return;
    }

    // Load portrait tiles starting after white tile
    set_bkg_data(DIFF_PORTRAIT_TILE_START, tile_count, tiles);

    // Draw 5x5 portrait
    uint8_t row_buf[5];
    for (uint8_t row = 0; row < 5; row++) {
        for (uint8_t col = 0; col < 5; col++) {
            row_buf[col] = DIFF_PORTRAIT_TILE_START + map[row * 5 + col];
        }
        set_bkg_tiles(DIFF_PORTRAIT_X, DIFF_PORTRAIT_Y + row, 5, 1, row_buf);
    }
}

/**
 * Initialize difficulty selection screen
 */
void init_difficulty_select(void) {
    // Disable display during VRAM writes
    DISPLAY_OFF;

    // Load white tile at index 0
    set_bkg_data(WHITE_TILE, 1, white_tile);

    // Fill screen with white
    fill_screen_white();

    // Draw selected opponent portrait
    draw_selected_portrait();

    // Load inverted font (black text on white background)
    load_font_inverted();

    // Draw opponent name
    draw_text_inverted(DIFF_NAME_X, DIFF_NAME_Y, opponent_names[selected_opponent]);

    // Draw title
    draw_text_inverted(DIFF_TITLE_X, DIFF_TITLE_Y, "SET DIFFICULTY");

    // Draw difficulty options
    for (uint8_t i = 0; i < DIFFICULTY_COUNT; i++) {
        draw_text_inverted(DIFF_OPTION_X, DIFF_OPTION_START_Y + i, difficulty_labels[i]);
    }

    // Load arrow sprite
    set_sprite_data(0, 1, arrow_tiles);
    set_sprite_tile(arrow_sprite_index, 0);

    // Position arrow at current selection (default: MEDIUM)
    current_selection = DIFFICULTY_MEDIUM;
    move_sprite(arrow_sprite_index, DIFF_ARROW_X,
                DIFF_ARROW_START_Y + (current_selection * DIFF_ARROW_SPACING));

    // Initialize transition state
    transition_phase = TRANSITION_IDLE;
    transition_timer = 0;

    prev_input = 0;

    SHOW_BKG;
    SHOW_SPRITES;
    DISPLAY_ON;
}

/**
 * Update difficulty selection screen (called every frame)
 */
void update_difficulty_select(void) {
    // Update transition animation if active
    if (update_transition()) {
        return;  // Skip input handling during transition
    }

    uint8_t input = joypad();
    uint8_t pressed = input & ~prev_input;

    // Navigate up/down
    if (pressed & J_UP) {
        if (current_selection > 0) {
            current_selection--;
            move_sprite(arrow_sprite_index, DIFF_ARROW_X,
                       DIFF_ARROW_START_Y + (current_selection * DIFF_ARROW_SPACING));
        }
    } else if (pressed & J_DOWN) {
        if (current_selection < DIFFICULTY_COUNT - 1) {
            current_selection++;
            move_sprite(arrow_sprite_index, DIFF_ARROW_X,
                       DIFF_ARROW_START_Y + (current_selection * DIFF_ARROW_SPACING));
        }
    }

    // Confirm with A
    if (pressed & J_A) {
        selected_difficulty = current_selection;
        transition_start(STATE_COINFLIP, TRANSITION_PHASE_COUNT_3);
    }

    // Back with B
    if (pressed & J_B) {
        next_state = STATE_OPPONENT_SELECT;
    }

    prev_input = input;
}

/**
 * Cleanup difficulty selection screen
 */
void cleanup_difficulty_select(void) {
    // Hide arrow sprite
    move_sprite(arrow_sprite_index, 0, 0);

    // Ensure display is on and clear transition state
    DISPLAY_ON;
    transition_phase = TRANSITION_IDLE;
}
