/**
 * cgb.c
 * Game Boy Color palette installation and BG-attribute / OBP0 helpers.
 * See cgb.h for the design rationale.
 */

#include <gb/gb.h>
#include <gb/cgb.h>
#include <stdint.h>
#include "util/cgb.h"

// Grayscale ramp matching the four DMG shades. Reused for BG palette 0, the
// "default" sprite palette, and the OBP0 sync helper below.
static const palette_color_t grayscale[4] = {
    RGB_WHITE, RGB_LIGHTGRAY, RGB_DARKGRAY, RGB_BLACK
};

static const palette_color_t portrait_palette[4] = {
    RGB_WHITE,
    RGB8(248, 224, 184),  // light beige (skin highlight)
    RGB8( 96,  56,  16),  // dark brown (outline / shadow)
    RGB_BLACK
};

static const palette_color_t rosette_palette[4] = {
    RGB_WHITE,
    RGB8(168, 208, 248),  // light blue
    RGB8( 40,  80, 184),  // deep blue
    RGB_BLACK
};

static const palette_color_t board_sand_palette[4] = {
    RGB_WHITE,
    RGB8(224, 184, 120),  // light sand
    RGB8(160, 112,  56),  // dark sand
    RGB_BLACK
};

static const palette_color_t coin_light_palette[4] = {
    RGB_WHITE,
    RGB8(248, 232, 144),  // warm yellow
    RGB8(216, 128,  56),  // orange
    RGB8( 96,  48,  16)   // dark brown (replaces black for warmer coin edge)
};

static const palette_color_t coin_dark_palette[4] = {
    RGB_WHITE,
    RGB8(176, 208, 232),  // light blue
    RGB8( 88, 128, 200),  // mid blue
    RGB8(  8,  24,  80)   // deep navy (replaces black for cooler coin edge)
};

void cgb_init_palettes(void) {
    if (_cpu != CGB_TYPE) return;

    set_bkg_palette(CGB_PAL_GRAYSCALE,  1, grayscale);
    set_bkg_palette(CGB_PAL_PORTRAIT,   1, portrait_palette);
    set_bkg_palette(CGB_PAL_ROSETTE,    1, rosette_palette);
    set_bkg_palette(CGB_PAL_BOARD_SAND, 1, board_sand_palette);
    set_bkg_palette(CGB_PAL_COIN_LIGHT, 1, coin_light_palette);
    set_bkg_palette(CGB_PAL_COIN_DARK,  1, coin_dark_palette);

    // All sprites use palette 0; it gets rewritten by cgb_set_obp0() to
    // track the DMG OBP0_REG remap. Seed it with plain grayscale so anything
    // drawn before the first cgb_set_obp0() call still looks right.
    set_sprite_palette(0, 1, grayscale);

    cgb_clear_bg_attributes();
}

void cgb_clear_bg_attributes(void) {
    if (_cpu != CGB_TYPE) return;
    VBK_REG = 1;
    fill_bkg_rect(0, 0, 32, 32, CGB_PAL_GRAYSCALE);
    VBK_REG = 0;
}

void cgb_set_bg_attr_rect(uint8_t x, uint8_t y,
                          uint8_t w, uint8_t h, uint8_t attr) {
    if (_cpu != CGB_TYPE) return;
    VBK_REG = 1;
    fill_bkg_rect(x, y, w, h, attr);
    VBK_REG = 0;
}

void cgb_set_bg_attrs_row(uint8_t x, uint8_t y,
                          uint8_t w, const uint8_t *attrs) {
    if (_cpu != CGB_TYPE) return;
    VBK_REG = 1;
    set_bkg_tiles(x, y, w, 1, attrs);
    VBK_REG = 0;
}

void cgb_set_obp0(uint8_t value) {
    OBP0_REG = value;
    if (_cpu != CGB_TYPE) return;

    // OBP0_REG encodes a remap of pixel shades 0-3 to output shades 0-3,
    // two bits per pixel value. Translate to a CGB sprite palette so the
    // visible result matches DMG.
    palette_color_t pal[4];
    pal[0] = RGB_WHITE;                          // sprite shade 0 is always transparent
    pal[1] = grayscale[(value >> 2) & 0x3];
    pal[2] = grayscale[(value >> 4) & 0x3];
    pal[3] = grayscale[(value >> 6) & 0x3];
    set_sprite_palette(0, 1, pal);
}
