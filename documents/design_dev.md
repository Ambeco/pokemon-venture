# Codebase, config and testing design

The base codebase, the config choices that shape the design, story mode and the end-to-end test tier.

## Base codebase

- Built on [rh-hideout/pokeemerald-expansion](https://github.com/rh-hideout/pokeemerald-expansion) 1.16.1 (itself built on pret/pokeemerald). Remote `upstream` points there; `origin` is `Ambeco/pokemon-venture`.
- Feature toggles live in `include/config/*.h` (`battle.h`, `pokemon.h`, `overworld.h`, `general.h`, `item.h`, etc.); most mod-wide behavior switches should be made there before touching code.
- Expansion already contains ~1000 Pokémon and ~1000 moves, plus mechanics such as Mega Evolution and Gigantamax. How these fit the mod is undecided; they could make fun mechanics.
- Expansion supports building FRLG as well as Emerald (`docs/tutorials/how_to_frlg.md`).

### Config choices

Settings changed from upstream defaults, and what each means for the design. Unlisted settings are unchanged.

Overworld Pokémon (the one that shapes later work):

- `OW_POKEMON_OBJECT_EVENTS`, `OW_FOLLOWERS_ENABLED`, `WE_OW_ENCOUNTERS`: wild Pokémon appear as walking sprites on the map and walking into one starts the battle, and the lead Pokémon follows the player. `WE_OWE_SPAWN_REPLACEMENT`, shiny sparkle, despawn sound, Feebas spots and the shiny/Feebas despawn protections are on; `WE_OWE_NO_REPEL_DEXNAV_COLLISION` is on. `OW_AMBIENT_CRIES` is `OWE_PRIORITY`, `OW_TIME_OF_DAY_ENCOUNTERS` is on, and `OW_GFX_COMPRESS` is off (it costs VRAM when many OW Pokémon are loaded).
- Encounters: the spawner draws from the map's wild tables, so generated runtime tables (Milestone 5) must be readable by it, per method (land, water) and per time of day. Route tables must be filled before the player can see a sprite, so lazy per-method generation applies only to rods, rock smash and the like.
- Art: every encountered species needs a map sprite, so type-recolored sprites ("Type-recolored sprites" (design_pokemon.md)) must also recolor OW and follower sprites, and `OW_SUBSTITUTE_PLACEHOLDER` covers missing ones. `OW_PKMN_OBJECTS_SHARE_PALETTES` stays off, so palettes compete with the 16 OBJ palettes. To sidestep that, overworld encounter sprites use one palette per first type (a recolor of the species' map sprite by its first type), not one per species.
- Object budget: spawned Pokémon, the follower, placeholder NPCs and runtime cutscene actors share the map's object-event table. A cutscene that needs more objects aggressively despawns overworld encounters to make room. The placeholder counts in "Building roles and NPC spawning" (design_chapters.md) assume the table has room; check `OBJECT_EVENTS_COUNT` before the runner is written.

Other changes:

- Shiny: `SHINY_ODDS` 80 (about 1/820, 10x vanilla), Shiny Charm adds 2 rolls, fishing chain on. OWE sparkle makes shinies visible before battle, so the higher rate is noticeable.
- Fishing boosts (follower friendship, proximity, time of day) are on.
- Battle UI: `B_SHOW_TYPES` and `B_SHOW_EFFECTIVENESS` are always on. With randomized types this is what lets a player read a matchup, and it overlaps "Battle damage estimates"; that design must not duplicate it.
- Summary: IV/EV info with exact values, move relearners (egg, TM, tutor), pre-evolution moves and reusable TMs are on. Move relearners and pre-evolution moves must be kept in mind for Milestone 6's learnset replacement; reusable TMs make TMs non-scarce, which suits generated moves.
- Field moves: Defog and Rock Climb are enabled. They act like HMs but are not gym HMs, and need no planner support as long as the main story is completable without them: they, like Dig, Teleport, Soft-Boiled, Whirlpool, Waterfall, Rock Smash, Headbutt, Sweet Scent, Milk Drink, Secret Power and Chatter, are non-HM field moves that only open optional content.
- Possible later change, as in Gen 7+ (Let's Go's "Chop Down", "Sky Dash", "Strong Push", "Sea Skim", "Light Up"): an NPC teaches "your Pokémon" a field move by setting a flag instead of giving an HM item, and the field move works while any party member could theoretically learn the move (`CanLearnTeachableMove`), even if it does not know it. This looks cheap to add: expansion already routes every field move through `gFieldMoveInfo[].isUnlockedFunc`, which today reads badge flags (`field_move.c`), but the "has a mon with the move" check is `MonKnowsMove` in places such as `PartyHasMonWithSurf` (`field_player_avatar.c`), so each move's party check also needs replacing. Not decided; it would turn the planner's HM items into flag-setting NPC lessons and drop the badge/HM pairing in "Vanilla stage overrides" (design_story.md).
- NPC followers (`FNPC_ENABLE_NPC_FOLLOWERS`) are on, usable by the cutscene runner; it costs SaveBlock3 space.
- Instant text is on (`TEXT_SPEED_INSTANT`). `FLAG_TEXT_SPEED_INSTANT` stays 0: a nonzero value must be a free saved flag, never a temp flag such as `FLAG_TEMP_1`.
- Held evolution items are usable from the bag, and ambient item descriptions show the first time an item is found.

## Story mode

A debug-menu toggle (Flags/Vars: "Toggle Story mode", `FLAG_VENTURE_STORY_MODE`; code `src/story/story_mode.c`) for playing through the story quickly and reviewing its chapters. While on, ordinary battles are skipped as wins at the battle-start hook (`DoStandardWildBattle`, `BattleSetup_StartTrainerBattle`), so the real end-of-battle callbacks still run and flag setting, whiteout checks and script continuation are unchanged.

- Skipped: plain single and double wild battles and trainer battles of non-boss classes. Real fights: boss trainer classes (Leader, Elite Four, Champion, Rival, which also covers Brendan, May, Wally and Steven, and the Aqua and Magma leaders and admins), partner and multi battles, Pyramid, Trainer Hill, Pike, Safari, ghost, legendary and roamer battles, tutorials and scripted wild battles, so their scripts still get exercised. A boss class in either slot of a two-opponent battle keeps the whole battle real.
- Rewards: every healthy party member gets the full experience (yield * level / 5, 1.5x for trainers, through the soft level cap) and EVs, with no sent-in split or level scaling, and levels up through the normal exp table. Each Pokémon that gained levels then gets the Rare Candy level-up flow in the party menu (`StoryMode_ShowLevelUpMenu`): stat pages, move learning with the forget prompt, and evolution. Trainer intro and defeat text are untouched, because the skip happens at `dotrainerbattle` between them. A trainer pays prize money from its data (last party level, no Amulet Coin).
- Wild catches: every skipped wild Pokémon is marked seen. It is given to the player (party, else PC) unless the party or a box already holds the same species (forms are separate species), shininess and an equal or better IV total; a tie counts as already owned. A corner popup (queued, shown by `StoryMode_Tick` from `OverworldBasic` once the field is visible) reports caught, already-owned and no-room results.
- Near-invulnerability: in real fights, `Cmd_datahpupdate` clamps damage to a player-side battler so it keeps 1 HP (not in link battles).

## New Game configuration and Nuzlocke mode

Later work. Choosing "New Game" on the title screen opens a configuration menu that sets the `RANDOMIZE_*` bits (see design.md, "Randomization switches"), such as randomized Pokémon types and moves, plus other settings. The result is stored in the save's randomization mask.

Nuzlocke mode is one of those settings. Entering a route (or other area) shows the map-name popup with a small icon saying whether a Pokémon has already been caught there. The same reminder icon appears in wild battles and on the region map.

## End-to-end tests

Two tiers. `make check` (the headless `mgba-rom-test` ROM) covers logic and game state, including real maps and NPC scripts through `Test_LoadMap`/`Test_LoadMapAt` (`include/test/map_loaded.h`), with message flows driven by `Test_RunFrames` and `Test_PressKeys` (an NPC script is talked to, its message waits for A, and a long text takes one A per page); it has no VBlank, so nothing reaches VRAM or OAM, and no real key handling. `test/e2e` covers what needs those: UI as drawn, real input and multi-screen flows. Use the e2e tier only when the in-ROM tier cannot cover a scenario.

`test/e2e` drives the built `pokeemerald.gba` headlessly through libmgba using node-mgba (pinned), at about 1000x real time. Tests are JSON (`test/e2e/README.md`): a snapshot recipe, key presses with a configurable delay, expectations on memory (by symbol name from `pokeemerald.map`, with game text decoded through `charmap.txt`), and an optional screenshot region compared against a committed golden image.

Snapshots are built from boot rather than committed: a recipe presses through New Game, then `teleport()` writes the save block's location and position and points `gMain.callback2` at `CB2_LoadMap`, which is what a warp does. Savestates embed ROM addresses, so they are cached per ROM hash and recipe code and rebuilt when either changes. The runner refuses a `.gba` and `.map` that are not from the same link. Every test runs a fixed settle delay (default 120 frames) after its last input before checking anything: screens with water or flower animation never become still, and the emulator is deterministic, so a frame count is reproducible. Animated regions are listed as `ignore` regions in the screenshot spec, and the message prompt is masked by default. A failing test writes its screenshot, diff, RAM dumps and a savestate to a gitignored `out/` folder, which is how a failure is investigated. `make e2e` builds the ROM and runs it (Node usually exists on the Windows side only, so the target falls back to `powershell.exe`). It is not part of `make check`, and has no CI step: CI only builds, it does not run `make check` either, and node-mgba needs a Node setup there.

## Alternatives considered

### Driving the Windows mGBA through its GDB server

Pros:
- Already installed; a standard debugger protocol.

Cons:
- No key input, savestate loading or screenshots over the protocol, so those need another channel anyway.
- Only the Qt build opens the server; the SDL build did not.

### Lua scripting inside mGBA (`mgba-qt --script`, or the mgba-mcp wrapper)

Pros:
- Full emulator API (keys, memory, screenshots, states) inside the emulator.

Cons:
- The Windows 0.10.5 build rejects `--script` and `-S`, so a script can only be loaded by hand in the GUI.
- Elsewhere it needs `mgba-qt` plus a virtual display, and the process has to be killed because quitting from Lua is unreliable.

### Python bindings to libmgba (libmgba-py or mGBA's own)

Pros:
- Headless, with keys, memory and a video buffer.

Cons:
- Must be built from source; no install route, undocumented mGBA version, savestate loading undocumented, last commit Dec 2024.

### Only the in-ROM test harness

Pros:
- Fastest, already in `make check`.

Cons:
- Without VBlank nothing reaches VRAM or OAM, so a UI bug such as text overflowing its box is invisible; no screenshots; keys bypass the game's own input reading. Kept as the first tier.
