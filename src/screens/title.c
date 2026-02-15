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
#include "util/sound.h"
#include "util/music.h"
#include "util/portrait.h"
#include "util/screen_utils.h"
#include "vram_layout.h"

// External references to generated assets
extern const uint8_t board_tiles_pieces_tiles[];
extern const uint8_t border_tiles[];
extern const unsigned char border_map[];
extern const unsigned char arrow_tiles[];

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

#define TITLE_BOARD_TILE_START VRAM_GAMEBOARD_START
#define TITLE_BOARD_TILE_COUNT 72

#define TITLE_BORDER_TILE_START 100
#define TITLE_PORTRAIT_TILE_START VRAM_PIECE_TILES_START

#define TITLE_PORTRAIT_BOX_X 13
#define TITLE_PORTRAIT_BOX_Y 6
#define TITLE_PORTRAIT_X (TITLE_PORTRAIT_BOX_X + 1)
#define TITLE_PORTRAIT_Y (TITLE_PORTRAIT_BOX_Y + 1)

// Empty square base tile for each square type (from board_tiles_pieces.png)
static const uint8_t title_square_empty_base[6] = {
    0, 12, 24, 36, 48, 60
};

typedef struct {
    uint8_t x;
    uint8_t y;
    uint8_t square_type;
} TitleBoardSquare_t;

// Same board coordinates as gameplay board rendering
static const TitleBoardSquare_t title_board_squares[] = {
    { 8, 6, 1}, { 6, 6, 2}, { 4, 6, 1}, { 2, 6, 0},
    { 2, 4, 4}, { 4, 4, 2}, { 6, 4, 5}, { 8, 4, 0},
    {10, 4, 2}, {12, 4, 5}, {14, 4, 1}, {16, 4, 2},
    {16, 6, 3}, {14, 6, 0},
    { 8, 2, 1}, { 6, 2, 2}, { 4, 2, 1}, { 2, 2, 0},
    {16, 2, 3}, {14, 2, 0}
};

static void draw_music_option(void) {
    clear_text_row(0, MENU_TEXT_ROW_2, 12);
    draw_text(MENU_TEXT_X, MENU_TEXT_ROW_2, is_music_enabled() ? "MUSIC ON" : "MUSIC OFF");
}

static void draw_board_square_empty(uint8_t x, uint8_t y, uint8_t square_type) {
    uint8_t tile = title_square_empty_base[square_type];
    set_bkg_tile_xy(x, y, TITLE_BOARD_TILE_START + tile);
    set_bkg_tile_xy((uint8_t)(x + 1), y, TITLE_BOARD_TILE_START + tile + 2);
    set_bkg_tile_xy(x, (uint8_t)(y + 1), TITLE_BOARD_TILE_START + tile + 1);
    set_bkg_tile_xy((uint8_t)(x + 1), (uint8_t)(y + 1), TITLE_BOARD_TILE_START + tile + 3);
}

static void draw_title_board(void) {
    uint8_t i;
    for (i = 0; i < (sizeof(title_board_squares) / sizeof(title_board_squares[0])); i++) {
        draw_board_square_empty(title_board_squares[i].x,
                                title_board_squares[i].y,
                                title_board_squares[i].square_type);
    }
}

static void draw_showcase_portrait(void) {
    load_portrait_tiles_for_char(showcase_opponent, TITLE_PORTRAIT_TILE_START);
    draw_portrait_expr(showcase_opponent, showcase_expr,
                       TITLE_PORTRAIT_TILE_START,
                       TITLE_PORTRAIT_X, TITLE_PORTRAIT_Y, 1);
}

static void draw_portrait_box(void) {
    draw_border_frame(TITLE_PORTRAIT_BOX_X, TITLE_PORTRAIT_BOX_Y, 7, 7,
                      TITLE_BORDER_TILE_START, border_map);
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

void init_title(void) {
    uint8_t row;

    DISPLAY_OFF;

    load_font();
    clear_rect((uint8_t)(FONT_TILE_START + CHAR_BLANK), 0, 0, 20, 18);

    // Draw board using board_tiles_pieces only (no board map/background)
    set_bkg_data(TITLE_BOARD_TILE_START, TITLE_BOARD_TILE_COUNT, board_tiles_pieces_tiles);
    draw_title_board();

    // Load border tiles for portrait frame
    set_bkg_data(TITLE_BORDER_TILE_START, 25, border_tiles);

    draw_text(5, 0, "ROYAL GAME");
    draw_text(7, 1, "OF UR");

    clear_text_row(0, MENU_TEXT_ROW_1, 12);
    clear_text_row(0, MENU_TEXT_ROW_2, 12);
    clear_text_row(0, MENU_TEXT_ROW_3, 12);
    draw_text(MENU_TEXT_X, MENU_TEXT_ROW_1, "START GAME");
    draw_music_option();
    draw_text(MENU_TEXT_X, MENU_TEXT_ROW_3, "LINK CABLE");

    set_sprite_data(VRAM_SPRITE_ARROW, 1, arrow_tiles);

    arrow_sprite_index = 0;
    set_sprite_tile(arrow_sprite_index, VRAM_SPRITE_ARROW);
    set_sprite_prop(arrow_sprite_index, 0);

    selected_option = MENU_START_GAME;
    move_sprite(arrow_sprite_index, ARROW_X, ARROW_START_Y + (selected_option * ARROW_SPACING));

    draw_portrait_box();
    showcase_opponent = TITLE_SHOWCASE_MERCHANT;
    showcase_expr = PORTRAIT_EXPR_HAPPY;
    showcase_timer = 0;
    showcase_toggles = 0;
    draw_showcase_portrait();

    // Re-clear UI rows after portrait tile loads so text stays intact
    for (row = 10; row < 18; row++) {
        clear_text_row(0, row, 12);
    }
    draw_text(MENU_TEXT_X, MENU_TEXT_ROW_1, "START GAME");
    draw_music_option();
    draw_text(MENU_TEXT_X, MENU_TEXT_ROW_3, "LINK CABLE");

    input_reset();

    BGP_REG = 0xE4;
    OBP0_REG = 0xE0;

    SHOW_BKG;
    SHOW_SPRITES;
    DISPLAY_ON;
}

void update_title(void) {
    if (update_transition()) {
        return;
    }

    input_update();
    update_showcase_animation();

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

void cleanup_title(void) {
    move_sprite(arrow_sprite_index, 0, 0);
    DISPLAY_ON;
}
