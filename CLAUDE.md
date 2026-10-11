# pokemon-venture

Procedurally generated Pokémon world mod on top of pokeemerald-expansion. Read `documents/` first (`design.md`, `remaining_work.md`, `notes.md`).

## Branches

- `Folders` is the real main branch (src/ and include/ reorganized into folders). Do development here.
- `master` is a "remote-copy" staging branch that mirrors upstream (`rh-hideout/pokeemerald-expansion`): pull upstream into `master`, then merge `master` into `Folders`. Don't develop on `master`.

## Pushing

- Push only when the tests pass (`make check -j`; filter with `make check TESTS="..."`). Don't push with failing tests.
- Do not open PRs unless asked.

## Environment

- Repo lives in WSL. From Windows, git needs `git -c safe.directory=*`; builds and tests are best run inside WSL (`wsl -d Ubuntu -- bash -lc "cd ~/decomps/pokemon-venture && make check -j"`).
