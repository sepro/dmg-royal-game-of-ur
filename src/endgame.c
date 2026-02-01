#include <gb/gb.h>
#include <stdint.h>
#include <string.h>
#include "game_types.h"
#include "endgame.h"
#include "opponent_data.h"
#include "font.h"
#include "input.h"

// External references
extern const uint8_t profile_01_tiles[];
extern const unsigned char profile_01_map[];
extern const uint8_t profile_02_tiles[];
extern const unsigned char profile_02_map[];
extern const uint8_t profile_03_tiles[];
extern const unsigned char profile_03_map[];
extern const uint8_t profile_04_tiles[];
extern const unsigned char profile_04_map[];

extern ScreenState_t next_state;
extern uint8_t selected_opponent;
extern uint8_t human_won;

// White tile data
static const uint8_t white_tile[16] = {
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};

// Fill screen with white
static void fill_screen_white(void) {
    uint8_t row[20];
    for (uint8_t i = 0; i < 20; i++) {
        row[i] = ENDGAME_WHITE_TILE;
    }
    for (uint8_t y = 0; y < 18; y++) {
        set_bkg_tiles(0, y, 20, 1, row);
    }
}

// Draw centered text (centers horizontally)
static void draw_centered_text(uint8_t y, const char *text) {
    uint8_t len = strlen(text);
    uint8_t x = (20 - len) / 2;
    draw_text_inverted(x, y, text);
}

// Draw opponent portrait
static void draw_opponent_portrait(void) {
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

    // Load portrait tiles
    set_bkg_data(ENDGAME_PORTRAIT_TILE_START, tile_count, tiles);

    // Draw 5x5 portrait
    uint8_t row_buf[5];
    for (uint8_t row = 0; row < 5; row++) {
        for (uint8_t col = 0; col < 5; col++) {
            row_buf[col] = ENDGAME_PORTRAIT_TILE_START + map[row * 5 + col];
        }
        set_bkg_tiles(ENDGAME_PORTRAIT_X, ENDGAME_PORTRAIT_Y + row, 5, 1, row_buf);
    }
}

void init_endgame(void) {
    DISPLAY_OFF;

    // Load white tile and fill screen
    set_bkg_data(ENDGAME_WHITE_TILE, 1, white_tile);
    fill_screen_white();

    // Load inverted font
    load_font_inverted();

    // Draw opponent portrait
    draw_opponent_portrait();

    // Draw result text
    if (human_won) {
        draw_centered_text(ENDGAME_RESULT_Y, "YOU WON !");
        draw_centered_text(ENDGAME_YOU_BEAT_Y, "YOU BEAT");
        draw_centered_text(ENDGAME_NAME_Y, opponent_names[selected_opponent]);
    } else {
        draw_centered_text(ENDGAME_RESULT_Y, "YOU LOST !");
    }

    // Draw instructions
    draw_centered_text(ENDGAME_PROMPT_Y, "PRESS A FOR");
    draw_centered_text(ENDGAME_PROMPT2_Y, "NEW GAME");

    // Reset input
    input_reset();

    SHOW_BKG;
    DISPLAY_ON;
}

void update_endgame(void) {
    input_update();

    // A button returns to opponent selection
    if (input_pressed(J_A)) {
        next_state = STATE_OPPONENT_SELECT;
    }
}

void cleanup_endgame(void) {
    DISPLAY_ON;
}
