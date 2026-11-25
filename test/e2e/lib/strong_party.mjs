import { makeMon } from "../recipes/route103_below_rival.mjs";

// SaveBlock1.flags is at 0x1270 (global.h); FLAG_BADGE01_GET..FLAG_BADGE08_GET are 0x867..0x86E.
const FLAGS_OFFSET = 0x1270;
const FLAG_BADGE01_GET = 0x867;
const NUM_BADGES = 8;

// With all eight badges a Pokémon with another trainer's OT obeys up to level 70. Level 50's experience (Treecko's
// medium-slow curve) is far enough from level 51 that a mid-game rival's win cannot level it up (a
// level-up would replace its inflated stats with real ones and it would lose).
const LEVEL = 50;
const EXPERIENCE = 117360;

// One strong Treecko as the whole party, and all eight badge flags.
export async function giveStrongPartyAndBadges(game) {
  const sb1 = await game.u32("gSaveBlock1Ptr");
  const mon = makeMon(LEVEL, EXPERIENCE);
  for (let i = 0; i < mon.length; i++) await game.w8("gParties", mon[i], i);
  await game.w8("gPartiesCount", 1);
  for (let flag = FLAG_BADGE01_GET; flag < FLAG_BADGE01_GET + NUM_BADGES; flag++) {
    const byte = sb1 + FLAGS_OFFSET + (flag >> 3);
    await game.w8(byte, (await game.emu.memory.read8(byte)) | (1 << (flag & 7)));
  }
}
