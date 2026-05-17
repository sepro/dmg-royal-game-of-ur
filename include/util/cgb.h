/**
 * cgb.h
 * Game Boy Color support helpers.
 *
 * The ROM is compiled with -Wm-yc (CGB-enhanced, DMG-compatible). On real CGB
 * hardware running this ROM, the BGP_REG and OBP*_REG palette registers are
 * ignored entirely; pixel shades index directly into the CGB palette selected
 * by each BG tile's attribute byte (BG) or each sprite's attribute byte
 * (sprites). To keep the existing DMG drawing code working unchanged on CGB,
 * we install a grayscale palette in slot 0 of both BG and sprite palette RAM
 * and add a few small helpers that:
 *
 *   - install named accent palettes (portraits, rosettes, board, coins)
 *   - paint BG attribute bytes so specific tiles use those accent palettes
 *   - keep CGB sprite palette 0 in sync with the OBP0_REG remap (so existing
 *     `OBP0_REG = 0xE0` style writes still produce the same shades on CGB)
 *
 * Every function in this header is a no-op on DMG, so callers do not need to
 * gate them on `_cpu == CGB_TYPE` themselves.
 */

#ifndef CGB_H
#define CGB_H

#include <stdint.h>

/**
 * BG palette slot assignments. Indices match the order in which
 * cgb_init_palettes() installs the palettes; this enum is the single source
 * of truth for both producers (cgb.c) and consumers (board, coinflip,
 * portrait, etc.).
 *
 * Palette 0 is a plain grayscale ramp matching the DMG shades, so any tile
 * with attribute byte 0 (the default after cgb_clear_bg_attributes) renders
 * exactly as it does on DMG.
 */
typedef enum {
    CGB_PAL_GRAYSCALE  = 0,  // white / light gray / dark gray / black
    CGB_PAL_PORTRAIT   = 1,  // white / beige / brown / black
    CGB_PAL_ROSETTE    = 2,  // white / light blue / deep blue / black
    CGB_PAL_BOARD_SAND = 3,  // white / light sand / dark sand / black
    CGB_PAL_COIN_LIGHT = 4,  // warm yellow / orange / dark brown
    CGB_PAL_COIN_DARK  = 5   // light blue / mid blue / deep navy
} CgbBgPalette_t;

/**
 * Install all background and sprite palettes used by the game, and zero the
 * BG attribute plane. Call once at boot, before any screen draws.
 * No-op on DMG.
 */
void cgb_init_palettes(void);

/**
 * Zero the entire 32x32 BG tile-attribute plane so every tile uses palette 0
 * (grayscale) with no flip and no priority. Call once per state transition,
 * before the next screen draws its tiles, so leftover palette assignments
 * from the previous screen don't bleed through.
 *
 * Caller must ensure the display is off, otherwise the 1024 attribute writes
 * race the LCD scan-out and may glitch the current frame.
 * No-op on DMG.
 */
void cgb_clear_bg_attributes(void);

/**
 * Fill a rectangular region of the BG attribute plane with a single palette.
 * Use this when every tile in the region should share the same palette.
 * No-op on DMG.
 */
void cgb_set_bg_attr_rect(uint8_t x, uint8_t y,
                          uint8_t w, uint8_t h, uint8_t attr);

/**
 * Write a row of per-tile attribute bytes to the BG attribute plane.
 * Use this when adjacent tiles need different palettes (e.g. the coin
 * shuffle animation where each tile is independently light or dark).
 * No-op on DMG.
 */
void cgb_set_bg_attrs_row(uint8_t x, uint8_t y,
                          uint8_t w, const uint8_t *attrs);

/**
 * Write OBP0_REG and keep CGB sprite palette 0 in sync. This wraps the
 * common pattern so callers cannot forget the sync step.
 *
 * On DMG the OBP0_REG byte remaps tile pixel shades 0-3 to output shades 0-3
 * directly. On CGB the register is ignored and shade N indexes CGB sprite
 * palette[N], so we translate the OBP0_REG byte into the equivalent CGB
 * palette and install it.
 */
void cgb_set_obp0(uint8_t value);

#endif // CGB_H
