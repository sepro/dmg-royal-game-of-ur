/**
 * difficulty_select.c
 * Difficulty selection screen implementation
 * White background with dark text, shows selected opponent portrait
 */

#include <gb/gb.h>
#include <stdint.h>
#include "game_types.h"
#include "screens/difficulty_select.h"
#include "screens/opponent_select.h"
#include "util/opponent_data.h"
#include "util/cgb.h"
#include "util/font.h"
#include "util/input.h"
#include "util/transition.h"
#include "util/portrait.h"
#include "util/screen_utils.h"
#include "util/sound.h"

// Portrait assets handled by portrait.c

// External reference to arrow sprite tiles
extern const uint8_t arrow_tiles[];

// External reference to border tiles (shared with pause/opponent screens)
extern const uint8_t border_tiles[];

// External reference to next_state from main.c
extern ScreenState_t next_state;

// Global: selected difficulty
uint8_t selected_difficulty = DIFFICULTY_MEDIUM;

// Local state
static uint8_t current_selection = DIFFICULTY_MEDIUM;
static const uint8_t arrow_sprite_index = 0;

// Difficulty option labels
static const char *difficulty_labels[DIFFICULTY_COUNT] = {
    "EASY",
    "MEDIUM",
    "HARD"
};

/**
 * Draw decorative border around lower half containing difficulty controls
 */
static void draw_difficulty_box(void) {
    draw_full_width_border(DIFF_BORDER_TILE_START, 8, 10, 0);
}

/**
 * Reverse bit order in a byte (abcd efgh -> hgfe dcba)
 */
static uint8_t reverse_bits(uint8_t v) {
    v = (uint8_t)(((v & 0xF0u) >> 4) | ((v & 0x0Fu) << 4));
    v = (uint8_t)(((v & 0xCCu) >> 2) | ((v & 0x33u) << 2));
    v = (uint8_t)(((v & 0xAAu) >> 1) | ((v & 0x55u) << 1));
    return v;
}

/**
 * Build mirrored tile set in VRAM from already-loaded portrait tiles
 */
static void mirror_tiles_to_vram(uint8_t src_base, uint8_t dst_base, uint8_t count) {
    uint8_t src_tile[16];
    uint8_t dst_tile[16];

    for (uint8_t tile = 0; tile < count; tile++) {
        get_bkg_data(src_base + tile, 1, src_tile);
        for (uint8_t row = 0; row < 8; row++) {
            dst_tile[row * 2] = reverse_bits(src_tile[row * 2]);
            dst_tile[row * 2 + 1] = reverse_bits(src_tile[row * 2 + 1]);
        }
        set_bkg_data(dst_base + tile, 1, dst_tile);
    }
}

/**
 * Draw the selected opponent portrait and mirrored copy
 */
static void draw_selected_portraits(void) {
    uint8_t loaded_tiles = load_portrait_tiles_for_char(selected_opponent, DIFF_PORTRAIT_TILE_START);

    // Safety: mirror tiles must fit before font starts at VRAM_FONT_START (141).
    // DIFF_MIRROR_TILE_BASE (64) + max ~40 tiles = ~104, so this always passes in practice.
    // If it fails, the mirrored portrait is silently skipped.
    if ((uint16_t)DIFF_MIRROR_TILE_BASE + loaded_tiles <= VRAM_FONT_START) {
        mirror_tiles_to_vram(DIFF_PORTRAIT_TILE_START, DIFF_MIRROR_TILE_BASE, loaded_tiles);
        draw_portrait_expr_mirrored(selected_opponent, PORTRAIT_EXPR_NORMAL,
                                    DIFF_MIRROR_TILE_BASE,
                                    DIFF_PORTRAIT_X, DIFF_PORTRAIT_Y, 1);
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
    fill_screen_with_tile(WHITE_TILE);

    // Draw selected opponent portraits (normal + mirrored)
    draw_selected_portraits();

    // Load and draw lower UI border box
    set_bkg_data(DIFF_BORDER_TILE_START, DIFF_BORDER_TILE_COUNT, border_tiles);
    draw_difficulty_box();

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

    // Set palettes
    BGP_REG = 0xE4;   // Standard background palette
    cgb_set_obp0(0xFC);  // Sprite palette for white background with dark text

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
            play_sfx(SFX_CURSOR);
        }
    } else if (input_pressed(J_DOWN)) {
        if (current_selection < DIFFICULTY_COUNT - 1) {
            current_selection++;
            move_sprite(arrow_sprite_index, DIFF_ARROW_X,
                       DIFF_ARROW_START_Y + (current_selection * DIFF_ARROW_SPACING));
            play_sfx(SFX_CURSOR);
        }
    }

    // Confirm with A
    if (input_pressed(J_A)) {
        play_sfx(SFX_CONFIRM);
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
