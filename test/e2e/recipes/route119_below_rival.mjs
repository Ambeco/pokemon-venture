import { bootToOverworld, teleport } from "../lib/recipes.mjs";
import { giveStrongPartyAndBadges } from "../lib/strong_party.mjs";

// MAP_ROUTE119 = (34 | (0 << 8)); the triggers are at (25, 31) and (26, 31), so (26, 32) is one step below.
export const description = "Fresh New Game with a level 50 Treecko and all badges, teleported to Route 119 one tile below the rival's trigger row.";

export async function build(game) {
  await bootToOverworld(game);
  await giveStrongPartyAndBadges(game);
  await teleport(game, { mapGroup: 0, mapNum: 34, x: 26, y: 32 });
}
