import { bootToOverworld, teleport } from "../lib/recipes.mjs";
import { giveStrongPartyAndBadges } from "../lib/strong_party.mjs";

// MAP_ROUTE110 = (25 | (0 << 8)); the rival waits at (34, 54) and the triggers are at (33..35, 56), so (34, 57) is
// one step below the middle trigger.
export const description = "Fresh New Game with a level 50 Treecko and all badges, teleported to Route 110 one tile below the rival's trigger row.";

export async function build(game) {
  await bootToOverworld(game);
  await giveStrongPartyAndBadges(game);
  await teleport(game, { mapGroup: 0, mapNum: 25, x: 34, y: 57 });
}
