// Board frame tiles - 13 frame-only tiles (no interior squares)
#ifndef METASPRITE_board_frame_H
#define METASPRITE_board_frame_H

#include <stdint.h>
#include <gbdk/platform.h>
#include <gbdk/metasprites.h>

#define board_frame_TILE_ORIGIN 0
#define board_frame_TILE_W 8
#define board_frame_TILE_H 8
#define board_frame_WIDTH 160
#define board_frame_HEIGHT 80
#define board_frame_TILE_COUNT 13
#define board_frame_PALETTE_COUNT 1
#define board_frame_COLORS_PER_PALETTE 4
#define board_frame_TOTAL_COLORS 4
#define board_frame_MAP_ATTRIBUTES 0

BANKREF_EXTERN(board_frame)

extern const palette_color_t board_frame_palettes[4];
extern const uint8_t board_frame_tiles[208];

extern const unsigned char board_frame_map[200];
#define board_frame_map_attributes board_frame_map

#endif
