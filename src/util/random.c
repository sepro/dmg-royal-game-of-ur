/**
 * random.c - Shared random number generator
 *
 * Implements a Galois Linear Feedback Shift Register (LFSR) for
 * pseudo-random number generation. This module consolidates random
 * number generation that was previously duplicated across multiple
 * game screens.
 */

#include "util/random.h"

// ============================================================================
// Static Variables
// ============================================================================

static uint16_t rand_state = 0xACE1;  // LFSR state (must be non-zero)

// ============================================================================
// Public Functions
// ============================================================================

/**
 * Seed the random number generator
 */
void seed_random(uint16_t seed_value) {
    rand_state = seed_value;
    if (rand_state == 0) rand_state = 0xACE1;  // Avoid zero state
}

/**
 * Get a pseudo-random byte using Galois LFSR
 */
uint8_t get_random(void) {
    uint8_t lsb = rand_state & 1;
    rand_state >>= 1;
    if (lsb) rand_state ^= 0xB400;
    return (uint8_t)(rand_state & 0xFF);
}

/**
 * Get a random value in range [min, max]
 */
uint8_t get_random_range(uint8_t min, uint8_t max) {
    if (min > max) return min;  // Defensive: prevent uint8 underflow
    return min + (get_random() % (max - min + 1));
}

/**
 * Get an unbiased random value in [0, range-1] using rejection sampling
 * Uses a bitmask to minimize rejections (e.g. range=100, mask=127, ~1.27 avg iterations)
 */
uint8_t get_random_unbiased(uint8_t range) {
    if (range == 0) return 0;

    // Find smallest bitmask >= range
    uint8_t mask = range - 1;
    mask |= mask >> 1;
    mask |= mask >> 2;
    mask |= mask >> 4;

    // Rejection sampling: mask then reject if >= range
    uint8_t value;
    do {
        value = get_random() & mask;
    } while (value >= range);

    return value;
}
