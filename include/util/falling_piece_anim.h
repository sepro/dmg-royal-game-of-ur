#ifndef FALLING_PIECE_ANIM_H
#define FALLING_PIECE_ANIM_H

#include <stdint.h>
#include "vram_layout.h"

// Shared falling destination-piece animation constants
#define FALLING_PIECE_SPAWN_DELAY_MIN 12   // Min frames between spawns (~0.2s)
#define FALLING_PIECE_SPAWN_DELAY_MAX 80   // Max frames between spawns (~1.3s)
#define FALLING_PIECE_MIN_X 12             // Keep inside visible area
#define FALLING_PIECE_MAX_X 152
#define FALLING_PIECE_START_Y_FP -3072     // -12 px in 8.8 fixed-point
#define FALLING_PIECE_DESPAWN_Y 144        // Remove when below 144px screen
#define FALLING_PIECE_INITIAL_VY_FP 400    // Initial downward velocity
#define FALLING_PIECE_ACCEL_FP 5           // 0.02 px/frame^2 in 8.8 fixed-point

#define FALLING_PIECE_WHITE_TILE_START VRAM_SPRITE_GAME_START
#define FALLING_PIECE_BLACK_TILE_START (VRAM_SPRITE_GAME_START + 4)

typedef struct FallingPieceAnimState {
    uint8_t active;
    uint8_t tile_base;
    uint8_t sprite_index;
    uint8_t x;
    int32_t y_fp;
    int32_t vy_fp;
    uint8_t spawn_timer;
} FallingPieceAnimState;

void falling_piece_anim_load_tiles(void);
void falling_piece_anim_init(FallingPieceAnimState *state, uint8_t sprite_index);
void falling_piece_anim_update(FallingPieceAnimState *state);
void falling_piece_anim_hide(FallingPieceAnimState *state);

#endif
