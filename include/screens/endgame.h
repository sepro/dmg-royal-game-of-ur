#ifndef ENDGAME_H
#define ENDGAME_H

#include <stdint.h>
#include "vram_layout.h"

// Screen layout constants
#define ENDGAME_WHITE_TILE 0
#define ENDGAME_PORTRAIT_TILE_START 1
#define ENDGAME_PORTRAIT_X 7
#define ENDGAME_PORTRAIT_Y 4    // Moved up 1 line from 5

// Border constants (7x7 frame around 5x5 portrait)
#define ENDGAME_BORDER_TILE_START VRAM_BORDER_START
#define ENDGAME_BORDER_WIDTH 7
#define ENDGAME_BORDER_HEIGHT 7

// Portrait animation timing
#define ENDGAME_ANIM_INTERVAL 40    // ~0.67s per expression at 60fps

// Text positions (y coordinates)
#define ENDGAME_RESULT_Y 1      // "YOU WON !" or "YOU LOST !" (moved up 2 lines from 3)
#define ENDGAME_YOU_BEAT_Y 11   // "YOU BEAT"
#define ENDGAME_NAME_Y 13       // Opponent name
#define ENDGAME_PROMPT_Y 15     // "PRESS A FOR"
#define ENDGAME_PROMPT2_Y 16    // "NEW GAME"

// Function prototypes
void init_endgame(void);
void update_endgame(void);
void cleanup_endgame(void);

#endif // ENDGAME_H
