// Thin, loud wrapper over node-mgba for driving pokeemerald by symbol name.
// Every failure names the symbol/address/frame so a failing test can be diagnosed from its message alone.
import { Mgba } from "node-mgba";
import crypto from "node:crypto";
import fs from "node:fs";
import path from "node:path";
import { fileURLToPath } from "node:url";

export const REPO_ROOT = path.resolve(path.dirname(fileURLToPath(import.meta.url)), "../../..");
export const E2E_ROOT = path.join(REPO_ROOT, "test", "e2e");
export const ROM_PATH = path.join(REPO_ROOT, "pokeemerald.gba");
export const MAP_PATH = path.join(REPO_ROOT, "pokeemerald.map");

// ~0.2s at the GBA's 59.73 fps.
export const DEFAULT_INPUT_DELAY_FRAMES = 12;
export const DEFAULT_HOLD_FRAMES = 6;
// Measured: the blinking scroll prompt changes pixels only within x 208-223, y 136-151.
export const MESSAGE_PROMPT_REGION = { x: 206, y: 134, width: 20, height: 20 };

export function romSha1() {
  return crypto.createHash("sha1").update(fs.readFileSync(ROM_PATH)).digest("hex");
}

// The .map must come from the same link as the .gba or every symbol address is wrong.
export function checkBuildIsCoherent() {
  for (const p of [ROM_PATH, MAP_PATH]) {
    if (!fs.existsSync(p)) throw new Error(`${p} is missing. Run \`make -j\` in the repo root (inside WSL) first.`);
  }
  const rom = fs.statSync(ROM_PATH).mtimeMs;
  const map = fs.statSync(MAP_PATH).mtimeMs;
  if (Math.abs(rom - map) > 60_000) {
    throw new Error(`pokeemerald.gba and pokeemerald.map were built ${Math.round(Math.abs(rom - map) / 1000)}s apart: they are not from the same link. Run \`make -j\` in the repo root.`);
  }
}

let charmap = null;
function loadCharmap() {
  if (charmap) return charmap;
  charmap = new Map();
  const text = fs.readFileSync(path.join(REPO_ROOT, "charmap.txt"), "utf8");
  for (const line of text.split(/\r?\n/)) {
    const m = /^'(.)'\s*=\s*([0-9A-Fa-f]{2})\s*(?:@.*)?$/.exec(line);
    if (m && !charmap.has(parseInt(m[2], 16))) charmap.set(parseInt(m[2], 16), m[1]);
  }
  return charmap;
}

// Decodes a game string (single-byte charmap). Control bytes: FE newline, FA scroll line, FB new page.
export function decodeGameString(bytes) {
  const map = loadCharmap();
  let out = "";
  for (let i = 0; i < bytes.length; i++) {
    const b = bytes[i];
    if (b === 0xff) break;
    if (b === 0xfe) out += "\n";
    else if (b === 0xfa) out += "\\l";
    else if (b === 0xfb) out += "\\p";
    else if (b === 0xfc || b === 0xfd) { out += `{${b.toString(16)}:${bytes[i + 1]?.toString(16)}}`; i++; }
    else out += map.get(b) ?? `{?${b.toString(16)}}`;
  }
  return out;
}

export class Game {
  constructor(emu) {
    this.emu = emu;
    this.frame = 0;
  }

  static async open() {
    checkBuildIsCoherent();
    const emu = await Mgba.load(ROM_PATH, { batterySavePath: false });
    await emu.symbols.loadGnuMapFile(MAP_PATH);
    return new Game(emu);
  }

  async close() { await this.emu.close(); }

  addr(symbol, offset = 0) {
    if (typeof symbol === "number") return symbol + offset;
    const rec = this.emu.symbols.resolve(symbol);
    if (!rec) throw new Error(`Symbol ${symbol} is not in pokeemerald.map. It may be static (not listed), optimized out, or the ROM is stale: run \`make -j\`.`);
    return rec.address + offset;
  }

  // Function pointers in the ROM are Thumb: the map lists the even address, but the stored pointer has bit 0 set.
  codePointer(symbol) { return this.addr(symbol) | 1; }

  async u8(symbol, offset = 0) { return this.emu.memory.read8(this.addr(symbol, offset)); }
  async u16(symbol, offset = 0) { return this.emu.memory.read16LE(this.addr(symbol, offset)); }
  async u32(symbol, offset = 0) { return (await this.emu.memory.read32LE(this.addr(symbol, offset))) >>> 0; }
  async bytes(symbol, length, offset = 0) { return this.emu.memory.slice(this.addr(symbol, offset), length); }

  async w8(symbol, value, offset = 0) { await this.emu.memory.write8(this.addr(symbol, offset), value & 0xff); }
  async w16(symbol, value, offset = 0) { await this.w8(symbol, value, offset); await this.w8(symbol, value >> 8, offset + 1); }
  async w32(symbol, value, offset = 0) { await this.w16(symbol, value & 0xffff, offset); await this.w16(symbol, (value >>> 16) & 0xffff, offset + 2); }

  async gameString(symbol, maxLength = 256, offset = 0) {
    return decodeGameString(await this.bytes(symbol, maxLength, offset));
  }

  async tick(frames) { await this.emu.controls.tick(frames); this.frame += frames; }

  // Waits `delay` frames, then holds the button for `hold` frames.
  async press(button, { hold = DEFAULT_HOLD_FRAMES, delay = DEFAULT_INPUT_DELAY_FRAMES } = {}) {
    if (delay > 0) await this.tick(delay);
    await this.emu.controls.press(button, hold);
    this.frame += hold;
  }

  // Polls `condition` every `interval` frames (0 = the condition advances the emulator itself); throws,
  // naming `what`, if it never holds within `timeoutFrames` emulator frames.
  async waitUntil(what, condition, { timeoutFrames = 1800, interval = 4 } = {}) {
    const start = this.frame;
    while (this.frame - start <= timeoutFrames) {
      if (await condition()) return this.frame - start;
      if (interval > 0) await this.tick(interval);
    }
    throw new Error(`Timed out after ${timeoutFrames} frames waiting for: ${what} (emulator frame ${this.frame})`);
  }

  async callback2() { return this.u32("gMain", 4); }
  async isOverworld() { return (await this.callback2()) === this.codePointer("CB2_Overworld"); }

  async screenshot(box) {
    return box ? this.emu.screen.crop(box, "png") : this.emu.screen.toPng();
  }

  // Waits until the frame stays unchanged for `stableFrames` frames (text finished printing, fades done).
  // `ignore` regions are blanked first; the default covers the message box's blinking scroll prompt.
  async waitForStableScreen({ stableFrames = 30, timeoutFrames = 1800, ignore = [MESSAGE_PROMPT_REGION] } = {}) {
    const W = 240;
    const raw = async () => {
      const buf = Buffer.from(await this.emu.screen.crop({ x: 0, y: 0, width: W, height: 160 }, "raw"));
      for (const r of ignore) {
        for (let y = r.y; y < r.y + r.height; y++) buf.fill(0, (y * W + r.x) * 4, (y * W + r.x + r.width) * 4);
      }
      return buf;
    };
    let prev = await raw();
    let still = 0;
    for (let waited = 0; waited <= timeoutFrames; waited += 2) {
      await this.tick(2);
      const cur = await raw();
      still = Buffer.compare(prev, cur) === 0 ? still + 2 : 0;
      prev = cur;
      if (still >= stableFrames) return;
    }
    throw new Error(`Screen never stayed unchanged for ${stableFrames} frames within ${timeoutFrames} frames (emulator frame ${this.frame}); is something animating or a script still running?`);
  }

  async saveSnapshot(file) {
    fs.mkdirSync(path.dirname(file), { recursive: true });
    if (!(await this.emu.states.saveToFile(file))) throw new Error(`Could not write savestate ${file}`);
  }

  async loadSnapshot(file) {
    if (!fs.existsSync(file)) throw new Error(`Snapshot ${file} does not exist.`);
    if (!(await this.emu.states.loadFromFile(file))) throw new Error(`libmgba rejected savestate ${file}: it was probably made by a different ROM build. Delete test/e2e/cache and rerun.`);
  }
}
