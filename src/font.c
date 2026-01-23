/**
 * font.c
 * Simple 8x8 font implementation for menu text
 * White text on black background
 */

#include <gb/gb.h>
#include <stdint.h>
#include "font.h"

/*
 * Font tile data - 8x8 pixels, 2bpp format
 * Each tile is 16 bytes (2 bytes per row)
 * White pixels (color 0): both bytes = 0 for that bit position
 * Black pixels (color 3): both bytes = 1 for that bit position
 *
 * Bit layout: bit 7 = leftmost pixel, bit 0 = rightmost pixel
 */
const uint8_t font_tiles[] = {
    // CHAR_SPACE (index 0) - all black
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,

    // CHAR_A (index 1)
    //  .####.
    // ##..##
    // ##..##
    // ######
    // ##..##
    // ##..##
    // ##..##
    // ......
    0xFF, 0xFF,  // row 0: all black (top margin)
    0xC3, 0xC3,  // row 1: .####. -> 11000011
    0x99, 0x99,  // row 2: ##..## -> 10011001
    0x99, 0x99,  // row 3: ##..## -> 10011001
    0x81, 0x81,  // row 4: ###### -> 10000001
    0x99, 0x99,  // row 5: ##..## -> 10011001
    0x99, 0x99,  // row 6: ##..## -> 10011001
    0xFF, 0xFF,  // row 7: all black (bottom margin)

    // CHAR_B (index 2)
    // #####.
    // ##..##
    // #####.
    // ##..##
    // ##..##
    // #####.
    0xFF, 0xFF,
    0x83, 0x83,  // #####. -> 10000011
    0x99, 0x99,  // ##..## -> 10011001
    0x83, 0x83,  // #####. -> 10000011
    0x99, 0x99,  // ##..## -> 10011001
    0x99, 0x99,  // ##..## -> 10011001
    0x83, 0x83,  // #####. -> 10000011
    0xFF, 0xFF,

    // CHAR_C (index 3)
    // .####.
    // ##..##
    // ##....
    // ##....
    // ##..##
    // .####.
    0xFF, 0xFF,
    0xC3, 0xC3,  // .####. -> 11000011
    0x99, 0x99,  // ##..## -> 10011001
    0x9F, 0x9F,  // ##.... -> 10011111
    0x9F, 0x9F,  // ##.... -> 10011111
    0x99, 0x99,  // ##..## -> 10011001
    0xC3, 0xC3,  // .####. -> 11000011
    0xFF, 0xFF,

    // CHAR_E (index 4)
    // ######
    // ##....
    // #####.
    // ##....
    // ##....
    // ######
    0xFF, 0xFF,
    0x81, 0x81,  // ###### -> 10000001
    0x9F, 0x9F,  // ##.... -> 10011111
    0x87, 0x87,  // #####. -> 10000111
    0x9F, 0x9F,  // ##.... -> 10011111
    0x9F, 0x9F,  // ##.... -> 10011111
    0x81, 0x81,  // ###### -> 10000001
    0xFF, 0xFF,

    // CHAR_G (index 5)
    // .####.
    // ##....
    // ##.###
    // ##..##
    // ##..##
    // .####.
    0xFF, 0xFF,
    0xC3, 0xC3,  // .####. -> 11000011
    0x9F, 0x9F,  // ##.... -> 10011111
    0x91, 0x91,  // ##.### -> 10010001
    0x99, 0x99,  // ##..## -> 10011001
    0x99, 0x99,  // ##..## -> 10011001
    0xC3, 0xC3,  // .####. -> 11000011
    0xFF, 0xFF,

    // CHAR_I (index 6)
    // ######
    // ..##..
    // ..##..
    // ..##..
    // ..##..
    // ######
    0xFF, 0xFF,
    0x81, 0x81,  // ###### -> 10000001
    0xE7, 0xE7,  // ..##.. -> 11100111
    0xE7, 0xE7,  // ..##.. -> 11100111
    0xE7, 0xE7,  // ..##.. -> 11100111
    0xE7, 0xE7,  // ..##.. -> 11100111
    0x81, 0x81,  // ###### -> 10000001
    0xFF, 0xFF,

    // CHAR_K (index 7)
    // ##..##
    // ##.##.
    // ####..
    // ####..
    // ##.##.
    // ##..##
    0xFF, 0xFF,
    0x99, 0x99,  // ##..## -> 10011001
    0x93, 0x93,  // ##.##. -> 10010011
    0x87, 0x87,  // ####.. -> 10000111
    0x87, 0x87,  // ####.. -> 10000111
    0x93, 0x93,  // ##.##. -> 10010011
    0x99, 0x99,  // ##..## -> 10011001
    0xFF, 0xFF,

    // CHAR_L (index 8)
    // ##....
    // ##....
    // ##....
    // ##....
    // ##....
    // ######
    0xFF, 0xFF,
    0x9F, 0x9F,  // ##.... -> 10011111
    0x9F, 0x9F,  // ##.... -> 10011111
    0x9F, 0x9F,  // ##.... -> 10011111
    0x9F, 0x9F,  // ##.... -> 10011111
    0x9F, 0x9F,  // ##.... -> 10011111
    0x81, 0x81,  // ###### -> 10000001
    0xFF, 0xFF,

    // CHAR_M (index 9)
    // ##..##
    // ######
    // ######
    // ##.###
    // ##..##
    // ##..##
    0xFF, 0xFF,
    0x99, 0x99,  // ##..## -> 10011001
    0x81, 0x81,  // ###### -> 10000001
    0x81, 0x81,  // ###### -> 10000001
    0x89, 0x89,  // ##.### -> 10001001
    0x99, 0x99,  // ##..## -> 10011001
    0x99, 0x99,  // ##..## -> 10011001
    0xFF, 0xFF,

    // CHAR_N (index 10)
    // ##..##
    // ###.##
    // ######
    // ##.###
    // ##..##
    // ##..##
    0xFF, 0xFF,
    0x99, 0x99,  // ##..## -> 10011001
    0x89, 0x89,  // ###.## -> 10001001
    0x81, 0x81,  // ###### -> 10000001
    0x91, 0x91,  // ##.### -> 10010001
    0x99, 0x99,  // ##..## -> 10011001
    0x99, 0x99,  // ##..## -> 10011001
    0xFF, 0xFF,

    // CHAR_R (index 11)
    // #####.
    // ##..##
    // ##..##
    // #####.
    // ##.##.
    // ##..##
    0xFF, 0xFF,
    0x83, 0x83,  // #####. -> 10000011
    0x99, 0x99,  // ##..## -> 10011001
    0x99, 0x99,  // ##..## -> 10011001
    0x83, 0x83,  // #####. -> 10000011
    0x93, 0x93,  // ##.##. -> 10010011
    0x99, 0x99,  // ##..## -> 10011001
    0xFF, 0xFF,

    // CHAR_S (index 12)
    // .####.
    // ##....
    // .####.
    // ....##
    // ....##
    // #####.
    0xFF, 0xFF,
    0xC3, 0xC3,  // .####. -> 11000011
    0x9F, 0x9F,  // ##.... -> 10011111
    0xC3, 0xC3,  // .####. -> 11000011
    0xF9, 0xF9,  // ....## -> 11111001
    0xF9, 0xF9,  // ....## -> 11111001
    0x83, 0x83,  // #####. -> 10000011
    0xFF, 0xFF,

    // CHAR_T (index 13)
    // ######
    // ..##..
    // ..##..
    // ..##..
    // ..##..
    // ..##..
    0xFF, 0xFF,
    0x81, 0x81,  // ###### -> 10000001
    0xE7, 0xE7,  // ..##.. -> 11100111
    0xE7, 0xE7,  // ..##.. -> 11100111
    0xE7, 0xE7,  // ..##.. -> 11100111
    0xE7, 0xE7,  // ..##.. -> 11100111
    0xE7, 0xE7,  // ..##.. -> 11100111
    0xFF, 0xFF,

    // CHAR_BLANK (index 14) - solid black for background clearing
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
};

/**
 * Load font tiles into VRAM starting at FONT_TILE_START
 */
void load_font(void) {
    set_bkg_data(FONT_TILE_START, FONT_CHAR_COUNT, font_tiles);
}

/**
 * Convert ASCII character to font tile index
 */
static uint8_t char_to_tile(char c) {
    switch (c) {
        case ' ': return FONT_TILE_START + CHAR_SPACE;
        case 'A': return FONT_TILE_START + CHAR_A;
        case 'B': return FONT_TILE_START + CHAR_B;
        case 'C': return FONT_TILE_START + CHAR_C;
        case 'E': return FONT_TILE_START + CHAR_E;
        case 'G': return FONT_TILE_START + CHAR_G;
        case 'I': return FONT_TILE_START + CHAR_I;
        case 'K': return FONT_TILE_START + CHAR_K;
        case 'L': return FONT_TILE_START + CHAR_L;
        case 'M': return FONT_TILE_START + CHAR_M;
        case 'N': return FONT_TILE_START + CHAR_N;
        case 'R': return FONT_TILE_START + CHAR_R;
        case 'S': return FONT_TILE_START + CHAR_S;
        case 'T': return FONT_TILE_START + CHAR_T;
        default:  return FONT_TILE_START + CHAR_SPACE;  // Unknown chars become space
    }
}

/**
 * Draw a string at the specified tile position
 */
void draw_text(uint8_t x, uint8_t y, const char *str) {
    uint8_t tile_buf[20];  // Max 20 tiles per row
    uint8_t i = 0;

    while (str[i] != '\0' && i < 20) {
        tile_buf[i] = char_to_tile(str[i]);
        i++;
    }

    if (i > 0) {
        set_bkg_tiles(x, y, i, 1, tile_buf);
    }
}

/**
 * Clear a row of tiles with black background
 */
void clear_text_row(uint8_t x, uint8_t y, uint8_t width) {
    uint8_t tile_buf[20];
    uint8_t blank_tile = (uint8_t)(FONT_TILE_START + CHAR_BLANK);

    for (uint8_t i = 0; i < width && i < 20; i++) {
        tile_buf[i] = blank_tile;
    }

    set_bkg_tiles(x, y, width, 1, tile_buf);
}
