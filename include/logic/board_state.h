/**
 * board_state.h
 * Board state types, position constants, and function declarations
 * Piece position tracking and board visualization
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
 * Mark a specific board square as needing redraw
 * @param player  PLAYER_HUMAN or PLAYER_CPU (for private squares 1-4, 13-14)
 * @param pos     Board position (1-14)
 */
void mark_square_dirty(uint8_t player, uint8_t pos);

/**
 * Mark all board squares as needing redraw
 * Call on initial board setup or after major state changes
 */
void mark_all_squares_dirty(void);

/**
 * Redraw only the squares marked as dirty
 * More efficient than update_board_display() when only a few squares changed
 */
void update_dirty_squares(void);

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
 * @param player       PLAYER_HUMAN or PLAYER_CPU
 * @param piece_idx    Index of piece to move (0-6)
 * @param roll         Dice roll result (1-4)
 * @param out_captured Pointer to store 1 if piece was captured, 0 otherwise (optional, can be null)
 * @return 1 if landed on rosette (extra turn), 0 otherwise
 */
uint8_t execute_move(uint8_t player, uint8_t piece_idx, uint8_t roll, uint8_t *out_captured);

/**
 * Get all valid moves for a player with given roll
 * @param player     PLAYER_HUMAN or PLAYER_CPU
 * @param roll       Dice roll result (1-4)
 * @param out_moves  Array to store piece indices (must be at least PIECES_PER_PLAYER)
 * @return Number of valid moves found
 */
uint8_t get_valid_moves(uint8_t player, uint8_t roll, uint8_t *out_moves);

/**
 * Get screen pixel coordinates for a board position
 * @param player  PLAYER_HUMAN or PLAYER_CPU (needed for private squares 1-4, 13-14)
 * @param pos     Board position (1-14)
 * @param out_x   Pointer to store sprite X coordinate
 * @param out_y   Pointer to store sprite Y coordinate
 * @return 1 if position is valid, 0 otherwise
 */
uint8_t get_position_screen_coords(uint8_t player, uint8_t pos, uint8_t *out_x, uint8_t *out_y);

/**
 * Get screen pixel coordinates for reserve indicator
 * @param player  PLAYER_HUMAN or PLAYER_CPU
 * @param out_x   Pointer to store sprite X coordinate
 * @param out_y   Pointer to store sprite Y coordinate
 */
void get_reserve_screen_coords(uint8_t player, uint8_t *out_x, uint8_t *out_y);

/**
 * Get screen pixel coordinates for bearoff indicator
 * @param player  PLAYER_HUMAN or PLAYER_CPU
 * @param out_x   Pointer to store sprite X coordinate
 * @param out_y   Pointer to store sprite Y coordinate
 */
void get_bearoff_screen_coords(uint8_t player, uint8_t *out_x, uint8_t *out_y);

/**
 * Paint CGB BG palettes for the in-game board: sand across the whole board
 * area, with the 5 rosette squares overwritten with the rosette palette.
 * Call once per game-screen init after the board frame map is in place.
 * No-op on DMG.
 */
void apply_board_cgb_palettes(void);

#endif // BOARD_STATE_H
