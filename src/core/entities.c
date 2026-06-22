#include <snes.h>
#include "entities.h"
#include "maps.h"
#include "system.h"

s16 p1_x = 64, p1_y = 196;
s16 p2_x = 192, p2_y = 196;

u8 p1_lives = 3, p1_bombs = 3;
u8 p2_lives = 3, p2_bombs = 3;

u8 p1_shot_state[MAX_SHOTS];
s16 p1_shot_x[MAX_SHOTS];
s16 p1_shot_y[MAX_SHOTS];
u8 p1_volley_idx = 0;
u8 p1_cooldown = 0;

u8 p2_shot_state[MAX_SHOTS];
s16 p2_shot_x[MAX_SHOTS];
s16 p2_shot_y[MAX_SHOTS];
u8 p2_volley_idx = 0;
u8 p2_cooldown = 0;

u16 en_active_mask = 0xFFFF; // 16 active bits
s16 en_x[MAX_ENEMIES];
s16 en_y[MAX_ENEMIES];
s16 en_dx[MAX_ENEMIES];
s16 en_dy[MAX_ENEMIES];

u8 item_active_mask = 0x3F; // 0x3F = 00111111 (6 bits active)
s16 item_x[MAX_ITEMS];
s16 item_y[MAX_ITEMS];
s16 item_dx[MAX_ITEMS];
s16 item_dy[MAX_ITEMS];

#define MOVE_ITEM(ID, BIT, OAM_ID)                           \
  if (item_active_mask & BIT)                                \
  {                                                          \
    item_x[ID] += item_dx[ID];                               \
    item_y[ID] += item_dy[ID];                               \
    if (item_x[ID] <= 0 && item_dx[ID] < 0)                  \
      item_dx[ID] = 2;                                       \
    else if (item_x[ID] >= 232 && item_dx[ID] > 0)           \
      item_dx[ID] = -2;                                      \
    if (item_y[ID] <= 0 && item_dy[ID] < 0)                  \
      item_dy[ID] = 2;                                       \
    else if (item_y[ID] >= 208 && item_dy[ID] > 0)           \
      item_dy[ID] = -2;                                      \
    oamSet(OAM_ID, item_x[ID], item_y[ID], 3, 0, 0, 140, 0); \
  }

#define COL_P1(ID, BIT, OAM_ID)                                                   \
  if (item_active_mask & BIT)                                                     \
  {                                                                               \
    if ((u16)(p1_x - item_x[ID] + 12) < 24 && (u16)(p1_y - item_y[ID] + 12) < 24) \
    {                                                                             \
      item_active_mask &= ~BIT;                                                   \
      oamSet(OAM_ID, 0, 240, 3, 0, 0, 140, 0);                                    \
      addScore(1, 10);                                                            \
      upd_p1_score = 1;                                                           \
    }                                                                             \
  }

#define COL_P2(ID, BIT, OAM_ID)                                                   \
  if (item_active_mask & BIT)                                                     \
  {                                                                               \
    if ((u16)(p2_x - item_x[ID] + 12) < 24 && (u16)(p2_y - item_y[ID] + 12) < 24) \
    {                                                                             \
      item_active_mask &= ~BIT;                                                   \
      oamSet(OAM_ID, 0, 240, 3, 0, 0, 140, 0);                                    \
      addScore(2, 10);                                                            \
      upd_p2_score = 1;                                                           \
    }                                                                             \
  }


void initEntities()
{
  u8 i;
  oamSetEx(0, OBJ_LARGE, OBJ_SHOW);
  oamSetEx(4, OBJ_LARGE, OBJ_SHOW);
  oamSetEx(8, OBJ_LARGE, OBJ_SHOW);
  oamSetEx(12, OBJ_LARGE, OBJ_SHOW);
  oamSetEx(16, OBJ_LARGE, OBJ_SHOW);
  oamSetEx(20, OBJ_LARGE, OBJ_SHOW);

  for (i = 0; i < MAX_ENEMIES; i++)
  {
    en_x[i] = rand() % 220;
    en_y[i] = rand() % 180;
    en_dx[i] = (rand() % 2 == 0) ? 1 : -1;
    en_dy[i] = (rand() % 2 == 0) ? 1 : -1;
    oamSetEx(24 + (i * 8), OBJ_LARGE, OBJ_SHOW);
    oamSetEx(28 + (i * 8), OBJ_LARGE, OBJ_SHOW);
  }

  for (i = 0; i < MAX_SHOTS; i++)
  {
    oamSetEx(192 + (i * 12), OBJ_LARGE, OBJ_SHOW);
    oamSetEx(196 + (i * 12), OBJ_LARGE, OBJ_SHOW);
    oamSetEx(200 + (i * 12), OBJ_LARGE, OBJ_SHOW);
    oamSetEx(264 + (i * 12), OBJ_LARGE, OBJ_SHOW);
    oamSetEx(268 + (i * 12), OBJ_LARGE, OBJ_SHOW);
    oamSetEx(272 + (i * 12), OBJ_LARGE, OBJ_SHOW);

    oamSet(192 + (i * 12), 0, 240, 3, 0, 0, 192, 0);
    oamSet(196 + (i * 12), 0, 240, 3, 0, 0, 192, 0);
    oamSet(200 + (i * 12), 0, 240, 3, 0, 0, 192, 0);
    oamSet(264 + (i * 12), 0, 240, 3, 0, 0, 192, 0);
    oamSet(268 + (i * 12), 0, 240, 3, 0, 0, 192, 0);
    oamSet(272 + (i * 12), 0, 240, 3, 0, 0, 192, 0);
    p1_shot_state[i] = 0;
    p2_shot_state[i] = 0;
  }

  for (i = 0; i < MAX_ITEMS; i++)
  {
    item_x[i] = 40 + (i * 32);
    item_y[i] = 40 + (i * 24);
    item_dx[i] = (i & 1) ? 2 : -2;
    item_dy[i] = (i & 2) ? 2 : -2;
    oamSetEx(484 + (i * 4), OBJ_LARGE, OBJ_SHOW);
  }
}

void updatePlayers(u16 pad1, u16 pad2)
{
    // ------------------------------------------------------------------
    // PLAYER 1 LOGIC
    // ------------------------------------------------------------------
    if (pad1 & KEY_UP)
      p1_y -= 2;
    if (pad1 & KEY_DOWN)
      p1_y += 2;
    if (pad1 & KEY_LEFT)
      p1_x -= 2;
    if (pad1 & KEY_RIGHT)
      p1_x += 2;

    if (p1_x < 8)
      p1_x = 8;
    else if (p1_x > 232)
      p1_x = 232;
    if (p1_y < 8)
      p1_y = 8;
    else if (p1_y > 208)
      p1_y = 208;

    if ((pad1 & KEY_X) && p1_bombs > 0 && p1_cooldown == 0)
    {
      p1_bombs--;
      upd_p1_bombs = 1;
      p1_cooldown = 30;
    }

    if (p1_cooldown > 0)
      p1_cooldown--;
    if ((pad1 & (KEY_A | KEY_B | KEY_Y)) && p1_cooldown == 0)
    {
      p1_shot_state[p1_volley_idx] = 7;
      p1_shot_x[p1_volley_idx] = p1_x;
      p1_shot_y[p1_volley_idx] = p1_y - 12;
      p1_cooldown = 6;
      p1_volley_idx++;
      if (p1_volley_idx >= MAX_SHOTS)
        p1_volley_idx = 0;
    }

    // ------------------------------------------------------------------
    // PLAYER 2 LOGIC
    // ------------------------------------------------------------------
    if (pad2 & KEY_UP)
      p2_y -= 2;
    if (pad2 & KEY_DOWN)
      p2_y += 2;
    if (pad2 & KEY_LEFT)
      p2_x -= 2;
    if (pad2 & KEY_RIGHT)
      p2_x += 2;

    if (p2_x < 8)
      p2_x = 8;
    else if (p2_x > 232)
      p2_x = 232;
    if (p2_y < 8)
      p2_y = 8;
    else if (p2_y > 208)
      p2_y = 208;

    if ((pad2 & KEY_X) && p2_bombs > 0 && p2_cooldown == 0)
    {
      p2_bombs--;
      upd_p2_bombs = 1;
      p2_cooldown = 30;
    }

    if (p2_cooldown > 0)
      p2_cooldown--;
    if ((pad2 & (KEY_A | KEY_B | KEY_Y)) && p2_cooldown == 0)
    {
      p2_shot_state[p2_volley_idx] = 7;
      p2_shot_x[p2_volley_idx] = p2_x;
      p2_shot_y[p2_volley_idx] = p2_y - 12;
      p2_cooldown = 6;
      p2_volley_idx++;
      if (p2_volley_idx >= MAX_SHOTS)
        p2_volley_idx = 0;
    }

    // ------------------------------------------------------------------
    // PLAYER OAM DRAW
    // ------------------------------------------------------------------
    oamSet(8, p1_x, p1_y, 3, 0, 0, 4, 1);
    oamSet(0, p1_x - 8, p1_y + 16, 3, 0, 0, 36, 1);
    oamSet(4, p1_x + 8, p1_y + 16, 3, 0, 0, 38, 1);

    oamSet(20, p2_x, p2_y, 3, 0, 0, 68, 2);
    oamSet(12, p2_x - 8, p2_y + 16, 3, 0, 0, 100, 2);
    oamSet(16, p2_x + 8, p2_y + 16, 3, 0, 0, 102, 2);
}

void updateItems()
{
    if (item_active_mask)
    {
      // 1. Unrolled Movement
      MOVE_ITEM(0, 0x0001, 484);
      MOVE_ITEM(1, 0x0002, 488);
      MOVE_ITEM(2, 0x0004, 492);
      MOVE_ITEM(3, 0x0008, 496);
      MOVE_ITEM(4, 0x0010, 500);
      MOVE_ITEM(5, 0x0020, 504);

      // 2. Unrolled Time-sliced Collision
      if ((gFrames & 1) == 0)
      {
        COL_P1(0, 0x0001, 484);
        COL_P1(1, 0x0002, 488);
        COL_P1(2, 0x0004, 492);
        COL_P1(3, 0x0008, 496);
        COL_P1(4, 0x0010, 500);
        COL_P1(5, 0x0020, 504);
      }
      else
      {
        COL_P2(0, 0x0001, 484);
        COL_P2(1, 0x0002, 488);
        COL_P2(2, 0x0004, 492);
        COL_P2(3, 0x0008, 496);
        COL_P2(4, 0x0010, 500);
        COL_P2(5, 0x0020, 504);
      }
    }
}

void updateEnemies()
{
    u8 i;
    for (i = 0; i < MAX_ENEMIES; i++)
    {
      u16 bit_e = BIT_MASK[i];
      if (en_active_mask & bit_e)
      {
        en_x[i] += en_dx[i];
        en_y[i] += en_dy[i];

        if (en_x[i] <= 0 && en_dx[i] < 0)
          en_dx[i] = 1;
        else if (en_x[i] >= 224 && en_dx[i] > 0)
          en_dx[i] = -1;

        if (en_y[i] <= 0 && en_dy[i] < 0)
          en_dy[i] = 1;
        else if (en_y[i] >= 208 && en_dy[i] > 0)
          en_dy[i] = -1;

        oamSet(24 + (i * 8), en_x[i], en_y[i], 3, 0, 0, 256, 3);
        oamSet(28 + (i * 8), en_x[i], en_y[i] + 16, 3, 0, 0, 288, 3);
      }
      else
      {
        // Respawn logic
        en_active_mask |= bit_e;
        en_x[i] = rand() % 220;
        en_y[i] = -32;
        en_dy[i] = 1;
      }
    }
}

void updateProjectiles(u8 *frame_score1, u8 *frame_score2)
{
    u8 i, e;
    // --- P1 SHOTS ---
    for (i = 0; i < MAX_SHOTS; i++)
    {
      u8 st = p1_shot_state[i];
      if (!st)
        continue;

      p1_shot_y[i] -= 16;
      if (p1_shot_y[i] < -16)
      {
        p1_shot_state[i] = 0;
        oamSet(192 + (i * 12), 0, 240, 3, 0, 0, 192, 0);
        oamSet(196 + (i * 12), 0, 240, 3, 0, 0, 192, 0);
        oamSet(200 + (i * 12), 0, 240, 3, 0, 0, 192, 0);
        continue;
      }

      s16 by = p1_shot_y[i];
      s16 bx = p1_shot_x[i];

      // Time-sliced math
      if ((gFrames & 1) == 0)
      {
        for (e = 0; e < MAX_ENEMIES; e++)
        {
          u16 bit_e = BIT_MASK[e];
          if (en_active_mask & bit_e)
          {
            if ((u16)(by - en_y[e] + 16) < 48)
            {
              if ((u16)(bx - en_x[e] + 32) < 64)
            {
                if ((st & 1) && (u16)((bx - 14) - en_x[e] + 24) < 48)
                {
                  st &= ~1;
                  en_active_mask &= ~bit_e;
                  en_y[e] = 240;
                  (*frame_score1)++;
                }
                if ((st & 2) && (u16)(bx - en_x[e] + 24) < 48)
                {
                  st &= ~2;
                  en_active_mask &= ~bit_e;
                  en_y[e] = 240;
                  (*frame_score1)++;
                }
                if ((st & 4) && (u16)((bx + 16) - en_x[e] + 24) < 48)
                {
                  st &= ~4;
                  en_active_mask &= ~bit_e;
                  en_y[e] = 240;
                  (*frame_score1)++;
                }
                if (!st)
                  break;
              }
            }
          }
        }
      }

      p1_shot_state[i] = st;
      oamSet(192 + (i * 12), bx - 14, (st & 1) ? by : 240, 3, 0, 0, 192, 0);
      oamSet(196 + (i * 12), bx, (st & 2) ? by : 240, 3, 0, 0, 192, 0);
      oamSet(200 + (i * 12), bx + 16, (st & 4) ? by : 240, 3, 0, 0, 192, 0);
    }

    // --- P2 SHOTS ---
    for (i = 0; i < MAX_SHOTS; i++)
    {
      u8 st = p2_shot_state[i];
      if (!st)
        continue;

      p2_shot_y[i] -= 16;
      if (p2_shot_y[i] < -16)
      {
        p2_shot_state[i] = 0;
        oamSet(264 + (i * 12), 0, 240, 3, 0, 0, 192, 0);
        oamSet(268 + (i * 12), 0, 240, 3, 0, 0, 192, 0);
        oamSet(272 + (i * 12), 0, 240, 3, 0, 0, 192, 0);
        continue;
      }

      s16 by = p2_shot_y[i];
      s16 bx = p2_shot_x[i];

      // Time-sliced math
      if ((gFrames & 1) == 1)
      {
        for (e = 0; e < MAX_ENEMIES; e++)
        {
          u16 bit_e = BIT_MASK[e];
          if (en_active_mask & bit_e)
          {
            if ((u16)(by - en_y[e] + 16) < 48)
            {
              if ((u16)(bx - en_x[e] + 32) < 64)
              {
                if ((st & 1) && (u16)((bx - 16) - en_x[e] + 24) < 48)
                {
                  st &= ~1;
                  en_active_mask &= ~bit_e;
                  en_y[e] = 240;
                  (*frame_score2)++;
                }
                if ((st & 2) && (u16)(bx - en_x[e] + 24) < 48)
                {
                  st &= ~2;
                  en_active_mask &= ~bit_e;
                  en_y[e] = 240;
                  (*frame_score2)++;
                }
                if ((st & 4) && (u16)((bx + 16) - en_x[e] + 24) < 48)
                {
                  st &= ~4;
                  en_active_mask &= ~bit_e;
                  en_y[e] = 240;
                  (*frame_score2)++;
                }
                if (!st)
                  break;
              }
            }
          }
        }
      }

      p2_shot_state[i] = st;
      oamSet(264 + (i * 12), bx - 16, (st & 1) ? by : 240, 3, 0, 0, 192, 0);
      oamSet(268 + (i * 12), bx, (st & 2) ? by : 240, 3, 0, 0, 192, 0);
      oamSet(272 + (i * 12), bx + 16, (st & 4) ? by : 240, 3, 0, 0, 192, 0);
    }
}
