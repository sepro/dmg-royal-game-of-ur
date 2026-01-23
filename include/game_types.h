/**
 * game_types.h
 * Common types, constants, and enumerations for Royal Game of Ur
 * NOTE: Renamed from types.h to avoid conflict with GBDK's types.h
 */

#ifndef GAME_TYPES_H
#define GAME_TYPES_H

#include <stdint.h>

// Screen states for the game state machine
typedef enum {
    STATE_TITLE,
    STATE_OPPONENT_SELECT,
    STATE_DIFFICULTY_SELECT,
    STATE_COINFLIP,
    STATE_GAME,
    STATE_ENDGAME
} ScreenState_t;

// Title screen menu options
typedef enum {
    MENU_START_GAME = 0,
    MENU_LINK_CABLE = 1,
    MENU_OPTION_COUNT = 2
} MenuOption_t;

// Constants
#define SCREEN_WIDTH 160
#define SCREEN_HEIGHT 144
#define TILE_SIZE 8

// Menu text positions (tile coordinates)
#define MENU_TEXT_X 5         // Text starts at tile column 5
#define MENU_TEXT_ROW_1 14    // "START GAME" at tile row 14
#define MENU_TEXT_ROW_2 16    // "LINK CABLE" at tile row 16

// Menu arrow sprite positions
// Sprite Y has 16px offset, so row 14 (pixel 112) = sprite Y 128
#define ARROW_X 36            // Pixel X position (left of text at tile 5 = pixel 40)
#define ARROW_START_Y 128     // Sprite Y for row 14 (112 + 16 offset)
#define ARROW_SPACING 16      // 2 tile rows = 16 pixels

// VBlank helper
#define WAIT_VBLANK wait_vbl_done()

#endif // GAME_TYPES_H
