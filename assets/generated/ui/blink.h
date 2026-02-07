/**
 * blink.h
 * Sparkle/blink sprite tile data - 4 animation frames
 * Manually created for title screen sparkle effect
 *
 * Frame 0: Single pixel dot
 * Frame 1: 3-pixel vertical line
 * Frame 2: Small cross (5 pixels)
 * Frame 3: Large cross (9 pixels)
 */

#ifndef BLINK_H
#define BLINK_H

#include <stdint.h>

// 4 frames x 16 bytes per tile = 64 bytes
// 2bpp format: 2 bytes per row, 8 rows per tile
// Color 1 = visible (bit in first byte), Color 0 = transparent
const uint8_t blink_tiles[64] = {
    // Frame 0: Single pixel dot at center
    0x00, 0x00,  // row 0
    0x00, 0x00,  // row 1
    0x00, 0x00,  // row 2
    0x10, 0x00,  // row 3: pixel 3 (center)
    0x00, 0x00,  // row 4
    0x00, 0x00,  // row 5
    0x00, 0x00,  // row 6
    0x00, 0x00,  // row 7

    // Frame 1: 3-pixel vertical line
    0x00, 0x00,  // row 0
    0x00, 0x00,  // row 1
    0x10, 0x00,  // row 2: pixel 3
    0x10, 0x00,  // row 3: pixel 3
    0x10, 0x00,  // row 4: pixel 3
    0x00, 0x00,  // row 5
    0x00, 0x00,  // row 6
    0x00, 0x00,  // row 7

    // Frame 2: Small cross (+ shape)
    0x00, 0x00,  // row 0
    0x00, 0x00,  // row 1
    0x10, 0x00,  // row 2: pixel 3
    0x38, 0x00,  // row 3: pixels 2,3,4 (horizontal line)
    0x10, 0x00,  // row 4: pixel 3
    0x00, 0x00,  // row 5
    0x00, 0x00,  // row 6
    0x00, 0x00,  // row 7

    // Frame 3: Larger cross (extended + shape)
    0x00, 0x00,  // row 0
    0x10, 0x00,  // row 1: pixel 3
    0x10, 0x00,  // row 2: pixel 3
    0x7C, 0x00,  // row 3: pixels 1,2,3,4,5 (wide horizontal)
    0x10, 0x00,  // row 4: pixel 3
    0x10, 0x00,  // row 5: pixel 3
    0x00, 0x00,  // row 6
    0x00, 0x00   // row 7
};

#endif // BLINK_H
