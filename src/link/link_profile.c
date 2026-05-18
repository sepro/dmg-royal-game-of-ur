/**
 * link_profile.c
 * Link cable profile selection screen implementation
 * 2x2 grid of portraits for link multiplayer avatar selection,
 * followed by profile exchange over link cable and VS reveal.
 */

#include <gb/gb.h>
#include <stdint.h>
#include "game_types.h"
#include "link/link_profile.h"
#include "link/link.h"
#include "util/opponent_data.h"
#include "util/font.h"
#include "util/input.h"
#include "util/transition.h"
#include "util/portrait.h"
#include "util/screen_utils.h"
#include "screens/coinflip.h"
#include "screens/game.h"
#include "util/sound.h"
#include "util/cgb.h"

// External reference to border tiles and map
extern const uint8_t border_tiles[];
extern const unsigned char border_map[];

// External reference to next_state from main.c
extern ScreenState_t next_state;

// Exported globals
uint8_t link_local_profile = 0;
uint8_t link_remote_profile = 0;

// Local state
static uint8_t current_selection = 0;
static uint8_t phase = LPROFILE_PHASE_SELECTING;
static uint8_t phase_timer = 0;
static uint8_t dot_count = 0;
static uint8_t remote_confirmed = 0;
static uint8_t peer_cancelled = 0;

// Portrait positions
static const uint8_t portrait_x[LPROFILE_COUNT] = {
    LPROFILE_0_X, LPROFILE_1_X, LPROFILE_2_X, LPROFILE_3_X
};
static const uint8_t portrait_y[LPROFILE_COUNT] = {
    LPROFILE_0_Y, LPROFILE_1_Y, LPROFILE_2_Y, LPROFILE_3_Y
};

/**
 * Draw a portrait in the 4-portrait grid using merged tileset
 */
static void draw_portrait_at(uint8_t idx) {
    draw_portrait_expr(idx, PORTRAIT_EXPR_NORMAL, LPROFILE_PORTRAIT_START,
                       portrait_x[idx], portrait_y[idx], 0);
}

/**
 * Clear the selection border around a portrait
 */
static void clear_border(uint8_t idx) {
    uint8_t x = portrait_x[idx] - 1;
    uint8_t y = portrait_y[idx] - 1;
    clear_border_frame(x, y, LPROFILE_BORDER_WIDTH, LPROFILE_BORDER_HEIGHT, LPROFILE_BLANK_TILE);
}

/**
 * Draw the selection border around a portrait
 */
static void draw_border(uint8_t idx) {
    uint8_t x = portrait_x[idx] - 1;
    uint8_t y = portrait_y[idx] - 1;
    draw_border_frame(x, y, LPROFILE_BORDER_WIDTH, LPROFILE_BORDER_HEIGHT,
                      LPROFILE_BORDER_START, border_map);
    cgb_set_bg_attr_frame(x, y, LPROFILE_BORDER_WIDTH, LPROFILE_BORDER_HEIGHT,
                          CGB_PAL_BORDER);
}

/**
 * Update description text for selected profile
 */
static void update_description(uint8_t idx) {
    clear_text_row_inverted(LPROFILE_DESC_X, LPROFILE_DESC_Y, LPROFILE_DESC_WIDTH);
    clear_text_row_inverted(LPROFILE_DESC_X, LPROFILE_DESC_Y + 1, LPROFILE_DESC_WIDTH);
    draw_text_inverted(LPROFILE_DESC_X, LPROFILE_DESC_Y, opponent_names[idx]);
    draw_text_inverted(LPROFILE_DESC_X, LPROFILE_DESC_Y + 1, "PRESS A TO SELECT");
}

/**
 * Get grid column from index
 */
static inline uint8_t grid_col(uint8_t idx) {
    return idx & 1;
}

/**
 * Get grid row from index
 */
static inline uint8_t grid_row(uint8_t idx) {
    return idx >> 1;
}

/**
 * Try to exchange profiles over link cable
 * Sends tagged profile byte (0x80|index) so stale values can't pass validation.
 * @return 1 if exchange complete (both sides confirmed), 0 if still waiting
 */
static uint8_t try_exchange_profiles(void) {
    uint8_t recv;
    uint8_t ok;
    uint8_t tagged_profile;

    if (remote_confirmed) {
        return 1;
    }

    tagged_profile = LINK_PROFILE_TAG | link_local_profile;

    if (link_role == LINK_ROLE_MASTER) {
        ok = link_exchange(tagged_profile, &recv);
    } else {
        ok = link_exchange_slave(tagged_profile, &recv, LINK_TRANSFER_WAIT);
    }

    if (ok) {
        if (recv == LINK_CANCEL_BYTE) {
            peer_cancelled = 1;
            return 0;
        }
        if (recv >= LINK_PROFILE_TAG && recv <= (LINK_PROFILE_TAG | LINK_PROFILE_MASK)) {
            link_remote_profile = recv & LINK_PROFILE_MASK;
            remote_confirmed = 1;
        }
    }

    return remote_confirmed;
}

/**
 * Show the VS reveal screen
 */
static void show_vs_reveal(void) {
    DISPLAY_OFF;

    // Clear screen
    fill_screen_with_tile(LPROFILE_BLANK_TILE);

    // Load merged tileset once, draw both portraits from it
    load_portrait_tiles(LPROFILE_VS_YOU_TILE_START);
    draw_portrait_expr(link_local_profile, PORTRAIT_EXPR_NORMAL,
                       LPROFILE_VS_YOU_TILE_START,
                       LPROFILE_VS_YOU_X, LPROFILE_VS_YOU_Y, 0);
    draw_portrait_expr(link_remote_profile, PORTRAIT_EXPR_NORMAL,
                       LPROFILE_VS_YOU_TILE_START,
                       LPROFILE_VS_OTHER_X, LPROFILE_VS_OTHER_Y, 0);

    // Load border tiles and draw borders around both portraits
    // Border starts after portrait tiles (117 + 1 = tile 118)
    set_bkg_data(LPROFILE_VS_BORDER_START, VRAM_BORDER_COUNT, border_tiles);
    draw_border_frame(LPROFILE_VS_YOU_BRD_X, LPROFILE_VS_YOU_BRD_Y,
                      LPROFILE_BORDER_WIDTH, LPROFILE_BORDER_HEIGHT,
                      LPROFILE_VS_BORDER_START, border_map);
    cgb_set_bg_attr_frame(LPROFILE_VS_YOU_BRD_X, LPROFILE_VS_YOU_BRD_Y,
                          LPROFILE_BORDER_WIDTH, LPROFILE_BORDER_HEIGHT,
                          CGB_PAL_BORDER);
    draw_border_frame(LPROFILE_VS_OTHER_BRD_X, LPROFILE_VS_OTHER_BRD_Y,
                      LPROFILE_BORDER_WIDTH, LPROFILE_BORDER_HEIGHT,
                      LPROFILE_VS_BORDER_START, border_map);
    cgb_set_bg_attr_frame(LPROFILE_VS_OTHER_BRD_X, LPROFILE_VS_OTHER_BRD_Y,
                          LPROFILE_BORDER_WIDTH, LPROFILE_BORDER_HEIGHT,
                          CGB_PAL_BORDER);

    // Draw labels
    draw_text_inverted(LPROFILE_VS_YOU_BRD_X, LPROFILE_VS_YOU_BRD_Y - 1, "YOU");
    draw_text_inverted(LPROFILE_VS_OTHER_BRD_X, LPROFILE_VS_OTHER_BRD_Y - 1, "OTHER");
    draw_text_inverted(LPROFILE_VS_TEXT_X, LPROFILE_VS_TEXT_Y, "VS");

    // Draw names on separate lines: local left-aligned, remote right-aligned
    draw_text_inverted(LPROFILE_DESC_X, LPROFILE_VS_NAME_Y,
                       opponent_names[link_local_profile]);
    {
        const char *rname = opponent_names[link_remote_profile];
        uint8_t len = 0;
        while (rname[len]) len++;
        draw_text_inverted(20 - len, LPROFILE_VS_NAME_Y + 1, rname);
    }

    // Draw prompt
    draw_text_inverted(5, LPROFILE_VS_PROMPT_Y, "PRESS A");

    DISPLAY_ON;
}

/**
 * Initialize link profile selection screen
 */
void init_link_profile(void) {
    DISPLAY_OFF;

    // Clear stale serial port state from link_connect without resetting role
    link_soft_reset();

    // Load white tile at index 0
    set_bkg_data(LPROFILE_BLANK_TILE, 1, white_tile);

    // Clear screen
    clear_rect(LPROFILE_BLANK_TILE, 0, 0, 20, 18);

    // Load merged portrait tileset (all 4 characters share one tileset)
    load_portrait_tiles(LPROFILE_PORTRAIT_START);

    // Load border tiles
    set_bkg_data(LPROFILE_BORDER_START, VRAM_BORDER_COUNT, border_tiles);

    // Draw all 4 portraits from merged tileset
    draw_portrait_at(0);
    draw_portrait_at(1);
    draw_portrait_at(2);
    draw_portrait_at(3);

    // Initialize selection
    current_selection = 0;
    phase = LPROFILE_PHASE_SELECTING;
    phase_timer = 0;
    dot_count = 0;
    remote_confirmed = 0;
    peer_cancelled = 0;

    // Draw initial border and description
    draw_border(current_selection);

    // Draw title
    draw_text_inverted(2, 0, "CHOOSE YOUR LOOK");

    // Draw description
    update_description(current_selection);

    // Setup display
    input_reset();
    BGP_REG = 0xE4;
    HIDE_SPRITES;
    SHOW_BKG;
    DISPLAY_ON;
}

/**
 * Update selecting phase - grid navigation and A/B buttons
 */
static void update_selecting(void) {
    uint8_t new_selection = current_selection;
    uint8_t col = grid_col(current_selection);
    uint8_t row = grid_row(current_selection);

    // D-pad navigation
    if (input_pressed(J_LEFT)) {
        if (col > 0) new_selection--;
    } else if (input_pressed(J_RIGHT)) {
        if (col < 1) new_selection++;
    } else if (input_pressed(J_UP)) {
        if (row > 0) new_selection -= 2;
    } else if (input_pressed(J_DOWN)) {
        if (row < 1) new_selection += 2;
    }

    // Update selection if changed
    if (new_selection != current_selection) {
        play_sfx(SFX_CURSOR);
        clear_border(current_selection);
        current_selection = new_selection;
        draw_border(current_selection);
        update_description(current_selection);
    }

    // A: confirm selection
    if (input_pressed(J_A)) {
        play_sfx(SFX_CONFIRM);
        link_local_profile = current_selection;
        phase = LPROFILE_PHASE_WAITING;
        phase_timer = 0;
        dot_count = 0;
        remote_confirmed = 0;

        // Show waiting message
        clear_text_row_inverted(LPROFILE_DESC_X, LPROFILE_DESC_Y, LPROFILE_DESC_WIDTH);
        clear_text_row_inverted(LPROFILE_DESC_X, LPROFILE_DESC_Y + 1, LPROFILE_DESC_WIDTH);
        draw_text_inverted(LPROFILE_DESC_X, LPROFILE_DESC_Y, opponent_names[current_selection]);
        draw_text_inverted(LPROFILE_DESC_X, LPROFILE_DESC_Y + 1, "WAITING");
    }

    // B: cancel back to title
    if (input_pressed(J_B)) {
        link_cancel();
        transition_start(STATE_TITLE, TRANSITION_PHASE_COUNT_3);
    }
}

/**
 * Draw waiting dots animation
 */
static void draw_waiting_dots(void) {
    draw_waiting_text(LPROFILE_DESC_X, (uint8_t)(LPROFILE_DESC_Y + 1), dot_count);
}

/**
 * Update waiting phase - animate dots and try profile exchange
 */
static void update_waiting(void) {
    // Animate dots
    phase_timer++;
    if (phase_timer >= LPROFILE_DOT_CYCLE) {
        phase_timer = 0;
        dot_count = (dot_count + 1) % 4;
        draw_waiting_dots();
    }

    // Check if peer cancelled
    if (peer_cancelled) {
        link_cancel();
        transition_start(STATE_TITLE, TRANSITION_PHASE_COUNT_3);
        return;
    }

    // Try to exchange profiles
    if (try_exchange_profiles()) {
        // Enter syncing phase before VS reveal
        phase = LPROFILE_PHASE_SYNCING;
        phase_timer = 0;

        // Show syncing status
        clear_text_row_inverted(LPROFILE_DESC_X, LPROFILE_DESC_Y + 1, LPROFILE_DESC_WIDTH);
        draw_text_inverted(LPROFILE_DESC_X, LPROFILE_DESC_Y + 1, "SYNCING...");
    }

    // B: cancel back to title
    if (input_pressed(J_B)) {
        link_cancel();
        transition_start(STATE_TITLE, TRANSITION_PHASE_COUNT_3);
    }
}

/**
 * Update syncing phase - wait for both sides to be ready before VS reveal
 */
static void update_syncing(void) {
    uint8_t sync_result;
    phase_timer++;

    sync_result = link_ready_sync();

    if (sync_result == 1) {
        // Both sides ready, show VS reveal
        phase = LPROFILE_PHASE_VS_REVEAL;
        phase_timer = 0;
        show_vs_reveal();
    } else if (sync_result == 2 || phase_timer >= LPROFILE_SYNC_TIMEOUT) {
        // Peer cancelled or timeout
        link_cancel();
        transition_start(STATE_TITLE, TRANSITION_PHASE_COUNT_3);
    }

    // Cancel with B during sync
    if (input_pressed(J_B)) {
        link_cancel();
        transition_start(STATE_TITLE, TRANSITION_PHASE_COUNT_3);
    }
}

/**
 * Update VS reveal phase - wait for A press or timeout, then start link game
 */
static void update_vs_reveal(void) {
    phase_timer++;

    if (input_pressed(J_A) || phase_timer >= LPROFILE_VS_DURATION) {
        // Set variables that init_game() reads from coinflip screen
        selected_side = (link_role == LINK_ROLE_MASTER) ? SIDE_LIGHT : SIDE_DARK;
        starting_player = (link_role == LINK_ROLE_MASTER) ? 0 : 1;
        selected_opponent = link_remote_profile;
        game_mode = GAME_MODE_LINK;
        transition_start(STATE_GAME, TRANSITION_PHASE_COUNT_3);
    }
}

/**
 * Update link profile selection screen (called every frame)
 */
void update_link_profile(void) {
    // Handle transition animation
    if (update_transition()) {
        return;
    }

    input_update();

    switch (phase) {
        case LPROFILE_PHASE_SELECTING:
            update_selecting();
            break;
        case LPROFILE_PHASE_WAITING:
            update_waiting();
            break;
        case LPROFILE_PHASE_SYNCING:
            update_syncing();
            break;
        case LPROFILE_PHASE_VS_REVEAL:
            update_vs_reveal();
            break;
        default:
            phase = LPROFILE_PHASE_SELECTING;
            break;
    }
}

/**
 * Cleanup link profile selection screen
 */
void cleanup_link_profile(void) {
    DISPLAY_ON;
}
