import { bootToOverworld, teleport } from "../lib/recipes.mjs";

// MAP_LILYCOVE_CITY = (5 | (0 << 8)); the city hinter Man3 stands at (28, 28).
export const description = "Fresh New Game, teleported to Lilycove City one tile below the hinter Man3.";

export async function build(game) {
  await bootToOverworld(game);
  await teleport(game, { mapGroup: 0, mapNum: 5, x: 28, y: 29 });
}
