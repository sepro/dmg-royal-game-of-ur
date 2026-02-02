/**
 * game.c
 * Game board display and main gameplay state
 * Phase 6 & 7: Complete board UI with player info and dice rolling
 */

#include <gb/gb.h>
#include <stdint.h>
#include "game_types.h"
#include "game.h"
#include "coinflip.h"
#include "difficulty_select.h"
#include "opponent_data.h"
#include "font.h"
#include "input.h"
#include "board_state.h"

// External references to generated board asset
extern const uint8_t board_tiles[];
extern const unsigned char board_map[];

// External references to sprite assets
extern const uint8_t dice_white_tiles[];
extern const uint8_t dice_black_tiles[];
extern const uint8_t piece_white_tiles[];
extern const uint8_t piece_black_tiles[];

// Phase 8c: Selection and destination preview sprite assets
extern const uint8_t selection_border_tiles[];
extern const uint8_t dest_piece_white_tiles[];
extern const uint8_t dest_piece_black_tiles[];

// External references to profile assets
extern const uint8_t profile_01_tiles[];
extern const unsigned char profile_01_map[];
extern const uint8_t profile_02_tiles[];
extern const unsigned char profile_02_map[];
extern const uint8_t profile_03_tiles[];
extern const unsigned char profile_03_map[];
extern const uint8_t profile_04_tiles[];
extern const unsigned char profile_04_map[];

// External reference to opponent data (from opponent_data.h and opponent_select.h)
// profile_tile_counts, opponent_names, selected_opponent declared in included headers

// External reference to border assets
extern const uint8_t border_tiles[];
extern const unsigned char border_map[];

// External reference to next_state from main.c
extern ScreenState_t next_state;

// External references from coinflip.c
extern uint8_t selected_side;
extern uint8_t starting_player;

// ============================================================================
// Game State Variables
// ============================================================================

// Win/lose state (exported for endgame screen)
uint8_t human_won = 0;

// Player state (exported for board_state.c)
uint8_t human_color;      // SIDE_LIGHT or SIDE_DARK
uint8_t cpu_color;        // Opposite of human_color
static uint8_t current_turn;     // 0 = human, 1 = cpu

// Piece position arrays (exported for board_state.c)
uint8_t human_pieces[PIECES_PER_PLAYER];  // Position of each piece (0=reserve, 1-14=board, 15=finished)
uint8_t cpu_pieces[PIECES_PER_PLAYER];

// Piece counts (derived from position arrays)
static uint8_t human_reserve;    // Pieces not yet on board
static uint8_t human_finished;   // Pieces that completed the path
static uint8_t cpu_reserve;
static uint8_t cpu_finished;

// Dice state
static uint8_t dice_values[NUM_DICE];  // Current value of each die (0 or 1)
static uint8_t dice_total;             // Sum of dice (0-4)
static uint8_t game_phase;             // Current game phase
static uint8_t roll_timer;             // Animation frame counter
static uint8_t roll_update_counter;    // Sub-frame counter for dice updates
static uint8_t result_timer;           // Timer for result display

// Random state (LFSR)
static uint16_t rand_state;
static uint8_t frame_counter;

// Turn and time tracking (Phase 8a)
static uint16_t turn_count;          // Number of turns played
static uint16_t elapsed_frames;      // Frames since game started (for time tracking)

// Pause state (Phase 8a)
static uint8_t is_paused;            // 1 = paused, 0 = running
static uint8_t pause_animating;      // 1 = window sliding, 0 = static
static uint8_t window_y;             // Current window Y position

// Phase 8c: Move selection state
static uint8_t valid_moves[PIECES_PER_PLAYER];  // Indices of pieces with valid moves
static uint8_t num_valid_moves;                  // Number of valid moves
static uint8_t selection_index;                  // Current index into valid_moves[]
static uint8_t dest_blink_timer;                 // Timer for destination blink
static uint8_t dest_blink_visible;               // 1 = destination visible, 0 = hidden
static uint8_t selection_sprites_loaded;         // 1 = sprites loaded into VRAM

// ============================================================================
// Forward Declarations (for functions used before definition)
// ============================================================================
static void switch_turn(void);
static void update_piece_counts(void);
static uint8_t check_win_condition(void);
static void update_reserve_display(void);
static void draw_prompt(const char *text);

// ============================================================================
// Random Number Generator (Galois LFSR)
// ============================================================================

/**
 * Get a pseudo-random byte using Galois LFSR
 */
static uint8_t get_random(void) {
    uint8_t lsb = rand_state & 1;
    rand_state >>= 1;
    if (lsb) rand_state ^= 0xB400;
    return (uint8_t)(rand_state & 0xFF);
}

/**
 * Seed the random number generator
 */
static void seed_random(void) {
    rand_state = DIV_REG ^ ((uint16_t)frame_counter << 8);
    if (rand_state == 0) rand_state = 0xACE1;
}

// ============================================================================
// UI Drawing Functions
// ============================================================================

/**
 * Fill UI area (rows 10-17) with background tile
 */
static void fill_ui_area(void) {
    uint8_t row[BOARD_WIDTH];

    for (uint8_t i = 0; i < BOARD_WIDTH; i++) {
        row[i] = GAME_BG_TILE;
    }

    for (uint8_t y = UI_START_Y; y <= UI_END_Y; y++) {
        set_bkg_tiles(0, y, BOARD_WIDTH, 1, row);
    }
}

/**
 * Draw the selected opponent's portrait on the right side of UI
 */
static void draw_opponent_portrait(void) {
    const uint8_t *tiles;
    const unsigned char *map;
    uint8_t tile_count;

    // Select the correct profile based on selected_opponent
    switch (selected_opponent) {
        case 0:
            tiles = profile_01_tiles;
            map = profile_01_map;
            tile_count = profile_tile_counts[0];
            break;
        case 1:
            tiles = profile_02_tiles;
            map = profile_02_map;
            tile_count = profile_tile_counts[1];
            break;
        case 2:
            tiles = profile_03_tiles;
            map = profile_03_map;
            tile_count = profile_tile_counts[2];
            break;
        case 3:
            tiles = profile_04_tiles;
            map = profile_04_map;
            tile_count = profile_tile_counts[3];
            break;
        default:
            return;
    }

    // Load portrait tiles after board tiles
    set_bkg_data(GAME_PORTRAIT_TILE_START, tile_count, tiles);

    // Draw 5x5 portrait using tilemap
    uint8_t row_buf[GAME_PORTRAIT_WIDTH];
    for (uint8_t row = 0; row < GAME_PORTRAIT_HEIGHT; row++) {
        for (uint8_t col = 0; col < GAME_PORTRAIT_WIDTH; col++) {
            row_buf[col] = GAME_PORTRAIT_TILE_START + map[row * GAME_PORTRAIT_WIDTH + col];
        }
        set_bkg_tiles(GAME_PORTRAIT_X, GAME_PORTRAIT_Y + row, GAME_PORTRAIT_WIDTH, 1, row_buf);
    }
}

/**
 * Draw a single digit at a position (uses inverted font)
 */
static void draw_digit(uint8_t x, uint8_t y, uint8_t digit) {
    char buf[2];
    buf[0] = '0' + digit;
    buf[1] = '\0';
    draw_text_inverted(x, y, buf);
}

/**
 * Draw player info row: "CPU" or "YOU" + piece sprite + "R:N F:N"
 */
static void draw_player_info(void) {
    // Draw CPU info (row 11)
    draw_text_inverted(UI_CPU_LABEL_X, UI_CPU_LABEL_Y, "CPU");
    // Leave space for piece sprite at tile 5
    draw_text_inverted(UI_CPU_RESERVE_X, UI_CPU_RESERVE_Y, "R:");
    draw_digit(UI_CPU_RESERVE_X + 2, UI_CPU_RESERVE_Y, cpu_reserve);
    draw_text_inverted(UI_CPU_FINISH_X, UI_CPU_FINISH_Y, "F:");
    draw_digit(UI_CPU_FINISH_X + 2, UI_CPU_FINISH_Y, cpu_finished);

    // Draw human info (row 13)
    draw_text_inverted(UI_HUMAN_LABEL_X, UI_HUMAN_LABEL_Y, "YOU");
    // Leave space for piece sprite at tile 5
    draw_text_inverted(UI_HUMAN_RESERVE_X, UI_HUMAN_RESERVE_Y, "R:");
    draw_digit(UI_HUMAN_RESERVE_X + 2, UI_HUMAN_RESERVE_Y, human_reserve);
    draw_text_inverted(UI_HUMAN_FINISH_X, UI_HUMAN_FINISH_Y, "F:");
    draw_digit(UI_HUMAN_FINISH_X + 2, UI_HUMAN_FINISH_Y, human_finished);
}

/**
 * Update reserve and finished counts on screen
 */
static void update_reserve_display(void) {
    draw_digit(UI_CPU_RESERVE_X + 2, UI_CPU_RESERVE_Y, cpu_reserve);
    draw_digit(UI_CPU_FINISH_X + 2, UI_CPU_FINISH_Y, cpu_finished);
    draw_digit(UI_HUMAN_RESERVE_X + 2, UI_HUMAN_RESERVE_Y, human_reserve);
    draw_digit(UI_HUMAN_FINISH_X + 2, UI_HUMAN_FINISH_Y, human_finished);
}

/**
 * Draw turn indicator text
 */
static void draw_turn_indicator(void) {
    clear_text_row_inverted(UI_TURN_X, UI_TURN_Y, 18);

    if (current_turn == 0) {
        draw_text_inverted(UI_TURN_X, UI_TURN_Y, "YOUR TURN");
    } else {
        draw_text_inverted(UI_TURN_X, UI_TURN_Y, "CPU TURN");
    }
}

/**
 * Draw action prompt text
 */
static void draw_prompt(const char *text) {
    clear_text_row_inverted(UI_PROMPT_X, UI_PROMPT_Y, 18);
    draw_text_inverted(UI_PROMPT_X, UI_PROMPT_Y, text);
}

/**
 * Draw roll result (e.g., "ROLL: 3")
 */
static void draw_roll_result(void) {
    char buf[10];
    buf[0] = 'R';
    buf[1] = 'O';
    buf[2] = 'L';
    buf[3] = 'L';
    buf[4] = ':';
    buf[5] = ' ';
    buf[6] = '0' + dice_total;
    buf[7] = '\0';

    clear_text_row_inverted(UI_PROMPT_X, UI_PROMPT_Y, 18);
    draw_text_inverted(UI_PROMPT_X, UI_PROMPT_Y, buf);
}

// ============================================================================
// Sprite Management
// ============================================================================

/**
 * Setup piece indicator sprites in OAM
 */
static void setup_piece_sprites(void) {
    // CPU piece sprite (always at top position)
    set_sprite_tile(OAM_CPU_PIECE, (cpu_color == SIDE_LIGHT) ? SPRITE_PIECE_WHITE : SPRITE_PIECE_BLACK);
    move_sprite(OAM_CPU_PIECE, PIECE_SPRITE_X, CPU_PIECE_SPRITE_Y);

    // Human piece sprite (always at bottom position)
    set_sprite_tile(OAM_HUMAN_PIECE, (human_color == SIDE_LIGHT) ? SPRITE_PIECE_WHITE : SPRITE_PIECE_BLACK);
    move_sprite(OAM_HUMAN_PIECE, PIECE_SPRITE_X, HUMAN_PIECE_SPRITE_Y);
}

/**
 * Setup dice sprites in OAM
 */
static void setup_dice_sprites(void) {
    // Position all 4 dice in a row
    for (uint8_t i = 0; i < NUM_DICE; i++) {
        // Initialize all dice as white (will be updated during animation)
        set_sprite_tile(OAM_DICE_0 + i, SPRITE_DICE_WHITE);
        move_sprite(OAM_DICE_0 + i, DICE_START_X + (i * DICE_SPACING), DICE_Y);
    }
}

/**
 * Show dice sprites
 */
static void show_dice(void) {
    for (uint8_t i = 0; i < NUM_DICE; i++) {
        move_sprite(OAM_DICE_0 + i, DICE_START_X + (i * DICE_SPACING), DICE_Y);
    }
}

/**
 * Hide dice sprites (move off screen)
 */
static void hide_dice(void) {
    for (uint8_t i = 0; i < NUM_DICE; i++) {
        move_sprite(OAM_DICE_0 + i, 0, 0);
    }
}

/**
 * Update dice sprite tiles based on current values
 * White triangle = 0, Black triangle = 1
 */
static void update_dice_sprites(void) {
    for (uint8_t i = 0; i < NUM_DICE; i++) {
        if (dice_values[i] == 0) {
            set_sprite_tile(OAM_DICE_0 + i, SPRITE_DICE_WHITE);
        } else {
            set_sprite_tile(OAM_DICE_0 + i, SPRITE_DICE_BLACK);
        }
    }
}

// ============================================================================
// Phase 8c: Move Selection Functions
// ============================================================================

/**
 * Load selection and destination preview sprite tiles into VRAM
 */
static void load_selection_sprites(void) {
    if (selection_sprites_loaded) return;

    // Load selection border tile (1 tile, uses flip flags for corners)
    set_sprite_data(VRAM_SPRITE_SELECTION_START, VRAM_SPRITE_SELECTION_COUNT, selection_border_tiles);

    // Load destination preview tiles (4 tiles each for 16x16)
    set_sprite_data(VRAM_SPRITE_DEST_WHITE_START, VRAM_SPRITE_DEST_WHITE_COUNT, dest_piece_white_tiles);
    set_sprite_data(VRAM_SPRITE_DEST_BLACK_START, VRAM_SPRITE_DEST_BLACK_COUNT, dest_piece_black_tiles);

    selection_sprites_loaded = 1;
}

/**
 * Setup selection border OAM (4 sprites using flip flags for 16x16 border)
 */
static void setup_selection_oam(void) {
    // All 4 corners use the same tile with different flip flags
    set_sprite_tile(OAM_SELECTION_TL, VRAM_SPRITE_SELECTION_START);
    set_sprite_tile(OAM_SELECTION_TR, VRAM_SPRITE_SELECTION_START);
    set_sprite_tile(OAM_SELECTION_BL, VRAM_SPRITE_SELECTION_START);
    set_sprite_tile(OAM_SELECTION_BR, VRAM_SPRITE_SELECTION_START);

    // Set flip flags for corners
    set_sprite_prop(OAM_SELECTION_TL, 0);                      // No flip (top-left)
    set_sprite_prop(OAM_SELECTION_TR, S_FLIPX);                // Flip X (top-right)
    set_sprite_prop(OAM_SELECTION_BL, S_FLIPY);                // Flip Y (bottom-left)
    set_sprite_prop(OAM_SELECTION_BR, S_FLIPX | S_FLIPY);      // Flip both (bottom-right)
}

/**
 * Setup destination preview OAM based on current player's color
 */
static void setup_destination_oam(void) {
    uint8_t tile_start = (human_color == SIDE_LIGHT) ?
                         VRAM_SPRITE_DEST_WHITE_START :
                         VRAM_SPRITE_DEST_BLACK_START;

    // Set tiles for 16x16 destination preview (4 x 8x8 tiles)
    set_sprite_tile(OAM_DEST_TL, tile_start);
    set_sprite_tile(OAM_DEST_TR, tile_start + 1);
    set_sprite_tile(OAM_DEST_BL, tile_start + 2);
    set_sprite_tile(OAM_DEST_BR, tile_start + 3);
}

/**
 * Position a 16x16 sprite composite at given pixel coordinates
 * @param base_oam  First OAM slot (expects 4 consecutive slots)
 * @param px        Top-left X coordinate (sprite coords, +8 offset already applied)
 * @param py        Top-left Y coordinate (sprite coords, +16 offset already applied)
 */
static void position_16x16_sprite(uint8_t base_oam, uint8_t px, uint8_t py) {
    move_sprite(base_oam,     px,     py);      // Top-left
    move_sprite(base_oam + 1, px + 8, py);      // Top-right
    move_sprite(base_oam + 2, px,     py + 8);  // Bottom-left
    move_sprite(base_oam + 3, px + 8, py + 8);  // Bottom-right
}

/**
 * Hide selection and destination sprites (move off screen)
 */
static void hide_selection_sprites(void) {
    // Hide selection border
    move_sprite(OAM_SELECTION_TL, 0, 0);
    move_sprite(OAM_SELECTION_TR, 0, 0);
    move_sprite(OAM_SELECTION_BL, 0, 0);
    move_sprite(OAM_SELECTION_BR, 0, 0);

    // Hide destination preview
    move_sprite(OAM_DEST_TL, 0, 0);
    move_sprite(OAM_DEST_TR, 0, 0);
    move_sprite(OAM_DEST_BL, 0, 0);
    move_sprite(OAM_DEST_BR, 0, 0);
}

/**
 * Position selection border on the currently selected piece
 */
static void position_selection_sprite(void) {
    uint8_t piece_idx = valid_moves[selection_index];
    uint8_t pos = human_pieces[piece_idx];
    uint8_t px, py;

    if (pos == POS_RESERVE) {
        // Piece in reserve - show at reserve indicator position
        get_reserve_screen_coords(PLAYER_HUMAN, &px, &py);
    } else {
        // Piece on board - get its position
        get_position_screen_coords(PLAYER_HUMAN, pos, &px, &py);
    }

    position_16x16_sprite(OAM_SELECTION_TL, px, py);
}

/**
 * Update destination preview position and blink animation
 */
static void update_destination_preview(void) {
    uint8_t piece_idx = valid_moves[selection_index];
    uint8_t current_pos = human_pieces[piece_idx];
    uint8_t dest_pos;
    uint8_t px, py;

    // Calculate destination position
    if (current_pos == POS_RESERVE) {
        dest_pos = dice_total;  // Enter at position = roll
    } else {
        dest_pos = current_pos + dice_total;
    }

    // Get screen coordinates for destination
    if (dest_pos >= POS_FINISHED) {
        // Bearing off - show at bearoff indicator
        get_bearoff_screen_coords(PLAYER_HUMAN, &px, &py);
    } else {
        get_position_screen_coords(PLAYER_HUMAN, dest_pos, &px, &py);
    }

    // Update blink timer
    dest_blink_timer++;
    if (dest_blink_timer >= DEST_BLINK_INTERVAL) {
        dest_blink_timer = 0;
        dest_blink_visible = !dest_blink_visible;
    }

    // Position or hide destination preview based on blink state
    if (dest_blink_visible) {
        position_16x16_sprite(OAM_DEST_TL, px, py);
    } else {
        // Hide destination preview
        move_sprite(OAM_DEST_TL, 0, 0);
        move_sprite(OAM_DEST_TR, 0, 0);
        move_sprite(OAM_DEST_BL, 0, 0);
        move_sprite(OAM_DEST_BR, 0, 0);
    }
}

/**
 * Start move selection mode for human player
 */
static void start_move_selection(void) {
    // Get all valid moves
    num_valid_moves = get_valid_moves(PLAYER_HUMAN, dice_total, valid_moves);

    if (num_valid_moves == 0) {
        // No valid moves - show message and prepare to switch turn
        draw_prompt("NO VALID MOVES");
        result_timer = NO_MOVES_PAUSE;
        game_phase = PHASE_CPU_THINK;  // Reuse as wait state
        return;
    }

    // Load sprites if not already loaded
    load_selection_sprites();

    // Setup OAM for selection and destination
    setup_selection_oam();
    setup_destination_oam();

    // Initialize selection state
    selection_index = 0;
    dest_blink_timer = 0;
    dest_blink_visible = 1;

    // Position selection on first valid piece
    position_selection_sprite();

    // Show destination preview
    update_destination_preview();

    // Show prompt
    draw_prompt("SELECT PIECE");

    // Transition to move selection phase
    game_phase = PHASE_SELECT_MOVE;
}

/**
 * Update move selection (handle input each frame)
 */
static void update_move_selection(void) {
    uint8_t selection_changed = 0;

    // Handle left/right navigation
    if (input_pressed(J_LEFT)) {
        if (selection_index > 0) {
            selection_index--;
        } else {
            selection_index = num_valid_moves - 1;  // Wrap to end
        }
        selection_changed = 1;
    } else if (input_pressed(J_RIGHT)) {
        selection_index++;
        if (selection_index >= num_valid_moves) {
            selection_index = 0;  // Wrap to start
        }
        selection_changed = 1;
    }

    // Update sprites if selection changed
    if (selection_changed) {
        position_selection_sprite();
        dest_blink_timer = 0;
        dest_blink_visible = 1;  // Reset blink to visible
    }

    // Update destination preview blink animation
    update_destination_preview();

    // Handle A button - confirm selection
    if (input_pressed(J_A)) {
        uint8_t piece_idx = valid_moves[selection_index];

        // Hide selection sprites
        hide_selection_sprites();

        // Execute the move
        uint8_t extra_turn = execute_move(PLAYER_HUMAN, piece_idx, dice_total);
        update_piece_counts();
        update_reserve_display();
        update_board_display();

        // Check win condition
        if (check_win_condition()) {
            human_won = 1;  // Human won
            next_state = STATE_ENDGAME;
            return;
        }

        // Handle rosette bonus or switch turn
        if (extra_turn) {
            draw_prompt("ROSETTE! GO AGAIN");
            result_timer = RESULT_PAUSE_FRAMES;
            game_phase = PHASE_ROSETTE_BONUS;
        } else {
            switch_turn();
        }
    }
}

// ============================================================================
// Pause Screen Functions (Phase 8a)
// ============================================================================

/**
 * Draw a number (up to 3 digits) at position in window
 */
static void draw_number_win(uint8_t x, uint8_t y, uint16_t num) {
    char buf[4];
    uint8_t i = 0;

    if (num >= 100) {
        buf[i++] = '0' + (num / 100);
        num %= 100;
        buf[i++] = '0' + (num / 10);
        buf[i++] = '0' + (num % 10);
    } else if (num >= 10) {
        buf[i++] = '0' + (num / 10);
        buf[i++] = '0' + (num % 10);
    } else {
        buf[i++] = '0' + num;
    }
    buf[i] = '\0';

    draw_text_at(x, y, buf, 1);  // 1 = use window layer
}

/**
 * Draw time in MM:SS format at position in window
 */
static void draw_time_win(uint8_t x, uint8_t y, uint16_t total_frames) {
    uint16_t total_seconds = total_frames / 60;
    uint8_t minutes = total_seconds / 60;
    uint8_t seconds = total_seconds % 60;

    char buf[6];
    buf[0] = '0' + (minutes / 10);
    buf[1] = '0' + (minutes % 10);
    buf[2] = ':';
    buf[3] = '0' + (seconds / 10);
    buf[4] = '0' + (seconds % 10);
    buf[5] = '\0';

    draw_text_at(x, y, buf, 1);  // 1 = use window layer
}

/**
 * Fill window with blank tiles
 */
static void clear_pause_window(void) {
    uint8_t blank_tile = VRAM_FONT_INVERTED_START;  // Blank inverted tile
    uint8_t row[PAUSE_WIN_WIDTH];

    for (uint8_t i = 0; i < PAUSE_WIN_WIDTH; i++) {
        row[i] = blank_tile;
    }

    for (uint8_t y = 0; y < PAUSE_WIN_HEIGHT; y++) {
        set_win_tiles(0, y, PAUSE_WIN_WIDTH, 1, row);
    }
}

/**
 * Draw decorative border around pause window
 * Uses border tiles to create a frame
 */
static void draw_pause_border(void) {
    uint8_t row_buf[PAUSE_WIN_WIDTH];

    // Top row (y=0): Left corner + repeated top edges + right corner
    row_buf[0] = (uint8_t)(PAUSE_BORDER_TILE_START + 0x00);  // Top-left corner
    for (uint8_t x = 1; x < 19; x++) {
        // Repeat top edge tiles (0x01-0x05 pattern)
        row_buf[x] = (uint8_t)(PAUSE_BORDER_TILE_START + 0x01 + ((x - 1) % 5));
    }
    row_buf[19] = (uint8_t)(PAUSE_BORDER_TILE_START + 0x06);  // Top-right corner
    set_win_tiles(0, 0, PAUSE_WIN_WIDTH, 1, row_buf);

    // Middle rows (y=1-16): Left edge + white fill + right edge
    for (uint8_t y = 1; y < 17; y++) {
        row_buf[0] = (uint8_t)(PAUSE_BORDER_TILE_START + 0x07);  // Left edge
        for (uint8_t x = 1; x < 19; x++) {
            row_buf[x] = (uint8_t)(PAUSE_BORDER_TILE_START + 0x08);  // White fill
        }
        row_buf[19] = (uint8_t)(PAUSE_BORDER_TILE_START + 0x09);  // Right edge
        set_win_tiles(0, y, PAUSE_WIN_WIDTH, 1, row_buf);
    }

    // Bottom row (y=17): Left corner + repeated bottom edges + right corner
    row_buf[0] = (uint8_t)(PAUSE_BORDER_TILE_START + 0x12);  // Bottom-left corner
    for (uint8_t x = 1; x < 19; x++) {
        // Repeat bottom edge tiles (0x13-0x17 pattern)
        row_buf[x] = (uint8_t)(PAUSE_BORDER_TILE_START + 0x13 + ((x - 1) % 5));
    }
    row_buf[19] = (uint8_t)(PAUSE_BORDER_TILE_START + 0x18);  // Bottom-right corner
    set_win_tiles(0, 17, PAUSE_WIN_WIDTH, 1, row_buf);
}

/**
 * Draw the pause screen content on the window layer
 */
static void draw_pause_content(void) {
    // Draw border frame first
    draw_pause_border();

    // Title
    draw_text_at(PAUSE_TITLE_X, PAUSE_TITLE_Y, "PAUSED", 1);

    // Turn count
    draw_text_at(PAUSE_TURN_X, PAUSE_TURN_Y, "TURN:", 1);
    draw_number_win(PAUSE_TURN_X + 6, PAUSE_TURN_Y, turn_count);

    // Time played
    draw_text_at(PAUSE_TIME_X, PAUSE_TIME_Y, "TIME:", 1);
    draw_time_win(PAUSE_TIME_X + 6, PAUSE_TIME_Y, elapsed_frames);

    // Player scores
    draw_text_at(PAUSE_YOU_X, PAUSE_YOU_Y, "YOU FINISHED:", 1);
    draw_number_win(PAUSE_YOU_X + 14, PAUSE_YOU_Y, human_finished);

    draw_text_at(PAUSE_CPU_X, PAUSE_CPU_Y, "CPU FINISHED:", 1);
    draw_number_win(PAUSE_CPU_X + 14, PAUSE_CPU_Y, cpu_finished);

    // Opponent name
    draw_text_at(PAUSE_VS_X, PAUSE_VS_Y, "VS", 1);
    draw_text_at(PAUSE_VS_X + 3, PAUSE_VS_Y, opponent_names[selected_opponent], 1);

    // Difficulty (split across two lines)
    draw_text_at(PAUSE_DIFF_X, PAUSE_DIFF_Y, "DIFFICULTY:", 1);
    switch (selected_difficulty) {
        case DIFFICULTY_EASY:
            draw_text_at(PAUSE_DIFF_X + 5, PAUSE_DIFF_Y + 1, "EASY", 1);
            break;
        case DIFFICULTY_MEDIUM:
            draw_text_at(PAUSE_DIFF_X + 5, PAUSE_DIFF_Y + 1, "MEDIUM", 1);
            break;
        case DIFFICULTY_HARD:
            draw_text_at(PAUSE_DIFF_X + 5, PAUSE_DIFF_Y + 1, "HARD", 1);
            break;
    }

    // Hint to unpause
    draw_text_at(PAUSE_HINT_X, PAUSE_HINT_Y, "PRESS START", 1);
}

/**
 * Start pause sequence - begin sliding window up
 */
static void start_pause(void) {
    is_paused = 1;
    pause_animating = 1;

    // Hide sprites so they don't render on top of window
    HIDE_SPRITES;

    // Load border tiles for pause window decoration
    set_win_data(PAUSE_BORDER_TILE_START, PAUSE_BORDER_COUNT, border_tiles);

    // Draw pause content before showing
    draw_pause_content();

    // Position window at bottom (off screen) and enable it
    window_y = PAUSE_WIN_Y_HIDDEN;
    move_win(PAUSE_WIN_X, window_y);
    SHOW_WIN;
}

/**
 * Start unpause sequence - begin sliding window down
 */
static void start_unpause(void) {
    pause_animating = 1;
}

/**
 * Update pause animation (window sliding)
 * Returns 1 if still animating, 0 if done
 */
static uint8_t update_pause_animation(void) {
    if (!pause_animating) return 0;

    if (is_paused) {
        // Sliding up (showing pause screen)
        if (window_y > PAUSE_WIN_Y_VISIBLE) {
            if (window_y >= PAUSE_ANIM_SPEED) {
                window_y -= PAUSE_ANIM_SPEED;
            } else {
                window_y = PAUSE_WIN_Y_VISIBLE;
            }
            move_win(PAUSE_WIN_X, window_y);
        } else {
            pause_animating = 0;  // Animation complete
        }
    } else {
        // Sliding down (hiding pause screen)
        if (window_y < PAUSE_WIN_Y_HIDDEN) {
            window_y += PAUSE_ANIM_SPEED;
            if (window_y >= PAUSE_WIN_Y_HIDDEN) {
                window_y = PAUSE_WIN_Y_HIDDEN;
                HIDE_WIN;
                SHOW_SPRITES;  // Restore sprites when fully unpaused
                pause_animating = 0;  // Animation complete
            }
            move_win(PAUSE_WIN_X, window_y);
        } else {
            HIDE_WIN;
            SHOW_SPRITES;  // Restore sprites when fully unpaused
            pause_animating = 0;
        }
    }

    return pause_animating;
}

// ============================================================================
// Dice Rolling Logic
// ============================================================================

/**
 * Start the dice roll animation
 */
static void start_dice_roll(void) {
    // Seed random with fresh entropy
    seed_random();

    // Initialize animation state
    game_phase = PHASE_ROLLING;
    roll_timer = ROLL_ANIM_DURATION;
    roll_update_counter = 0;

    // Show dice sprites
    show_dice();

    // Clear prompt and show rolling indicator
    draw_prompt("ROLLING");
}

/**
 * Update dice during animation (randomize visuals)
 */
static void randomize_dice(void) {
    for (uint8_t i = 0; i < NUM_DICE; i++) {
        dice_values[i] = get_random() & 1;
    }
    update_dice_sprites();
}

/**
 * Generate final dice values and calculate total
 */
static void finalize_dice_roll(void) {
    dice_total = 0;
    for (uint8_t i = 0; i < NUM_DICE; i++) {
        dice_values[i] = get_random() & 1;
        dice_total += dice_values[i];
    }
    update_dice_sprites();
}

/**
 * Update the dice roll animation
 */
static void update_dice_animation(void) {
    if (roll_timer > 0) {
        roll_update_counter++;
        if (roll_update_counter >= ROLL_UPDATE_INTERVAL) {
            roll_update_counter = 0;
            randomize_dice();
        }
        roll_timer--;
    } else {
        // Animation complete, generate final result
        finalize_dice_roll();
        draw_roll_result();

        // Transition to result display phase
        game_phase = PHASE_SHOW_RESULT;
        result_timer = RESULT_PAUSE_FRAMES;
    }
}

/**
 * Switch turn to the other player
 */
static void switch_turn(void) {
    current_turn = (current_turn == 0) ? 1 : 0;
    turn_count++;  // Increment turn counter
    draw_turn_indicator();

    // Reset to wait for roll
    game_phase = PHASE_WAIT_ROLL;
    hide_dice();
    draw_prompt("PRESS A TO ROLL");
}

/**
 * Recalculate piece counts from position arrays
 */
static void update_piece_counts(void) {
    human_reserve = 0;
    human_finished = 0;
    cpu_reserve = 0;
    cpu_finished = 0;

    for (uint8_t i = 0; i < PIECES_PER_PLAYER; i++) {
        if (human_pieces[i] == POS_RESERVE) human_reserve++;
        else if (human_pieces[i] == POS_FINISHED) human_finished++;

        if (cpu_pieces[i] == POS_RESERVE) cpu_reserve++;
        else if (cpu_pieces[i] == POS_FINISHED) cpu_finished++;
    }
}

/**
 * Check if current player has won (all pieces finished)
 */
static uint8_t check_win_condition(void) {
    if (current_turn == 0) {
        return (human_finished >= PIECES_PER_PLAYER) ? 1 : 0;
    } else {
        return (cpu_finished >= PIECES_PER_PLAYER) ? 1 : 0;
    }
}

// ============================================================================
// Main Game Functions
// ============================================================================

/**
 * Initialize game screen
 */
void init_game(void) {
    // Disable display during VRAM writes
    DISPLAY_OFF;

    // Load board tiles into VRAM
    set_bkg_data(GAME_BOARD_TILE_START, GAME_BOARD_TILE_COUNT, board_tiles);

    // Load piece overlay tiles (Phase 8b)
    load_piece_tiles();

    // Draw the board tilemap at top of screen
    set_bkg_tiles(BOARD_X, BOARD_Y, BOARD_WIDTH, BOARD_HEIGHT, board_map);

    // Fill UI area with background color
    fill_ui_area();

    // Draw opponent portrait on right side
    draw_opponent_portrait();

    // Load inverted font for UI text
    load_font_inverted();

    // Load sprite tiles
    set_sprite_data(SPRITE_PIECE_WHITE, 1, piece_white_tiles);
    set_sprite_data(SPRITE_PIECE_BLACK, 1, piece_black_tiles);
    set_sprite_data(SPRITE_DICE_WHITE, 1, dice_white_tiles);
    set_sprite_data(SPRITE_DICE_BLACK, 1, dice_black_tiles);

    // Initialize game state based on coin flip
    human_color = selected_side;
    cpu_color = (selected_side == SIDE_LIGHT) ? SIDE_DARK : SIDE_LIGHT;
    current_turn = starting_player;

    // Initialize piece position arrays (Phase 8b)
    for (uint8_t i = 0; i < PIECES_PER_PLAYER; i++) {
        human_pieces[i] = POS_RESERVE;
        cpu_pieces[i] = POS_RESERVE;
    }

    // Initialize piece counts from arrays
    human_reserve = PIECES_PER_PLAYER;
    human_finished = 0;
    cpu_reserve = PIECES_PER_PLAYER;
    cpu_finished = 0;

    // Initialize game phase
    game_phase = PHASE_WAIT_ROLL;
    dice_total = 0;
    for (uint8_t i = 0; i < NUM_DICE; i++) {
        dice_values[i] = 0;
    }

    // Initialize random state
    frame_counter = 0;
    seed_random();

    // Initialize turn and time tracking (Phase 8a)
    turn_count = 1;          // Start at turn 1
    elapsed_frames = 0;

    // Initialize pause state (Phase 8a)
    is_paused = 0;
    pause_animating = 0;
    window_y = PAUSE_WIN_Y_HIDDEN;
    HIDE_WIN;  // Ensure window starts hidden

    // Initialize move selection state (Phase 8c)
    num_valid_moves = 0;
    selection_index = 0;
    selection_sprites_loaded = 0;

    // Draw UI elements
    draw_player_info();
    draw_turn_indicator();
    draw_prompt("PRESS A TO ROLL");

    // Draw initial board state (Phase 8b)
    update_board_display();

    // Setup sprites
    setup_piece_sprites();
    setup_dice_sprites();
    hide_dice();  // Start with dice hidden

    // Clear input state
    input_reset();

    // Enable display
    SHOW_SPRITES;
    SHOW_BKG;
    DISPLAY_ON;
}

/**
 * Update game screen (called every frame)
 */
void update_game(void) {
    // Increment frame counter for randomness entropy
    frame_counter++;

    // Update input state
    input_update();

    // Handle pause toggle with START button
    if (input_pressed(J_START) && !pause_animating) {
        if (is_paused) {
            // Unpause
            is_paused = 0;
            start_unpause();
        } else {
            // Pause
            start_pause();
        }
    }

    // Update pause animation if active
    if (pause_animating) {
        update_pause_animation();
        return;  // Don't update game while animating
    }

    // If paused, don't update game logic or time
    if (is_paused) {
        return;
    }

    // Increment elapsed time (only when not paused)
    elapsed_frames++;

    // Game phase state machine
    switch (game_phase) {
        case PHASE_WAIT_ROLL:
            // Human turn: wait for A press
            // CPU turn: auto-roll after brief delay
            if (current_turn == 0) {
                // Human player
                if (input_pressed(J_A)) {
                    start_dice_roll();
                }
            } else {
                // CPU player - auto-roll
                // Use frame counter to add slight delay
                if ((frame_counter & 0x1F) == 0) {
                    start_dice_roll();
                }
            }
            break;

        case PHASE_ROLLING:
            update_dice_animation();
            break;

        case PHASE_SHOW_RESULT:
            if (result_timer > 0) {
                result_timer--;
            } else {
                // Check for zero roll (no moves possible)
                if (dice_total == 0) {
                    draw_prompt("NO MOVES");
                    result_timer = RESULT_PAUSE_FRAMES;
                    game_phase = PHASE_CPU_THINK;  // Use as wait state before switch
                } else if (current_turn == 0) {
                    // Human player - start interactive move selection
                    start_move_selection();
                } else {
                    // CPU player - go to CPU move selection
                    game_phase = PHASE_SELECT_MOVE;
                    result_timer = RESULT_PAUSE_FRAMES;
                }
            }
            break;

        case PHASE_SELECT_MOVE:
            if (current_turn == 0) {
                // Human player - interactive move selection
                update_move_selection();
            } else {
                // CPU player - random move with brief delay
                if (result_timer > 0) {
                    result_timer--;
                } else {
                    uint8_t piece_idx;

                    if (find_random_valid_move(PLAYER_CPU, dice_total, &piece_idx)) {
                        uint8_t extra_turn = execute_move(PLAYER_CPU, piece_idx, dice_total);
                        update_piece_counts();
                        update_reserve_display();
                        update_board_display();

                        if (check_win_condition()) {
                            human_won = 0;  // CPU won, human lost
                            next_state = STATE_ENDGAME;
                        } else if (extra_turn) {
                            draw_prompt("ROSETTE! GO AGAIN");
                            result_timer = RESULT_PAUSE_FRAMES;
                            game_phase = PHASE_ROSETTE_BONUS;
                        } else {
                            switch_turn();
                        }
                    } else {
                        draw_prompt("NO VALID MOVES");
                        result_timer = NO_MOVES_PAUSE;
                        game_phase = PHASE_CPU_THINK;  // Use as wait state
                    }
                }
            }
            break;

        case PHASE_CPU_THINK:
            // Used as wait state after "NO VALID MOVES"
            if (result_timer > 0) {
                result_timer--;
            } else {
                switch_turn();
            }
            break;

        case PHASE_ROSETTE_BONUS:
            // Wait then go back to roll phase for extra turn
            if (result_timer > 0) {
                result_timer--;
            } else {
                game_phase = PHASE_WAIT_ROLL;
                hide_dice();
                draw_prompt("PRESS A TO ROLL");
            }
            break;

        default:
            break;
    }
}

/**
 * Cleanup game screen
 */
void cleanup_game(void) {
    HIDE_SPRITES;
}
