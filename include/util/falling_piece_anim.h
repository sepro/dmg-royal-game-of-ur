/**
 * falling_piece_anim.h
 * Falling piece title and endgame screen
 */

#ifndef FALLING_PIECE_ANIM_H
#define FALLING_PIECE_ANIM_H

#include <stdint.h>
#include "vram_layout.h"

// Shared falling destination-piece animation constants (pixel-based, size-optimized)
#define FALLING_PIECE_SPAWN_DELAY_MIN 12   // Min frames between spawns (~0.2s)
#define FALLING_PIECE_SPAWN_DELAY_MAX 80   // Max frames between spawns (~1.3s)
#define FALLING_PIECE_MIN_X 12             // Keep inside visible area
#define FALLING_PIECE_MAX_X 152
#define FALLING_PIECE_START_Y -12          // Spawn above visible area
#define FALLING_PIECE_DESPAWN_Y 144        // Remove when below screen
#define FALLING_PIECE_INITIAL_VY 2         // Initial downward velocity (px/frame)
#define FALLING_PIECE_ACCEL_TICK_MASK 0x01 // Accelerate every other frame

#define FALLING_PIECE_WHITE_TILE_START VRAM_SPRITE_GAME_START
#define FALLING_PIECE_BLACK_TILE_START (VRAM_SPRITE_GAME_START + 4)

typedef struct FallingPieceAnimState {
    uint8_t active;
    uint8_t sprite_index;
    uint8_t x;
    uint8_t spawn_timer;
    uint8_t accel_tick;
    int16_t y;
    int8_t vy;
} FallingPieceAnimState;

void falling_piece_anim_load_tiles(void);
void falling_piece_anim_init(FallingPieceAnimState *state, uint8_t sprite_index);
void falling_piece_anim_update(FallingPieceAnimState *state);
void falling_piece_anim_hide(FallingPieceAnimState *state);

#endif
