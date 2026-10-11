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
