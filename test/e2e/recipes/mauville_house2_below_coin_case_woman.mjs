import { bootToOverworld, teleport } from "../lib/recipes.mjs";

// MAP_MAUVILLE_CITY_HOUSE2 = (4 | (10 << 8)); the Coin Case woman stands at (4, 5).
export const description = "Fresh New Game, teleported to the Coin Case woman's house in Mauville one tile below her, with no Harbor Mail.";

export async function build(game) {
  await bootToOverworld(game);
  await teleport(game, { mapGroup: 10, mapNum: 4, x: 4, y: 6 });
}
