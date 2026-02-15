#include <gb/gb.h>
#include <stdint.h>
#include "util/falling_piece_anim.h"
#include "util/random.h"

extern const uint8_t dest_piece_white_tiles[];
extern const uint8_t dest_piece_black_tiles[];

static void move_falling_piece(uint8_t sprite_index, uint8_t sprite_x, uint8_t sprite_y) {
    move_sprite(sprite_index + 0, sprite_x, sprite_y);
    move_sprite(sprite_index + 1, sprite_x + 8u, sprite_y);
    move_sprite(sprite_index + 2, sprite_x, sprite_y + 8u);
    move_sprite(sprite_index + 3, sprite_x + 8u, sprite_y + 8u);
}

void falling_piece_anim_load_tiles(void) {
    set_sprite_data(FALLING_PIECE_WHITE_TILE_START, 4, dest_piece_white_tiles);
    set_sprite_data(FALLING_PIECE_BLACK_TILE_START, 4, dest_piece_black_tiles);
}

void falling_piece_anim_hide(FallingPieceAnimState *state) {
    state->active = 0;
    move_falling_piece(state->sprite_index, 0, 0);
    state->spawn_timer = get_random_range(FALLING_PIECE_SPAWN_DELAY_MIN, FALLING_PIECE_SPAWN_DELAY_MAX);
}

void falling_piece_anim_init(FallingPieceAnimState *state, uint8_t sprite_index) {
    state->sprite_index = sprite_index;
    falling_piece_anim_hide(state);
}

static void falling_piece_anim_spawn(FallingPieceAnimState *state) {
    state->active = 1;
    state->x = get_random_range(FALLING_PIECE_MIN_X, FALLING_PIECE_MAX_X);
    state->y_fp = FALLING_PIECE_START_Y_FP;
    state->vy_fp = FALLING_PIECE_INITIAL_VY_FP;

    state->tile_base = (get_random_range(0, 1) == 0) ? FALLING_PIECE_WHITE_TILE_START : FALLING_PIECE_BLACK_TILE_START;

    set_sprite_tile(state->sprite_index + 0, state->tile_base + 0u);
    set_sprite_tile(state->sprite_index + 1, state->tile_base + 1u);
    set_sprite_tile(state->sprite_index + 2, state->tile_base + 2u);
    set_sprite_tile(state->sprite_index + 3, state->tile_base + 3u);

    move_falling_piece(state->sprite_index, state->x, (uint8_t)((state->y_fp >> 8) + 16));
}

void falling_piece_anim_update(FallingPieceAnimState *state) {
    int32_t y_px;

    if (!state->active) {
        if (state->spawn_timer > 0) {
            state->spawn_timer--;
        } else {
            falling_piece_anim_spawn(state);
        }
        return;
    }

    state->vy_fp += FALLING_PIECE_ACCEL_FP;
    state->y_fp += state->vy_fp;

    y_px = (state->y_fp >> 8);
    if (y_px >= FALLING_PIECE_DESPAWN_Y) {
        falling_piece_anim_hide(state);
        return;
    }

    move_falling_piece(state->sprite_index, state->x, (uint8_t)(y_px + 16));
}
