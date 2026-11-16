import { bootToOverworld, teleport } from "../lib/recipes.mjs";

// SaveBlock1.bag.keyItems: the bag starts at 0x560 and holds 30 items slots of 4 bytes first (global.h, struct Bag);
// BAG_KEYITEMS_COUNT is 30. A slot with a non-zero item id is taken, whatever its quantity.
const KEY_ITEMS_OFFSET = 0x560 + 30 * 4;
const KEY_ITEMS_COUNT = 30;

export const description = "As mossdeep_house3_below_fisherman, but all 30 Key Items slots are taken, so the Super Rod cannot be received.";

export async function build(game) {
  await bootToOverworld(game);
  const sb1 = await game.u32("gSaveBlock1Ptr");
  for (let i = 0; i < KEY_ITEMS_COUNT; i++) {
    await game.w16(sb1 + KEY_ITEMS_OFFSET + i * 4, 1 + i); // item ids 1..30, none of them the Super Rod
    await game.w16(sb1 + KEY_ITEMS_OFFSET + i * 4 + 2, 0);
  }
  await teleport(game, { mapGroup: 14, mapNum: 6, x: 4, y: 5 });
}
