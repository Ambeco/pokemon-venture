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

- Moving files breaks path-keyed build config. Fixed: `ld_script_test.ld` (`src/audio/m4a.o`, `src/util/libisagbprn.o`) and the Makefile per-file flag rule for `src/util/librfu_intr.c` (it needs ARM-mode flags; a stale `gba/` path silently compiled it wrong and the test ROM executed garbage). After any further file move, grep `Makefile` and `ld_script*.ld` for the old path.
- `make check -j` with all 12 cores runs WSL (7.9 GB) out of memory when the ROM misbehaves (runner buffers output unboundedly). Use `-j4` and `ulimit -v`; helper scripts live outside the repo in `~/decomps/.*.sh`.
- Upstream `master` passes `make check TESTS=Spikes` (4/4). `Folders` still fails 2/4 (`Spikes damage on switch in`, `Spikes do not damage airborne Pokemon`, reported only as "src/pokenav/config_changes.c:121: 108 bytes not freed"); symbol sizes now match upstream, and preprocessed sources differ only in `__FILE__` path strings. Cause not yet found; suspect something path-dependent in the test runner.
- A clean upstream worktree for comparison is at `~/decomps/pv-master` (outside the repo).
