#include <gb/gb.h>
#include <stdint.h>
#include <string.h>
#include "game_types.h"
#include "screens/endgame.h"
#include "util/opponent_data.h"
#include "util/font.h"
#include "util/input.h"
#include "util/portrait.h"
#include "util/screen_utils.h"
#include "screens/game.h"
#include "link/link.h"

// Portrait assets handled by portrait.c

extern const uint8_t border_tiles[];
extern const unsigned char border_map[];

extern ScreenState_t next_state;
extern uint8_t selected_opponent;
extern uint8_t human_won;

// White tile data
static const uint8_t white_tile[16] = {
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};

// Draw centered text (centers horizontally)
static void draw_centered_text(uint8_t y, const char *text) {
    uint8_t len = strlen(text);
    uint8_t x = (20 - len) / 2;
    draw_text_inverted(x, y, text);
}

// Draw opponent portrait
static void draw_opponent_portrait(void) {
    draw_portrait(selected_opponent, ENDGAME_PORTRAIT_TILE_START, ENDGAME_PORTRAIT_X, ENDGAME_PORTRAIT_Y);
}

// Draw border around portrait
static void draw_border(void) {
    uint8_t x = ENDGAME_PORTRAIT_X - 1;  // Border is 1 tile outside portrait
    uint8_t y = ENDGAME_PORTRAIT_Y - 1;
    draw_border_frame(x, y, ENDGAME_BORDER_WIDTH, ENDGAME_BORDER_HEIGHT,
                      ENDGAME_BORDER_TILE_START, border_map);
}

void init_endgame(void) {
    DISPLAY_OFF;

    // Load white tile and fill screen
    set_bkg_data(ENDGAME_WHITE_TILE, 1, white_tile);
    fill_screen_with_tile(ENDGAME_WHITE_TILE);

    // Load border tiles
    set_bkg_data(ENDGAME_BORDER_TILE_START, 25, border_tiles);

    // Draw opponent portrait
    draw_opponent_portrait();

    // Draw border around portrait
    draw_border();

    // Draw result text (offset 1 character to the right)
    if (human_won) {
        const char *result_text = "YOU WON !";
        uint8_t x = (20 - strlen(result_text)) / 2 + 1;  // Centered + 1 offset
        draw_text_inverted(x, ENDGAME_RESULT_Y, result_text);
        draw_centered_text(ENDGAME_YOU_BEAT_Y, "YOU BEAT");
        draw_centered_text(ENDGAME_NAME_Y, opponent_names[selected_opponent]);
    } else {
        const char *result_text = "YOU LOST !";
        uint8_t x = (20 - strlen(result_text)) / 2 + 1;  // Centered + 1 offset
        draw_text_inverted(x, ENDGAME_RESULT_Y, result_text);
        if (game_mode == GAME_MODE_LINK) {
            draw_centered_text(ENDGAME_YOU_BEAT_Y, "DEFEATED BY");
            draw_centered_text(ENDGAME_NAME_Y, opponent_names[selected_opponent]);
        }
    }

    // Draw instructions
    draw_centered_text(ENDGAME_PROMPT_Y, "PRESS A FOR");
    draw_centered_text(ENDGAME_PROMPT2_Y, "NEW GAME");

    // Reset input
    input_reset();

    // Set background palette
    BGP_REG = 0xE4;

    SHOW_BKG;
    DISPLAY_ON;
}

void update_endgame(void) {
    input_update();

    if (input_pressed(J_A)) {
        if (game_mode == GAME_MODE_LINK) {
            // Link mode: reset and return to title
            game_mode = GAME_MODE_SINGLE;
            link_reset();
            next_state = STATE_TITLE;
        } else {
            // Single player: return to opponent selection
            next_state = STATE_OPPONENT_SELECT;
        }
    }
}

void cleanup_endgame(void) {
    DISPLAY_ON;
}
