#include <snes.h>
#include "maps.h"
#include "entities.h"
#include "system.h"

u8 score1[6] = {0, 0, 0, 0, 0, 0};
u8 score2[6] = {0, 0, 0, 0, 0, 0};

// WRAM Arrays (Shadow Buffers)
u16 buf_p1_score_top[6], buf_p1_score_bot[6];
u16 buf_p2_score_top[6], buf_p2_score_bot[6];
u16 buf_p1_lives_top[MAX_ICONS], buf_p1_lives_bot[MAX_ICONS];
u16 buf_p2_lives_top[MAX_ICONS], buf_p2_lives_bot[MAX_ICONS];
u16 buf_p1_bombs_top[MAX_ICONS], buf_p1_bombs_bot[MAX_ICONS];
u16 buf_p2_bombs_top[MAX_ICONS], buf_p2_bombs_bot[MAX_ICONS];

u8 upd_p1_score = 1, upd_p2_score = 1;
u8 upd_p1_lives = 1, upd_p2_lives = 1;
u8 upd_p1_bombs = 1, upd_p2_bombs = 1;

scroll bgMain;
scroll bgSub;
background bgInfo;

void addScore(u8 player, u8 pts)
{
  if (!pts)
    return;
  u8 *score_array = (player == 1) ? score1 : score2;
  score_array[0] += pts;

  u8 i = 0;
  while (score_array[i] > 9)
  {
    score_array[i] -= 10;
    i++;
    if (i > 5)
    {
      score_array[5] = 9;
      break;
    } // Cap at 999999
    score_array[i]++;
  }
}

void updateScoreBuffers()
{
  u8 i;
  if (upd_p1_score)
  {
    for (i = 0; i < 6; i++)
    {
      buf_p1_score_top[i] = TILE_ATTR(PAL_SCORE, TILE_TOP_0 + score1[5 - i]);
      buf_p1_score_bot[i] = TILE_ATTR(PAL_SCORE, TILE_BOT_0 + score1[5 - i]);
    }
  }
  if (upd_p2_score)
  {
    for (i = 0; i < 6; i++)
    {
      buf_p2_score_top[i] = TILE_ATTR(PAL_SCORE, TILE_TOP_0 + score2[5 - i]);
      buf_p2_score_bot[i] = TILE_ATTR(PAL_SCORE, TILE_BOT_0 + score2[5 - i]);
    }
  }
}

void updateIconBuffers()
{
  u8 i;
  if (upd_p1_lives)
  {
    for (i = 0; i < MAX_ICONS; i++)
    {
      u16 tile_top = (i < p1_lives) ? TILE_TOP_LIVES : TILE_BLANK;
      u16 tile_bot = (i < p1_lives) ? TILE_BOT_LIVES : TILE_BLANK;
      buf_p1_lives_top[i] = TILE_ATTR(PAL_P1_LIVES, tile_top);
      buf_p1_lives_bot[i] = TILE_ATTR(PAL_P1_LIVES, tile_bot);
    }
  }
  if (upd_p2_lives)
  {
    for (i = 0; i < MAX_ICONS; i++)
    {
      u16 tile_top = (i < p2_lives) ? TILE_TOP_LIVES : TILE_BLANK;
      u16 tile_bot = (i < p2_lives) ? TILE_BOT_LIVES : TILE_BLANK;
      buf_p2_lives_top[i] = TILE_ATTR(PAL_P2_LIVES, tile_top);
      buf_p2_lives_bot[i] = TILE_ATTR(PAL_P2_LIVES, tile_bot);
    }
  }
  if (upd_p1_bombs)
  {
    for (i = 0; i < MAX_ICONS; i++)
    {
      u16 tile_top = (i < p1_bombs) ? TILE_TOP_BOMB : TILE_BLANK;
      u16 tile_bot = (i < p1_bombs) ? TILE_BOT_BOMB : TILE_BLANK;
      buf_p1_bombs_top[i] = TILE_ATTR(PAL_P1_BOMBS, tile_top);
      buf_p1_bombs_bot[i] = TILE_ATTR(PAL_P1_BOMBS, tile_bot);
    }
  }
  if (upd_p2_bombs)
  {
    for (i = 0; i < MAX_ICONS; i++)
    {
      u16 tile_top = (i < p2_bombs) ? TILE_TOP_BOMB : TILE_BLANK;
      u16 tile_bot = (i < p2_bombs) ? TILE_BOT_BOMB : TILE_BLANK;
      buf_p2_bombs_top[i] = TILE_ATTR(PAL_P2_BOMBS, tile_top);
      buf_p2_bombs_bot[i] = TILE_ATTR(PAL_P2_BOMBS, tile_bot);
    }
  }
}

void updateBGs(int id, u8 *pgfx, u16 adrspr, int size)
{
  if (id == 0)
  {
    bgInfo.bg1.adrgfxvram = adrspr;
    bgInfo.bg1.gfxoffset = pgfx;
    bgInfo.bg1.size = size;
    bgInfo.refreshBG1 = true;
  }
  else
  {
    bgInfo.bg2.adrgfxvram = adrspr;
    bgInfo.bg2.gfxoffset = pgfx;
    bgInfo.bg2.size = size;
    bgInfo.refreshBG2 = true;
  }
}

void handleScroll(int id, scroll *s)
{
  if (s->scrY > s->maxScrollY)
  {
    if ((gFrames & 3) == 3)
      s->scrY -= 1;
  }
  if (s->scrY <= (u16)(s->mapPage << 8))
  {
    s->mapPage--;
    s->vRamFirstPage = !s->vRamFirstPage;
    u16 vram = s->vram;
    if (s->vRamFirstPage == false)
      vram += 1024;
    updateBGs(id, s->bgMap + 2048 * s->mapPage, vram, 2048);
  }

  if (id == 0 && s->scrY > 512)
    bgSetScroll(0, 0, s->scrY + 255);
  if (id == 1)
    bgSetScroll(1, 0, s->scrY + 255);
}
