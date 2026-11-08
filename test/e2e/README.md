# End-to-end tests

Drive the real built ROM (`pokeemerald.gba`) headlessly, with real key presses, memory checks and screenshot comparisons.
Use these only for what `make check` (`mgba-rom-test`) cannot cover: UI as drawn, real input handling, multi-screen flows.
It runs on libmgba 0.11 through [node-mgba](https://github.com/ARISE-Foundation/node-mgba) (pinned in `package.json`), about 1000x real time.

## Running

Node is installed on the Windows side only, so run from PowerShell:

    cd test\e2e
    npm install --ignore-scripts      # once
    node run.mjs                      # all tests
    node run.mjs --filter Maniac      # name substring
    node run.mjs --update-golden      # (re)write golden images from the current screen; review them first
    node run.mjs --rebuild-snapshots  # ignore cached snapshots

Build the ROM first (`make -j` in WSL): the runner needs `pokeemerald.gba` and the `pokeemerald.map` from the same link, and fails if they are stale.
It is not part of `make check`.

## Layout

- `tests/*.json`: test files, each one test or an array of tests.
- `recipes/*.mjs`: snapshot recipes (`export async function build(game)`); a snapshot is built from boot the first time a test needs it.
- `lib/game.mjs`: the `Game` wrapper (symbols by name, memory, keys, waits); `lib/recipes.mjs`: `bootToOverworld`, `teleport`.
- `goldens/`: committed golden images. `cache/` (snapshots) and `out/` (failure diagnostics) are gitignored.

## Test format

    {
      "name": "unique, human-readable",
      "snapshot": "recipe name",
      "inputDelayFrames": 12,                       // optional; default 12 (~0.2 s) before each press
      "settleFrames": 120,                          // optional; frames run after the last step, before any check
      "steps": [
        { "press": "UP" },                          // optional "hold" (default 6) and "delay"
        { "wait": 30 },
        { "waitStable": true }                      // until the screen stops changing; avoid on maps with water/flower animation
      ],
      "expect": [
        { "symbol": "gStringVar4", "text": "decoded game text, \n and \\p for control codes" },
        { "symbol": "gSpecialVar_Result", "u16": 1 },       // also u8, u32, "offset", or raw "address"
        { "symbol": "gMain", "offset": 4, "bytes": "ab cd" }
      ],
      "screenshot": { "region": {"x":0,"y":104,"width":240,"height":56}, "golden": "goldens/name.png" }
    }

Symbols come from `pokeemerald.map`; static symbols are not listed there.

## Snapshots

Savestates embed ROM addresses, so they are never committed: they are cached under `cache/` keyed by the ROM hash and the recipe code, and rebuilt when either changes.
A recipe plays through New Game with key presses, then `teleport()` writes the save block's location and restarts the map load.
Snapshot and screenshot generation were checked to be deterministic across fresh rebuilds.

## Investigating a failure

A failing test writes `out/<test name>/`: `report.txt`, `actual_full.png`, `actual_region.png`, `diff.png` (pixels that differ, in magenta), `ewram.bin`, `iwram.bin` and `final.ss` (a savestate of the failing moment).
Read RAM dumps with addresses from `pokeemerald.map` (EWRAM starts at `0x02000000`, IWRAM at `0x03000000`).
