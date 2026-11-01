# pokemon-venture

Procedurally generated Pokémon region mod on top of pokeemerald-expansion. Read `documents/` first (`design.md`, `remaining_work.md`, `notes.md`).

## Branches

- `Folders` is the real main branch (src/ and include/ reorganized into folders). Do development here.
- `master` is a "remote-copy" staging branch that mirrors upstream (`rh-hideout/pokeemerald-expansion`): pull upstream into `master`, then merge `master` into `Folders`. Don't develop on `master`.

## Pushing

- Push only when the tests pass (`make check -j`; filter with `make check TESTS="..."`). Don't push with failing tests.
- Do not open PRs unless asked.

## Environment

- Repo lives in WSL. From Windows, git needs `git -c safe.directory=*`; builds and tests are best run inside WSL (`wsl -d Ubuntu -- bash -lc "cd ~/decomps/pokemon-venture && make check -j"`).
- `make check TESTS=` takes one name-prefix pattern (`TESTS='A B'` finds nothing); include the colon for names like `RegionDump: Foo`.
- Test builds have little IWRAM: put large static test buffers in EWRAM (`static EWRAM_DATA char buf[...]`), or the link fails with "IWRAM overflowed".
- Test files can't use initialized `.data` (the test linker script discards it): use `EWRAM_DATA` for shared test globals. `Test_MgbaPrintf` has no width flags (`%08x` prints `8x`).
- Full `make check -j4` takes over 10 minutes: when running it in the background, set its timeout to an hour.
- `make check TESTS='Region'` runs just the region tests (about 45, a few minutes).
- The Grep tool silently matches nothing for brace globs like `{A,B}/scripts.inc` on this path: use `**/scripts.inc` or one path per call.
