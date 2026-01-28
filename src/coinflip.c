/**
 * coinflip.c
 * Side selection and coin flip screen implementation
 * White background with two coin choices, then animated coin flip
 */

#include <gb/gb.h>
#include <stdint.h>
#include "game_types.h"
#include "coinflip.h"
#include "font.h"
#include "input.h"

// External references to generated coin assets
extern const uint8_t light_coin_tiles[];
extern const uint8_t dark_coin_tiles[];

// External reference to border tiles (reused from opponent select)
extern const uint8_t border_tiles[];

// External reference to next_state from main.c
extern ScreenState_t next_state;

// Global state (exported)
uint8_t selected_side = SIDE_LIGHT;
uint8_t coin_result = SIDE_LIGHT;
uint8_t starting_player = 0;

// Local state
static uint8_t current_selection = SIDE_LIGHT;
static uint8_t prev_selection = SIDE_LIGHT;

// Animation state
static uint8_t anim_phase = ANIM_PHASE_NONE;
static uint8_t anim_timer = 0;
static uint8_t anim_counter = 0;  // For chaotic update timing

// Random state for coin flip
static uint16_t rand_state = 0;
static uint8_t frame_counter = 0;

// Locked tiles bitmask (25 bits, one per animation tile)
// Using 4 bytes to hold 32 bits
static uint8_t locked_tiles[4] = {0, 0, 0, 0};
static uint8_t locked_count = 0;

// Transition animation state
static uint8_t transition_timer = 0;
static uint8_t transition_phase = TRANSITION_IDLE;
static ScreenState_t transition_target = STATE_COINFLIP;
static uint8_t transition_phase_count = 0;

// White tile data (8x8 pixels, all color 0 = white on DMG)
static const uint8_t white_tile[16] = {
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};

// Position arrays for selection border
static const uint8_t coin_x[2] = { COINFLIP_LIGHT_X, COINFLIP_DARK_X };
static const uint8_t coin_y[2] = { COINFLIP_LIGHT_Y, COINFLIP_DARK_Y };

/**
 * Get a pseudo-random byte using Galois LFSR
 */
static uint8_t get_random(void) {
    uint8_t lsb = rand_state & 1;
    rand_state >>= 1;
    if (lsb) rand_state ^= 0xB400;
    return (uint8_t)(rand_state & 0xFF);
}

/**
 * Check if a tile is locked
 */
static uint8_t is_tile_locked(uint8_t tile_idx) {
    uint8_t byte_idx = tile_idx >> 3;
    uint8_t bit_idx = tile_idx & 7;
    return (locked_tiles[byte_idx] >> bit_idx) & 1;
}

/**
 * Mark a tile as locked
 */
static void lock_tile(uint8_t tile_idx) {
    uint8_t byte_idx = tile_idx >> 3;
    uint8_t bit_idx = tile_idx & 7;
    if (!is_tile_locked(tile_idx)) {
        locked_tiles[byte_idx] |= (1 << bit_idx);
        locked_count++;
    }
}

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
        row[i] = COINFLIP_WHITE_TILE;
    }
    for (uint8_t y = 0; y < 18; y++) {
        set_bkg_tiles(0, y, 20, 1, row);
    }
}

/**
 * Draw a coin at the specified position
 * @param x Tile X position
 * @param y Tile Y position
 * @param tile_base Starting VRAM tile index for this coin
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
 * Draw the selection border around a coin
 */
static void draw_border(uint8_t idx) {
    uint8_t x = coin_x[idx] - 1;  // Border is 1 tile outside coin
    uint8_t y = coin_y[idx] - 1;

    // Border tile indices
    const uint8_t B_TL = COINFLIP_BORDER_TILE_START + 0;
    const uint8_t B_T  = COINFLIP_BORDER_TILE_START + 1;
    const uint8_t B_TR = COINFLIP_BORDER_TILE_START + 2;
    const uint8_t B_L  = COINFLIP_BORDER_TILE_START + 3;
    const uint8_t B_R  = COINFLIP_BORDER_TILE_START + 4;
    const uint8_t B_BL = COINFLIP_BORDER_TILE_START + 5;
    const uint8_t B_B  = COINFLIP_BORDER_TILE_START + 6;
    const uint8_t B_BR = COINFLIP_BORDER_TILE_START + 7;

    // Top row
    set_bkg_tile_xy(x, y, B_TL);
    for (uint8_t i = 1; i < COINFLIP_BORDER_WIDTH - 1; i++) {
        set_bkg_tile_xy(x + i, y, B_T);
    }
    set_bkg_tile_xy(x + COINFLIP_BORDER_WIDTH - 1, y, B_TR);

    // Middle rows
    for (uint8_t i = 1; i < COINFLIP_BORDER_HEIGHT - 1; i++) {
        set_bkg_tile_xy(x, y + i, B_L);
        set_bkg_tile_xy(x + COINFLIP_BORDER_WIDTH - 1, y + i, B_R);
    }

    // Bottom row
    set_bkg_tile_xy(x, y + COINFLIP_BORDER_HEIGHT - 1, B_BL);
    for (uint8_t i = 1; i < COINFLIP_BORDER_WIDTH - 1; i++) {
        set_bkg_tile_xy(x + i, y + COINFLIP_BORDER_HEIGHT - 1, B_B);
    }
    set_bkg_tile_xy(x + COINFLIP_BORDER_WIDTH - 1, y + COINFLIP_BORDER_HEIGHT - 1, B_BR);
}

/**
 * Clear the selection border around a coin
 */
static void clear_border(uint8_t idx) {
    uint8_t x = coin_x[idx] - 1;
    uint8_t y = coin_y[idx] - 1;

    // Clear top and bottom rows
    for (uint8_t i = 0; i < COINFLIP_BORDER_WIDTH; i++) {
        set_bkg_tile_xy(x + i, y, COINFLIP_WHITE_TILE);
        set_bkg_tile_xy(x + i, y + COINFLIP_BORDER_HEIGHT - 1, COINFLIP_WHITE_TILE);
    }

    // Clear left and right edges (middle rows)
    for (uint8_t i = 1; i < COINFLIP_BORDER_HEIGHT - 1; i++) {
        set_bkg_tile_xy(x, y + i, COINFLIP_WHITE_TILE);
        set_bkg_tile_xy(x + COINFLIP_BORDER_WIDTH - 1, y + i, COINFLIP_WHITE_TILE);
    }
}

/**
 * Draw the animation coin with mixed light/dark tiles
 * Uses locked_tiles bitmask to determine which tiles show result
 */
static void draw_animation_coin(void) {
    uint8_t row_buf[COIN_WIDTH];
    uint8_t light_base = COINFLIP_LIGHT_TILE_START;
    uint8_t dark_base = COINFLIP_DARK_TILE_START;

    for (uint8_t row = 0; row < COIN_HEIGHT; row++) {
        for (uint8_t col = 0; col < COIN_WIDTH; col++) {
            uint8_t tile_idx = row * COIN_WIDTH + col;
            uint8_t tile_offset = tile_idx;

            if (is_tile_locked(tile_idx)) {
                // Locked: show result side
                if (coin_result == SIDE_LIGHT) {
                    row_buf[col] = light_base + tile_offset;
                } else {
                    row_buf[col] = dark_base + tile_offset;
                }
            } else {
                // Not locked: randomize
                if (get_random() & 1) {
                    row_buf[col] = light_base + tile_offset;
                } else {
                    row_buf[col] = dark_base + tile_offset;
                }
            }
        }
        set_bkg_tiles(COINFLIP_ANIM_X, COINFLIP_ANIM_Y + row, COIN_WIDTH, 1, row_buf);
    }
}

/**
 * Draw the final animation coin showing result
 */
static void draw_result_coin(void) {
    uint8_t tile_base = (coin_result == SIDE_LIGHT) ?
                        COINFLIP_LIGHT_TILE_START : COINFLIP_DARK_TILE_START;
    draw_coin(COINFLIP_ANIM_X, COINFLIP_ANIM_Y, tile_base);
}

/**
 * Lock random unlocked tiles during lock-in phase
 * Returns number of tiles locked this call
 */
static uint8_t lock_random_tiles(uint8_t max_count) {
    uint8_t count = 0;

    // Try to lock up to max_count tiles
    for (uint8_t attempts = 0; attempts < 50 && count < max_count; attempts++) {
        uint8_t tile_idx = get_random() % COIN_TILES;
        if (!is_tile_locked(tile_idx)) {
            lock_tile(tile_idx);
            count++;
        }
    }

    return count;
}

/**
 * Clear result text area
 */
static void clear_result_text(void) {
    clear_text_row_inverted(COINFLIP_RESULT_X, COINFLIP_RESULT_Y, 14);
}

/**
 * Draw result text
 */
static void draw_result_text(void) {
    if (coin_result == SIDE_LIGHT) {
        draw_text_inverted(COINFLIP_RESULT_X, COINFLIP_RESULT_Y, "LIGHT STARTS");
    } else {
        draw_text_inverted(COINFLIP_RESULT_X, COINFLIP_RESULT_Y, "DARK STARTS");
    }
}

/**
 * Start the coin flip animation
 */
static void start_animation(void) {
    // Seed random from DIV register and frame counter
    rand_state = DIV_REG ^ ((uint16_t)frame_counter << 8);
    if (rand_state == 0) rand_state = 0xACE1;  // Avoid zero state

    // Determine result
    coin_result = get_random() & 1;

    // Calculate starting player:
    // If player chose correctly, they start (0 = human)
    // Otherwise AI starts (1 = AI)
    starting_player = (coin_result == selected_side) ? 0 : 1;

    // Reset lock state
    locked_tiles[0] = 0;
    locked_tiles[1] = 0;
    locked_tiles[2] = 0;
    locked_tiles[3] = 0;
    locked_count = 0;

    // Start chaotic phase
    anim_phase = ANIM_PHASE_CHAOTIC;
    anim_timer = ANIM_CHAOTIC_DURATION;
    anim_counter = 0;

    // Hide selection coins and show animation area
    // Clear selection area
    for (uint8_t y = COINFLIP_LIGHT_Y - 1; y < COINFLIP_LIGHT_Y + COIN_HEIGHT + 1; y++) {
        for (uint8_t x = 0; x < 20; x++) {
            set_bkg_tile_xy(x, y, COINFLIP_WHITE_TILE);
        }
    }

    // Clear labels
    clear_text_row_inverted(COINFLIP_LIGHT_LABEL_X, COINFLIP_LIGHT_LABEL_Y, 5);
    clear_text_row_inverted(COINFLIP_DARK_LABEL_X, COINFLIP_DARK_LABEL_Y, 4);

    // Draw initial chaotic coin
    draw_animation_coin();
}

/**
 * Update animation state machine
 */
static void update_animation(void) {
    switch (anim_phase) {
        case ANIM_PHASE_CHAOTIC:
            anim_counter++;
            if (anim_counter >= ANIM_CHAOTIC_INTERVAL) {
                anim_counter = 0;
                draw_animation_coin();
            }
            anim_timer--;
            if (anim_timer == 0) {
                anim_phase = ANIM_PHASE_LOCKIN;
                anim_timer = ANIM_LOCKIN_DURATION;
            }
            break;

        case ANIM_PHASE_LOCKIN:
            anim_counter++;
            // Lock tiles with increasing frequency as time progresses
            // Start slow (every 8 frames), end fast (every 2 frames)
            {
                uint8_t lock_interval = 8 - ((ANIM_LOCKIN_DURATION - anim_timer) / 12);
                if (lock_interval < 2) lock_interval = 2;

                if (anim_counter >= lock_interval) {
                    anim_counter = 0;
                    // Lock 1-3 tiles per interval
                    uint8_t to_lock = 1 + (get_random() % 3);
                    lock_random_tiles(to_lock);
                    draw_animation_coin();
                }
            }

            // Check if all tiles locked
            if (locked_count >= COIN_TILES) {
                anim_phase = ANIM_PHASE_COMPLETE;
                anim_timer = ANIM_COMPLETE_DURATION;
                draw_result_coin();
            } else {
                anim_timer--;
                if (anim_timer == 0) {
                    // Force lock remaining tiles
                    for (uint8_t i = 0; i < COIN_TILES; i++) {
                        lock_tile(i);
                    }
                    anim_phase = ANIM_PHASE_COMPLETE;
                    anim_timer = ANIM_COMPLETE_DURATION;
                    draw_result_coin();
                }
            }
            break;

        case ANIM_PHASE_COMPLETE:
            anim_timer--;
            if (anim_timer == 0) {
                anim_phase = ANIM_PHASE_RESULT;
                anim_timer = ANIM_RESULT_DURATION;
                draw_result_text();
            }
            break;

        case ANIM_PHASE_RESULT:
            anim_timer--;
            // Allow early skip with A button
            if (anim_timer == 0 || input_pressed(J_A)) {
                anim_phase = ANIM_PHASE_NONE;
                transition_start(STATE_GAME, TRANSITION_PHASE_COUNT_3);
            }
            break;

        default:
            break;
    }
}

/**
 * Initialize coin flip screen
 */
void init_coinflip(void) {
    // Disable display during VRAM writes
    DISPLAY_OFF;

    // Load white tile at index 0
    set_bkg_data(COINFLIP_WHITE_TILE, 1, white_tile);

    // Fill screen with white
    fill_screen_white();

    // Load coin tiles
    set_bkg_data(COINFLIP_LIGHT_TILE_START, COINFLIP_LIGHT_TILE_COUNT, light_coin_tiles);
    set_bkg_data(COINFLIP_DARK_TILE_START, COINFLIP_DARK_TILE_COUNT, dark_coin_tiles);

    // Load border tiles
    set_bkg_data(COINFLIP_BORDER_TILE_START, 8, border_tiles);

    // Load inverted font (black text on white background)
    load_font_inverted();

    // Draw title
    draw_text_inverted(COINFLIP_TITLE_X, COINFLIP_TITLE_Y, "CHOOSE YOUR SIDE");

    // Draw both coins
    draw_coin(COINFLIP_LIGHT_X, COINFLIP_LIGHT_Y, COINFLIP_LIGHT_TILE_START);
    draw_coin(COINFLIP_DARK_X, COINFLIP_DARK_Y, COINFLIP_DARK_TILE_START);

    // Draw labels
    draw_text_inverted(COINFLIP_LIGHT_LABEL_X, COINFLIP_LIGHT_LABEL_Y, "LIGHT");
    draw_text_inverted(COINFLIP_DARK_LABEL_X, COINFLIP_DARK_LABEL_Y, "DARK");

    // Initialize selection
    current_selection = SIDE_LIGHT;
    prev_selection = SIDE_LIGHT;
    selected_side = SIDE_LIGHT;

    // Draw initial selection border
    draw_border(current_selection);

    // Initialize animation state
    anim_phase = ANIM_PHASE_NONE;
    anim_timer = 0;
    frame_counter = 0;

    // Initialize transition state
    transition_phase = TRANSITION_IDLE;
    transition_timer = 0;

    // Clear input state
    input_reset();

    // Hide sprites (not used on this screen)
    HIDE_SPRITES;
    SHOW_BKG;
    DISPLAY_ON;
}

/**
 * Update coin flip screen (called every frame)
 */
void update_coinflip(void) {
    // Increment frame counter for randomness seeding
    frame_counter++;

    // Update transition animation if active
    if (update_transition()) {
        return;
    }

    // Update animation if active
    if (anim_phase != ANIM_PHASE_NONE) {
        // Update input for early skip detection
        input_update();
        update_animation();
        return;
    }

    // Update input state
    input_update();

    // Handle Left/Right selection
    if (input_pressed(J_LEFT)) {
        if (current_selection == SIDE_DARK) {
            clear_border(current_selection);
            prev_selection = current_selection;
            current_selection = SIDE_LIGHT;
            draw_border(current_selection);
        }
    } else if (input_pressed(J_RIGHT)) {
        if (current_selection == SIDE_LIGHT) {
            clear_border(current_selection);
            prev_selection = current_selection;
            current_selection = SIDE_DARK;
            draw_border(current_selection);
        }
    }

    // Confirm with A - start coin flip
    if (input_pressed(J_A)) {
        selected_side = current_selection;
        start_animation();
    }

    // Back with B - return to difficulty select
    if (input_pressed(J_B)) {
        next_state = STATE_DIFFICULTY_SELECT;
    }
}

/**
 * Cleanup coin flip screen
 */
void cleanup_coinflip(void) {
    // Ensure display is on and clear animation state
    DISPLAY_ON;
    anim_phase = ANIM_PHASE_NONE;
    transition_phase = TRANSITION_IDLE;
}
