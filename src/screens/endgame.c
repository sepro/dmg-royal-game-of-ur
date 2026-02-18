/**
 * endgame.c
 * Victory/defeat screen implementation
 * Displays result text, portrait animation, and falling piece celebration
 */

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
#include "link/link_profile.h"
#include "util/sound.h"
#include "util/falling_piece_anim.h"

extern const uint8_t border_tiles[];
extern const unsigned char border_map[];

extern ScreenState_t next_state;
extern uint8_t selected_opponent;
extern uint8_t human_won;

// Portrait animation state
static uint8_t anim_counter;
static uint8_t anim_showing_expr;  // 0 = normal, 1 = expression
static uint8_t anim_active;
static uint8_t anim_target_expr;   // PORTRAIT_EXPR_SAD or PORTRAIT_EXPR_HAPPY
static uint8_t anim_local_expr;    // Link mode: local player's target expression

static uint8_t show_falling_piece_anim;
static FallingPieceAnimState endgame_falling_piece_anim;

// Draw centered text (centers horizontally)
static void draw_centered_text(uint8_t y, const char *text) {
    uint8_t len = strlen(text);
    uint8_t x = (20 - len) / 2;
    draw_text_inverted(x, y, text);
}

// Draw opponent portrait
static void draw_opponent_portrait(void) {
    draw_portrait_expr(selected_opponent, PORTRAIT_EXPR_NORMAL,
                       ENDGAME_PORTRAIT_TILE_START,
                       ENDGAME_PORTRAIT_X, ENDGAME_PORTRAIT_Y, 0);
}

// Draw border around portrait
static void draw_border(void) {
    uint8_t x = ENDGAME_PORTRAIT_X - 1;
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

    // Load full merged portrait tileset (endgame has ample VRAM)
    load_portrait_tiles(ENDGAME_PORTRAIT_TILE_START);

    // Initialize animation state
    anim_active = 0;
    anim_counter = 0;
    anim_showing_expr = 0;
    show_falling_piece_anim = 0;

    if (game_mode == GAME_MODE_LINK) {
        // Link mode: dual portraits side by side
        // Left = local player, Right = opponent (remote)
        draw_portrait_expr(link_local_profile, PORTRAIT_EXPR_NORMAL,
                           ENDGAME_PORTRAIT_TILE_START,
                           ENDGAME_LINK_LEFT_X, ENDGAME_LINK_LEFT_Y, 0);
        draw_border_frame(ENDGAME_LINK_LEFT_BRD_X, ENDGAME_LINK_LEFT_BRD_Y,
                          ENDGAME_BORDER_WIDTH, ENDGAME_BORDER_HEIGHT,
                          ENDGAME_BORDER_TILE_START, border_map);

        draw_portrait_expr(selected_opponent, PORTRAIT_EXPR_NORMAL,
                           ENDGAME_PORTRAIT_TILE_START,
                           ENDGAME_LINK_RIGHT_X, ENDGAME_LINK_RIGHT_Y, 0);
        draw_border_frame(ENDGAME_LINK_RIGHT_BRD_X, ENDGAME_LINK_RIGHT_BRD_Y,
                          ENDGAME_BORDER_WIDTH, ENDGAME_BORDER_HEIGHT,
                          ENDGAME_BORDER_TILE_START, border_map);

        // Labels under portraits
        draw_text_inverted(3, ENDGAME_LINK_NAME_Y, "YOU");
        draw_text_inverted(14, ENDGAME_LINK_NAME_Y, "OTHER");

        // Animation: winner=happy, loser=sad
        if (human_won) {
            anim_local_expr = PORTRAIT_EXPR_HAPPY;
            anim_target_expr = PORTRAIT_EXPR_SAD;
            show_falling_piece_anim = 1;
        } else {
            anim_local_expr = PORTRAIT_EXPR_SAD;
            anim_target_expr = PORTRAIT_EXPR_HAPPY;
        }
        anim_active = 1;
    } else {
        // Single player: centered portrait
        draw_opponent_portrait();
        draw_border();

        if (human_won) {
            anim_target_expr = PORTRAIT_EXPR_SAD;
            anim_active = 1;
            show_falling_piece_anim = 1;
        } else {
            anim_target_expr = PORTRAIT_EXPR_HAPPY;
            anim_active = 1;
        }

        // "YOU BEAT" / opponent name text (single player only)
        if (human_won) {
            draw_centered_text(ENDGAME_YOU_BEAT_Y, "YOU BEAT");
            draw_centered_text(ENDGAME_NAME_Y, opponent_names[selected_opponent]);
        }
    }

    // Result text (shared by both modes)
    if (human_won) {
        const char *result_text = "YOU WON !";
        uint8_t x = (20 - strlen(result_text)) / 2 + 1;
        draw_text_inverted(x, ENDGAME_RESULT_Y, result_text);
    } else {
        const char *result_text = "YOU LOST !";
        uint8_t x = (20 - strlen(result_text)) / 2 + 1;
        draw_text_inverted(x, ENDGAME_RESULT_Y, result_text);
    }

    // Draw instructions
    draw_centered_text(ENDGAME_PROMPT_Y, "PRESS A FOR");
    draw_centered_text(ENDGAME_PROMPT2_Y, "NEW GAME");

    // Reset input
    input_reset();

    // Set palettes
    BGP_REG = 0xE4;
    // Falling piece sprites are authored for OBP0=0xE0 (same as title/game).
    // Without this, palette state from previous screens (e.g. game cleanup 0xFC)
    // can make endgame falling pieces render with incorrect shades.
    OBP0_REG = 0xE0;

    if (show_falling_piece_anim) {
        falling_piece_anim_load_tiles();
        falling_piece_anim_init(&endgame_falling_piece_anim, ENDGAME_FALLING_PIECE_SPRITE_INDEX);
        SHOW_SPRITES;
    } else {
        HIDE_SPRITES;
    }

    SHOW_BKG;
    DISPLAY_ON;

    // Play victory or loss chime
    play_sfx(human_won ? SFX_VICTORY : SFX_LOSS);
}

void update_endgame(void) {
    input_update();

    // Animate portrait expression
    if (anim_active) {
        anim_counter++;
        if (anim_counter >= ENDGAME_ANIM_INTERVAL) {
            anim_counter = 0;
            anim_showing_expr = !anim_showing_expr;

            if (game_mode == GAME_MODE_LINK) {
                // Link mode: animate both portraits
                uint8_t local_expr = anim_showing_expr ? anim_local_expr : PORTRAIT_EXPR_NORMAL;
                uint8_t remote_expr = anim_showing_expr ? anim_target_expr : PORTRAIT_EXPR_NORMAL;
                draw_portrait_expr(link_local_profile, local_expr,
                                   ENDGAME_PORTRAIT_TILE_START,
                                   ENDGAME_LINK_LEFT_X, ENDGAME_LINK_LEFT_Y, 0);
                draw_portrait_expr(selected_opponent, remote_expr,
                                   ENDGAME_PORTRAIT_TILE_START,
                                   ENDGAME_LINK_RIGHT_X, ENDGAME_LINK_RIGHT_Y, 0);
            } else {
                // Single player: animate opponent portrait only
                uint8_t expr = anim_showing_expr ? anim_target_expr : PORTRAIT_EXPR_NORMAL;
                draw_portrait_expr(selected_opponent, expr,
                                   ENDGAME_PORTRAIT_TILE_START,
                                   ENDGAME_PORTRAIT_X, ENDGAME_PORTRAIT_Y, 0);
            }
        }
    }


    if (show_falling_piece_anim) {
        falling_piece_anim_update(&endgame_falling_piece_anim);
    }

    if (input_pressed(J_A)) {
        play_sfx(SFX_CONFIRM);
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
    if (show_falling_piece_anim) {
        falling_piece_anim_hide(&endgame_falling_piece_anim);
    }

    HIDE_SPRITES;
    DISPLAY_ON;
}
