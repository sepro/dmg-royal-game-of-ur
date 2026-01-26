/**
 * title.c
 * Title screen implementation
 */

#include <gb/gb.h>
#include <stdint.h>
#include "game_types.h"
#include "title.h"
#include "font.h"

// Forward declarations for generated assets
// (Actual data is compiled separately from assets/generated/*.h)
extern const unsigned char title_tiles[];
extern const unsigned char title_map[];
extern const unsigned char arrow_tiles[];
extern const uint8_t blink_tiles[];

// External reference to next_state from main.c
extern ScreenState_t next_state;

// Title screen state
static uint8_t selected_option = MENU_START_GAME;
static uint8_t arrow_sprite_index = 0;

// Input state for edge detection
static uint8_t prev_input = 0;

// Blink animation state
static uint8_t blink_state;        // BLINK_STATE_HIDDEN or BLINK_STATE_ANIMATING
static uint8_t blink_frame;        // Current frame (0-3)
static uint8_t blink_timer;        // Frame counter for animation/delay
static uint8_t blink_x, blink_y;   // Current sprite position

// Transition animation state
static uint8_t transition_timer = 0;
static uint8_t transition_phase = TRANSITION_IDLE;
static ScreenState_t transition_target = STATE_TITLE;
static uint8_t transition_phase_count = 0;

/**
 * Get a pseudo-random value from DIV register
 */
static uint8_t get_random(void) {
    return DIV_REG;
}

/**
 * Get a random value in range [min, max]
 */
static uint8_t get_random_range(uint8_t min, uint8_t max) {
    return min + (get_random() % (max - min + 1));
}

/**
 * Start a new blink animation at a random position
 */
static void blink_start_animation(void) {
    blink_state = BLINK_STATE_ANIMATING;
    blink_frame = 0;
    blink_timer = BLINK_ANIM_SPEED;

    // Pick random position within bounds
    blink_x = get_random_range(BLINK_MIN_X, BLINK_MAX_X);
    blink_y = get_random_range(BLINK_MIN_Y, BLINK_MAX_Y);

    // Set initial tile and position
    set_sprite_tile(BLINK_SPRITE_INDEX, BLINK_TILE_START + blink_frame);
    move_sprite(BLINK_SPRITE_INDEX, blink_x, blink_y);
}

/**
 * Hide sprite and start delay before next animation
 */
static void blink_start_delay(void) {
    blink_state = BLINK_STATE_HIDDEN;
    blink_timer = get_random_range(BLINK_DELAY_MIN, BLINK_DELAY_MAX);

    // Hide sprite by moving off-screen
    move_sprite(BLINK_SPRITE_INDEX, 0, 0);
}

/**
 * Update blink animation (called every frame)
 */
static void update_blink(void) {
    if (blink_state == BLINK_STATE_HIDDEN) {
        // Countdown delay timer
        if (blink_timer > 0) {
            blink_timer--;
        } else {
            // Delay finished, start new animation
            blink_start_animation();
        }
    } else {
        // Animating state
        if (blink_timer > 0) {
            blink_timer--;
        } else {
            // Advance to next frame
            blink_frame++;
            if (blink_frame >= BLINK_FRAME_COUNT) {
                // Animation complete, start delay
                blink_start_delay();
            } else {
                // Show next frame
                blink_timer = BLINK_ANIM_SPEED;
                set_sprite_tile(BLINK_SPRITE_INDEX, BLINK_TILE_START + blink_frame);
            }
        }
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
 * Initialize title screen
 */
void init_title(void) {
    // Disable display during VRAM writes
    DISPLAY_OFF;

    // Load background tiles (139 unique tiles after deduplication)
    set_bkg_data(0, 139, title_tiles);

    // Load background map (20x18 tiles for 160x144 screen)
    set_bkg_tiles(0, 0, 20, 18, title_map);

    // Load font tiles for menu text
    load_font();

    // Clear menu area with black background and draw menu text
    // Menu is at rows 14 and 16, text starts at x=5 (arrow at x=3)
    clear_text_row(0, MENU_TEXT_ROW_1, 20);
    clear_text_row(0, MENU_TEXT_ROW_2, 20);
    draw_text(MENU_TEXT_X, MENU_TEXT_ROW_1, "START GAME");
    draw_text(MENU_TEXT_X, MENU_TEXT_ROW_2, "LINK CABLE");

    // Load arrow sprite tiles (into sprite pattern table, separate from BG)
    set_sprite_data(0, 1, arrow_tiles);

    // Load blink sprite tiles (4 frames starting at tile 1)
    set_sprite_data(BLINK_TILE_START, BLINK_FRAME_COUNT, blink_tiles);

    // Set up arrow sprite (sprite 0)
    arrow_sprite_index = 0;
    set_sprite_tile(arrow_sprite_index, 0);

    // Position arrow at first menu option
    selected_option = MENU_START_GAME;
    move_sprite(arrow_sprite_index, ARROW_X, ARROW_START_Y + (selected_option * ARROW_SPACING));

    // Initialize blink animation in hidden state with random delay
    blink_start_delay();

    // Initialize transition state
    transition_phase = TRANSITION_IDLE;
    transition_timer = 0;

    // Clear previous input state
    prev_input = 0;

    // Enable display
    SHOW_BKG;
    SHOW_SPRITES;
    DISPLAY_ON;
}

/**
 * Update title screen (called every frame)
 */
void update_title(void) {
    // Update transition animation if active
    if (update_transition()) {
        return;  // Skip input handling during transition
    }

    uint8_t input = joypad();
    uint8_t pressed = input & ~prev_input;  // Edge detection: newly pressed buttons

    // Update sparkle animation
    update_blink();

    // Handle up/down navigation
    if (pressed & J_UP) {
        if (selected_option > 0) {
            selected_option--;
            move_sprite(arrow_sprite_index, ARROW_X, ARROW_START_Y + (selected_option * ARROW_SPACING));
        }
    } else if (pressed & J_DOWN) {
        if (selected_option < MENU_OPTION_COUNT - 1) {
            selected_option++;
            move_sprite(arrow_sprite_index, ARROW_X, ARROW_START_Y + (selected_option * ARROW_SPACING));
        }
    }

    // Handle A button (confirm selection)
    if (pressed & J_A) {
        if (selected_option == MENU_START_GAME) {
            transition_start(STATE_OPPONENT_SELECT, TRANSITION_PHASE_COUNT_3);
        } else if (selected_option == MENU_LINK_CABLE) {
            transition_start(STATE_TITLE, TRANSITION_PHASE_COUNT_1);
        }
    }

    // Store current input for next frame
    prev_input = input;
}

/**
 * Cleanup title screen
 */
void cleanup_title(void) {
    // Hide arrow sprite
    move_sprite(arrow_sprite_index, 0, 0);

    // Hide blink sprite
    move_sprite(BLINK_SPRITE_INDEX, 0, 0);

    // Ensure display is on and clear transition state
    DISPLAY_ON;
    transition_phase = TRANSITION_IDLE;

    // Clear sprite data if needed
    // (for now, we'll keep sprites loaded as they may be reused)
}
