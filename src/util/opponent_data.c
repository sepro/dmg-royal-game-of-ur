/**
 * opponent_data.c
 * Implementation of shared opponent profile data
 */

#include "util/opponent_data.h"

const uint8_t profile_tile_counts[OPPONENT_COUNT] = {25, 21, 20, 20};

const char *opponent_names[OPPONENT_COUNT] = {
    "THE SCHOLAR",
    "THE MERCHANT",
    "THE MUSICIAN",
    "THE PRIESTESS"
};

const char *opponent_descs[OPPONENT_COUNT] = {
    "WISE AND PATIENT",
    "CUNNING TRADER",
    "MELODIC SOUL",
    "DIVINE GUIDANCE"
};
