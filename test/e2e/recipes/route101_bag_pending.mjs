import { bootToOverworld, teleport } from "../lib/recipes.mjs";

// MAP_ROUTE101 = (16 | (0 << 8)). The rescue scene runs when the map loads with VAR_SAVE_BIRCH_BEAT at 2; the recipe
// dismisses its messages and stops once the stage is SAVE_BIRCH_AWAIT_BAG (3), with the player at (10, 19).
// SaveBlock1.vars is at 0x139C (global.h); VAR_SAVE_BIRCH_BEAT is var 0x4022, index 0x22.
const VAR_SAVE_BIRCH_BEAT_OFFSET = 0x139c + 0x22 * 2;

export const description = "Fresh New Game on Route 101 after Birch's rescue scene, waiting for the bag to be opened.";

export async function build(game) {
  await bootToOverworld(game);
  let sb1 = await game.u32("gSaveBlock1Ptr");
  await game.w16(sb1 + VAR_SAVE_BIRCH_BEAT_OFFSET, 2);
  await teleport(game, { mapGroup: 0, mapNum: 16, x: 10, y: 19 });
  for (let i = 0; i < 60; i++) {
    sb1 = await game.u32("gSaveBlock1Ptr");
    if ((await game.emu.memory.read16LE(sb1 + VAR_SAVE_BIRCH_BEAT_OFFSET)) === 3) break;
    await game.press("A", { delay: 60 });
  }
  await game.tick(120);
}
