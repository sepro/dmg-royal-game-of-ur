/**
 * board_state.c
 * Board state tracking and visualization
 * Piece position tracking, move validation, and board rendering
 */

#include <gb/gb.h>
#include <stdint.h>
#include "logic/board_state.h"
#include "screens/coinflip.h"
#include "vram_layout.h"
#include "screens/game.h"
#include "util/cgb.h"
#include "util/random.h"

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
// Dirty Square Tracking
// ============================================================================

/**
 * Dirty flags for 20 unique visual squares:
 * Bits 0-3:   Human private (pos 1-4)
 * Bits 4-5:   Human exit (pos 13-14)
 * Bits 6-13:  Shared (pos 5-12)
 * Bits 14-17: CPU private (pos 1-4)
 * Bits 18-19: CPU exit (pos 13-14)
 */
static uint32_t dirty_squares = 0;

/**
 * Convert (player, position) to dirty bit index
 */
static uint8_t get_square_index(uint8_t player, uint8_t pos) {
    if (pos >= 5 && pos <= 12) {
        // Shared squares: bits 6-13
        return 6 + (pos - 5);
    } else if (player == PLAYER_HUMAN) {
        // Human private: bits 0-5
        if (pos <= 4) return pos - 1;       // pos 1-4 -> bits 0-3
        else return 4 + (pos - 13);          // pos 13-14 -> bits 4-5
    } else {
        // CPU private: bits 14-19
        if (pos <= 4) return 14 + (pos - 1); // pos 1-4 -> bits 14-17
        else return 18 + (pos - 13);          // pos 13-14 -> bits 18-19
    }
}

// ============================================================================
// Tile Index Lookup Table
// ============================================================================

/**
 * Tile indices for each [square_type][piece_state] combination
 * board_tiles_pieces.png layout (48x96 pixels = 6x12 tiles of 8x8 each):
 * - 3 columns: empty (0), white piece (1), black piece (2)
 * - 6 rows: rosette (0), type A (1), B (2), C (3), D (4), E (5)
 * - Each 16x16 square uses 4 tiles: [0]top-left, [+1]bottom-left, [+2]top-right, [+3]bottom-right
 * - Source tiles are numbered 0-71 (6 wide × 12 tall)
 * - After loading to VRAM at VRAM_PIECE_TILES_START, add that offset to use them
 */
static const uint8_t piece_tiles[6][3] = {
    // SQUARE_TYPE_ROSETTE (row 0)
    { 0,4,8},
    // SQUARE_TYPE_A (row 1)
    {12,16,20},
    // SQUARE_TYPE_B (row 2)
    {24, 28, 32},
    // SQUARE_TYPE_C (row 3)
    {36, 40, 44},
    // SQUARE_TYPE_D (row 4)
    {48, 52, 56},
    // SQUARE_TYPE_E (row 5)
    {60, 64, 68}
};

// ============================================================================
// Drawing Functions
// ============================================================================

/**
 * Draw a single 2x2 tile board square
 */
static void draw_board_square(uint8_t tile_x, uint8_t tile_y,
                              uint8_t square_type, uint8_t piece_state) {
    const uint8_t tile = piece_tiles[square_type][piece_state];

    // Draw 2x2 tile block using explicit tile indices from lookup table
    // Tiles are source indices (0-71), add VRAM_PIECE_TILES_START for VRAM position
    set_bkg_tile_xy(tile_x,     tile_y,     VRAM_PIECE_TILES_START + tile);
    set_bkg_tile_xy(tile_x,     tile_y + 1, VRAM_PIECE_TILES_START + tile + 1);
    set_bkg_tile_xy(tile_x + 1, tile_y,     VRAM_PIECE_TILES_START + tile + 2);
    set_bkg_tile_xy(tile_x + 1, tile_y + 1, VRAM_PIECE_TILES_START + tile + 3);
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
    // Mark all squares dirty and update them
    mark_all_squares_dirty();
    update_dirty_squares();
}

void mark_square_dirty(uint8_t player, uint8_t pos) {
    if (pos >= 1 && pos <= 14) {
        uint8_t idx = get_square_index(player, pos);
        dirty_squares |= (1UL << idx);
    }
}

void mark_all_squares_dirty(void) {
    dirty_squares = 0xFFFFF;  // Lower 20 bits set
}

void update_dirty_squares(void) {
    if (dirty_squares == 0) return;

    BoardSquare_t sq;
    uint8_t piece;

    // Check human private squares (bits 0-3: pos 1-4)
    for (uint8_t i = 0; i < 4; i++) {
        if (dirty_squares & (1UL << i)) {
            uint8_t pos = i + 1;
            piece = get_piece_at(pos, PLAYER_HUMAN);
            sq = p1_squares[pos - 1];
            draw_board_square(sq.tile_x, sq.tile_y, sq.square_type, piece);
        }
    }

    // Check human exit squares (bits 4-5: pos 13-14)
    for (uint8_t i = 4; i < 6; i++) {
        if (dirty_squares & (1UL << i)) {
            uint8_t pos = 13 + (i - 4);
            piece = get_piece_at(pos, PLAYER_HUMAN);
            sq = p1_squares[pos - 1];
            draw_board_square(sq.tile_x, sq.tile_y, sq.square_type, piece);
        }
    }

    // Check shared squares (bits 6-13: pos 5-12)
    for (uint8_t i = 6; i < 14; i++) {
        if (dirty_squares & (1UL << i)) {
            uint8_t pos = 5 + (i - 6);
            piece = get_piece_at_shared(pos);
            sq = p1_squares[pos - 1];
            draw_board_square(sq.tile_x, sq.tile_y, sq.square_type, piece);
        }
    }

    // Check CPU private squares (bits 14-17: pos 1-4)
    for (uint8_t i = 14; i < 18; i++) {
        if (dirty_squares & (1UL << i)) {
            uint8_t pos = (i - 14) + 1;
            piece = get_piece_at(pos, PLAYER_CPU);
            sq = p2_private_squares[pos - 1];
            draw_board_square(sq.tile_x, sq.tile_y, sq.square_type, piece);
        }
    }

    // Check CPU exit squares (bits 18-19: pos 13-14)
    for (uint8_t i = 18; i < 20; i++) {
        if (dirty_squares & (1UL << i)) {
            uint8_t pos = 13 + (i - 18);
            piece = get_piece_at(pos, PLAYER_CPU);
            uint8_t idx = 4 + (i - 18);  // p2_private_squares indices 4-5
            sq = p2_private_squares[idx];
            draw_board_square(sq.tile_x, sq.tile_y, sq.square_type, piece);
        }
    }

    dirty_squares = 0;
}

uint8_t is_rosette(uint8_t pos) {
    return (pos == 4 || pos == 8 || pos == 14);
}

// Top-left (x, y) BG tile coords of all 5 rosette squares on the game board,
// each square being 2x2 BG tiles. Mirrors the rosette entries in p1_squares[]
// / p2_private_squares[] above.
static const uint8_t rosette_tile_xy[5][2] = {
    { 2, 2}, {14, 2},
    { 8, 4},
    { 2, 6}, {14, 6}
};

void apply_board_cgb_palettes(void) {
    // Sand palette across the entire 20x10 board area, then rosette palette
    // over the 5 rosette squares (2x2 BG tiles each).
    cgb_set_bg_attr_rect(BOARD_X, BOARD_Y, BOARD_WIDTH, BOARD_HEIGHT, CGB_PAL_BOARD_SAND);
    for (uint8_t i = 0; i < 5; i++) {
        cgb_set_bg_attr_rect(rosette_tile_xy[i][0], rosette_tile_xy[i][1],
                             2, 2, CGB_PAL_ROSETTE);
    }
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

uint8_t execute_move(uint8_t player, uint8_t piece_idx, uint8_t roll, uint8_t *out_captured) {
    uint8_t *pieces = (player == PLAYER_HUMAN) ? human_pieces : cpu_pieces;
    uint8_t *opponent = (player == PLAYER_HUMAN) ? cpu_pieces : human_pieces;
    uint8_t opponent_id = (player == PLAYER_HUMAN) ? PLAYER_CPU : PLAYER_HUMAN;
    uint8_t current_pos = pieces[piece_idx];
    uint8_t new_pos;
    uint8_t captured = 0;

    // Mark source square dirty (if on board)
    if (current_pos >= 1 && current_pos <= 14) {
        mark_square_dirty(player, current_pos);
    }

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
                // Note: The captured square will be marked dirty below as destination
                opponent[i] = POS_RESERVE;
                captured = 1;
                break;
            }
        }
    }

    // Report capture if caller provided output pointer
    if (out_captured != (void *)0) {
        *out_captured = captured;
    }

    // Mark destination square dirty (if on board)
    if (new_pos >= 1 && new_pos <= 14) {
        mark_square_dirty(player, new_pos);
    }

    // Move the piece
    pieces[piece_idx] = new_pos;

    // Return 1 if landed on rosette (extra turn)
    if (new_pos != POS_FINISHED && is_rosette(new_pos)) {
        return 1;
    }

    return 0;
}

uint8_t get_valid_moves(uint8_t player, uint8_t roll, uint8_t *out_moves) {
    uint8_t num_valid = 0;
    uint8_t reserve_added = 0;  // Track if we've already added a reserve piece
    uint8_t *pieces = (player == PLAYER_HUMAN) ? human_pieces : cpu_pieces;

    for (uint8_t i = 0; i < PIECES_PER_PLAYER; i++) {
        if (is_valid_move(player, i, roll)) {
            // If this is a reserve piece and we've already added one, skip it
            // (all reserve pieces represent the same move)
            if (pieces[i] == POS_RESERVE) {
                if (reserve_added) {
                    continue;  // Skip duplicate reserve move
                }
                reserve_added = 1;  // Mark that we've added a reserve piece
            }

            out_moves[num_valid] = i;
            num_valid++;
        }
    }

    return num_valid;
}

uint8_t get_position_screen_coords(uint8_t player, uint8_t pos, uint8_t *out_x, uint8_t *out_y) {
    if (pos < 1 || pos > 14) {
        return 0;  // Invalid position
    }

    uint8_t tile_x, tile_y;

    // Shared squares (5-12) use same coordinates for both players
    if (pos >= 5 && pos <= 12) {
        tile_x = p1_squares[pos - 1].tile_x;
        tile_y = p1_squares[pos - 1].tile_y;
    } else if (player == PLAYER_HUMAN) {
        // Human private squares (1-4, 13-14)
        tile_x = p1_squares[pos - 1].tile_x;
        tile_y = p1_squares[pos - 1].tile_y;
    } else {
        // CPU private squares (1-4, 13-14)
        uint8_t idx;
        if (pos <= 4) {
            idx = pos - 1;  // 0-3 for positions 1-4
        } else {
            idx = pos - 9;  // 4-5 for positions 13-14
        }
        tile_x = p2_private_squares[idx].tile_x;
        tile_y = p2_private_squares[idx].tile_y;
    }

    // Convert tile coordinates to sprite pixel coordinates
    // Sprite X offset is +8, Y offset is +16 on Game Boy
    *out_x = tile_x * 8 + 8;
    *out_y = tile_y * 8 + 16;

    return 1;
}

void get_reserve_screen_coords(uint8_t player, uint8_t *out_x, uint8_t *out_y) {
    if (player == PLAYER_HUMAN) {
        *out_x = (uint8_t)(HUMAN_RESERVE_TILE_X * 8 + 8);
        *out_y = (uint8_t)(HUMAN_RESERVE_TILE_Y * 8 + 16);
    } else {
        *out_x = (uint8_t)(CPU_RESERVE_TILE_X * 8 + 8);
        *out_y = (uint8_t)(CPU_RESERVE_TILE_Y * 8 + 16);
    }
}

void get_bearoff_screen_coords(uint8_t player, uint8_t *out_x, uint8_t *out_y) {
    if (player == PLAYER_HUMAN) {
        *out_x = (uint8_t)(HUMAN_BEAROFF_TILE_X * 8 + 8);
        *out_y = (uint8_t)(HUMAN_BEAROFF_TILE_Y * 8 + 16);
    } else {
        *out_x = (uint8_t)(CPU_BEAROFF_TILE_X * 8 + 8);
        *out_y = (uint8_t)(CPU_BEAROFF_TILE_Y * 8 + 16);
    }
}
