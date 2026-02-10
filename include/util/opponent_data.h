/**
 * opponent_data.h
 * Shared opponent profile data (names, descriptions)
 */

#ifndef OPPONENT_DATA_H
#define OPPONENT_DATA_H

#include <stdint.h>
#include "screens/opponent_select.h"  // For OPPONENT_COUNT

// Opponent names (max 18 chars to fit in DESC_TEXT_WIDTH)
extern const char *opponent_names[OPPONENT_COUNT];

// Opponent descriptions (max 18 chars to fit in DESC_TEXT_WIDTH)
extern const char *opponent_descs[OPPONENT_COUNT];

#endif // OPPONENT_DATA_H
