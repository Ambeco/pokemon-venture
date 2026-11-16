import { bootToOverworld, teleport } from "../lib/recipes.mjs";

// MAP_MOSSDEEP_CITY_HOUSE3 = (6 | (14 << 8)); the Super Rod fisherman stands at (4, 4).
export const description = "Fresh New Game, teleported to the Super Rod fisherman's house in Mossdeep one tile below him.";

export async function build(game) {
  await bootToOverworld(game);
  await teleport(game, { mapGroup: 14, mapNum: 6, x: 4, y: 5 });
}
