# Notes

## Environment / toolchain

- Repo lives in WSL (`\\wsl.localhost\Ubuntu\home\mooin\decomps\pokemon-venture`). Git from the Windows side fails with "dubious ownership"; use `git -c safe.directory=* ...` (or run git inside WSL).
- Build: `make -j$(nproc)`. Tests: `make check -j` (filter with `make check TESTS="Spikes"`); see `docs/tutorials/how_to_testing_system.md`. Tests live in `test/` (battle tests are the bulk, under `test/battle/`).
- Upstream's own docs are in `docs/` (mdbook); `docs/tutorials/` has how-tos for new Pokémon, moves, trainers, FRLG, follower NPCs, DexNav, etc.

## Codebase layout

- `src/` ~387 C files (flat), `include/` headers, `include/config/` feature toggles, `data/maps/` ~944 maps, `test/` test suite, `tools/` build tools (mapjson, trainerproc, wild_encounters, etc.).
- Branch `origin/Folders` (2 commits, not merged) reorganizes `src/` and `include/` into subfolders — pure renames, ~690 files.

## Branches

- `Folders` (src/ and include/ reorganized into subfolders) is the working main branch; GitHub's default branch should be changed to it (Settings > Branches). `master` tracks upstream layout.
- An earlier implementation of fully-random species types (about 4 months before 2026-10-10) was not found in this clone's reflog, branches, or fsck; it may live in another clone. It needs a redo anyway.

## Folders branch test status

`make check` passes on `Folders` (4780 passed, 0 failed). Moving files broke path-keyed config in four places; after any further move, grep for the old path in `Makefile`, `ld_script*.ld`, and `test/`:

- `ld_script_test.ld` named `src/m4a.o` and `src/libisagbprn.o`.
- The Makefile per-file flags rule for `librfu_intr.c` (needs ARM-mode flags; a stale path silently compiled it wrong and the ROM ran garbage).
- `test/test_runner.c` exempts `src/pokenav/config_changes.c` from the leak check by path; a stale path failed every PARAMETRIZE battle test.
- Link order: the Makefile links `src/` objects sorted by basename, as when `src/` was flat. In directory order, the Z-Move animation test crashes (a latent upstream memory-layout bug, root cause not found). If it reappears after other layout changes, suspect a buffer overrun in the animation code.

Running tests: use `make check -j4` (all 12 cores plus a misbehaving ROM exhausts WSL's 7.9 GB; add `ulimit -v`). Upstream worktree for comparison: `~/decomps/pv-master` (outside the repo).
- Kanto (FRLG) content is in the repo: 419 `data/maps/*_Frlg` maps (incl. Sevii Islands) with full scripts, Kanto wild encounters in `wild_encounters.json`, `src/data/trainers_frlg.*`. Built only by `make firered`/`leafgreen` (`docs/tutorials/how_to_frlg.md`); `tools/mapjson` drops maps whose `region` differs from the build, so the Emerald ROM cannot reach them.
- `core.filemode=false` is set in this clone's `.git/config` (local, untracked) because Windows git over `\\wsl.localhost` sees every executable as `100644`. New executable scripts need `git update-index --chmod=+x <file>`.
- In the test build `gTrainers` is a stub (`test/test_runner_battle.c`, `battle/trainer_control.h`), so tests cannot read real trainer parties; check real trainer data offline against `src/data/trainers.party`.
- Test limits: `RUN_OVERWORLD_SCRIPT` runs only specials and variable commands (no `lock`/`faceplayer`/`waitmessage`/objects). NPC bindings are tested by reading script bytes (`goto` and `call` are opcode + 4-byte pointer, `setvar` is opcode + 2 + 2) and the map's `events->objectEvents[].script` via `Overworld_GetMapHeaderByGroupAndId` (`test/pokemon_center_news.c`). No test exercises message display; see `remaining_work.md`, "End-to-end tests".
