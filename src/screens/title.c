/**
 * title.c
 * Title screen implementation
 */

#include <gb/gb.h>
#include <stdint.h>
#include "game_types.h"
#include "screens/title.h"
#include "screens/game.h"
#include "util/font.h"
#include "util/input.h"
#include "util/transition.h"
#include "util/sound.h"
#include "util/music.h"
#include "util/portrait.h"

// External references to generated assets
extern const uint8_t board_tiles[];
extern const unsigned char board_map[];
extern const unsigned char arrow_tiles[];
extern const uint8_t selection_border_tiles[];

// External reference to next_state from main.c
extern ScreenState_t next_state;

// Title screen state
static uint8_t selected_option = MENU_START_GAME;
static uint8_t arrow_sprite_index = 0;

// Portrait showcase animation
static uint8_t showcase_opponent;
static uint8_t showcase_expr;
static uint8_t showcase_timer;
static uint8_t showcase_toggles;

#define TITLE_SHOWCASE_MERCHANT   1
#define TITLE_SHOWCASE_PRIESTESS  3
#define TITLE_SHOWCASE_TOGGLE_INTERVAL 16
#define TITLE_SHOWCASE_TOGGLES_PER_SWAP 8

#define TITLE_PORTRAIT_BOX_X 13
#define TITLE_PORTRAIT_BOX_Y 6
#define TITLE_PORTRAIT_X (TITLE_PORTRAIT_BOX_X + 1)
#define TITLE_PORTRAIT_Y (TITLE_PORTRAIT_BOX_Y + 1)

#define BOX_SPRITE_TOP_LEFT      1
#define BOX_SPRITE_TOP_RIGHT     2
#define BOX_SPRITE_BOTTOM_LEFT   3
#define BOX_SPRITE_BOTTOM_RIGHT  4
#define BOX_SPRITE_TILE          1
#define TITLE_PORTRAIT_TILE_START VRAM_PIECE_TILES_START

static void draw_music_option(void) {
    clear_text_row(0, MENU_TEXT_ROW_2, 12);
    draw_text(MENU_TEXT_X, MENU_TEXT_ROW_2, is_music_enabled() ? "MUSIC ON" : "MUSIC OFF");
}

static void draw_showcase_portrait(void) {
    load_portrait_tiles_for_char(showcase_opponent, TITLE_PORTRAIT_TILE_START);
    draw_portrait_expr(showcase_opponent, showcase_expr,
                       TITLE_PORTRAIT_TILE_START,
                       TITLE_PORTRAIT_X, TITLE_PORTRAIT_Y, 1);
}

static void draw_portrait_box(void) {
    uint8_t x_left = (uint8_t)(TITLE_PORTRAIT_BOX_X * 8 + 8);
    uint8_t y_top = (uint8_t)(TITLE_PORTRAIT_BOX_Y * 8 + 16);
    uint8_t x_right = (uint8_t)((TITLE_PORTRAIT_BOX_X + 6) * 8 + 8);
    uint8_t y_bottom = (uint8_t)((TITLE_PORTRAIT_BOX_Y + 6) * 8 + 16);

    set_sprite_tile(BOX_SPRITE_TOP_LEFT, BOX_SPRITE_TILE);
    set_sprite_prop(BOX_SPRITE_TOP_LEFT, 0);
    move_sprite(BOX_SPRITE_TOP_LEFT, x_left, y_top);

    set_sprite_tile(BOX_SPRITE_TOP_RIGHT, BOX_SPRITE_TILE);
    set_sprite_prop(BOX_SPRITE_TOP_RIGHT, S_FLIPX);
    move_sprite(BOX_SPRITE_TOP_RIGHT, x_right, y_top);

    set_sprite_tile(BOX_SPRITE_BOTTOM_LEFT, BOX_SPRITE_TILE);
    set_sprite_prop(BOX_SPRITE_BOTTOM_LEFT, S_FLIPY);
    move_sprite(BOX_SPRITE_BOTTOM_LEFT, x_left, y_bottom);

    set_sprite_tile(BOX_SPRITE_BOTTOM_RIGHT, BOX_SPRITE_TILE);
    set_sprite_prop(BOX_SPRITE_BOTTOM_RIGHT, S_FLIPX | S_FLIPY);
    move_sprite(BOX_SPRITE_BOTTOM_RIGHT, x_right, y_bottom);
}

static void update_showcase_animation(void) {
    showcase_timer++;
    if (showcase_timer < TITLE_SHOWCASE_TOGGLE_INTERVAL) {
        return;
    }

    showcase_timer = 0;
    showcase_toggles++;

    showcase_expr = (showcase_expr == PORTRAIT_EXPR_NORMAL)
        ? PORTRAIT_EXPR_HAPPY
        : PORTRAIT_EXPR_NORMAL;
    draw_showcase_portrait();

    if (showcase_toggles >= TITLE_SHOWCASE_TOGGLES_PER_SWAP) {
        showcase_toggles = 0;
        showcase_expr = PORTRAIT_EXPR_HAPPY;
        showcase_opponent = (showcase_opponent == TITLE_SHOWCASE_MERCHANT)
            ? TITLE_SHOWCASE_PRIESTESS
            : TITLE_SHOWCASE_MERCHANT;
        draw_showcase_portrait();
    }
}

/**
 * Initialize title screen
 */
void init_title(void) {
    uint8_t row;

    // Disable display during VRAM writes
    DISPLAY_OFF;

    // Board-only title scene (black surroundings, no title artwork tiles)
    set_bkg_data(GAME_BOARD_TILE_START, GAME_BOARD_TILE_COUNT, board_tiles);
    set_bkg_tiles(BOARD_X, BOARD_Y, BOARD_WIDTH, BOARD_HEIGHT, board_map);

    // Load font and clear lower rows to black
    load_font();
    for (row = 10; row < 18; row++) {
        clear_text_row(0, row, 20);
    }

    // Title text centered near top
    draw_text(5, 0, "ROYAL GAME");
    draw_text(7, 1, "OF UR");

    // Menu order: START GAME, MUSIC ON/OFF, LINK CABLE
    clear_text_row(0, MENU_TEXT_ROW_1, 12);
    clear_text_row(0, MENU_TEXT_ROW_2, 12);
    clear_text_row(0, MENU_TEXT_ROW_3, 12);
    draw_text(MENU_TEXT_X, MENU_TEXT_ROW_1, "START GAME");
    draw_music_option();
    draw_text(MENU_TEXT_X, MENU_TEXT_ROW_3, "LINK CABLE");

    // Load arrow and selection-border corner sprites
    set_sprite_data(VRAM_SPRITE_ARROW, 1, arrow_tiles);
    set_sprite_data(BOX_SPRITE_TILE, 1, selection_border_tiles);

    // Set up arrow sprite
    arrow_sprite_index = 0;
    set_sprite_tile(arrow_sprite_index, VRAM_SPRITE_ARROW);
    set_sprite_prop(arrow_sprite_index, 0);

    selected_option = MENU_START_GAME;
    move_sprite(arrow_sprite_index, ARROW_X, ARROW_START_Y + (selected_option * ARROW_SPACING));

    // Draw portrait showcase frame and initialize animation state
    draw_portrait_box();
    showcase_opponent = TITLE_SHOWCASE_MERCHANT;
    showcase_expr = PORTRAIT_EXPR_HAPPY;
    showcase_timer = 0;
    showcase_toggles = 0;
    draw_showcase_portrait();

    // Clear input state
    input_reset();

    // Set palettes
    BGP_REG = 0xE4;
    OBP0_REG = 0xE0;

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
        return;
    }

    // Update input state
    input_update();

    // Update right-side portrait showcase animation
    update_showcase_animation();

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
        } else if (selected_option == MENU_MUSIC_TOGGLE) {
            set_music_enabled(!is_music_enabled());
            draw_music_option();
        } else if (selected_option == MENU_LINK_CABLE) {
            transition_start(STATE_LINK_CONNECT, TRANSITION_PHASE_COUNT_3);
        }
    }
}

/**
 * Cleanup title screen
 */
void cleanup_title(void) {
    move_sprite(arrow_sprite_index, 0, 0);
    move_sprite(BOX_SPRITE_TOP_LEFT, 0, 0);
    move_sprite(BOX_SPRITE_TOP_RIGHT, 0, 0);
    move_sprite(BOX_SPRITE_BOTTOM_LEFT, 0, 0);
    move_sprite(BOX_SPRITE_BOTTOM_RIGHT, 0, 0);
    DISPLAY_ON;
}
