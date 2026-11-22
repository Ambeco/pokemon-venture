import { bootToOverworld, teleport } from "../lib/recipes.mjs";

// MAP_DEWFORD_TOWN = (11 | (0 << 8)); the Old Rod fisherman starts at (12, 14) and wanders one tile left and right.
const MAP_OFFSET = 7;
const OBJECT_EVENT_SIZE = 0x24;
const OBJECT_EVENTS_COUNT = 16;
const MOVEMENT_TYPE_WANDER_LEFT_AND_RIGHT = 0x5;
const WALK_HOLD_FRAMES = 24;

export const description = "Fresh New Game, teleported to Dewford Town one tile below the (frozen) Old Rod fisherman, facing up.";

// The fisherman wanders, so his tile at snapshot time is not fixed: freeze him in place, then walk the player
// along the row below him to the column under him.
export async function build(game) {
  await bootToOverworld(game);
  await placePlayerBelowFisherman(game);
}

// Shared with dewford_town_old_rod_given.mjs.
export async function placePlayerBelowFisherman(game) {
  await teleport(game, { mapGroup: 0, mapNum: 11, x: 12, y: 15 });

  let fisherman = -1;
  for (let i = 0; i < OBJECT_EVENTS_COUNT; i++) {
    const base = game.addr("gObjectEvents") + i * OBJECT_EVENT_SIZE;
    const initialX = await game.emu.memory.read16LE(base + 0x0c); // initialCoords
    const initialY = await game.emu.memory.read16LE(base + 0x0e);
    if (await game.emu.memory.read8(base + 0x06) === MOVEMENT_TYPE_WANDER_LEFT_AND_RIGHT && initialX === 12 + MAP_OFFSET && initialY === 14 + MAP_OFFSET) { fisherman = base; break; }
  }
  if (fisherman < 0) throw new Error("No wandering NPC starting at (12, 14) found in gObjectEvents: is the Old Rod fisherman still there?");
  // ObjectEvent.frozen (bit 0 of byte 1) stops his wandering; a talk freezes and later unfreezes him as usual.
  await game.emu.memory.write8(fisherman + 0x01, (await game.emu.memory.read8(fisherman + 0x01)) | 0x01);
  await game.tick(40); // let a step already in progress finish

  const x = (await game.emu.memory.read16LE(fisherman + 0x10)) - MAP_OFFSET;
  const y = (await game.emu.memory.read16LE(fisherman + 0x12)) - MAP_OFFSET;
  if (y !== 14 || x < 11 || x > 13) throw new Error(`The fisherman is at (${x}, ${y}), outside the expected wander range (11-13, 14).`);
  // A short tap only turns the player; holding walks one tile.
  for (let dx = x - 12; dx !== 0; dx -= Math.sign(dx)) await game.press(dx < 0 ? "LEFT" : "RIGHT", { hold: WALK_HOLD_FRAMES });
  await game.press("UP"); // turns to face him, adjacent at (x, 15)
  await game.tick(30);
}
