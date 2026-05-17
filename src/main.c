/**
 * main.c
 * Entry point and main game loop for Royal Game of Ur
 */

#include <gb/gb.h>
#include <gb/cgb.h>
#include <stdint.h>
#include "game_types.h"
#include "screens/title.h"
#include "screens/opponent_select.h"
#include "screens/difficulty_select.h"
#include "screens/coinflip.h"
#include "screens/game.h"
#include "screens/endgame.h"
#include "link/link_connect.h"
#include "link/link_profile.h"
#include "util/font.h"
#include "util/sound.h"
#include "util/music.h"
#include "util/portrait.h"

// Global game state
ScreenState_t current_state = STATE_TITLE;
ScreenState_t next_state = STATE_TITLE;

/**
 * Main entry point
 */
void main(void) {
    // Ensure display is off before any VRAM writes
    // (TGB Dual and other emulators may not guarantee safe VRAM access at boot)
    DISPLAY_OFF;

    // On Game Boy Color, install a grayscale CGB palette so the existing DMG
    // BGP_REG/OBP*_REG writes throughout the game produce the same four shades
    // they do on DMG, instead of the boot ROM's compatibility colorization.
    // Palette 1 is the portrait palette (white / beige / brown / black) used
    // for tiles whose BG attribute byte selects it.
    if (_cpu == CGB_TYPE) {
        static const palette_color_t grayscale[4] = {
            RGB_WHITE, RGB_LIGHTGRAY, RGB_DARKGRAY, RGB_BLACK
        };
        static const palette_color_t portrait_palette[4] = {
            RGB_WHITE,
            RGB8(248, 224, 184),
            RGB8( 96,  56,  16),
            RGB_BLACK
        };
        static const palette_color_t rosette_palette[4] = {
            RGB_WHITE,
            RGB8(168, 208, 248),
            RGB8( 40,  80, 184),
            RGB_BLACK
        };
        static const palette_color_t board_sand_palette[4] = {
            RGB_WHITE,
            RGB8(224, 184, 120),
            RGB8(160, 112,  56),
            RGB_BLACK
        };
        set_bkg_palette(0, 1, grayscale);
        set_bkg_palette(1, 1, portrait_palette);
        set_bkg_palette(2, 1, rosette_palette);
        set_bkg_palette(3, 1, board_sand_palette);
        set_sprite_palette(0, 1, grayscale);
        set_sprite_palette(1, 1, grayscale);
        clear_bg_attributes();
    }

    // Set background palette explicitly (boot ROM normally sets this to 0xFC,
    // but some emulators like TGB Dual may not emulate the boot ROM correctly)
    BGP_REG = 0xE4;

    // Load font tiles once at boot (shared across all screens)
    load_font_inverted();

    // Initialize sound system (ch3 wave + ch4 noise)
    init_sound();

    // Start background music on ch1 + ch2
    init_music();

    // Initialize the title screen
    init_title();

    // Enable display with background and sprites
    DISPLAY_ON;
    SHOW_BKG;
    SHOW_SPRITES;

    // Main game loop
    while (1) {
        // Handle state transitions
        if (next_state != current_state) {
            // Cleanup current state if needed
            switch (current_state) {
                case STATE_TITLE:
                    cleanup_title();
                    break;
                case STATE_OPPONENT_SELECT:
                    cleanup_opponent_select();
                    break;
                case STATE_DIFFICULTY_SELECT:
                    cleanup_difficulty_select();
                    break;
                case STATE_COINFLIP:
                    cleanup_coinflip();
                    break;
                case STATE_GAME:
                    cleanup_game();
                    break;
                case STATE_ENDGAME:
                    cleanup_endgame();
                    break;
                case STATE_LINK_CONNECT:
                    cleanup_link_connect();
                    break;
                case STATE_LINK_PROFILE_SELECT:
                    cleanup_link_profile();
                    break;
                default:
                    break;
            }

            // Initialize next state
            current_state = next_state;
            if (_cpu == CGB_TYPE) {
                clear_bg_attributes();
            }
            switch (current_state) {
                case STATE_TITLE:
                    init_title();
                    break;
                case STATE_OPPONENT_SELECT:
                    init_opponent_select();
                    break;
                case STATE_DIFFICULTY_SELECT:
                    init_difficulty_select();
                    break;
                case STATE_COINFLIP:
                    init_coinflip();
                    break;
                case STATE_GAME:
                    init_game();
                    break;
                case STATE_ENDGAME:
                    init_endgame();
                    break;
                case STATE_LINK_CONNECT:
                    init_link_connect();
                    break;
                case STATE_LINK_PROFILE_SELECT:
                    init_link_profile();
                    break;
                default:
                    break;
            }
        }

        // Update current state
        switch (current_state) {
            case STATE_TITLE:
                update_title();
                break;
            case STATE_OPPONENT_SELECT:
                update_opponent_select();
                break;
            case STATE_DIFFICULTY_SELECT:
                update_difficulty_select();
                break;
            case STATE_COINFLIP:
                update_coinflip();
                break;
            case STATE_GAME:
                update_game();
                break;
            case STATE_ENDGAME:
                update_endgame();
                break;
            case STATE_LINK_CONNECT:
                update_link_connect();
                break;
            case STATE_LINK_PROFILE_SELECT:
                update_link_profile();
                break;
            default:
                break;
        }

        // Advance sound sequencer (chime multi-note effects)
        update_sound();

        // Advance background music sequencer
        update_music();

        // Wait for VBlank before next frame
        wait_vbl_done();
    }
}
