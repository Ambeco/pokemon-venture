import { bootToOverworld, teleport } from "../lib/recipes.mjs";

// MAP_LITTLEROOT_TOWN_PROFESSOR_BIRCHS_LAB = (4 | (1 << 8)). The hand-over runs from the lab's on-frame script as
// soon as VAR_SAVE_BIRCH_BEAT is SAVE_BIRCH_STARTER_TAKEN (4); the var is written last, so the snapshot is taken as the scene starts.
const SPECIES_TREECKO = 252;
// SaveBlock1.vars is at 0x139C (global.h); VAR_SAVE_BIRCH_BEAT is var 0x4022, index 0x22.
const VAR_SAVE_BIRCH_BEAT_OFFSET = 0x139c + 0x22 * 2;

export const description = "Fresh New Game with a Treecko, teleported into Birch's Lab with the starter hand-over about to start.";

function makeMon() {
  const mon = Buffer.alloc(100);
  mon[19] = 0x02; // hasSpecies
  const secure = 32;
  mon.writeUInt16LE(SPECIES_TREECKO, secure);
  mon.writeUInt32LE(125, secure + 4);
  mon[secure + 9] = 70;
  let checksum = 0;
  for (let i = 0; i < 48; i += 2) checksum = (checksum + mon.readUInt16LE(secure + i)) & 0xffff;
  mon.writeUInt16LE(checksum, 28);
  mon[84] = 5;
  mon[85] = 0xff;
  for (const offset of [86, 88, 90, 92, 94, 96, 98]) mon.writeUInt16LE(20, offset);
  return mon;
}

export async function build(game) {
  await bootToOverworld(game);
  const mon = makeMon();
  for (let i = 0; i < mon.length; i++) await game.w8("gParties", mon[i], i);
  await game.w8("gPartiesCount", 1);
  await teleport(game, { mapGroup: 1, mapNum: 4, x: 6, y: 5 });
  const sb1 = await game.u32("gSaveBlock1Ptr");
  await game.w16(sb1 + VAR_SAVE_BIRCH_BEAT_OFFSET, 4);
}
