# Tech Debt

One line per item, roughly highest priority first. Delete an item when it is paid off.

- Mod code lives in upstream's `src/util/` and `include/util/` (region_*, chapter, npc_talk, city_hinters, plot_stage, county_chain, story_mode, rival beside trig/decompress/librfu): no folder says what is ours; move into e.g. `src/region/`, `src/story/` (also `src/field/scene.c`).
- `notes.md` is stale and contradictory: "Codebase layout" says `src/` is flat and `Folders` is unmerged, while "Branches" says it is main; Folders-status, test-limit and chapter-port notes are mixed together and several bullets exceed the lean rule.
- `remaining_work.md` mixes milestone work with ~60 "open design questions" (many are design content, not work) and has multi-clause items that cannot be deleted whole; the 1a milestone is a grab bag.
- `src/field/scene.c` (~1100 lines) holds the step interpreter, path search, triggers and the scene table; split by responsibility.
- Scenes bind actors by raw local ids per `struct Scene`, door facing is hardcoded "south", and the trigger table is hand-written; ids are bare u8/u16 with no types.
- County var slices: `GetCountyVarSlice(county)` swaps Littleroot/Oldale with slices 0/1, so "county id" and "slice" silently differ; legacy var block cannot shrink from the middle.
- `IdentityRng` and salts take bare numbers (`include/constants/identity_salts.h`); easy to pass the wrong kind of id.
- Scripts using `switch` clobber `VAR_0x8000` (broke `give`; `SceneLoadGive` now reloads it): `custom` scene scripts have an unexpected side effect, guarded only by `tools/misc/check_scene_custom.py`.
- Region data is partly hand-assigned for Emerald (pitstops, class assignments, hinter and Pokémon Center news NPC bindings) while accessors ignore the generator's deal; hand and generated paths will diverge.
- Headless tests cannot run battles, `waitfanfare`, `Std_ObtainItem` or `lock`/`waitmessage` scripts (no VBlank): scenes with these are only covered by slow e2e recipes.
- `test/e2e` recipes hardcode save-block offsets (e.g. `0x560 + 30*4`) and need Windows Node plus a built ROM; not part of `make check`.
- `Test_LoadMap` leaves state that made `Battle strings` time out depending on test grouping (root cause never found; worked around by `Test_RestartAfterTest`); the test heap is not zeroed between tests.
- Test builds are short of EWRAM/IWRAM (a few hundred bytes broke the link): tests share buffers like `sWalkDirs`; `.data` is discarded and `Test_MgbaPrintf` lacks `%x` and width flags.
- Link order must be basename-sorted or the Z-Move animation test crashes (latent upstream memory-layout bug, root cause unknown).
- Path-keyed config (`ld_script_test.ld`, Makefile `librfu_intr.c` flags, `test/test_runner.c` leak-check exemption) breaks silently on file moves.
- Touching `include/constants/vars.h` rebuilds nearly everything (20+ min); full `make check` takes 10+ min; `grep -r` over the WSL share is very slow.
- `data/maps/*/events.inc` is generated and gitignored (edit `map.json` only); `core.filemode=false` and `safe.directory=*` are needed from Windows git: unexpected prerequisites not enforced anywhere.
- Doc files named inconsistently (`Emerald.md`, `Emerald_walkthrough.md`, `Kanto.md`, `vars.md` vs `design_*.md`); `design_pokemon.md`, `design_story.md` and `design_region.md` (240-280 lines each) cover several topics apiece.
- `src/util/chapter.c` (637 lines) mixes the stage resolver with per-chapter hint strings and `sXxxFar/Close` tables (6 arrays per new stage trio, hand-copied); hint text belongs in `src/data/region/` beside `emerald_chapters.h`, ideally one table row per stage.
- Each chapter port hand-edits 6+ places (`map.json`/`scripts.inc` vars, `emerald_chapters.h`, `scenes.h`, `scene_custom.inc`, `vars.h`, `debug.inc`, docs); no checklist or checker ties a retired legacy var to the stage var that replaced it.
- `custom` scene steps in `data/scripts/scene_custom.inc` are raw script labels with no declared inputs/outputs (VAR_0x8000 clobber, `waitfanfare`); each new one risks the same bug and only e2e catches it.
- e2e coverage is per-scene JSON plus hand-written recipes (`test/e2e/recipes/*.mjs`, 7 files) with save-block poking per case; the Lilycove hinter box-frame golden failed 2026-11-15 uninvestigated.
- Retiring legacy vars (`VAR_RIVAL_DEX_BEAT` replaced three state vars) has no save migration: saves past the tutorial need a fresh game; the choice is implicit in each port rather than a stated policy.
- Chapter `producedKey` for the first key is undecided (Save Birch, Cutter's House and Mossdeep each set `producesMainGet`/plot-get adapters differently): the plot-get adapter is a per-chapter special case.
