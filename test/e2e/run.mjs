// End-to-end test runner. Usage: node run.mjs [--filter text] [--update-golden] [--rebuild-snapshots]
// See README.md for the test file format.
import crypto from "node:crypto";
import fs from "node:fs";
import path from "node:path";
import { pathToFileURL } from "node:url";
import sharp from "sharp";
import { Game, E2E_ROOT, DEFAULT_INPUT_DELAY_FRAMES, DEFAULT_HOLD_FRAMES, MESSAGE_PROMPT_REGION, romSha1, checkBuildIsCoherent } from "./lib/game.mjs";

const args = process.argv.slice(2);
const flag = (name) => args.includes(name);
const filter = args.includes("--filter") ? args[args.indexOf("--filter") + 1] : "";
const updateGolden = flag("--update-golden");
const rebuildSnapshots = flag("--rebuild-snapshots");

const sha = (text) => crypto.createHash("sha1").update(text).digest("hex");
const safeName = (s) => s.replace(/[^A-Za-z0-9_.-]+/g, "_").slice(0, 80);

async function snapshotPath(recipeName) {
  const recipeFile = path.join(E2E_ROOT, "recipes", `${recipeName}.mjs`);
  if (!fs.existsSync(recipeFile)) {
    const known = fs.readdirSync(path.join(E2E_ROOT, "recipes")).map((f) => f.replace(/\.mjs$/, ""));
    throw new Error(`Snapshot recipe "${recipeName}" not found at ${recipeFile}. Known recipes: ${known.join(", ")}`);
  }
  // Savestates embed ROM addresses, so the key includes the ROM hash and the recipe's code.
  const code = fs.readFileSync(recipeFile, "utf8") + fs.readFileSync(path.join(E2E_ROOT, "lib", "recipes.mjs"), "utf8") + fs.readFileSync(path.join(E2E_ROOT, "lib", "game.mjs"), "utf8");
  const file = path.join(E2E_ROOT, "cache", `${recipeName}.${romSha1().slice(0, 10)}.${sha(code).slice(0, 10)}.ss`);
  if (rebuildSnapshots || !fs.existsSync(file)) {
    const game = await Game.open();
    try {
      const recipe = await import(pathToFileURL(recipeFile).href);
      await recipe.build(game);
      await game.saveSnapshot(file);
      console.log(`  built snapshot ${path.basename(file)} (${game.frame} frames)`);
    } finally {
      await game.close();
    }
  }
  return file;
}

function parseHex(s) {
  const clean = s.replace(/[^0-9a-fA-F]/g, "");
  if (clean.length % 2) throw new Error(`Hex string "${s}" has an odd number of digits.`);
  return Buffer.from(clean, "hex");
}

// Frames to run after the last step before checking anything. Fixed, not "until still": water, flowers and
// the message prompt animate forever, and the emulator is deterministic, so a frame count is reproducible.
const DEFAULT_SETTLE_FRAMES = 120;

async function runSteps(game, test) {
  const delay = test.inputDelayFrames ?? DEFAULT_INPUT_DELAY_FRAMES;
  for (const [i, step] of (test.steps ?? []).entries()) {
    const where = `step ${i + 1} ${JSON.stringify(step)}`;
    try {
      if (step.press) await game.press(step.press, { hold: step.hold ?? DEFAULT_HOLD_FRAMES, delay: step.delay ?? delay });
      else if (step.wait !== undefined) await game.tick(step.wait);
      else if (step.waitStable) await game.waitForStableScreen(typeof step.waitStable === "object" ? step.waitStable : {});
      else throw new Error(`Unknown step. Use {"press": "A"}, {"wait": frames} or {"waitStable": true}.`);
    } catch (e) {
      throw new Error(`${where}: ${e.message}`);
    }
  }
  await game.tick(test.settleFrames ?? DEFAULT_SETTLE_FRAMES);
}

async function checkMemory(game, expect) {
  const failures = [];
  for (const e of expect ?? []) {
    const target = e.symbol ?? e.address;
    const label = `${e.symbol ?? "0x" + e.address.toString(16)}${e.offset ? "+" + e.offset : ""}`;
    if (e.text !== undefined) {
      const actual = await game.gameString(target, Math.max(256, e.text.length * 2), e.offset ?? 0);
      if (actual !== e.text) failures.push(`${label}: expected text ${JSON.stringify(e.text)} but the game holds ${JSON.stringify(actual)}`);
    } else if (e.bytes !== undefined) {
      const want = parseHex(e.bytes);
      const got = await game.bytes(target, want.length, e.offset ?? 0);
      if (Buffer.compare(want, Buffer.from(got)) !== 0) failures.push(`${label}: expected bytes ${want.toString("hex")} but found ${Buffer.from(got).toString("hex")}`);
    } else {
      const width = ["u8", "u16", "u32"].find((k) => e[k] !== undefined);
      if (!width) throw new Error(`Expectation ${JSON.stringify(e)} needs one of text, bytes, u8, u16, u32.`);
      const got = await game[width](target, e.offset ?? 0);
      if (got !== e[width]) failures.push(`${label}: expected ${width} ${e[width]} (0x${e[width].toString(16)}) but found ${got} (0x${got.toString(16)})`);
    }
  }
  return failures;
}

async function checkScreenshot(game, spec, outDir) {
  const box = spec.region;
  const decode = async (buf) => sharp(buf).ensureAlpha().raw().toBuffer({ resolveWithObject: true });
  // Animated parts (the message prompt by default) are blanked in both the golden and the capture.
  const ignore = spec.ignore ?? [MESSAGE_PROMPT_REGION];
  const grabbed = await decode(Buffer.from(await game.emu.screen.crop(box, "png")));
  const { width, height } = grabbed.info;
  for (const r of ignore) {
    for (let y = Math.max(r.y, box.y); y < Math.min(r.y + r.height, box.y + height); y++) {
      const x0 = Math.max(r.x, box.x), x1 = Math.min(r.x + r.width, box.x + width);
      if (x1 > x0) grabbed.data.fill(0, ((y - box.y) * width + (x0 - box.x)) * 4, ((y - box.y) * width + (x1 - box.x)) * 4);
    }
  }
  const crop = await sharp(grabbed.data, { raw: { width, height, channels: 4 } }).png().toBuffer();
  const goldenFile = path.join(E2E_ROOT, spec.golden);
  if (updateGolden) {
    fs.mkdirSync(path.dirname(goldenFile), { recursive: true });
    fs.writeFileSync(goldenFile, crop);
    return { updated: true };
  }
  if (!fs.existsSync(goldenFile)) return { failure: `golden image ${spec.golden} does not exist. Review ${path.relative(E2E_ROOT, outDir)}/actual_region.png, then run with --update-golden to create it.`, crop };
  const want = await decode(fs.readFileSync(goldenFile));
  const got = grabbed;
  if (want.info.width !== got.info.width || want.info.height !== got.info.height) {
    return { failure: `golden is ${want.info.width}x${want.info.height} but the region is ${got.info.width}x${got.info.height}`, crop };
  }
  let diffPixels = 0;
  const diff = Buffer.alloc(want.data.length);
  for (let i = 0; i < want.data.length; i += 4) {
    const same = want.data[i] === got.data[i] && want.data[i + 1] === got.data[i + 1] && want.data[i + 2] === got.data[i + 2];
    if (!same) diffPixels++;
    diff[i] = same ? 0 : 255; diff[i + 1] = 0; diff[i + 2] = same ? 0 : 255; diff[i + 3] = 255;
  }
  if (diffPixels === 0) return {};
  fs.writeFileSync(path.join(outDir, "diff.png"), await sharp(diff, { raw: { width: want.info.width, height: want.info.height, channels: 4 } }).png().toBuffer());
  return { failure: `${diffPixels} of ${want.info.width * want.info.height} pixels differ from ${spec.golden} (see ${path.relative(E2E_ROOT, outDir)}/diff.png)`, crop };
}

async function dumpDiagnostics(game, outDir, crop, report) {
  fs.mkdirSync(outDir, { recursive: true });
  fs.writeFileSync(path.join(outDir, "report.txt"), report);
  try {
    fs.writeFileSync(path.join(outDir, "actual_full.png"), await game.screenshot());
    if (crop) fs.writeFileSync(path.join(outDir, "actual_region.png"), crop);
    fs.writeFileSync(path.join(outDir, "ewram.bin"), await game.emu.memory.readRegion("EWRAM", 0, 0x40000));
    fs.writeFileSync(path.join(outDir, "iwram.bin"), await game.emu.memory.readRegion("IWRAM", 0, 0x8000));
    await game.saveSnapshot(path.join(outDir, "final.ss"));
  } catch (e) {
    fs.appendFileSync(path.join(outDir, "report.txt"), `\n(could not write all diagnostics: ${e.message})\n`);
  }
}

async function runTest(test) {
  const outDir = path.join(E2E_ROOT, "out", safeName(test.name));
  fs.rmSync(outDir, { recursive: true, force: true });
  const snapshot = await snapshotPath(test.snapshot);
  const game = await Game.open();
  let crop;
  try {
    await game.loadSnapshot(snapshot);
    await runSteps(game, test);
    const failures = await checkMemory(game, test.expect);
    if (test.screenshot) {
      fs.mkdirSync(outDir, { recursive: true });
      const r = await checkScreenshot(game, test.screenshot, outDir);
      crop = r.crop;
      if (r.failure) failures.push(`screenshot: ${r.failure}`);
      if (r.updated) console.log(`  updated golden ${test.screenshot.golden}`);
    }
    if (failures.length) {
      await dumpDiagnostics(game, outDir, crop, `${test.name}\n${failures.join("\n")}\n`);
      return failures;
    }
    fs.rmSync(outDir, { recursive: true, force: true });
    return [];
  } catch (e) {
    await dumpDiagnostics(game, outDir, crop, `${test.name}\nERROR: ${e.message}\n${e.stack}\n`);
    return [`ERROR: ${e.message}`];
  } finally {
    await game.close();
  }
}

async function main() {
  checkBuildIsCoherent();
  const dir = path.join(E2E_ROOT, "tests");
  const tests = fs.readdirSync(dir).filter((f) => f.endsWith(".json")).flatMap((f) => {
    const parsed = JSON.parse(fs.readFileSync(path.join(dir, f), "utf8"));
    return (Array.isArray(parsed) ? parsed : [parsed]).map((t) => ({ ...t, file: f }));
  }).filter((t) => t.name.includes(filter));
  if (!tests.length) { console.error(`No tests match filter "${filter}" in ${dir}`); process.exit(2); }

  let failed = 0;
  for (const test of tests) {
    const t0 = Date.now();
    const failures = await runTest(test);
    const secs = ((Date.now() - t0) / 1000).toFixed(1);
    if (failures.length) { failed++; console.log(`FAIL ${test.name} (${secs}s)`); for (const f of failures) console.log(`  - ${f}`); }
    else console.log(`PASS ${test.name} (${secs}s)`);
  }
  console.log(`\n${tests.length - failed}/${tests.length} passed`);
  process.exit(failed ? 1 : 0);
}

main().catch((e) => { console.error(e.stack ?? e); process.exit(2); });
