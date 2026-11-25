import { bootToOverworld, teleport } from "../lib/recipes.mjs";

// MAP_ROUTE103 = (18 | (0 << 8)); the rival stands at (10, 3) facing east, so (10, 4) is the tile below him.
const SPECIES_TREECKO = 252;
const MOVE_SCRATCH = 10;

// The player's party is gParties[B_TRAINER_PLAYER] (index 0), not the save block copy. SaveBlock1.flags is at 0x1270 (global.h).
const FLAGS_OFFSET = 0x1270;
const FLAG_HIDE_ROUTE_103_RIVAL = 723;

export const description = "Fresh New Game with a level 10 Treecko, teleported to Route 103 one tile below the visible rival.";

// Level 10 is the most a Pokémon with another trainer's OT obeys with no badges (a higher one naps instead of
// attacking). Its experience is just above level 10's start (560), so the rival's win cannot level it up, and its stats
// are set well above what level 10 gives, so it wins in a turn or two.
// A struct Pokemon (include/pokemon/pokemon.h) with personality and otId 0: the key that encrypts the substructs is
// personality ^ otId = 0, so they are stored in the clear, and 0 % 24 puts them in the order growth, attacks, EVs, misc.
export function makeMon(level = 10, experience = 600) {
  const mon = Buffer.alloc(100);
  mon[19] = 0x02; // hasSpecies (bit 1; bit 0 is isBadEgg)
  const secure = 32;
  mon.writeUInt16LE(SPECIES_TREECKO, secure); // growth: species in the low 11 bits
  mon.writeUInt32LE(experience, secure + 4); // experience (21 bits)
  mon[secure + 9] = 70; // friendship
  mon.writeUInt16LE(MOVE_SCRATCH, secure + 12); // attacks: move1 in the low 11 bits
  mon[secure + 20] = 35; // pp1
  let checksum = 0;
  for (let i = 0; i < 48; i += 2) checksum = (checksum + mon.readUInt16LE(secure + i)) & 0xffff;
  mon.writeUInt16LE(checksum, 28);
  mon[84] = level;
  mon[85] = 0xff; // mail: none
  for (const [offset, value] of [[86, 400], [88, 400], [90, 300], [92, 300], [94, 300], [96, 300], [98, 300]]) {
    mon.writeUInt16LE(value, offset); // hp, maxHP, attack, defense, speed, spAttack, spDefense
  }
  return mon;
}

export async function build(game) {
  await bootToOverworld(game);
  const sb1 = await game.u32("gSaveBlock1Ptr");
  const mon = makeMon();
  for (let i = 0; i < mon.length; i++) await game.w8("gParties", mon[i], i);
  await game.w8("gPartiesCount", 1);
  const flagByte = sb1 + FLAGS_OFFSET + (FLAG_HIDE_ROUTE_103_RIVAL >> 3);
  await game.w8(flagByte, (await game.emu.memory.read8(flagByte)) & ~(1 << (FLAG_HIDE_ROUTE_103_RIVAL & 7)));
  await teleport(game, { mapGroup: 0, mapNum: 18, x: 10, y: 4 });
}
