/**
 * ai.c
 * AI opponent logic for THE MERCHANT (greedy strategy)
 * Ported from Python greedy_agent.py
 *
 * Evaluates board positions and selects the best move based on:
 * - Piece advancement
 * - Rosette control (safety and center rosette bonus)
 * - Capture opportunities
 * - Vulnerability to capture
 */

#include <gb/gb.h>
#include <stdint.h>
#include "ai.h"
#include "board_state.h"
#include "game.h"
#include "difficulty_select.h"
#include "random.h"

// ============================================================================
// External References
// ============================================================================

// Piece position arrays from game.c
extern uint8_t human_pieces[PIECES_PER_PLAYER];
extern uint8_t cpu_pieces[PIECES_PER_PLAYER];

// ============================================================================
// Dice Probability Weights
// ============================================================================

/**
 * Probability weights for dice rolls 0-4
 * With 4 binary dice, probabilities are:
 *   Roll 0: 1/16 (0000)
 *   Roll 1: 4/16 (0001, 0010, 0100, 1000)
 *   Roll 2: 6/16 (0011, 0101, 0110, 1001, 1010, 1100)
 *   Roll 3: 4/16 (0111, 1011, 1101, 1110)
 *   Roll 4: 1/16 (1111)
 */
static const uint8_t dice_weights[5] = {1, 4, 6, 4, 1};

// ============================================================================
// Helper Functions
// ============================================================================

/**
 * Check if a position is in the war zone (shared squares)
 */
static uint8_t is_in_war_zone(uint8_t pos) {
    return (pos >= WAR_ZONE_START && pos <= WAR_ZONE_END);
}

/**
 * Calculate vulnerability score for a set of pieces
 * Returns weighted sum of capture threats from opponent
 *
 * @param pieces     Array of piece positions to evaluate
 * @param opponent   Array of opponent piece positions
 * @return Vulnerability score (higher = more vulnerable)
 */
static int16_t calculate_vulnerability(uint8_t *pieces, uint8_t *opponent) {
    int16_t vulnerability = 0;

    for (uint8_t i = 0; i < PIECES_PER_PLAYER; i++) {
        uint8_t pos = pieces[i];

        // Only pieces in war zone (5-12) can be captured
        // Rosette at position 8 is safe
        if (!is_in_war_zone(pos) || pos == 8) {
            continue;
        }

        // Check each opponent piece that could capture this one
        for (uint8_t j = 0; j < PIECES_PER_PLAYER; j++) {
            uint8_t opp_pos = opponent[j];

            // Opponent must be on board (not reserve or finished)
            if (opp_pos == POS_RESERVE || opp_pos == POS_FINISHED) {
                continue;
            }

            // Calculate distance opponent would need to roll
            int8_t distance = (int8_t)pos - (int8_t)opp_pos;

            // Valid capture if distance is 1-4 squares ahead
            if (distance >= 1 && distance <= 4) {
                vulnerability += dice_weights[distance];
            }
        }
    }

    return vulnerability;
}

/**
 * Calculate capture threat score (our ability to capture opponent)
 *
 * @param pieces     Array of our piece positions
 * @param opponent   Array of opponent piece positions
 * @return Threat score (higher = more capture opportunities)
 */
static int16_t calculate_capture_threats(uint8_t *pieces, uint8_t *opponent) {
    int16_t threats = 0;

    for (uint8_t i = 0; i < PIECES_PER_PLAYER; i++) {
        uint8_t pos = pieces[i];

        // Must be on board to threaten
        if (pos == POS_RESERVE || pos == POS_FINISHED) {
            continue;
        }

        // Check each opponent piece we could capture
        for (uint8_t j = 0; j < PIECES_PER_PLAYER; j++) {
            uint8_t opp_pos = opponent[j];

            // Target must be in war zone and not on rosette
            if (!is_in_war_zone(opp_pos) || opp_pos == 8) {
                continue;
            }

            // Calculate distance we would need to roll
            int8_t distance = (int8_t)opp_pos - (int8_t)pos;

            // Valid threat if distance is 1-4 squares ahead
            if (distance >= 1 && distance <= 4) {
                threats += dice_weights[distance];
            }
        }
    }

    return threats;
}

/**
 * Evaluate a player's position
 *
 * @param pieces     Array of piece positions to evaluate
 * @param opponent   Array of opponent piece positions
 * @return Position score (higher = better)
 */
static int16_t evaluate_player(uint8_t *pieces, uint8_t *opponent) {
    int16_t score = 0;

    for (uint8_t i = 0; i < PIECES_PER_PLAYER; i++) {
        uint8_t pos = pieces[i];

        if (pos == POS_FINISHED) {
            // Finished pieces are worth a lot
            score += WEIGHT_SCORED;
        } else if (pos != POS_RESERVE) {
            // Advancement bonus (position 1-14)
            score += (int16_t)pos * WEIGHT_ADVANCEMENT;

            // Rosette bonus (safety)
            if (is_rosette(pos)) {
                score += WEIGHT_ROSETTE_SAFETY;

                // Extra bonus for center rosette (position 8)
                if (pos == 8) {
                    score += WEIGHT_CENTER_ROSETTE;
                }
            }
        }
        // Reserve pieces contribute 0 to score
    }

    // Vulnerability penalty (divide by 16 to normalize probability)
    int16_t vulnerability = calculate_vulnerability(pieces, opponent);
    score += (vulnerability * WEIGHT_VULNERABILITY) / 16;

    // Capture threat bonus (divide by 16 to normalize probability)
    int16_t threats = calculate_capture_threats(pieces, opponent);
    score += (threats * WEIGHT_CAPTURE_THREAT) / 16;

    return score;
}

/**
 * Simulate a move and evaluate the resulting position
 *
 * @param piece_idx  Index of piece to move
 * @param roll       Dice roll value
 * @return Evaluation score (higher = better for CPU)
 */
static int16_t evaluate_move(uint8_t piece_idx, uint8_t roll) {
    // Create temporary copies of piece arrays
    uint8_t temp_cpu[PIECES_PER_PLAYER];
    uint8_t temp_human[PIECES_PER_PLAYER];

    for (uint8_t i = 0; i < PIECES_PER_PLAYER; i++) {
        temp_cpu[i] = cpu_pieces[i];
        temp_human[i] = human_pieces[i];
    }

    // Simulate the move
    uint8_t current_pos = temp_cpu[piece_idx];
    uint8_t new_pos;

    if (current_pos == POS_RESERVE) {
        new_pos = roll;
    } else {
        new_pos = current_pos + roll;
    }

    // Cap at finished
    if (new_pos >= POS_FINISHED) {
        new_pos = POS_FINISHED;
    }

    // Check for capture (in war zone, not on rosette)
    if (is_in_war_zone(new_pos) && new_pos != 8) {
        for (uint8_t i = 0; i < PIECES_PER_PLAYER; i++) {
            if (temp_human[i] == new_pos) {
                temp_human[i] = POS_RESERVE;
                break;
            }
        }
    }

    // Apply the move
    temp_cpu[piece_idx] = new_pos;

    // Evaluate: CPU score minus human score
    int16_t cpu_score = evaluate_player(temp_cpu, temp_human);
    int16_t human_score = evaluate_player(temp_human, temp_cpu);

    return cpu_score - human_score;
}

// ============================================================================
// Public Functions
// ============================================================================

uint8_t ai_select_move(uint8_t roll, uint8_t *out_piece_idx) {
    uint8_t valid_moves[PIECES_PER_PLAYER];
    uint8_t num_valid;

    // Get all valid moves
    num_valid = get_valid_moves(PLAYER_CPU, roll, valid_moves);

    if (num_valid == 0) {
        return 0;  // No valid moves
    }

    // If only one move, return it immediately
    if (num_valid == 1) {
        *out_piece_idx = valid_moves[0];
        return 1;
    }

    // Determine AI behavior based on difficulty
    uint8_t ai_threshold;
    switch (selected_difficulty) {
        case DIFFICULTY_EASY:
            ai_threshold = AI_CHANCE_EASY;
            break;
        case DIFFICULTY_MEDIUM:
            ai_threshold = AI_CHANCE_MEDIUM;
            break;
        case DIFFICULTY_HARD:
        default:
            ai_threshold = AI_CHANCE_HARD;
            break;
    }

    // Roll to see if we use AI or random
    uint8_t chance_roll = get_random() % 100;

    if (chance_roll >= ai_threshold) {
        // Random move (for lower difficulties)
        uint8_t choice = get_random() % num_valid;
        *out_piece_idx = valid_moves[choice];
        return 1;
    }

    // AI evaluation: find the best move
    int16_t best_score = -32000;  // Very low initial score
    uint8_t best_move = valid_moves[0];

    for (uint8_t i = 0; i < num_valid; i++) {
        int16_t score = evaluate_move(valid_moves[i], roll);

        if (score > best_score) {
            best_score = score;
            best_move = valid_moves[i];
        }
    }

    *out_piece_idx = best_move;
    return 1;
}
