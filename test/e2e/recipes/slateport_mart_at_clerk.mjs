import { bootToOverworld, teleport } from "../lib/recipes.mjs";

// MAP_SLATEPORT_CITY_MART = (13 | (9 << 8)); the clerk stands at (1, 3) facing east, behind the counter.
export const description = "Fresh New Game, teleported to Slateport's Mart across the counter from the clerk, with no Harbor Mail.";

export async function build(game) {
  await bootToOverworld(game);
  await teleport(game, { mapGroup: 9, mapNum: 13, x: 2, y: 3 });
}
