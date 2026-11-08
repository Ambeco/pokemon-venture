// Reusable steps for building snapshots (see recipes/).
import { DEFAULT_HOLD_FRAMES } from "./game.mjs";

// Plays through the intro and New Game by pressing A (and START on the title) until the overworld runs.
export async function bootToOverworld(game, { timeoutFrames = 9000 } = {}) {
  let i = 0;
  await game.waitUntil("the game to reach CB2_Overworld after New Game", async () => {
    await game.press(i++ % 12 < 6 ? "A" : "START", { hold: DEFAULT_HOLD_FRAMES, delay: 14 });
    return game.isOverworld();
  }, { timeoutFrames, interval: 0 });
  // Let the fade-in and the truck setup settle.
  await game.tick(180);
}

// Warps the player to a map tile without walking there: writes SaveBlock1.location/pos (a warp's effect) and
// restarts the map load. Only valid once New Game has finished, since NewGameInitData overwrites the location.
export async function teleport(game, { mapGroup, mapNum, x, y }) {
  if (!(await game.isOverworld())) throw new Error("teleport() needs the overworld to be running first (call bootToOverworld).");
  const sb1 = await game.u32("gSaveBlock1Ptr"); // moves when the heap resets, so read it fresh
  if (sb1 < 0x02000000 || sb1 >= 0x03000000) throw new Error(`gSaveBlock1Ptr = 0x${sb1.toString(16)} is not in EWRAM; the game is not in a state that can be teleported.`);

  await game.w16(sb1 + 0x00, x); // pos.x, pos.y
  await game.w16(sb1 + 0x02, y);
  await game.w8(sb1 + 0x04, mapGroup); // location: group, num, warpId (-1), x, y
  await game.w8(sb1 + 0x05, mapNum);
  await game.w8(sb1 + 0x06, 0xff);
  await game.w16(sb1 + 0x08, x);
  await game.w16(sb1 + 0x0a, y);

  await game.w32("gFieldCallback", 0); // drop the truck sequence if it is still pending
  await game.w32("gFieldCallback2", 0);
  await game.w32("gMain", game.codePointer("CB2_LoadMap"), 4);
  await game.waitUntil(`map ${mapGroup}.${mapNum} to finish loading`, () => game.isOverworld(), { timeoutFrames: 900, interval: 2 });
  await game.tick(60);
}
