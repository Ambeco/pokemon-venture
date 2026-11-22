import { bootToOverworld, teleport } from "../lib/recipes.mjs";

// SaveBlock1.bag.items starts at 0x560; each ItemSlot is a u16 item id and a u16 quantity (global.h, struct Bag).
const BAG_ITEMS_OFFSET = 0x560;
const ITEM_HARBOR_MAIL = 200;

export const description = "As slateport_mart_at_clerk, but already holding a Harbor Mail (as if bought earlier).";

export async function build(game) {
  await bootToOverworld(game);
  const sb1 = await game.u32("gSaveBlock1Ptr");
  await game.w16(sb1 + BAG_ITEMS_OFFSET, ITEM_HARBOR_MAIL);
  await game.w16(sb1 + BAG_ITEMS_OFFSET + 2, 1);
  await teleport(game, { mapGroup: 9, mapNum: 13, x: 2, y: 3 });
}
