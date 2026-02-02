/**
 * random.h - Shared random number generator
 *
 * Galois LFSR implementation for pseudo-random number generation.
 * Used across all game screens for consistent random quality.
 */

#ifndef RANDOM_H
#define RANDOM_H

#include <gb/gb.h>

/**
 * Seed the random number generator with DIV register and frame counter
 */
void seed_random(uint16_t seed_value);

/**
 * Get a pseudo-random byte using Galois LFSR
 */
uint8_t get_random(void);

/**
 * Get a random value in range [min, max]
 */
uint8_t get_random_range(uint8_t min, uint8_t max);

#endif // RANDOM_H
