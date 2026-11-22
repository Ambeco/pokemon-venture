import { bootToOverworld, teleport } from "../lib/recipes.mjs";

// SaveBlock1.bag.items starts at 0x560; each ItemSlot is a u16 item id and a u16 quantity (global.h, struct Bag).
const BAG_ITEMS_OFFSET = 0x560;
const ITEM_HARBOR_MAIL = 200;

export const description = "As mauville_house2_below_coin_case_woman, but with one Harbor Mail in the bag, so she offers the trade.";

export async function build(game) {
  await bootToOverworld(game);
  const sb1 = await game.u32("gSaveBlock1Ptr");
  await game.w16(sb1 + BAG_ITEMS_OFFSET, ITEM_HARBOR_MAIL);
  await game.w16(sb1 + BAG_ITEMS_OFFSET + 2, 1);
  await teleport(game, { mapGroup: 10, mapNum: 4, x: 4, y: 6 });
}
