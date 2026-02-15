/**
 * title.c
 * Title screen implementation
 */

#include <gb/gb.h>
#include <stdint.h>
#include "game_types.h"
#include "screens/title.h"
#include "util/font.h"
#include "util/input.h"
#include "util/transition.h"
#include "util/random.h"
#include "util/sound.h"
#include "util/falling_piece_anim.h"

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

// Blink animation state
static uint8_t blink_state;        // BLINK_STATE_HIDDEN or BLINK_STATE_ANIMATING
static uint8_t blink_frame;        // Current frame (0-3)
static uint8_t blink_timer;        // Frame counter for animation/delay
static uint8_t blink_x, blink_y;   // Current sprite position

#if TITLE_ENABLE_FALLING_PIECES
static FallingPieceAnimState falling_piece_anim_state;
#endif

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

#if TITLE_ENABLE_FALLING_PIECES
    // Load falling piece sprite tiles (white + black, 16x16 each = 4 tiles each)
    falling_piece_anim_load_tiles();
#endif

    // Set up arrow sprite (sprite 0)
    arrow_sprite_index = 0;
    set_sprite_tile(arrow_sprite_index, 0);

    // Position arrow at first menu option
    selected_option = MENU_START_GAME;
    move_sprite(arrow_sprite_index, ARROW_X, ARROW_START_Y + (selected_option * ARROW_SPACING));

    // Initialize blink animation in hidden state with random delay
    blink_start_delay();

#if TITLE_ENABLE_FALLING_PIECES
    // Initialize falling piece animation in hidden state with random delay
    falling_piece_anim_init(&falling_piece_anim_state, FALLING_PIECE_SPRITE_INDEX);
#endif

    // Clear input state
    input_reset();

    // Set palettes
    BGP_REG = 0xE4;   // Standard background palette
    OBP0_REG = 0xE0;  // Sprite palette for dark background with light text

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

    // Update input state
    input_update();

    // Update sparkle animation
    update_blink();

#if TITLE_ENABLE_FALLING_PIECES
    // Update falling piece animation
    falling_piece_anim_update(&falling_piece_anim_state);
#endif

    // Handle up/down navigation
    if (input_pressed(J_UP)) {
        if (selected_option > 0) {
            selected_option--;
            move_sprite(arrow_sprite_index, ARROW_X, ARROW_START_Y + (selected_option * ARROW_SPACING));
            play_sfx(SFX_CURSOR);
        }
    } else if (input_pressed(J_DOWN)) {
        if (selected_option < MENU_OPTION_COUNT - 1) {
            selected_option++;
            move_sprite(arrow_sprite_index, ARROW_X, ARROW_START_Y + (selected_option * ARROW_SPACING));
            play_sfx(SFX_CURSOR);
        }
    }

    // Handle A button (confirm selection)
    if (input_pressed(J_A)) {
        play_sfx(SFX_CONFIRM);
        if (selected_option == MENU_START_GAME) {
            transition_start(STATE_OPPONENT_SELECT, TRANSITION_PHASE_COUNT_3);
        } else if (selected_option == MENU_LINK_CABLE) {
            transition_start(STATE_LINK_CONNECT, TRANSITION_PHASE_COUNT_3);
        }
    }
}

/**
 * Cleanup title screen
 */
void cleanup_title(void) {
    // Hide arrow sprite
    move_sprite(arrow_sprite_index, 0, 0);

    // Hide blink sprite
    move_sprite(BLINK_SPRITE_INDEX, 0, 0);

#if TITLE_ENABLE_FALLING_PIECES
    // Hide falling piece sprite
    falling_piece_anim_hide(&falling_piece_anim_state);
#endif

    // Ensure display is on
    DISPLAY_ON;
}
