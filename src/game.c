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
#include "font.h"
#include "input.h"

// External references to generated board asset
extern const uint8_t board_tiles[];
extern const unsigned char board_map[];

// External references to sprite assets
extern const uint8_t dice_white_tiles[];
extern const uint8_t dice_black_tiles[];
extern const uint8_t piece_white_tiles[];
extern const uint8_t piece_black_tiles[];

// External reference to next_state from main.c
extern ScreenState_t next_state;

// External references from coinflip.c
extern uint8_t selected_side;
extern uint8_t starting_player;

// ============================================================================
// Game State Variables
// ============================================================================

// Player state
static uint8_t human_color;      // SIDE_LIGHT or SIDE_DARK
static uint8_t cpu_color;        // Opposite of human_color
static uint8_t current_turn;     // 0 = human, 1 = cpu

// Piece counts
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
    draw_turn_indicator();

    // Reset to wait for roll
    game_phase = PHASE_WAIT_ROLL;
    hide_dice();
    draw_prompt("PRESS A TO ROLL");
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

    // Draw the board tilemap at top of screen
    set_bkg_tiles(BOARD_X, BOARD_Y, BOARD_WIDTH, BOARD_HEIGHT, board_map);

    // Fill UI area with background color
    fill_ui_area();

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

    // Initialize piece counts
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

    // Draw UI elements
    draw_player_info();
    draw_turn_indicator();
    draw_prompt("PRESS A TO ROLL");

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

    // B button returns to coinflip for testing
    if (input_pressed(J_B)) {
        next_state = STATE_COINFLIP;
        return;
    }

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
                // Check for zero roll (no moves)
                if (dice_total == 0) {
                    draw_prompt("NO MOVES");
                    // Wait briefly then switch turn
                    result_timer = RESULT_PAUSE_FRAMES;
                    game_phase = PHASE_SELECT_MOVE;  // Use as intermediate state
                } else {
                    // Has moves - go to move selection (Phase 8)
                    // For now, just switch turns as placeholder
                    game_phase = PHASE_SELECT_MOVE;
                    result_timer = RESULT_PAUSE_FRAMES;
                }
            }
            break;

        case PHASE_SELECT_MOVE:
            // Placeholder for Phase 8 move selection
            // For now, just wait and switch turns
            if (result_timer > 0) {
                result_timer--;
            } else {
                switch_turn();
            }
            break;

        case PHASE_CPU_THINK:
            // Placeholder for AI (Phase 8+)
            switch_turn();
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
