/**
 * ai.h
 * AI opponent logic for THE MERCHANT (greedy strategy)
 * Ported from Python greedy_agent.py
 */

#ifndef AI_H
#define AI_H

#include <stdint.h>

// ============================================================================
// Evaluation Weights for THE MERCHANT (greedy strategy)
// ============================================================================
#define WEIGHT_ADVANCEMENT      10   // Points per square advanced
#define WEIGHT_SCORED          150   // Bonus per finished piece
#define WEIGHT_CENTER_ROSETTE   80   // Extra bonus for position 8 (war zone rosette)
#define WEIGHT_ROSETTE_SAFETY   20   // Bonus for any rosette (4, 8, 14)
#define WEIGHT_VULNERABILITY   -30   // Penalty for capturable pieces
#define WEIGHT_CAPTURE_THREAT   25   // Bonus for threatening opponent

// ============================================================================
// Difficulty Thresholds (% chance to pick optimal move)
// ============================================================================
#define AI_CHANCE_EASY    40   // 40% chance to use AI, 60% random
#define AI_CHANCE_MEDIUM  65   // 65% chance to use AI, 35% random
#define AI_CHANCE_HARD   100   // Always use AI

// ============================================================================
// Board Zone Constants
// ============================================================================
#define WAR_ZONE_START 5
#define WAR_ZONE_END 12

// ============================================================================
// Function Prototypes
// ============================================================================

/**
 * Select the best move for the CPU player
 *
 * Uses greedy evaluation based on difficulty setting:
 * - EASY: 40% chance to use AI, otherwise random
 * - MEDIUM: 65% chance to use AI, otherwise random
 * - HARD: Always uses AI evaluation
 *
 * @param roll           Dice roll result (1-4)
 * @param out_piece_idx  Pointer to store selected piece index
 * @return 1 if a valid move was found, 0 if no valid moves
 */
uint8_t ai_select_move(uint8_t roll, uint8_t *out_piece_idx);

#endif // AI_H
