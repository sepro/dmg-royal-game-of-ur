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

// ============================================================================
// External References
// ============================================================================

// Piece position arrays from game.c
extern uint8_t human_pieces[7];
extern uint8_t cpu_pieces[7];

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
static const BoardSquare_t p1_squares[14] = {
    { 8, 2, SQUARE_TYPE_A,       0}, // Pos 1
    { 6, 2, SQUARE_TYPE_B,       0}, // Pos 2
    { 4, 2, SQUARE_TYPE_A,       0}, // Pos 3
    { 2, 2, SQUARE_TYPE_ROSETTE, 1}, // Pos 4 (rosette)
    { 2, 4, SQUARE_TYPE_D,       0}, // Pos 5
    { 4, 4, SQUARE_TYPE_B,       0}, // Pos 6
    { 6, 4, SQUARE_TYPE_E,       0}, // Pos 7
    { 8, 4, SQUARE_TYPE_ROSETTE, 1}, // Pos 8 (rosette)
    {10, 4, SQUARE_TYPE_B,       0}, // Pos 9
    {12, 4, SQUARE_TYPE_E,       0}, // Pos 10
    {14, 4, SQUARE_TYPE_A,       0}, // Pos 11
    {16, 4, SQUARE_TYPE_B,       0}, // Pos 12
    {16, 2, SQUARE_TYPE_C,       0}, // Pos 13
    {14, 2, SQUARE_TYPE_ROSETTE, 1}, // Pos 14 (rosette)
};

// P2's private squares only (positions 1-4 and 13-14)
// Shared squares (5-12) use same coordinates as P1
static const BoardSquare_t p2_private_squares[6] = {
    { 8, 6, SQUARE_TYPE_A,       0}, // Pos 1
    { 6, 6, SQUARE_TYPE_B,       0}, // Pos 2
    { 4, 6, SQUARE_TYPE_A,       0}, // Pos 3
    { 2, 6, SQUARE_TYPE_ROSETTE, 1}, // Pos 4 (rosette)
    {16, 6, SQUARE_TYPE_C,       0}, // Pos 13
    {14, 6, SQUARE_TYPE_ROSETTE, 1}, // Pos 14 (rosette)
};

// ============================================================================
// Tile Index Calculation
// ============================================================================

/**
 * Get base tile index for a square type and piece state
 * board_tiles_pieces.png layout:
 * - 3 columns: empty, white piece, black piece
 * - 6 rows: rosette, type A, B, C, D, E
 * - Each cell is 2x2 tiles (16x16 pixels) = 4 tiles
 * - Tiles arranged row by row: row0_col0, row0_col1, row0_col2, row1_col0, ...
 */
static uint8_t get_tile_base(uint8_t square_type, uint8_t piece_state) {
    // Each row has 3 columns x 4 tiles = 12 tiles
    // Each column has 4 tiles (2x2)
    // tile_index = (row * 12) + (col * 4)
    // But png2asset outputs tiles left-to-right, top-to-bottom
    // So: (row * 3 * 4) + (col * 4) = (row * 12) + (col * 4)

    // However, png2asset processes 8x8 tiles in row order:
    // For a 48x96 image: 6 tiles wide, 12 tiles tall
    // Row 0 of squares = tile rows 0-1 (16 pixels)
    // Each 16x16 square uses 4 tiles in this pattern within its 2x2 area

    // Corrected calculation for png2asset tile ordering:
    // Image is 48x96 pixels = 6x12 tiles (8x8 each)
    // Each square type row is 2 tile rows (16 pixels)
    // Each piece state column is 2 tile columns (16 pixels)
    // Tiles are numbered left-to-right, top-to-bottom

    // For row R (0-5) and column C (0-2):
    // Top-left tile of 16x16 square = (R * 2) * 6 + (C * 2)
    // Pattern within 2x2:
    //   [base+0] [base+1]
    //   [base+6] [base+7]

    uint8_t base = (square_type * 2 * 6) + (piece_state * 2);
    return VRAM_PIECE_TILES_START + base;
}

// ============================================================================
// Drawing Functions
// ============================================================================

/**
 * Draw a single 2x2 tile board square
 */
static void draw_board_square(uint8_t tile_x, uint8_t tile_y,
                              uint8_t square_type, uint8_t piece_state) {
    uint8_t base = get_tile_base(square_type, piece_state);

    // Draw 2x2 tile block
    // png2asset tile layout within 2x2 square:
    //   [base+0] [base+1]
    //   [base+6] [base+7]  (next row is +6 because image is 6 tiles wide)
    set_bkg_tile_xy(tile_x,     tile_y,     base);
    set_bkg_tile_xy(tile_x + 1, tile_y,     base + 1);
    set_bkg_tile_xy(tile_x,     tile_y + 1, base + 6);
    set_bkg_tile_xy(tile_x + 1, tile_y + 1, base + 7);
}

/**
 * Get piece state at a position for a specific player's private section
 */
static uint8_t get_piece_at(uint8_t pos, uint8_t player) {
    uint8_t *pieces = (player == PLAYER_HUMAN) ? human_pieces : cpu_pieces;
    uint8_t color = (player == PLAYER_HUMAN) ? human_color : cpu_color;

    for (uint8_t i = 0; i < 7; i++) {
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
    for (uint8_t i = 0; i < 7; i++) {
        if (human_pieces[i] == pos) {
            return (human_color == SIDE_LIGHT) ? PIECE_WHITE : PIECE_BLACK;
        }
    }
    // Check CPU pieces
    for (uint8_t i = 0; i < 7; i++) {
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
    for (uint8_t i = 0; i < 7; i++) {
        if (pieces[i] == new_pos && i != piece_idx) return 0;
    }

    // Check capture rules for shared squares
    if (new_pos >= 5 && new_pos <= 12) {
        // Can't capture on rosette (position 8)
        if (is_rosette(new_pos)) {
            for (uint8_t i = 0; i < 7; i++) {
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
        for (uint8_t i = 0; i < 7; i++) {
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
    uint8_t valid_moves[7];
    uint8_t num_valid = 0;

    // Find all valid moves
    for (uint8_t i = 0; i < 7; i++) {
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
