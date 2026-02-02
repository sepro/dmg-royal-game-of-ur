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
#include "input.h"
#include "transition.h"
#include "portrait.h"

// Portrait assets handled by portrait.c

// External reference to arrow sprite tiles
extern const uint8_t arrow_tiles[];

// External reference to next_state from main.c
extern ScreenState_t next_state;

// Global: selected difficulty
uint8_t selected_difficulty = DIFFICULTY_MEDIUM;

// Local state
static uint8_t current_selection = DIFFICULTY_MEDIUM;
static const uint8_t arrow_sprite_index = 0;

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
    draw_portrait(selected_opponent, DIFF_PORTRAIT_TILE_START, DIFF_PORTRAIT_X, DIFF_PORTRAIT_Y);
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

    // Clear input state
    input_reset();

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

    // Update input state
    input_update();

    // Navigate up/down
    if (input_pressed(J_UP)) {
        if (current_selection > 0) {
            current_selection--;
            move_sprite(arrow_sprite_index, DIFF_ARROW_X,
                       DIFF_ARROW_START_Y + (current_selection * DIFF_ARROW_SPACING));
        }
    } else if (input_pressed(J_DOWN)) {
        if (current_selection < DIFFICULTY_COUNT - 1) {
            current_selection++;
            move_sprite(arrow_sprite_index, DIFF_ARROW_X,
                       DIFF_ARROW_START_Y + (current_selection * DIFF_ARROW_SPACING));
        }
    }

    // Confirm with A
    if (input_pressed(J_A)) {
        selected_difficulty = current_selection;
        transition_start(STATE_COINFLIP, TRANSITION_PHASE_COUNT_3);
    }

    // Back with B
    if (input_pressed(J_B)) {
        next_state = STATE_OPPONENT_SELECT;
    }
}

/**
 * Cleanup difficulty selection screen
 */
void cleanup_difficulty_select(void) {
    // Hide arrow sprite
    move_sprite(arrow_sprite_index, 0, 0);

    // Ensure display is on
    DISPLAY_ON;
}
