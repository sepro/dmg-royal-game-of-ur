#ifndef ENDGAME_H
#define ENDGAME_H

#include <stdint.h>

// Screen layout constants
#define ENDGAME_WHITE_TILE 0
#define ENDGAME_PORTRAIT_TILE_START 1
#define ENDGAME_PORTRAIT_X 7
#define ENDGAME_PORTRAIT_Y 5

// Text positions (y coordinates)
#define ENDGAME_RESULT_Y 3      // "YOU WON !" or "YOU LOST !"
#define ENDGAME_YOU_BEAT_Y 11   // "YOU BEAT"
#define ENDGAME_NAME_Y 13       // Opponent name
#define ENDGAME_PROMPT_Y 15     // "PRESS A FOR"
#define ENDGAME_PROMPT2_Y 16    // "NEW GAME"

// Function prototypes
void init_endgame(void);
void update_endgame(void);
void cleanup_endgame(void);

#endif // ENDGAME_H
