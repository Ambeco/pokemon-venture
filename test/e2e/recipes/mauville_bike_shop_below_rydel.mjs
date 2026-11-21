import { bootToOverworld, teleport } from "../lib/recipes.mjs";

// MAP_MAUVILLE_CITY_BIKE_SHOP = (1 | (10 << 8)); Rydel stands at (2, 5).
export const description = "Fresh New Game, teleported to Rydel's Cycles in Mauville one tile below him.";

export async function build(game) {
  await bootToOverworld(game);
  await teleport(game, { mapGroup: 10, mapNum: 1, x: 2, y: 6 });
}
