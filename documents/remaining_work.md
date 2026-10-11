# Remaining Work

## Milestones

The staged hardcode-first roadmap (design.md, "Development approach"). Each milestone leaves the game playable and `make check` green. Code pointers are starting points found while planning, not verified designs.

### Milestone 0: Foundations

Prerequisites for everything else (design.md stages 1-4: hardcoded Emerald county division, world tiles, city/location metadata, hint NPCs).

- Seeded generator module: a deterministic, self-contained PRNG and hash (seed = player ID, plus per-purpose salts such as county, map, NPC), independent of the game's `gRngValue`; written as pure functions with no GBA dependencies so they can be unit tested.
- World seed accessor wrapping `gSaveBlock2Ptr->playerTrainerId`; remove the debug menu's "New Trainer ID" option (`src/util/debug.c:663`, `DebugAction_Player_Id`) so the seed never changes.
- Test strategy for randomization: many existing tests assert real species types and learnsets (e.g. `ASSUME(GetSpeciesType(...))`), so every randomization milestone needs a compile-time switch (or fixed identity mapping under `TESTING`) plus new tests that sweep many seeds and check invariants.
- Hardcoded world data tables (`src/data/world/`?) for the 5x4 counties over Emerald's 28x15 region map (`src/data/region_map/region_map_layout.h`): terrain, biome, population, special location, story index, McGuffin keys/locks, capital (Lilycove), story-arc id, world-tile roles, per-city themes/buildings/gym types/leaders.
- Debug tool to print the whole world (counties, tiles, indices, McGuffins) as text, used by every later milestone.
- Arc/subplot state storage: pick save vars/flags (bitpacked) for arc stage and subplot stages; verify against `SaveBlock` space.

### Milestone 1: NPC texts shuffled, still hinting the current arcs and subplot

- Text pools: per-theme, per-building-type, per-gym-type, "near"/"far" arc hints, arc-state thank-yous; parameterized with city and leader names.
- A generic NPC talk special that picks a text from (county, city, NPC local id, arc states); replace fixed `msgbox` scripts in `data/maps/*/scripts.inc` with it, via a conversion tool rather than by hand.
- At map load, guarantee at least one hint NPC per arc in each city/route.
- Done when: every arc state is hinted from every city, and a seed sweep finds no city without a hint.

### Milestone 2: Subplots shuffled randomly

- Subplot data format (linear stages, hint texts, state texts, trigger place, rewards).
- Generic cutscene runner (NPC runs up, text, optional battle/item, runs off) implemented once as a parameterized script/special, using map-script hooks for "finished route" and "left building" triggers.
- Convert the first few Emerald subplots (Cut man, Briney's Wingull, Devon Goods, Old Rod quiz, Flash, Captain Stern) to the data format; assign a subplot per city at the world layer.
- Done when: the same city gets different subplots for different seeds, each completable.

### Milestone 3: Rival and story-arc cutscenes no longer at fixed locations

- Implement route-story-index and county-story-index lookups over the hardcoded world (design.md, "Route story indices").
- Arc cutscene placement: arc stage to county-story-index to route/city/building, without storing it.
- Replace Emerald's fixed rival and Wally encounters, and the Magma/Aqua beats, with generic cutscenes; preserve the text.
- Done when: rival battles occur at the right gym positions on generated places, and arcs finish.

### Milestone 4: Randomized Pokémon types

- Species metadata tool: base-evolution table, sprite-implied element overrides, and the biome type-affinity tables.
- Stride mapping (mod 25) and seeded type selection; route every type read through the single accessor `GetSpeciesType` (`include/pokemon/pokemon.h:824`), and check every direct read of `types` in battle, Pokédex, AI, and the summary screens.
- Type icons before names (check existing icons first).
- Ability interactions; decide on form changes, Mega/Tera types.
- Done when: all tests pass with the generator off, and seed-sweep tests confirm 1-2 valid types, base-form single type, evolutions sharing the base's mapping.

### Milestone 5: Randomized encounter tables

- Tag species metadata: commonness, domestication, habitat requirements/biases, level range; estimated levels for non-level evolutions.
- Runtime table builder replacing the generated `gWildMonHeaders` (from `src/data/wild_encounters.json`); the encounter code (`src/field/wild_encounter.c`, `ChooseWildMonIndex_*`, `TryGenerateWildMon`) currently reads fixed 12-slot ROM tables, so check whether it can take variable sizes and ratios.
- Per-method tables (eager: walk, surf, grass, dark grass; lazy: rods, rock smash, headbutt, spots), time-of-day and weather recalculation.
- Integrations: DexNav, Pokédex area map, roamers, outbreaks, Feebas, Safari Zones.
- Done when: seed sweep finds no empty required tables and levels fit route story indices.

### Milestone 6: Randomized Pokémon moves

- Appendage bitfields for species and requirement/stat tags for ~1000 moves (tool-assisted, with human review).
- Replace level-up learnsets (`GetSpeciesLevelUpLearnset`, `pokemon.h:834`, `src/data/pokemon/level_up_learnsets`) and teachable learnsets (generated by `tools/learnset_helpers`) with the learn-level generator and validity filter; initial moveset (`GiveBoxMonInitialMoveset`, `src/pokemon/pokemon.c:1566`), move relearner, tutors, egg moves.
- Done when: no species lacks a usable attacking move early, and AI and tests still run.

### Milestone 7: Randomized trainers

- Replace party data from `src/data/trainers.party` with a generator in the trainer party code (`CreateNPCTrainerPartyFromTrainer`, `src/battle/battle_main.c:1864`), using trainer classes (ordinary, boss, grunt), story-index levels, and the filter rules.
- Keep authored teams for the boss tier until generated teams feel right.

### Milestone 8: Procedural buildings and route terrain

- Runtime map generation: routes first (grass, dirt, trees, water, rocks via per-biome algorithms), then cities (village/town/city algorithms); there is precedent for runtime map generation in `src/battle/battle_pyramid.c` and metatile edits via `MapGridSetMetatileIdAt`; check map size limits and how events, warps, and object templates are created at runtime.
- Per-biome tileset/palette variants; keep memory within EWRAM limits.
- Done when: a generated route is walkable end to end with correct collision, connections, and warps.

### Milestone 9: Inter-county McGuffins shuffled

- Abstract McGuffin kinds (blocking NPC, HM obstacle, key item) behind one interface; place obstacles at generated chokepoints; shuffle which HM or item unlocks what; decouple HM use from fixed locations.

### Milestone 10: Cities and special locations randomized within each county

- Layer-3 placement (city in the middle 3x3, special location rules, themes, names, island formation) and layer-4 route generation; generated region-map display, fly points, and name popups.

### Milestone 11: Procedural county-story-indices

- Layer-2 generator with McGuffin boundaries and the Elite Four/start/capital placement tendencies; a seed-sweep solvability check (every seed completable).

### Milestone 12: Procedural terrain types and biomes

- Layer-1 generator: outer ring mountain/ocean, inland extensions, humidity/temperature gradients with a guaranteed non-mountain desert, 16-of-25 biome subset selection.

### Cross-cutting

- Budget CPU and RAM for regeneration; decide what to cache.
- Seed-sweep test tooling reused by all milestones.
- Rewrite `README.md` as the fork's own README.

## Open design questions and unscheduled work

- Collect the overall plan for the mod from the user and record it in `design.md`.
- Pin down world generation details: terrain/biome/population/special-location types, story-order algorithm, key/lock rules, capital selection, and how generated data maps onto pokeemerald maps, warps, and encounters.
- Settle how the 5x4 grid of 6x4-tile counties (with off-limits edge tiles) overlays Emerald's region map for stage 1, where much of the 28x15 grid is ocean.
- Clarify route rules: tie-breaking and duplicate-route handling when several adjacency rules apply to the same county pair.
- Define the special-location catalog (inside-city vs outside-city lists, how a location is chosen for a county, relation to terrain/biome/story order).
- Decide how to integrate the underground tunnels around Saffron (Underground Path) into the world model; keep Red/Blue map reproducibility as a design check.
- Define the 5x5 biome grid (axis values, the 25 biomes, which 16 each 4x4 subset yields), the meaning of each 2-bit population level (0 = none, 1 = village, 2 = town, 3 = city; how it relates to terrain/biome), and whether the world is regenerated on demand vs cached in RAM (cost of regeneration).
- Design the three settlement generation algorithms (village, town, city): building footprints, path layout, how they map onto GBA map layouts/metatiles, and how gyms/Pokémon Centers/Marts get placed in each.
- Species typing: species whose sprite strongly implies an element always keep that element; base forms get one type and evolutions gain the second (algorithm needed); decide interactions with abilities.
- Move pools: define the appendage bitfield vocabulary, per-move requirement/stat tagging for ~1000 moves, the power-sorted level bucketing, the learn-level list generator, and filter probabilities (type match, stat match); research how egg moves work in mainline games and finalize the egg move rule.
- Species<->biome mapping: pick the hardcoded stride (coprime to 25) and inverse over the ~541 evolutionary lines; verify that species of one biome end up evenly spaced and that traded Pokémon (other trainer ID) map sensibly.
- Define the route species filters (level-plausibility using evolution levels, domestication by edge distance, habitat, random drop), the random-drop rate, any further filters, and the catch-rate-proportional encounter table; plus the starter/legendary/pseudo-legendary exclusion lists.
- Define the 25-biome type affinity tables (18 types, log-scale encoding).
- Budget CPU: profile/estimate the cost of regenerating world data, species mapping, encounter filtering, and move picks on GBA hardware; decide what to cache.
- Design the main story arcs (a handful, deeper than Red/Blue) and how their beats map onto county story indices.
- Design subplot data format (linear stages, hint texts, story-state texts) and author the special-location and city subplots (TV-episode style, introducing gym leaders before their battles).
- Design a generic mechanism for triggering subplot stage-advancement cutscenes (no plan yet).
- Write NPC talking points for named buildings and per gym type, with parameterized city/leader names.
- Estimate ROM budget for subplot text/script data.
- Decide whether towns get a Mart, define the per-bag-category city stores, and the daily trader generation (daily seeding, trader count by population, catch-rate similarity and level-overlap rules).
- Implement type icons before Pokémon names in all text: pokeemerald-expansion is believed to already have text icons for each type (unverified; check before adding new glyphs); handling of name buffers and text width, and nickname/name-entry screens.
- Decide which expansion mechanics (Mega Evolution, Gigantamax, Z-Moves, Terastallization, etc.) and alternate forms to keep, and how procedural typing/move pools interact with them (e.g. Mega types, Tera types, form-specific moves).
- Biome art and terrain: decide which biomes share tilesets (palette variants) and which get unique sprites; design the per-biome terrain placement algorithms (trees, grass types, dirt, water, rocks, hills); map the 25 biomes to type-affinity tables and to Emerald's existing tilesets for the hardcoded stage.
- Find the latent memory-layout bug behind the Z-Move animation test crash seen when linking in directory order (see notes.md), so the basename-sorted link order can be dropped.
- Design the geography generator: outer-ring mountain/ocean stretches, inland mountain and ocean extensions, island formation, and humidity/temperature gradients at world-tile granularity that guarantee a non-mountain desert-like biome.
- Define the landform list (beach, volcano, farmland, sandbars, fjords, salt flats, mesas, river basin) and cave variants (underground river, bioluminescence, crystals, sinkholes, magma, ice), and how each biases tiles, encounters and layouts.
- Define the species metadata fields (habitat requirements/biases from sprite appearance, domestication, base evolution, level-range enum, commonness) and tag all ~1000 species; reconcile image-based habitat biases with the species-to-biome stride mapping that decides types (the mapping decides a species' types; habitat flags decide where it appears).
- Check why Emerald caps land encounters at 12 slots and fixed slot ratios, and whether the encounter code can take dynamic table sizes and ratios.
- Define the per-method encounter tables (eager: walk, surf, grass, dark grass; lazy: rods, rock smash, headbutt, berry trees, spots), time-of-day and weather filters, and table recalculation when time or weather changes.
- Hardcode estimated encounter levels for non-level evolutions (stones ~29, trade, friendship, etc.).
- Pin down route-story-index edge cases: a county with no lower-index neighbors (the start), several adjacent lower-index counties, and what "highest story index" means for choosing the main route into a county.
- Audit existing rules against the layering principle (design.md "Layered generation"): e.g. route-story-index rules must depend only on county-story-indices and connectivity, and species/encounter generation must not need generated map data.
- Define trainer team generation: the trainer classes (ordinary, boss, low-level grunt), how route-story-index maps to trainer and wild levels and hidden items, and tune the grunt catch-rate cutoff (starts at 255).
- Define the theme catalog (requirements, sprite sets, gym types, NPC texts) and how the world layer assigns themes to spread gym types; define how placement tendencies (Elite Four corner, start near it, capital far, coast/mountain city nudges) are implemented within the layering principle.
- Decide how strongly gym assignment follows the average gym-order table (soft bias strength, Elite Four preference for late types) alongside theme requirements.
- Content roadmap: Emerald rival arc and story arc first (preserving cutscene text), then the Kanto story arc and Red/Blue/Yellow rival arc, then other mainline games; subplots from Emerald, then RBY and others, and eventually TV-episode subplots.
- Adapt the arc progression design to GBA CPU/memory limits: where arc and subplot state lives in the save, how the cutscene target (county-story-index, route/city, building) is derived without storing it, and how it fits the layering principle.
