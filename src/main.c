/**
 * main.c
 * Entry point and main game loop for Royal Game of Ur
 */

#include <gb/gb.h>
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
#include "util/cgb.h"
#include "util/font.h"
#include "util/sound.h"
#include "util/music.h"

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

    // CGB: install accent palettes and zero the BG attribute plane. No-op on DMG.
    cgb_init_palettes();

    // Set background palette explicitly (boot ROM normally sets this to 0xFC,
    // but some emulators like TGB Dual may not emulate the boot ROM correctly).
    // Ignored on CGB; the grayscale CGB palette installed above produces the
    // same shades.
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

            // Wipe leftover BG attributes from the previous screen before the
            // next init runs. Done with the display off so the 1024 attribute
            // writes don't race the LCD; each init_* turns the display back
            // on once it has drawn.
            DISPLAY_OFF;
            cgb_clear_bg_attributes();

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
