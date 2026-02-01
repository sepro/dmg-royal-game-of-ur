/**
 * board_state.c
 * Board state tracking and visualization
 * Phase 8b: Piece position tracking, move validation, and board rendering
 */

#include <gb/gb.h>
#include <stdint.h>
#include "board_state.h"
#include "coinflip.h"
#include "vram_layout.h"
#include "game.h"

// ============================================================================
// External References
// ============================================================================

// Piece position arrays from game.c
extern uint8_t human_pieces[PIECES_PER_PLAYER];
extern uint8_t cpu_pieces[PIECES_PER_PLAYER];

// Player colors from game.c
extern uint8_t human_color;
extern uint8_t cpu_color;

// Piece tile data from generated assets
extern const uint8_t board_tiles_pieces_tiles[];

// ============================================================================
// Board Coordinate Lookup Tables
// ============================================================================

// P1's full path (index 0-13 for positions 1-14)
// Coordinates are top-left tile of 2x2 square
// Human player path - now on bottom row (y=6)
static const BoardSquare_t p1_squares[14] = {
    { 8, 6, SQUARE_TYPE_A,       0}, // Pos 1
    { 6, 6, SQUARE_TYPE_B,       0}, // Pos 2
    { 4, 6, SQUARE_TYPE_A,       0}, // Pos 3
    { 2, 6, SQUARE_TYPE_ROSETTE, 1}, // Pos 4 (rosette)
    { 2, 4, SQUARE_TYPE_D,       0}, // Pos 5
    { 4, 4, SQUARE_TYPE_B,       0}, // Pos 6
    { 6, 4, SQUARE_TYPE_E,       0}, // Pos 7
    { 8, 4, SQUARE_TYPE_ROSETTE, 1}, // Pos 8 (rosette)
    {10, 4, SQUARE_TYPE_B,       0}, // Pos 9
    {12, 4, SQUARE_TYPE_E,       0}, // Pos 10
    {14, 4, SQUARE_TYPE_A,       0}, // Pos 11
    {16, 4, SQUARE_TYPE_B,       0}, // Pos 12
    {16, 6, SQUARE_TYPE_C,       0}, // Pos 13
    {14, 6, SQUARE_TYPE_ROSETTE, 1}, // Pos 14 (rosette)
};

// P2's private squares only (positions 1-4 and 13-14)
// Shared squares (5-12) use same coordinates as P1
// CPU player path - now on top row (y=2)
static const BoardSquare_t p2_private_squares[6] = {
    { 8, 2, SQUARE_TYPE_A,       0}, // Pos 1
    { 6, 2, SQUARE_TYPE_B,       0}, // Pos 2
    { 4, 2, SQUARE_TYPE_A,       0}, // Pos 3
    { 2, 2, SQUARE_TYPE_ROSETTE, 1}, // Pos 4 (rosette)
    {16, 2, SQUARE_TYPE_C,       0}, // Pos 13
    {14, 2, SQUARE_TYPE_ROSETTE, 1}, // Pos 14 (rosette)
};

// ============================================================================
// Tile Index Lookup Table
// ============================================================================

/**
 * Tile indices for each [square_type][piece_state] combination
 * board_tiles_pieces.png layout (48x96 pixels = 6x12 tiles of 8x8 each):
 * - 3 columns: empty (0), white piece (1), black piece (2)
 * - 6 rows: rosette (0), type A (1), B (2), C (3), D (4), E (5)
 * - Each 16x16 square uses 4 tiles: [0]top-left, [1]top-right, [2]bottom-left, [3]bottom-right
 * - Source tiles are numbered 0-71 (6 wide × 12 tall)
 * - After loading to VRAM at VRAM_PIECE_TILES_START, add that offset to use them
 */
static const uint8_t piece_tiles[6][3][4] = {
    // SQUARE_TYPE_ROSETTE (row 0)
    {
        {  0,  2,  1,  3 },  // PIECE_NONE
        {  4,  6,  5,  7 },  // PIECE_WHITE
        {  8,  10, 9, 11 },  // PIECE_BLACK
    },
    // SQUARE_TYPE_A (row 1)
    {
        { 12, 14, 13, 15 },  // PIECE_NONE
        { 16, 18, 17, 19 },  // PIECE_WHITE
        { 20, 22, 21, 23 },  // PIECE_BLACK
    },
    // SQUARE_TYPE_B (row 2)
    {
        { 24, 26, 25, 27 },  // PIECE_NONE
        { 28, 30, 29, 31 },  // PIECE_WHITE
        { 32, 34, 33, 35 },  // PIECE_BLACK
    },
    // SQUARE_TYPE_C (row 3)
    {
        { 36, 38, 37, 39 },  // PIECE_NONE
        { 40, 42, 41, 43 },  // PIECE_WHITE
        { 44, 46, 45, 47 },  // PIECE_BLACK
    },
    // SQUARE_TYPE_D (row 4)
    {
        { 48, 50, 49, 51 },  // PIECE_NONE
        { 52, 54, 53, 55 },  // PIECE_WHITE
        { 56, 58, 57, 59 },  // PIECE_BLACK
    },
    // SQUARE_TYPE_E (row 5)
    {
        { 60, 62, 61, 63 },  // PIECE_NONE
        { 64, 66, 65, 67 },  // PIECE_WHITE
        { 68, 70, 69, 71 },  // PIECE_BLACK
    },
};

// ============================================================================
// Drawing Functions
// ============================================================================

/**
 * Draw a single 2x2 tile board square
 */
static void draw_board_square(uint8_t tile_x, uint8_t tile_y,
                              uint8_t square_type, uint8_t piece_state) {
    const uint8_t *tiles = piece_tiles[square_type][piece_state];

    // Draw 2x2 tile block using explicit tile indices from lookup table
    // Tiles are source indices (0-71), add VRAM_PIECE_TILES_START for VRAM position
    set_bkg_tile_xy(tile_x,     tile_y,     VRAM_PIECE_TILES_START + tiles[0]);
    set_bkg_tile_xy(tile_x + 1, tile_y,     VRAM_PIECE_TILES_START + tiles[1]);
    set_bkg_tile_xy(tile_x,     tile_y + 1, VRAM_PIECE_TILES_START + tiles[2]);
    set_bkg_tile_xy(tile_x + 1, tile_y + 1, VRAM_PIECE_TILES_START + tiles[3]);
}

/**
 * Get piece state at a position for a specific player's private section
 */
static uint8_t get_piece_at(uint8_t pos, uint8_t player) {
    uint8_t *pieces = (player == PLAYER_HUMAN) ? human_pieces : cpu_pieces;
    uint8_t color = (player == PLAYER_HUMAN) ? human_color : cpu_color;

    for (uint8_t i = 0; i < PIECES_PER_PLAYER; i++) {
        if (pieces[i] == pos) {
            return (color == SIDE_LIGHT) ? PIECE_WHITE : PIECE_BLACK;
        }
    }
    return PIECE_NONE;
}

/**
 * Get piece state at a shared position (5-12)
 * Either player could have a piece here
 */
static uint8_t get_piece_at_shared(uint8_t pos) {
    // Check human pieces
    for (uint8_t i = 0; i < PIECES_PER_PLAYER; i++) {
        if (human_pieces[i] == pos) {
            return (human_color == SIDE_LIGHT) ? PIECE_WHITE : PIECE_BLACK;
        }
    }
    // Check CPU pieces
    for (uint8_t i = 0; i < PIECES_PER_PLAYER; i++) {
        if (cpu_pieces[i] == pos) {
            return (cpu_color == SIDE_LIGHT) ? PIECE_WHITE : PIECE_BLACK;
        }
    }
    return PIECE_NONE;
}

// ============================================================================
// Public Functions
// ============================================================================

void load_piece_tiles(void) {
    set_bkg_data(VRAM_PIECE_TILES_START, VRAM_PIECE_TILES_COUNT, board_tiles_pieces_tiles);
}

void update_board_display(void) {
    BoardSquare_t sq;
    uint8_t piece;

    // Draw human private squares (positions 1-4)
    for (uint8_t pos = 1; pos <= 4; pos++) {
        piece = get_piece_at(pos, PLAYER_HUMAN);
        sq = p1_squares[pos - 1];
        draw_board_square(sq.tile_x, sq.tile_y, sq.square_type, piece);
    }

    // Draw human exit squares (positions 13-14)
    for (uint8_t pos = 13; pos <= 14; pos++) {
        piece = get_piece_at(pos, PLAYER_HUMAN);
        sq = p1_squares[pos - 1];
        draw_board_square(sq.tile_x, sq.tile_y, sq.square_type, piece);
    }

    // Draw shared squares (positions 5-12) - check both players
    for (uint8_t pos = 5; pos <= 12; pos++) {
        piece = get_piece_at_shared(pos);
        sq = p1_squares[pos - 1];
        draw_board_square(sq.tile_x, sq.tile_y, sq.square_type, piece);
    }

    // Draw CPU private squares (positions 1-4)
    for (uint8_t i = 0; i < 4; i++) {
        piece = get_piece_at(i + 1, PLAYER_CPU);
        sq = p2_private_squares[i];
        draw_board_square(sq.tile_x, sq.tile_y, sq.square_type, piece);
    }

    // Draw CPU exit squares (positions 13-14)
    for (uint8_t i = 4; i < 6; i++) {
        uint8_t pos = (i == 4) ? 13 : 14;
        piece = get_piece_at(pos, PLAYER_CPU);
        sq = p2_private_squares[i];
        draw_board_square(sq.tile_x, sq.tile_y, sq.square_type, piece);
    }
}

uint8_t is_rosette(uint8_t pos) {
    return (pos == 4 || pos == 8 || pos == 14);
}

uint8_t is_valid_move(uint8_t player, uint8_t piece_idx, uint8_t roll) {
    if (roll == 0) return 0;

    uint8_t *pieces = (player == PLAYER_HUMAN) ? human_pieces : cpu_pieces;
    uint8_t *opponent = (player == PLAYER_HUMAN) ? cpu_pieces : human_pieces;
    uint8_t current_pos = pieces[piece_idx];
    uint8_t new_pos;

    // Can't move finished pieces
    if (current_pos == POS_FINISHED) return 0;

    // Calculate new position
    if (current_pos == POS_RESERVE) {
        new_pos = roll;  // Enter board at position = roll
    } else {
        new_pos = current_pos + roll;
    }

    // Must land exactly on finish or before
    if (new_pos > POS_FINISHED) return 0;

    // If finishing, need exact roll (pos 14 + roll to get 15)
    if (new_pos == POS_FINISHED) {
        // Valid - piece bears off
        return 1;
    }

    // Can't land on own piece
    for (uint8_t i = 0; i < PIECES_PER_PLAYER; i++) {
        if (pieces[i] == new_pos && i != piece_idx) return 0;
    }

    // Check capture rules for shared squares
    if (new_pos >= 5 && new_pos <= 12) {
        // Can't capture on rosette (position 8)
        if (is_rosette(new_pos)) {
            for (uint8_t i = 0; i < PIECES_PER_PLAYER; i++) {
                if (opponent[i] == new_pos) return 0;  // Opponent on rosette = safe
            }
        }
    }

    return 1;
}

uint8_t execute_move(uint8_t player, uint8_t piece_idx, uint8_t roll) {
    uint8_t *pieces = (player == PLAYER_HUMAN) ? human_pieces : cpu_pieces;
    uint8_t *opponent = (player == PLAYER_HUMAN) ? cpu_pieces : human_pieces;
    uint8_t current_pos = pieces[piece_idx];
    uint8_t new_pos;

    // Calculate new position
    if (current_pos == POS_RESERVE) {
        new_pos = roll;
    } else {
        new_pos = current_pos + roll;
    }

    // Handle finish
    if (new_pos >= POS_FINISHED) {
        new_pos = POS_FINISHED;
    }

    // Check for capture on shared squares (5-12, excluding rosette 8)
    if (new_pos >= 5 && new_pos <= 12 && !is_rosette(new_pos)) {
        for (uint8_t i = 0; i < PIECES_PER_PLAYER; i++) {
            if (opponent[i] == new_pos) {
                // Capture! Send opponent piece back to reserve
                opponent[i] = POS_RESERVE;
                break;
            }
        }
    }

    // Move the piece
    pieces[piece_idx] = new_pos;

    // Return 1 if landed on rosette (extra turn)
    if (new_pos != POS_FINISHED && is_rosette(new_pos)) {
        return 1;
    }

    return 0;
}

uint8_t find_random_valid_move(uint8_t player, uint8_t roll, uint8_t *out_piece_idx) {
    uint8_t valid_moves[PIECES_PER_PLAYER];
    uint8_t num_valid = 0;

    // Find all valid moves
    for (uint8_t i = 0; i < PIECES_PER_PLAYER; i++) {
        if (is_valid_move(player, i, roll)) {
            valid_moves[num_valid] = i;
            num_valid++;
        }
    }

    if (num_valid == 0) {
        return 0;  // No valid moves
    }

    // Pick a random valid move using DIV_REG for entropy
    uint8_t choice = DIV_REG % num_valid;
    *out_piece_idx = valid_moves[choice];

    return 1;
}
