/**
 * board_state.h
 * Board state types, position constants, and function declarations
 * Phase 8b: Piece position tracking and board visualization
 */

#ifndef BOARD_STATE_H
#define BOARD_STATE_H

#include <stdint.h>

// ============================================================================
// Position Constants
// ============================================================================
#define POS_RESERVE   0    // Piece not yet on board
#define POS_FINISHED  15   // Piece completed the path

// ============================================================================
// Square Type Constants (match board_tiles.png row order)
// ============================================================================
#define SQUARE_TYPE_ROSETTE 0  // Rosette: Positions 4, 8, 14
#define SQUARE_TYPE_A       1  // Positions 1, 3, 11
#define SQUARE_TYPE_B       2  // Positions 2, 6, 9, 12
#define SQUARE_TYPE_C       3  // Position 13
#define SQUARE_TYPE_D       4  // Position 5
#define SQUARE_TYPE_E       5  // Positions 7, 10

// ============================================================================
// Piece State Constants (match board_tiles.png column order)
// ============================================================================
#define PIECE_NONE   0
#define PIECE_WHITE  1
#define PIECE_BLACK  2

// ============================================================================
// Player Constants
// ============================================================================
#define PLAYER_HUMAN 0
#define PLAYER_CPU   1

// ============================================================================
// Board Square Structure
// ============================================================================
typedef struct {
    uint8_t tile_x;      // Tile column (0-19)
    uint8_t tile_y;      // Tile row (0-9)
    uint8_t square_type; // SQUARE_TYPE_* constant
    uint8_t is_rosette;  // 1 if rosette, 0 otherwise
} BoardSquare_t;

// ============================================================================
// Function Prototypes
// ============================================================================

/**
 * Load piece tiles into VRAM
 * Call during init_game() after loading board background tiles
 */
void load_piece_tiles(void);

/**
 * Redraw all board squares based on current piece positions
 * Call after any piece moves, captures, or game state changes
 */
void update_board_display(void);

/**
 * Check if a move is valid
 * @param player     PLAYER_HUMAN or PLAYER_CPU
 * @param piece_idx  Index of piece to move (0-6)
 * @param roll       Dice roll result (1-4)
 * @return 1 if valid, 0 if invalid
 */
uint8_t is_valid_move(uint8_t player, uint8_t piece_idx, uint8_t roll);

/**
 * Check if position is a rosette square
 * @param pos  Board position (1-14)
 * @return 1 if rosette, 0 otherwise
 */
uint8_t is_rosette(uint8_t pos);

/**
 * Execute a move for a player's piece
 * @param player     PLAYER_HUMAN or PLAYER_CPU
 * @param piece_idx  Index of piece to move (0-6)
 * @param roll       Dice roll result (1-4)
 * @return 1 if landed on rosette (extra turn), 0 otherwise
 */
uint8_t execute_move(uint8_t player, uint8_t piece_idx, uint8_t roll);

/**
 * Find a random valid move for a player
 * @param player        PLAYER_HUMAN or PLAYER_CPU
 * @param roll          Dice roll result (1-4)
 * @param out_piece_idx Pointer to store the piece index if move found
 * @return 1 if a valid move was found, 0 if no valid moves
 */
uint8_t find_random_valid_move(uint8_t player, uint8_t roll, uint8_t *out_piece_idx);

#endif // BOARD_STATE_H
