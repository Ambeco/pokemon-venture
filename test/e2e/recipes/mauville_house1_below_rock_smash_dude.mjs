import { bootToOverworld, teleport } from "../lib/recipes.mjs";

// MAP_MAUVILLE_CITY_HOUSE1 = (2 | (10 << 8)); the Rock Smash dude stands at (4, 4).
export const description = "Fresh New Game, teleported to the Rock Smash dude's house in Mauville one tile below him.";

export async function build(game) {
  await bootToOverworld(game);
  await teleport(game, { mapGroup: 10, mapNum: 2, x: 4, y: 5 });
}
