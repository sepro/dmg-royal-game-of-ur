/**
 * font.c
 * Simple 8x8 font implementation for menu text
 * White text on black background
 * Full uppercase alphabet A-Z plus space and blank
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
 * Inverted: 0xFF = all black, 0x00 = all white
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

    // CHAR_D (index 4)
    // ####..
    // ##.##.
    // ##..##
    // ##..##
    // ##.##.
    // ####..
    0xFF, 0xFF,
    0x87, 0x87,  // ####.. -> 10000111
    0x93, 0x93,  // ##.##. -> 10010011
    0x99, 0x99,  // ##..## -> 10011001
    0x99, 0x99,  // ##..## -> 10011001
    0x93, 0x93,  // ##.##. -> 10010011
    0x87, 0x87,  // ####.. -> 10000111
    0xFF, 0xFF,

    // CHAR_E (index 5)
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

    // CHAR_F (index 6)
    // ######
    // ##....
    // #####.
    // ##....
    // ##....
    // ##....
    0xFF, 0xFF,
    0x81, 0x81,  // ###### -> 10000001
    0x9F, 0x9F,  // ##.... -> 10011111
    0x87, 0x87,  // #####. -> 10000111
    0x9F, 0x9F,  // ##.... -> 10011111
    0x9F, 0x9F,  // ##.... -> 10011111
    0x9F, 0x9F,  // ##.... -> 10011111
    0xFF, 0xFF,

    // CHAR_G (index 7)
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

    // CHAR_H (index 8)
    // ##..##
    // ##..##
    // ######
    // ##..##
    // ##..##
    // ##..##
    0xFF, 0xFF,
    0x99, 0x99,  // ##..## -> 10011001
    0x99, 0x99,  // ##..## -> 10011001
    0x81, 0x81,  // ###### -> 10000001
    0x99, 0x99,  // ##..## -> 10011001
    0x99, 0x99,  // ##..## -> 10011001
    0x99, 0x99,  // ##..## -> 10011001
    0xFF, 0xFF,

    // CHAR_I (index 9)
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

    // CHAR_J (index 10)
    // ....##
    // ....##
    // ....##
    // ....##
    // ##..##
    // .####.
    0xFF, 0xFF,
    0xF9, 0xF9,  // ....## -> 11111001
    0xF9, 0xF9,  // ....## -> 11111001
    0xF9, 0xF9,  // ....## -> 11111001
    0xF9, 0xF9,  // ....## -> 11111001
    0x99, 0x99,  // ##..## -> 10011001
    0xC3, 0xC3,  // .####. -> 11000011
    0xFF, 0xFF,

    // CHAR_K (index 11)
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

    // CHAR_L (index 12)
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

    // CHAR_M (index 13)
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

    // CHAR_N (index 14)
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

    // CHAR_O (index 15)
    // .####.
    // ##..##
    // ##..##
    // ##..##
    // ##..##
    // .####.
    0xFF, 0xFF,
    0xC3, 0xC3,  // .####. -> 11000011
    0x99, 0x99,  // ##..## -> 10011001
    0x99, 0x99,  // ##..## -> 10011001
    0x99, 0x99,  // ##..## -> 10011001
    0x99, 0x99,  // ##..## -> 10011001
    0xC3, 0xC3,  // .####. -> 11000011
    0xFF, 0xFF,

    // CHAR_P (index 16)
    // #####.
    // ##..##
    // ##..##
    // #####.
    // ##....
    // ##....
    0xFF, 0xFF,
    0x83, 0x83,  // #####. -> 10000011
    0x99, 0x99,  // ##..## -> 10011001
    0x99, 0x99,  // ##..## -> 10011001
    0x83, 0x83,  // #####. -> 10000011
    0x9F, 0x9F,  // ##.... -> 10011111
    0x9F, 0x9F,  // ##.... -> 10011111
    0xFF, 0xFF,

    // CHAR_Q (index 17)
    // .####.
    // ##..##
    // ##..##
    // ##..##
    // ##.##.
    // .###.#
    0xFF, 0xFF,
    0xC3, 0xC3,  // .####. -> 11000011
    0x99, 0x99,  // ##..## -> 10011001
    0x99, 0x99,  // ##..## -> 10011001
    0x99, 0x99,  // ##..## -> 10011001
    0x93, 0x93,  // ##.##. -> 10010011
    0xC5, 0xC5,  // .###.# -> 11000101
    0xFF, 0xFF,

    // CHAR_R (index 18)
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

    // CHAR_S (index 19)
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

    // CHAR_T (index 20)
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

    // CHAR_U (index 21)
    // ##..##
    // ##..##
    // ##..##
    // ##..##
    // ##..##
    // .####.
    0xFF, 0xFF,
    0x99, 0x99,  // ##..## -> 10011001
    0x99, 0x99,  // ##..## -> 10011001
    0x99, 0x99,  // ##..## -> 10011001
    0x99, 0x99,  // ##..## -> 10011001
    0x99, 0x99,  // ##..## -> 10011001
    0xC3, 0xC3,  // .####. -> 11000011
    0xFF, 0xFF,

    // CHAR_V (index 22)
    // ##..##
    // ##..##
    // ##..##
    // ##..##
    // .####.
    // ..##..
    0xFF, 0xFF,
    0x99, 0x99,  // ##..## -> 10011001
    0x99, 0x99,  // ##..## -> 10011001
    0x99, 0x99,  // ##..## -> 10011001
    0x99, 0x99,  // ##..## -> 10011001
    0xC3, 0xC3,  // .####. -> 11000011
    0xE7, 0xE7,  // ..##.. -> 11100111
    0xFF, 0xFF,

    // CHAR_W (index 23)
    // ##..##
    // ##..##
    // ##.###
    // ######
    // ######
    // ##..##
    0xFF, 0xFF,
    0x99, 0x99,  // ##..## -> 10011001
    0x99, 0x99,  // ##..## -> 10011001
    0x89, 0x89,  // ##.### -> 10001001
    0x81, 0x81,  // ###### -> 10000001
    0x81, 0x81,  // ###### -> 10000001
    0x99, 0x99,  // ##..## -> 10011001
    0xFF, 0xFF,

    // CHAR_X (index 24)
    // ##..##
    // ##..##
    // .####.
    // .####.
    // ##..##
    // ##..##
    0xFF, 0xFF,
    0x99, 0x99,  // ##..## -> 10011001
    0x99, 0x99,  // ##..## -> 10011001
    0xC3, 0xC3,  // .####. -> 11000011
    0xC3, 0xC3,  // .####. -> 11000011
    0x99, 0x99,  // ##..## -> 10011001
    0x99, 0x99,  // ##..## -> 10011001
    0xFF, 0xFF,

    // CHAR_Y (index 25)
    // ##..##
    // ##..##
    // .####.
    // ..##..
    // ..##..
    // ..##..
    0xFF, 0xFF,
    0x99, 0x99,  // ##..## -> 10011001
    0x99, 0x99,  // ##..## -> 10011001
    0xC3, 0xC3,  // .####. -> 11000011
    0xE7, 0xE7,  // ..##.. -> 11100111
    0xE7, 0xE7,  // ..##.. -> 11100111
    0xE7, 0xE7,  // ..##.. -> 11100111
    0xFF, 0xFF,

    // CHAR_Z (index 26)
    // ######
    // ....##
    // ...##.
    // ..##..
    // .##...
    // ######
    0xFF, 0xFF,
    0x81, 0x81,  // ###### -> 10000001
    0xF9, 0xF9,  // ....## -> 11111001
    0xF3, 0xF3,  // ...##. -> 11110011
    0xE7, 0xE7,  // ..##.. -> 11100111
    0xCF, 0xCF,  // .##... -> 11001111
    0x81, 0x81,  // ###### -> 10000001
    0xFF, 0xFF,

    // CHAR_BLANK (index 27) - solid black for background clearing
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,

    // CHAR_WHITE (index 28) - solid white for inverted text background
    // (Note: only used in inverted font, but included for index consistency)
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,

    // CHAR_0 (index 29)
    // .####.
    // ##..##
    // ##..##
    // ##..##
    // ##..##
    // .####.
    0xFF, 0xFF,
    0xC3, 0xC3,  // .####. -> 11000011
    0x99, 0x99,  // ##..## -> 10011001
    0x99, 0x99,  // ##..## -> 10011001
    0x99, 0x99,  // ##..## -> 10011001
    0x99, 0x99,  // ##..## -> 10011001
    0xC3, 0xC3,  // .####. -> 11000011
    0xFF, 0xFF,

    // CHAR_1 (index 30)
    // ..##..
    // .###..
    // ..##..
    // ..##..
    // ..##..
    // ######
    0xFF, 0xFF,
    0xE7, 0xE7,  // ..##.. -> 11100111
    0xC7, 0xC7,  // .###.. -> 11000111
    0xE7, 0xE7,  // ..##.. -> 11100111
    0xE7, 0xE7,  // ..##.. -> 11100111
    0xE7, 0xE7,  // ..##.. -> 11100111
    0x81, 0x81,  // ###### -> 10000001
    0xFF, 0xFF,

    // CHAR_2 (index 31)
    // .####.
    // ##..##
    // ....##
    // .####.
    // ##....
    // ######
    0xFF, 0xFF,
    0xC3, 0xC3,  // .####. -> 11000011
    0x99, 0x99,  // ##..## -> 10011001
    0xF9, 0xF9,  // ....## -> 11111001
    0xC3, 0xC3,  // .####. -> 11000011
    0x9F, 0x9F,  // ##.... -> 10011111
    0x81, 0x81,  // ###### -> 10000001
    0xFF, 0xFF,

    // CHAR_3 (index 32)
    // .####.
    // ##..##
    // ...##.
    // ...##.
    // ##..##
    // .####.
    0xFF, 0xFF,
    0xC3, 0xC3,  // .####. -> 11000011
    0x99, 0x99,  // ##..## -> 10011001
    0xF3, 0xF3,  // ...##. -> 11110011
    0xF3, 0xF3,  // ...##. -> 11110011
    0x99, 0x99,  // ##..## -> 10011001
    0xC3, 0xC3,  // .####. -> 11000011
    0xFF, 0xFF,

    // CHAR_4 (index 33)
    // ##..##
    // ##..##
    // ######
    // ....##
    // ....##
    // ....##
    0xFF, 0xFF,
    0x99, 0x99,  // ##..## -> 10011001
    0x99, 0x99,  // ##..## -> 10011001
    0x81, 0x81,  // ###### -> 10000001
    0xF9, 0xF9,  // ....## -> 11111001
    0xF9, 0xF9,  // ....## -> 11111001
    0xF9, 0xF9,  // ....## -> 11111001
    0xFF, 0xFF,

    // CHAR_5 (index 34)
    // ######
    // ##....
    // #####.
    // ....##
    // ##..##
    // .####.
    0xFF, 0xFF,
    0x81, 0x81,  // ###### -> 10000001
    0x9F, 0x9F,  // ##.... -> 10011111
    0x83, 0x83,  // #####. -> 10000011
    0xF9, 0xF9,  // ....## -> 11111001
    0x99, 0x99,  // ##..## -> 10011001
    0xC3, 0xC3,  // .####. -> 11000011
    0xFF, 0xFF,

    // CHAR_6 (index 35)
    // .####.
    // ##....
    // #####.
    // ##..##
    // ##..##
    // .####.
    0xFF, 0xFF,
    0xC3, 0xC3,  // .####. -> 11000011
    0x9F, 0x9F,  // ##.... -> 10011111
    0x83, 0x83,  // #####. -> 10000011
    0x99, 0x99,  // ##..## -> 10011001
    0x99, 0x99,  // ##..## -> 10011001
    0xC3, 0xC3,  // .####. -> 11000011
    0xFF, 0xFF,

    // CHAR_7 (index 36)
    // ######
    // ....##
    // ...##.
    // ..##..
    // ..##..
    // ..##..
    0xFF, 0xFF,
    0x81, 0x81,  // ###### -> 10000001
    0xF9, 0xF9,  // ....## -> 11111001
    0xF3, 0xF3,  // ...##. -> 11110011
    0xE7, 0xE7,  // ..##.. -> 11100111
    0xE7, 0xE7,  // ..##.. -> 11100111
    0xE7, 0xE7,  // ..##.. -> 11100111
    0xFF, 0xFF,

    // CHAR_8 (index 37)
    // .####.
    // ##..##
    // .####.
    // ##..##
    // ##..##
    // .####.
    0xFF, 0xFF,
    0xC3, 0xC3,  // .####. -> 11000011
    0x99, 0x99,  // ##..## -> 10011001
    0xC3, 0xC3,  // .####. -> 11000011
    0x99, 0x99,  // ##..## -> 10011001
    0x99, 0x99,  // ##..## -> 10011001
    0xC3, 0xC3,  // .####. -> 11000011
    0xFF, 0xFF,

    // CHAR_9 (index 38)
    // .####.
    // ##..##
    // ##..##
    // .#####
    // ....##
    // .####.
    0xFF, 0xFF,
    0xC3, 0xC3,  // .####. -> 11000011
    0x99, 0x99,  // ##..## -> 10011001
    0x99, 0x99,  // ##..## -> 10011001
    0xC1, 0xC1,  // .##### -> 11000001
    0xF9, 0xF9,  // ....## -> 11111001
    0xC3, 0xC3,  // .####. -> 11000011
    0xFF, 0xFF,

    // CHAR_COLON (index 39)
    // ......
    // ..##..
    // ..##..
    // ......
    // ..##..
    // ..##..
    0xFF, 0xFF,
    0xFF, 0xFF,  // ...... -> 11111111
    0xE7, 0xE7,  // ..##.. -> 11100111
    0xE7, 0xE7,  // ..##.. -> 11100111
    0xFF, 0xFF,  // ...... -> 11111111
    0xE7, 0xE7,  // ..##.. -> 11100111
    0xE7, 0xE7,  // ..##.. -> 11100111
    0xFF, 0xFF,
};

/**
 * Load font tiles into VRAM starting at FONT_TILE_START
 */
void load_font(void) {
    set_bkg_data(FONT_TILE_START, FONT_CHAR_COUNT, font_tiles);
}

/**
 * Convert ASCII character to font tile index
 * @param c Character to convert (A-Z, 0-9, : supported, others become space)
 * @param base Base tile index (FONT_TILE_START or FONT_INVERTED_TILE_START)
 * @return Tile index for the character
 */
static uint8_t char_to_tile(char c, uint8_t base) {
    if (c >= 'A' && c <= 'Z') {
        return base + CHAR_A + (c - 'A');  // base + 1 + (c - 'A')
    }
    if (c >= '0' && c <= '9') {
        return base + CHAR_0 + (c - '0');  // base + 29 + (c - '0')
    }
    if (c == ':') {
        return base + CHAR_COLON;  // base + 39
    }
    return base + CHAR_SPACE;  // Space (index 0) for unknown chars
}

/**
 * Draw a string at the specified tile position
 */
void draw_text(uint8_t x, uint8_t y, const char *str) {
    uint8_t tile_buf[20];  // Max 20 tiles per row
    uint8_t i = 0;

    while (str[i] != '\0' && i < 20) {
        tile_buf[i] = char_to_tile(str[i], FONT_TILE_START);
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

/**
 * Load inverted font tiles into VRAM starting at FONT_INVERTED_TILE_START
 * Generates inverted tiles at runtime by XOR'ing normal font with 0xFF
 * This saves ROM compared to storing pre-inverted data
 */
void load_font_inverted(void) {
    uint8_t inverted_buffer[16];  // Buffer for one tile (16 bytes)

    // Generate and load all inverted character tiles (40 total)
    // This includes letters, space, blank, white, numbers, and colon
    for (uint8_t tile_idx = 0; tile_idx < FONT_CHAR_COUNT; tile_idx++) {
        // Calculate source offset in font_tiles array
        const uint8_t* source = font_tiles + (tile_idx * 16);

        // Invert each byte of the tile (XOR with 0xFF)
        for (uint8_t byte_idx = 0; byte_idx < 16; byte_idx++) {
            inverted_buffer[byte_idx] = source[byte_idx] ^ 0xFF;
        }

        // Load single inverted tile into VRAM
        set_bkg_data(FONT_INVERTED_TILE_START + tile_idx, 1, inverted_buffer);
    }

    // Special case: CHAR_WHITE tile should be all white (0x00), not inverted
    // The source CHAR_WHITE is 0x00, which inverts to 0xFF (black) - wrong!
    // Override with explicit white tile for clear_text_row_inverted()
    for (uint8_t byte_idx = 0; byte_idx < 16; byte_idx++) {
        inverted_buffer[byte_idx] = 0x00;
    }
    set_bkg_data((uint8_t)(FONT_INVERTED_TILE_START + CHAR_WHITE), 1, inverted_buffer);
}

/**
 * Draw a string using inverted font (black text on white background)
 */
void draw_text_inverted(uint8_t x, uint8_t y, const char *str) {
    uint8_t tile_buf[20];  // Max 20 tiles per row
    uint8_t i = 0;

    while (str[i] != '\0' && i < 20) {
        tile_buf[i] = char_to_tile(str[i], FONT_INVERTED_TILE_START);
        i++;
    }

    if (i > 0) {
        set_bkg_tiles(x, y, i, 1, tile_buf);
    }
}

/**
 * Clear a row of tiles with white background (for inverted text screens)
 */
void clear_text_row_inverted(uint8_t x, uint8_t y, uint8_t width) {
    uint8_t tile_buf[20];
    uint8_t white_tile = (uint8_t)(FONT_INVERTED_TILE_START + CHAR_WHITE);

    for (uint8_t i = 0; i < width && i < 20; i++) {
        tile_buf[i] = white_tile;
    }

    set_bkg_tiles(x, y, width, 1, tile_buf);
}

// External reference to border tiles (from border.c)
extern const uint8_t border_tiles[];

// Number of border tiles (from border.h)
#define BORDER_TILE_COUNT_IMG 25

/**
 * Load border tiles inverted (XOR with 0xFF) into VRAM
 * Used for dark background screens where border needs light-on-dark appearance
 */
void load_border_inverted(uint8_t vram_start) {
    uint8_t inverted_buffer[16];

    for (uint8_t tile_idx = 0; tile_idx < BORDER_TILE_COUNT_IMG; tile_idx++) {
        const uint8_t* source = border_tiles + (tile_idx * 16);

        // Invert each byte of the tile (XOR with 0xFF)
        for (uint8_t byte_idx = 0; byte_idx < 16; byte_idx++) {
            inverted_buffer[byte_idx] = source[byte_idx] ^ 0xFF;
        }

        // Load single inverted tile into VRAM
        set_bkg_data(vram_start + tile_idx, 1, inverted_buffer);
    }
}
