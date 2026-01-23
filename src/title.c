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

// External reference to next_state from main.c
extern ScreenState_t next_state;

// Title screen state
static uint8_t selected_option = MENU_START_GAME;
static uint8_t arrow_sprite_index = 0;

// Input state for edge detection
static uint8_t prev_input = 0;

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

    // Set up arrow sprite (sprite 0)
    arrow_sprite_index = 0;
    set_sprite_tile(arrow_sprite_index, 0);

    // Position arrow at first menu option
    selected_option = MENU_START_GAME;
    move_sprite(arrow_sprite_index, ARROW_X, ARROW_START_Y + (selected_option * ARROW_SPACING));

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
    uint8_t input = joypad();
    uint8_t pressed = input & ~prev_input;  // Edge detection: newly pressed buttons

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
            // Flash screen as placeholder transition
            DISPLAY_OFF;
            delay(50);
            DISPLAY_ON;
            delay(50);
            DISPLAY_OFF;
            delay(50);
            DISPLAY_ON;

            // TODO: Transition to opponent select when Phase 3 is implemented
            // For now, just stay on title screen
            // next_state = STATE_OPPONENT_SELECT;
        } else if (selected_option == MENU_LINK_CABLE) {
            // Link cable not yet implemented - flash screen
            DISPLAY_OFF;
            delay(100);
            DISPLAY_ON;
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

    // Clear sprite data if needed
    // (for now, we'll keep sprites loaded as they may be reused)
}
