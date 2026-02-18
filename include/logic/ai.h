/**
 * ai.h
 * AI opponent logic for multiple opponent types
 * - THE MERCHANT: Greedy strategy
 * - THE MUSICIAN: Turn economy strategy
 * - THE PRIESTESS: Phase-based strategy
 * - THE SCHOLAR: Adaptive strategy
 */

#ifndef AI_H
#define AI_H

#include <stdint.h>

// ============================================================================
// Opponent Type Constants (matches opponent selection order)
// ============================================================================
#define OPPONENT_SCHOLAR    0   // Adaptive strategy
#define OPPONENT_MERCHANT   1   // Greedy AI
#define OPPONENT_MUSICIAN   2   // Turn economy AI
#define OPPONENT_PRIESTESS  3   // Phase-based AI

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
// Evaluation Weights for THE MUSICIAN (turn economy strategy)
// ============================================================================
#define TE_EXTRA_TURN_BONUS     140  // Bonus for landing on rosette (extra turn)
#define TE_EFFICIENCY_WEIGHT     70  // Weight for turns-to-win minimization
#define TE_OPPONENT_DELAY        30  // Bonus for moves that slow opponent
#define TE_SCORE_BONUS          140  // Bonus for scoring a piece (2 * efficiency)

// Turn estimation constants (scaled by 10 to avoid floats)
#define TURNS_PER_RESERVE        75  // 7.5 turns to enter + traverse on average
#define TURNS_PER_DISTANCE        6  // 0.6 turns per remaining square

// ============================================================================
// Game Phase Constants for THE PRIESTESS (phase-based strategy)
// ============================================================================
#define PHASE_OPENING   0
#define PHASE_MIDGAME   1
#define PHASE_ENDGAME   2

#define PHASE_OPENING_MAX   2   // 0-2 scored = opening
#define PHASE_MIDGAME_MAX   5   // 3-5 scored = midgame, 6+ = endgame

// OPENING: Safe positioning, get pieces moving
#define PH_OPENING_ADVANCEMENT      12
#define PH_OPENING_SCORED          150
#define PH_OPENING_CENTER_ROSETTE   90
#define PH_OPENING_ROSETTE_SAFETY   35
#define PH_OPENING_VULNERABILITY   -20
#define PH_OPENING_CAPTURE_THREAT   15

// MIDGAME: Aggressive war zone control
#define PH_MIDGAME_ADVANCEMENT      10
#define PH_MIDGAME_SCORED          150
#define PH_MIDGAME_CENTER_ROSETTE  100
#define PH_MIDGAME_ROSETTE_SAFETY   25
#define PH_MIDGAME_VULNERABILITY   -35
#define PH_MIDGAME_CAPTURE_THREAT   40

// ENDGAME: Race to finish
#define PH_ENDGAME_ADVANCEMENT      18
#define PH_ENDGAME_SCORED          180
#define PH_ENDGAME_CENTER_ROSETTE   50
#define PH_ENDGAME_ROSETTE_SAFETY   15
#define PH_ENDGAME_VULNERABILITY   -25
#define PH_ENDGAME_CAPTURE_THREAT   20

// ============================================================================
// Evaluation Weights for THE SCHOLAR (adaptive strategy)
// Switches between profiles based on whether CPU is ahead or behind
// ============================================================================
#define ADAPT_THRESHOLD         50   // Score difference to trigger mode switch

// Adaptation mode constants
#define ADAPT_MODE_BALANCED    0
#define ADAPT_MODE_DEFENSIVE   1
#define ADAPT_MODE_AGGRESSIVE  2

// BALANCED weights (used when game is close, within threshold)
#define AD_BALANCED_ADVANCEMENT      12
#define AD_BALANCED_SCORED          140
#define AD_BALANCED_CENTER_ROSETTE   70
#define AD_BALANCED_ROSETTE_SAFETY   25
#define AD_BALANCED_VULNERABILITY   -35
#define AD_BALANCED_CAPTURE_THREAT   30

// DEFENSIVE weights (used when CPU is ahead - protect the lead)
#define AD_DEFENSIVE_ADVANCEMENT      8
#define AD_DEFENSIVE_SCORED         150
#define AD_DEFENSIVE_CENTER_ROSETTE 100
#define AD_DEFENSIVE_ROSETTE_SAFETY  35
#define AD_DEFENSIVE_VULNERABILITY  -50
#define AD_DEFENSIVE_CAPTURE_THREAT  20

// AGGRESSIVE weights (used when CPU is behind - catch up)
#define AD_AGGRESSIVE_ADVANCEMENT    15
#define AD_AGGRESSIVE_SCORED        150
#define AD_AGGRESSIVE_CENTER_ROSETTE 60
#define AD_AGGRESSIVE_ROSETTE_SAFETY 15
#define AD_AGGRESSIVE_VULNERABILITY -20
#define AD_AGGRESSIVE_CAPTURE_THREAT 40

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
 * Uses opponent-specific evaluation based on difficulty setting:
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
