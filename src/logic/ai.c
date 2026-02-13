/**
 * ai.c
 * AI opponent logic for multiple opponent types
 *
 * THE SCHOLAR (adaptive strategy):
 * - Evaluates current position to determine if winning or losing
 * - When ahead: Uses defensive weights (protect the lead)
 * - When behind: Uses aggressive weights (catch up)
 * - When even: Uses balanced weights
 *
 * THE MERCHANT (greedy strategy):
 * - Evaluates board positions based on advancement, rosettes, captures
 *
 * THE MUSICIAN (turn economy strategy):
 * - Minimizes turns to win, values extra turns highly
 *
 * THE PRIESTESS (phase-based strategy):
 * - Adapts evaluation weights based on game phase (opening/midgame/endgame)
 * - Opening: Safe positioning, balanced play, secure center rosette
 * - Midgame: Aggressive war zone control, prioritize captures
 * - Endgame: Race to finish, maximum advancement
 */

#include <gb/gb.h>
#include <stdint.h>
#include "logic/ai.h"
#include "logic/board_state.h"
#include "screens/game.h"
#include "screens/difficulty_select.h"
#include "screens/opponent_select.h"
#include "util/random.h"

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

// ============================================================================
// Phase-Based Evaluation Functions (THE PRIESTESS)
// ============================================================================

/**
 * Detect current game phase based on total pieces scored
 *
 * @return PHASE_OPENING, PHASE_MIDGAME, or PHASE_ENDGAME
 */
static uint8_t detect_game_phase(void) {
    uint8_t total_scored = 0;

    for (uint8_t i = 0; i < PIECES_PER_PLAYER; i++) {
        if (cpu_pieces[i] == POS_FINISHED) total_scored++;
        if (human_pieces[i] == POS_FINISHED) total_scored++;
    }

    if (total_scored <= PHASE_OPENING_MAX) {
        return PHASE_OPENING;
    } else if (total_scored <= PHASE_MIDGAME_MAX) {
        return PHASE_MIDGAME;
    } else {
        return PHASE_ENDGAME;
    }
}

/**
 * Evaluate a player's position using phase-specific weights
 *
 * @param pieces     Array of piece positions to evaluate
 * @param opponent   Array of opponent piece positions
 * @param phase      Current game phase
 * @return Position score (higher = better)
 */
static int16_t evaluate_player_phased(uint8_t *pieces, uint8_t *opponent, uint8_t phase) {
    int16_t score = 0;

    // Select weights based on phase
    int16_t w_advancement, w_scored, w_center_rosette, w_rosette_safety;
    int16_t w_vulnerability, w_capture_threat;

    switch (phase) {
        case PHASE_OPENING:
            w_advancement = PH_OPENING_ADVANCEMENT;
            w_scored = PH_OPENING_SCORED;
            w_center_rosette = PH_OPENING_CENTER_ROSETTE;
            w_rosette_safety = PH_OPENING_ROSETTE_SAFETY;
            w_vulnerability = PH_OPENING_VULNERABILITY;
            w_capture_threat = PH_OPENING_CAPTURE_THREAT;
            break;
        case PHASE_MIDGAME:
            w_advancement = PH_MIDGAME_ADVANCEMENT;
            w_scored = PH_MIDGAME_SCORED;
            w_center_rosette = PH_MIDGAME_CENTER_ROSETTE;
            w_rosette_safety = PH_MIDGAME_ROSETTE_SAFETY;
            w_vulnerability = PH_MIDGAME_VULNERABILITY;
            w_capture_threat = PH_MIDGAME_CAPTURE_THREAT;
            break;
        case PHASE_ENDGAME:
        default:
            w_advancement = PH_ENDGAME_ADVANCEMENT;
            w_scored = PH_ENDGAME_SCORED;
            w_center_rosette = PH_ENDGAME_CENTER_ROSETTE;
            w_rosette_safety = PH_ENDGAME_ROSETTE_SAFETY;
            w_vulnerability = PH_ENDGAME_VULNERABILITY;
            w_capture_threat = PH_ENDGAME_CAPTURE_THREAT;
            break;
    }

    for (uint8_t i = 0; i < PIECES_PER_PLAYER; i++) {
        uint8_t pos = pieces[i];

        if (pos == POS_FINISHED) {
            // Finished pieces are worth a lot
            score += w_scored;
        } else if (pos != POS_RESERVE) {
            // Advancement bonus (position 1-14)
            score += (int16_t)pos * w_advancement;

            // Rosette bonus (safety)
            if (is_rosette(pos)) {
                score += w_rosette_safety;

                // Extra bonus for center rosette (position 8)
                if (pos == 8) {
                    score += w_center_rosette;
                }
            }
        }
        // Reserve pieces contribute 0 to score
    }

    // Vulnerability penalty (divide by 16 to normalize probability)
    int16_t vulnerability = calculate_vulnerability(pieces, opponent);
    score += (vulnerability * w_vulnerability) / 16;

    // Capture threat bonus (divide by 16 to normalize probability)
    int16_t threats = calculate_capture_threats(pieces, opponent);
    score += (threats * w_capture_threat) / 16;

    return score;
}

/**
 * Simulate a move and evaluate using phase-based weights
 *
 * @param piece_idx  Index of piece to move
 * @param roll       Dice roll value
 * @param phase      Current game phase
 * @return Evaluation score (higher = better for CPU)
 */
static int16_t evaluate_phase_move(uint8_t piece_idx, uint8_t roll, uint8_t phase) {
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
    int16_t cpu_score = evaluate_player_phased(temp_cpu, temp_human, phase);
    int16_t human_score = evaluate_player_phased(temp_human, temp_cpu, phase);

    return cpu_score - human_score;
}

// ============================================================================
// Adaptive Evaluation Functions (THE SCHOLAR)
// ============================================================================

/**
 * Evaluate a player's position using adaptive weights
 *
 * @param pieces     Array of piece positions to evaluate
 * @param opponent   Array of opponent piece positions
 * @param mode       ADAPT_MODE_BALANCED, DEFENSIVE, or AGGRESSIVE
 * @return Position score (higher = better)
 */
static int16_t evaluate_player_adaptive(uint8_t *pieces, uint8_t *opponent, uint8_t mode) {
    int16_t score = 0;
    int16_t w_advancement, w_scored, w_center_rosette, w_rosette_safety;
    int16_t w_vulnerability, w_capture_threat;

    switch (mode) {
        case ADAPT_MODE_DEFENSIVE:
            w_advancement = AD_DEFENSIVE_ADVANCEMENT;
            w_scored = AD_DEFENSIVE_SCORED;
            w_center_rosette = AD_DEFENSIVE_CENTER_ROSETTE;
            w_rosette_safety = AD_DEFENSIVE_ROSETTE_SAFETY;
            w_vulnerability = AD_DEFENSIVE_VULNERABILITY;
            w_capture_threat = AD_DEFENSIVE_CAPTURE_THREAT;
            break;
        case ADAPT_MODE_AGGRESSIVE:
            w_advancement = AD_AGGRESSIVE_ADVANCEMENT;
            w_scored = AD_AGGRESSIVE_SCORED;
            w_center_rosette = AD_AGGRESSIVE_CENTER_ROSETTE;
            w_rosette_safety = AD_AGGRESSIVE_ROSETTE_SAFETY;
            w_vulnerability = AD_AGGRESSIVE_VULNERABILITY;
            w_capture_threat = AD_AGGRESSIVE_CAPTURE_THREAT;
            break;
        case ADAPT_MODE_BALANCED:
        default:
            w_advancement = AD_BALANCED_ADVANCEMENT;
            w_scored = AD_BALANCED_SCORED;
            w_center_rosette = AD_BALANCED_CENTER_ROSETTE;
            w_rosette_safety = AD_BALANCED_ROSETTE_SAFETY;
            w_vulnerability = AD_BALANCED_VULNERABILITY;
            w_capture_threat = AD_BALANCED_CAPTURE_THREAT;
            break;
    }

    for (uint8_t i = 0; i < PIECES_PER_PLAYER; i++) {
        uint8_t pos = pieces[i];
        if (pos == POS_FINISHED) {
            score += w_scored;
        } else if (pos != POS_RESERVE) {
            score += (int16_t)pos * w_advancement;
            if (is_rosette(pos)) {
                score += w_rosette_safety;
                if (pos == 8) {
                    score += w_center_rosette;
                }
            }
        }
    }

    int16_t vulnerability = calculate_vulnerability(pieces, opponent);
    score += (vulnerability * w_vulnerability) / 16;

    int16_t threats = calculate_capture_threats(pieces, opponent);
    score += (threats * w_capture_threat) / 16;

    return score;
}

/**
 * Detect which adaptive mode to use based on current position
 * Evaluates both players with balanced weights to determine advantage
 *
 * @return ADAPT_MODE_BALANCED, DEFENSIVE, or AGGRESSIVE
 */
static uint8_t detect_adaptive_mode(void) {
    int16_t cpu_score = evaluate_player_adaptive(cpu_pieces, human_pieces, ADAPT_MODE_BALANCED);
    int16_t human_score = evaluate_player_adaptive(human_pieces, cpu_pieces, ADAPT_MODE_BALANCED);
    int16_t advantage = cpu_score - human_score;

    if (advantage > ADAPT_THRESHOLD) {
        return ADAPT_MODE_DEFENSIVE;   // Ahead - protect lead
    } else if (advantage < -ADAPT_THRESHOLD) {
        return ADAPT_MODE_AGGRESSIVE;  // Behind - catch up
    } else {
        return ADAPT_MODE_BALANCED;    // Close game
    }
}

/**
 * Simulate a move and evaluate using adaptive weights
 *
 * @param piece_idx  Index of piece to move
 * @param roll       Dice roll value
 * @param mode       Adaptive mode to use
 * @return Evaluation score (higher = better for CPU)
 */
static int16_t evaluate_adaptive_move(uint8_t piece_idx, uint8_t roll, uint8_t mode) {
    uint8_t temp_cpu[PIECES_PER_PLAYER];
    uint8_t temp_human[PIECES_PER_PLAYER];

    for (uint8_t i = 0; i < PIECES_PER_PLAYER; i++) {
        temp_cpu[i] = cpu_pieces[i];
        temp_human[i] = human_pieces[i];
    }

    uint8_t current_pos = temp_cpu[piece_idx];
    uint8_t new_pos;

    if (current_pos == POS_RESERVE) {
        new_pos = roll;
    } else {
        new_pos = current_pos + roll;
    }

    if (new_pos >= POS_FINISHED) {
        new_pos = POS_FINISHED;
    }

    if (is_in_war_zone(new_pos) && new_pos != 8) {
        for (uint8_t i = 0; i < PIECES_PER_PLAYER; i++) {
            if (temp_human[i] == new_pos) {
                temp_human[i] = POS_RESERVE;
                break;
            }
        }
    }

    temp_cpu[piece_idx] = new_pos;

    int16_t cpu_score = evaluate_player_adaptive(temp_cpu, temp_human, mode);
    int16_t human_score = evaluate_player_adaptive(temp_human, temp_cpu, mode);

    return cpu_score - human_score;
}

/**
 * Select best move using adaptive evaluation (THE SCHOLAR)
 * Detects game advantage, then evaluates moves with appropriate weights
 */
static uint8_t select_adaptive_move(uint8_t roll, uint8_t num_valid, uint8_t *valid_moves) {
    uint8_t mode = detect_adaptive_mode();
    int16_t best_score = -32000;
    uint8_t best_move = valid_moves[0];

    for (uint8_t i = 0; i < num_valid; i++) {
        int16_t score = evaluate_adaptive_move(valid_moves[i], roll, mode);
        if (score > best_score) {
            best_score = score;
            best_move = valid_moves[i];
        }
    }

    return best_move;
}

// ============================================================================
// Greedy Evaluation Functions (THE MERCHANT)
// ============================================================================

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
// Turn Economy Functions (THE MUSICIAN)
// ============================================================================

/**
 * Estimate turns needed to win for a set of pieces
 * Returns value scaled by 10 to avoid floats
 *
 * @param pieces  Array of piece positions
 * @return Estimated turns * 10
 */
static int16_t estimate_turns_to_win(uint8_t *pieces) {
    int16_t total = 0;

    for (uint8_t i = 0; i < PIECES_PER_PLAYER; i++) {
        uint8_t pos = pieces[i];

        if (pos == POS_FINISHED) {
            // Already finished: 0 turns
            continue;
        } else if (pos == POS_RESERVE) {
            // In reserve: ~7.5 turns to enter and traverse
            total += TURNS_PER_RESERVE;
        } else {
            // On board: distance to finish * turns per square
            total += (int16_t)(POS_FINISHED - pos) * TURNS_PER_DISTANCE;
        }
    }

    return total;
}

/**
 * Evaluate a move using turn economy strategy
 * Prioritizes moves that:
 * - Grant extra turns (landing on rosettes)
 * - Score pieces efficiently
 * - Minimize our turns while maximizing opponent's turns
 *
 * @param piece_idx  Index of piece to move
 * @param roll       Dice roll value
 * @return Evaluation score (higher = better)
 */
static int16_t evaluate_turn_economy_move(uint8_t piece_idx, uint8_t roll) {
    int16_t score = 0;

    // Create temporary copies of piece arrays
    uint8_t temp_cpu[PIECES_PER_PLAYER];
    uint8_t temp_human[PIECES_PER_PLAYER];

    for (uint8_t i = 0; i < PIECES_PER_PLAYER; i++) {
        temp_cpu[i] = cpu_pieces[i];
        temp_human[i] = human_pieces[i];
    }

    // Calculate current turn estimates before the move
    int16_t cpu_turns_before = estimate_turns_to_win(temp_cpu);
    int16_t human_turns_before = estimate_turns_to_win(temp_human);

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

    // Extra turn bonus for landing on rosette
    if (is_rosette(new_pos)) {
        score += TE_EXTRA_TURN_BONUS;
    }

    // Scoring bonus for finishing a piece
    if (new_pos == POS_FINISHED) {
        score += TE_SCORE_BONUS;
    }

    // Calculate turn estimates after the move
    int16_t cpu_turns_after = estimate_turns_to_win(temp_cpu);
    int16_t human_turns_after = estimate_turns_to_win(temp_human);

    // CPU improvement: reduce our turns to win
    // Divide by 10 to unscale the turn estimates
    int16_t cpu_improvement = (cpu_turns_before - cpu_turns_after) * TE_EFFICIENCY_WEIGHT / 10;
    score += cpu_improvement;

    // Opponent delay: increase their turns (from captures)
    int16_t opponent_delay = (human_turns_after - human_turns_before) * TE_OPPONENT_DELAY / 10;
    score += opponent_delay;

    return score;
}

// ============================================================================
// Move Selection Helpers
// ============================================================================

/**
 * Select a random move from valid moves
 */
static uint8_t select_random_move(uint8_t num_valid, uint8_t *valid_moves) {
    uint8_t choice = get_random_unbiased(num_valid);
    return valid_moves[choice];
}

/**
 * Select best move using greedy evaluation (THE MERCHANT)
 */
static uint8_t select_greedy_move(uint8_t roll, uint8_t num_valid, uint8_t *valid_moves) {
    int16_t best_score = -32000;
    uint8_t best_move = valid_moves[0];

    for (uint8_t i = 0; i < num_valid; i++) {
        int16_t score = evaluate_move(valid_moves[i], roll);

        if (score > best_score) {
            best_score = score;
            best_move = valid_moves[i];
        }
    }

    return best_move;
}

/**
 * Select best move using turn economy evaluation (THE MUSICIAN)
 */
static uint8_t select_turn_economy_move(uint8_t roll, uint8_t num_valid, uint8_t *valid_moves) {
    int16_t best_score = -32000;
    uint8_t best_move = valid_moves[0];

    for (uint8_t i = 0; i < num_valid; i++) {
        int16_t score = evaluate_turn_economy_move(valid_moves[i], roll);

        if (score > best_score) {
            best_score = score;
            best_move = valid_moves[i];
        }
    }

    return best_move;
}

/**
 * Select best move using phase-based evaluation (THE PRIESTESS)
 */
static uint8_t select_phase_move(uint8_t roll, uint8_t num_valid, uint8_t *valid_moves) {
    uint8_t phase = detect_game_phase();
    int16_t best_score = -32000;
    uint8_t best_move = valid_moves[0];

    for (uint8_t i = 0; i < num_valid; i++) {
        int16_t score = evaluate_phase_move(valid_moves[i], roll, phase);

        if (score > best_score) {
            best_score = score;
            best_move = valid_moves[i];
        }
    }

    return best_move;
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
    uint8_t chance_roll = get_random_unbiased(100);

    if (chance_roll >= ai_threshold) {
        // Random move (for lower difficulties)
        *out_piece_idx = select_random_move(num_valid, valid_moves);
        return 1;
    }

    // AI evaluation: dispatch based on opponent type
    switch (selected_opponent) {
        case OPPONENT_SCHOLAR:
            // Adaptive AI - switches between defensive/balanced/aggressive
            *out_piece_idx = select_adaptive_move(roll, num_valid, valid_moves);
            break;

        case OPPONENT_PRIESTESS:
            // Phase-based AI - adapts strategy to game phase
            *out_piece_idx = select_phase_move(roll, num_valid, valid_moves);
            break;

        case OPPONENT_MERCHANT:
            // Greedy AI - evaluates board position
            *out_piece_idx = select_greedy_move(roll, num_valid, valid_moves);
            break;

        case OPPONENT_MUSICIAN:
            // Turn economy AI - minimizes turns to win
            *out_piece_idx = select_turn_economy_move(roll, num_valid, valid_moves);
            break;

        default:
            // Fallback to greedy
            *out_piece_idx = select_greedy_move(roll, num_valid, valid_moves);
            break;
    }

    return 1;
}
