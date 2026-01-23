/**
 * title.h
 * Title screen state management
 */

#ifndef TITLE_H
#define TITLE_H

#include <stdint.h>

/**
 * Initialize title screen state
 * Loads background tiles and map, sets up arrow sprite
 */
void init_title(void);

/**
 * Update title screen state (called every frame)
 * Handles input and arrow movement
 */
void update_title(void);

/**
 * Cleanup title screen state
 * Called when transitioning to another state
 */
void cleanup_title(void);

#endif // TITLE_H
