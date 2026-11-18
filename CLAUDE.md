# pokemon-venture

Procedurally generated Pokémon region mod on top of pokeemerald-expansion. Read `documents/` first (`design.md`, `remaining_work.md`, `notes.md`). `design.md` is an index: read only the `design_*.md` (and `Emerald*.md`) files the task touches.

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
- Touching `include/constants/vars.h` rebuilds nearly everything (20+ minutes): run the full `make check` once instead of several targeted runs after it.
- `data/maps/*/events.inc` is generated and gitignored: edit `map.json` only.
- A county's var slice is `GetCountyVarSlice(county)`, not the county id: Littleroot and Oldale swap with slices 0 and 1 until the legacy var block shrinks (design_story.md, "Story state storage").
- Don't edit repo files with Windows Python `open()`: it rewrites LF files as CRLF. Use Edit/Write, or run Python inside WSL.
- The Bash tool is Git-Bash: multi-line strings and `/tmp` paths passed to `wsl ... bash -lc "..."` get mangled. Write a script to `//wsl.localhost/Ubuntu/tmp/x.sh` and run `wsl -d Ubuntu -- bash -lc "bash /tmp/x.sh"`, then delete it. The same goes for Python or C text with many quotes or apostrophes: a `python3 - <<'EOF'` heredoc can be rejected before anything runs, so write the script (and any large C block it splices) with the Write tool, run it in WSL, then delete it. Recursive `grep -r` over the `\\wsl.localhost` share is very slow: run it inside WSL.
- The Bash tool mangles `\\p` (becomes `\p`) and accents (é becomes �) in text it writes, and Windows Python needs `encoding='utf-8'` to read repo files: edit e2e expected-text JSON with the Edit tool.
- A headless `make check` that ends with no result lines (or "Killed") means a new test hangs in the ROM (suspect `waitfanfare` or a menu wait): remove or isolate it, and cover that flow with an e2e test.
- `make e2e` runs `test/e2e` (needs a built ROM; falls back to `powershell.exe` for Node). Not part of `make check`.
- The Grep tool silently matches nothing for brace globs like `{A,B}/scripts.inc` on this path: use `**/scripts.inc` or one path per call.
- After the ROM (`pokeemerald.gba`, from `make`) is rebuilt and the tests pass, copy it over `C:/Users/mooin/Dropbox/roms/GBA/pokemon-venture.gba` (the user plays from there; it is outside the repo, so mention the copy in the closing message).
