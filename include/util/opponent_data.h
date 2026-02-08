/**
 * opponent_data.h
 * Shared opponent profile data (names, descriptions, tile counts)
 */

#ifndef OPPONENT_DATA_H
#define OPPONENT_DATA_H

#include <stdint.h>
#include "screens/opponent_select.h"  // For OPPONENT_COUNT

// Tile counts per profile (calculated from generated asset sizes)
extern const uint8_t profile_tile_counts[OPPONENT_COUNT];

// Sad expression tile counts per profile
extern const uint8_t profile_sad_tile_counts[OPPONENT_COUNT];

// Opponent names (max 18 chars to fit in DESC_TEXT_WIDTH)
extern const char *opponent_names[OPPONENT_COUNT];

// Opponent descriptions (max 18 chars to fit in DESC_TEXT_WIDTH)
extern const char *opponent_descs[OPPONENT_COUNT];

#endif // OPPONENT_DATA_H
