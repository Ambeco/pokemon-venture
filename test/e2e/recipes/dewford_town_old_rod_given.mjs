import { bootToOverworld } from "../lib/recipes.mjs";
import { placePlayerBelowFisherman } from "./dewford_town_below_fisherman.mjs";

// Dewford's chapter stage var: slot 0 of county (0, 3) = 0x4020 + 18 * 4 + 1 (scene.c SCENE_DEWFORD_OLD_ROD_BEAT_VAR).
const VAR_DEWFORD_OLD_ROD_BEAT = 0x4069;

export const description = "As dewford_town_below_fisherman, but the Old Rod chapter is already done (stage 1).";

export async function build(game) {
  await bootToOverworld(game);
  // Set before the map loads is not possible (teleport restarts the load), but the script reads the var at talk time.
  await placePlayerBelowFisherman(game);
  const sb1 = await game.u32("gSaveBlock1Ptr");
  await game.w16(sb1 + 0x139c + 2 * (VAR_DEWFORD_OLD_ROD_BEAT - 0x4000), 1);
}
