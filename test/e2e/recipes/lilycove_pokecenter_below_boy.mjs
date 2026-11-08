import { bootToOverworld, teleport } from "../lib/recipes.mjs";

// MAP_LILYCOVE_CITY_POKEMON_CENTER_1F = (6 | (13 << 8)); the Boy stands at (10, 7).
export const description = "Fresh New Game, teleported to Lilycove's Pokemon Center 1F one tile below the Boy.";

export async function build(game) {
  await bootToOverworld(game);
  await teleport(game, { mapGroup: 13, mapNum: 6, x: 10, y: 8 });
}
