#ifndef ENTITIES_H
#define ENTITIES_H

#include <snes.h>

extern s16 p1_x, p1_y;
extern s16 p2_x, p2_y;
extern u8 p1_lives, p1_bombs;
extern u8 p2_lives, p2_bombs;

#define MAX_SHOTS 6
extern u8 p1_shot_state[MAX_SHOTS];
extern s16 p1_shot_x[MAX_SHOTS];
extern s16 p1_shot_y[MAX_SHOTS];
extern u8 p1_volley_idx;
extern u8 p1_cooldown;

extern u8 p2_shot_state[MAX_SHOTS];
extern s16 p2_shot_x[MAX_SHOTS];
extern s16 p2_shot_y[MAX_SHOTS];
extern u8 p2_volley_idx;
extern u8 p2_cooldown;

#define MAX_ENEMIES 16
extern u16 en_active_mask;
extern s16 en_x[MAX_ENEMIES];
extern s16 en_y[MAX_ENEMIES];
extern s16 en_dx[MAX_ENEMIES];
extern s16 en_dy[MAX_ENEMIES];

#define MAX_ITEMS 6
extern u8 item_active_mask;
extern s16 item_x[MAX_ITEMS];
extern s16 item_y[MAX_ITEMS];
extern s16 item_dx[MAX_ITEMS];
extern s16 item_dy[MAX_ITEMS];

// Functions
void initEntities(void);
void updatePlayers(u16 pad1, u16 pad2);
void updateItems(void);
void updateEnemies(void);
void updateProjectiles(u8 *frame_score1, u8 *frame_score2);

#endif
