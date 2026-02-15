/**
 * title.h
 * Title screen state management
 */

#ifndef TITLE_H
#define TITLE_H

/**
 * Initialize title screen state
 */
void init_title(void);

/**
 * Update title screen state (called every frame)
 */
void update_title(void);

/**
 * Cleanup title screen state
 */
void cleanup_title(void);

#endif // TITLE_H
