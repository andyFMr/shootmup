/*---------------------------------------------------------------------------------
    Shmup Demo - Ultra Performance & Micro-DMA UI System
    Architecture: Struct of Arrays (SoA), Bitmasks, Time-Slicing, Loop Unrolling
    --andyFMr
---------------------------------------------------------------------------------*/
#include <snes.h>

#include "../res/gfx/entities/spr_enemies1.inc"
#include "../res/gfx/entities/spr_fighters1.inc"
#include "../res/gfx/entities/spr_fighters2.inc"
#include "../res/gfx/entities/spr_fx.inc"
#include "../res/gfx/stage/stage_bg1.inc"
#include "../res/gfx/stage/stage_bg2.inc"
#include "../res/gfx/stage/stage_bg3.inc"

#include "core/system.h"
#include "core/maps.h"
#include "core/entities.h"

// ==============================================================================
// ENTRY POINT
// ==============================================================================
int main(void)
{
  // --- Init Backgrounds ---
  bgInitTileSet(0, &stage_bg1_til, &stage_bg1_pal, 2, (&stage_bg1_tilend - &stage_bg1_til), (&stage_bg1_palend - &stage_bg1_pal), BG_16COLORS, 0x2000);
  bgSetMapPtr(0, 0x2800, SC_32x64);
  bgInitTileSet(1, &stage_bg2_til, &stage_bg2_pal, 4, (&stage_bg2_tilend - &stage_bg2_til), (&stage_bg2_palend - &stage_bg2_pal), BG_16COLORS, 0x3000);
  bgSetMapPtr(1, 0x6000, SC_32x64);
  bgInitTileSet(2, &stage_bg3_til, &stage_bg3_pal, 0, (&stage_bg3_tilend - &stage_bg3_til), 16 * 2 * 2, BG_16COLORS, 0x7000);
  bgSetMapPtr(2, 0x6800, SC_32x32); // BG3 is our UI layer

  WaitForVBlank();
  dmaCopyVram(&stage_bg3_map, 0x6800, 2048);
  setMode(BG_MODE1, BG3_MODE1_PRIORITY_HIGH);
  bgSetScroll(2, 0, 48);

  // --- Init OAM & Graphics ---
  oamInitGfxAttr(0x0000, OBJ_SIZE8_L16);
  dmaCopyVram((u8 *)&spr_fighters1_til, 0x0000, 0x800);
  dmaCopyVram((u8 *)&spr_fighters2_til, 0x0400, 0x800);
  dmaCopyVram((u8 *)&spr_enemies1_til, 0x1000, 0x800);
  dmaCopyVram((u8 *)&spr_fx_til, 0x0800, 0xC00);
  dmaCopyVram((u8 *)&spr_fx_til + 0x1000, 0x0C00, 0x800);

  WaitForVBlank();
  dmaCopyCGram(&spr_fighters1_pal, 128 + 1 * 16, 16 * 2);
  dmaCopyCGram(&spr_fighters2_pal, 128 + 2 * 16, 16 * 2);
  dmaCopyCGram(&spr_enemies1_pal, 128 + 3 * 16, 16 * 2);
  dmaCopyCGram(&spr_fx_pal, 128 + 0 * 16, 16 * 2);
  setPaletteColor(0, 0);

  // --- Init Scroll Engine ---
  bgMain.id = 0;
  bgMain.scrX = 255;
  bgMain.scrY = 2304 - 255 - 16;
  bgMain.vram = 0x2800;
  bgMain.vRamFirstPage = true;
  bgMain.mapPage = 7;
  bgMain.bgMap = &stage_bg1_map;
  bgMain.maxScrollY = 1;

  bgSub.id = 1;
  bgSub.scrX = 255;
  bgSub.scrY = 1760 - 255;
  bgSub.vram = 0x6000;
  bgSub.vRamFirstPage = true;
  bgSub.mapPage = 7;
  bgSub.bgMap = &stage_bg2_map;
  bgSub.maxScrollY = 1;

  updateBGs(0, &stage_bg1_map, 0x2800, 2048);
  updateBGs(1, &stage_bg2_map, 0x6000, 2048);

  updateScoreBuffers();
  updateIconBuffers();
  nmiSet(myconsoleVblank);
  bgSetScroll(0, 0, 2048 - 255);
  bgSetScroll(1, 0, 1792 - 255);

  // --- Init Entities ---
  initEntities();

  setFadeEffect(FADE_IN);

  // ==============================================================================
  // MAIN GAME LOOP
  // ==============================================================================
  while (1)
  {
    gFrames++;

    u16 pad1 = padsCurrent(0);
    u16 pad2 = padsCurrent(1);
    u8 frame_score1 = 0;
    u8 frame_score2 = 0;

    updatePlayers(pad1, pad2);
    updateItems();
    updateEnemies();
    updateProjectiles(&frame_score1, &frame_score2);

    // ------------------------------------------------------------------
    // SCORE & STATE UPDATES (Free CPU Time)
    // ------------------------------------------------------------------
    if (frame_score1)
    {
      addScore(1, frame_score1);
      upd_p1_score = 1;
    }
    if (frame_score2)
    {
      addScore(2, frame_score2);
      upd_p2_score = 1;
    }

    if (upd_p1_score || upd_p2_score)
      updateScoreBuffers();
    if (upd_p1_lives || upd_p2_lives || upd_p1_bombs || upd_p2_bombs)
      updateIconBuffers();

    // ------------------------------------------------------------------
    // VBLANK WAIT & SCROLL
    // ------------------------------------------------------------------
    WaitForVBlank();

    handleScroll(0, &bgMain);
    handleScroll(1, &bgSub);
  }
  return 0;
}