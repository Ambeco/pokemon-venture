# Remaining Work

## Milestones

The staged hardcode-first roadmap (design.md, "Development approach"). Each milestone leaves the game playable and `make check` green. Code pointers are starting points found while planning, not verified designs.

### Milestone 0: Foundations

Prerequisites for everything else (design.md stages 1-3: hardcoded Emerald county division, region tiles, city/location metadata).

- Add salt constants (map, NPC, ...) to `include/constants/identity_salts.h` and `test/identity_salts.c` as callers appear; `IdentityRng` takes bare numbers.
- Routes: `region_route.c` gives each county its default half of every inter-county route (owner straight, other L or Z). Still to do: feed tile ownership to route-story-indices; Emerald's Fallarbor unlock has no lock tile (`GetRouteLock` finds none), so its implicit key is unreachable from a lock; use the pool ids (`GetCountyMcGuffinId`) when a county's subplot gates an inner location; intra-county routes exist only in counties with a city (Emerald's cityless counties with several specials get none), and nothing yet routes a cityless county's locations to each other; when generation exists, keep generated locations off a county's outermost row and column (by construction, not testable on Emerald's table); add an accessor for the route id/MAPSEC between neighbors, for NPC text about adjacent cities (the city accessors already take any county id, so a neighbor's name and theme need no special function).
- Landmark/detour/pitstop split (design.md, "Special locations"): the class field, Emerald's classification, route planning that skips pitstops and pitstop placement on free route tiles exist. Still to do: make the generator use the pitstop deal (`DealCountyPitstops` exists but the stride deal leaves many counties with few or no pitstops, or reuses them, while the pool (17 entries for 24 counties) is small; before turning it on, add stub landmarks, detours and pitstops that reuse existing maps with no stateful content, such as houses with 1-2 NPCs that babble, then later pull in the FR/LG landmarks, detours and pitstops with their maps and subplots; and is tested, but Emerald's table still assigns pitstops by hand and the tile accessors do not read the deal); the cityless-county pitstop fallback (the test only flags a cityless county with no routed place, and Emerald's Route 119 and 117 were bumped to detours for it); show only landmarks on the region-map screen once the generated region replaces Emerald's map (`IsLocationOnMap`); pitstop capacity: Emerald met it only by bumping Sealed Chamber, Mirage Tower and Desert Ruins to detours (the deal caps a county's pitstops by its free tiles); review Emerald's class assignments (`emerald_tiles.h` comment lists them).
- Extend the region dump (`src/util/region_dump.c`) as more data appears. Milestones 1 and 2 consume the city accessors; the gym scripts and NPC text are untouched so far.
- Story state storage (design.md, "Story state storage"; inventory in `vars.md`, done): the first 32 vars are temp and object-GFX, so reservations are named regions in `0x4020`-`0x40FF`, not Var0-31. Remaining steps: (1) the main-line counts are in `Emerald.md` ("Per-county storage needs"); still to count are authored-subplot stage vars (after Milestone 1a's bucketing) and the optional inter-county stories (the slice is 4 vars per county, 3 used by the main line); (4) retire Emerald vars as Milestones 1a, 2 and 3 land: the 62 legacy vars in `vars.h` shrink toward zero, and county slices become usable as they go (only the first 14 are free now). Also trim `test/vars.c`'s table as vars are retired or packed.

### Milestone 1a: Subplot abstraction groundwork

Unblocks Milestone 2 and fixes the interfaces Milestone 1 builds on (design.md, "Subplot abstraction").

- Review the `Emerald.md` "Lock/key buckets" (done as a first pass; the 135 entries are bucketed and the city and special-location ones listed): confirm the cross-host key/lock pairs, then treat them as planner unlock steps when paper-testing below.
- Fold the paper-test rules (design.md, "Interface rules from paper tests") into the descriptor format when implementing: derived stages, parameterized hints, ordered stage predicates, multi-building cutscenes, cross-host effects as produced keys. Implement the hard Flash limit (an unlit player cannot go more than about 6 tiles from the entrance of a `requires_flash` map; design.md, "Dark caves"), and make Flash a route-unlock McGuffin in the planner like the other HMs.
- Check the placeholder planner ranges in `Emerald.md` ("Planner inputs").
- Place the generic blocker script (`route_blocker.inc`) as an NPC for the two planner blockers (indices 39, 67): the sea route into Mossdeep needs a placement decision, and check vanilla cannot reach either spot before the previous list is done. Grow the blocker pool toward about 96 with generated texts later.
- Main-plot descriptor, stage accessor and read-only adapter are done (`plot_stage.h`). Still to do: the stored accessor backend for main-line lists (the host county's completion mask, Milestone 3), the full story-reference resolver with its build tool (only `{HINT_ANCHOR}` exists, via `ExpandHintTemplate`), and consumers of the subplot descriptor's requirements, produced keys and parameterized hints (declared in `subplot.h`, unused; the Lilycove Museum port has two stages and no hint NPCs yet, and no script calls `GetCountySlotHint`). Check the blocking/reported-only classification in `src/data/region/emerald_plot_gets.h` (Emerald.md, "Step predicates"): it errs toward reported-only where unsure.
- Once the descriptor and McGuffin abstraction exist, also abstract Kanto's subplots and McGuffins (`data/maps/*_Frlg` scripts, `src/data/trainers_frlg.*`; notes.md) and add them to the subplot and McGuffin lists.
- Done when: the accessor and adapter pass tests, one subplot is ported (Lilycove Museum, met), and the NPC-talk query returns the right stage for it. The swap itself is Milestone 2's done-when.

- Hinter follow-ups (`city_hinters.h`): bind hinters to NPCs for generated maps at map load (Emerald's Lilycove binds its two by hand); give main-beat hints per-item detail (`WriteMainBeatHint` only names the first item's kind and the host city; gym type, HM, leader name) and bind them with `ShowMainBeatHint` for generated maps; decide who places far hinters for subplots hosted in other counties.
- Check what Emerald and FRLG use for path surfaces in villages, towns and cities, and settle the paving, dirt and grass rule (design.md, "City planning" step 4).
- Pokémon Center news object (design.md, "City planning"): place the news NPC in Pokémon Centers (`ShowPokemonCenterNews` exists, unbound; Emerald's Lilycove could be hand-bound first); mark ported subplots that give a main-plot get with `producesMainGet` and `mainGet` as they are ported (none of Emerald's does yet).

### Milestone 1: NPC texts shuffled, still hinting the current arcs and subplot

Builds on Milestone 1a's accessor and descriptor, so it needs no redesign when subplots are ported.


- Text pools: per-theme, per-building-type, per-gym-type, "near"/"far" arc hints, arc-state thank-yous; parameterized with city and leader names.
- Kanto text extraction (`Kanto.md`, "What the ROM can carry"): a build tool copies Kanto's generic texts out of `data/maps/*_Frlg/scripts.inc` into their own data file, which the pools above draw from alongside Hoenn's. City and island sign slogans (17) go into the city table as themes; building-role talk (fan club, name rater, school, bike shop, museum, ...) and generic NPC and sign texts need classifying by role and theme, which is this milestone's pool design. Only the Emerald-reusable roles are extracted; skip story texts and the deferred roles.
- A generic NPC talk special that picks a text from (county, city, NPC local id, arc states); replace fixed `msgbox` scripts in `data/maps/*/scripts.inc` with it, via a conversion tool rather than by hand.
- At map load, guarantee at least one hint NPC per arc in each city/route.
- Done when: every arc state is hinted from every city, and a seed sweep finds no city without a hint.

### Milestone 2: Subplots shuffled randomly

- Subplot data format (linear stages, hint texts, state texts, trigger place, rewards).
- Generic cutscene runner (NPC runs up, text, optional battle/item, runs off) implemented once as a parameterized script/special, using map-script hooks for "finished route" and "left building" triggers.
- Convert the first few Emerald subplots (Cut man, Briney's Wingull, Devon Goods, Old Rod quiz, Flash, Captain Stern) to the data format; assign a subplot per city at the region layer.
- Done when: for some seed, two city-bucket subplots whose role and progression requirements both hosts meet are swapped, both are completable, and NPCs in both cities hint correctly; and the same city gets different subplots for different seeds, each completable. After bucketing out main arc, gyms and gates, Emerald may not contain two qualifying city subplots; if so, author one small extra subplot for the test.

### Milestone 3: Rival and story-arc cutscenes no longer at fixed locations

- Implement route-story-index and county-story-index lookups over the hardcoded region (design.md, "Route story indices").
- Arc cutscene placement: main-story-index list (rival or arc items) to host county to route/city/building, without storing it; maintain the main-story-index and the per-county completion masks for the main arc.
- Replace Emerald's fixed rival and Wally encounters, and the Magma/Aqua beats, with generic cutscenes; preserve the text.
- Done when: rival battles occur at the right gym positions on generated places, and arcs finish.

### Milestone 4: Randomized Pokémon types

- Species metadata tool: base-evolution table, sprite-implied element overrides, and the biome type-affinity tables.
- Fixed-type exceptions to the stride mapping: the Eevee line (Eevee pure Normal, each Eeveelution its own single type), plus the sprite-implied element list. The "evolutions share the base's mapping" invariant needs these exempted.
- Stone evolutions (decided; sprite-implied species are locked first, as above): any remaining base species that evolves with an elemental stone has its type drawn only from the types that have a stone, and evolves with the matching stone. For multi-stone lines (Gloom, Poliwhirl, Eevee), use the union of the stone types and choose the evolution by the stone used. Still to list: which types have stones (Fire, Water, Electric, Grass, Ice, Dark via Dusk, Fairy via Shiny, possibly Psychic via Dawn), and the leftover species this applies to. Evolved forms still gain an unrestricted second type.
- Stride mapping (mod 12, over the region's 4x3 biome subset) and seeded type selection; route every type read through the single accessor `GetSpeciesType` (`include/pokemon/pokemon.h:824`), and check every direct read of `types` in battle, Pokédex, AI, and the summary screens.
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

- Abstract McGuffin kinds (blocking NPC, HM obstacle, key item) behind one interface, driven by the planner's unlock and McGuffin slots; place obstacles at generated chokepoints; shuffle which HM or item unlocks what; decouple HM use from fixed locations.

### Milestone 10: Cities and special locations randomized within each county

- Layer-3 placement (city placement away from county edges, special location rules, themes, names, island formation) and layer-4 route generation; generated region-map display, fly points, and name popups.
- Enclaves (design.md, "Enclaves"): generate Enclaves and Enclave-only counties in layers 2/3 (the hardcoded Emerald data, the route and tile rules and tests exist; still to do: the at-least-one-non-Enclave rule when generating, and the region-map screen showing mapped Enclaves once unlocked), the port destination menu that checks it (should fail loudly if the region has no port), and a more specific per-Enclave access mechanism later; the Emerald tables still list Southern Island, Mirage Island and the Battle Frontier as routed.
- Reserved legendary locations: write the separate algorithm that picks which reserved county is the solo post-Elite-Four one and ties the other 2-3 into the gym order (gym order need not follow county-story-index, and reserved counties hold the last indices, after the Elite Four's). Also decide how a county without an index is reached, since it lacks the usual lower neighbor route, and what step 5's "about 19" is exactly (24 counties = 20 indices 0-19, Elite Four, and 3 reserved).

### Milestone 11: Procedural county-story-indices

- Layer-2 generator with McGuffin boundaries and the Elite Four/start/capital placement tendencies; completability must follow from the algorithm's construction, not from checking seeds.

### Milestone 12: Procedural terrain types and biomes

- Layer-1 generator: outer ring mountain/ocean, inland extensions, humidity/temperature gradients with a guaranteed non-mountain desert, 4x3 biome subset selection (one of 6).

### Cross-cutting

- Budget CPU and RAM for regeneration; decide what to cache (the route planner replans a county per lookup, with a one-county cache; a plan costs roughly 100k cycles by a rough estimate, from the test timeouts).
- Convert the region invariant tests (`test/region_*.c`) to `FOR_EACH_SWEEP_SEED` once the generator reads the seed.
- Rewrite `README.md` as the fork's own README.

## Open design questions and unscheduled work

- Collect the overall plan for the mod from the user and record it in `design.md`.
- Pin down region generation details: terrain/biome/population/special-location types, story-order algorithm, key/lock rules, capital selection, and how generated data maps onto pokeemerald maps, warps, and encounters.
- Emerald McGuffins (next session): read the user's Emerald walkthrough and list every McGuffin key and lock it shows (location, approximate story index, requirements); then read how a sample is implemented (flags, vars, badges, key items, HMs, subplot state) and use the code to complete the list in `Emerald.md`, so NPC dialogue can be keyed off them.
- Decide whether a city's count of inside-city special locations equals its population (1-3); Emerald averages ~0.6 per city (~1.2 counting gyms), see `Emerald.md`.
- Emerald county table (`Emerald.md`): assign population and keys/locks, and derive county-story-indices from the visit order.
- Decide whether to adopt the super-common species list (design.md, "Route encounter selection"), and what "too few species" means for a table.
- Clarify route rules: tie-breaking and duplicate-route handling when several adjacency rules apply to the same county pair.
- Define the special-location catalog (inside-city vs outside-city lists, how a location is chosen for a county, relation to terrain/biome/story order).
- Decide how to integrate the underground tunnels around Saffron (Underground Path) into the region model; keep Red/Blue map reproducibility as a design check.
- Define the 5x5 biome grid (axis values, the 25 biomes, which 12 each 4x3 subset yields), the meaning of each 2-bit population level (0 = none, 1 = village, 2 = town, 3 = city; how it relates to terrain/biome), and whether the region is regenerated on demand vs cached in RAM (cost of regeneration).
- Design the three settlement generation algorithms (village, town, city): building footprints, path layout, how they map onto GBA map layouts/metatiles, and how gyms/Pokémon Centers/Marts get placed in each.
- Species typing: species whose sprite strongly implies an element always keep that element; base forms get one type and evolutions gain the second (algorithm needed); decide interactions with abilities.
- Move pools: define the appendage bitfield vocabulary, per-move requirement/stat tagging for ~1000 moves, the power-sorted level bucketing, the learn-level list generator, and filter probabilities (type match, stat match); research how egg moves work in mainline games and finalize the egg move rule.
- Species<->biome mapping: pick the hardcoded stride (coprime to 12) and inverse over the ~541 evolutionary lines; verify that species of one biome end up evenly spaced and that traded Pokémon (other trainer ID) map sensibly.
- Define the route species filters (level-plausibility using evolution levels, domestication by edge distance, habitat, random drop), the random-drop rate, any further filters, and the catch-rate-proportional encounter table; plus the starter/legendary/pseudo-legendary exclusion lists.
- Define the 25-biome type affinity tables (18 types, log-scale encoding).
- Budget CPU: profile/estimate the cost of regenerating region data, species mapping, encounter filtering, and move picks on GBA hardware; decide what to cache.
- Design the main story arcs (a handful, deeper than Red/Blue) and how their beats map onto county story indices.
- Design subplot data format (linear stages, hint texts, story-state texts) and author the special-location and city subplots (TV-episode style, introducing gym leaders before their battles).
- Design a generic mechanism for triggering subplot stage-advancement cutscenes (no plan yet).
- Write NPC talking points for named buildings and per gym type, with parameterized city/leader names.
- Estimate ROM budget for subplot text/script data.
- Decide whether towns get a Mart, define the per-bag-category city stores, and the daily trader generation (daily seeding, trader count by population, catch-rate similarity and level-overlap rules).
- Implement battle damage estimates (design.md, "Battle damage estimates"): move menu, in-battle summary moves page, and party-menu switch grades.
- Implement the gym guide's party-specific advice (design.md, "Gym guide advice"); depends on the encounter tables for the in-county species suggestions.
- Debug menu entry showing the current route's encounter tables (every method, time of day and weather); needed once Milestone 5 generates them.
- Turn on party-wide experience (`I_EXP_SHARE_FLAG`, always set) and add the underleveled bonus and overleveled penalty (design.md, "Experience"); open: the bonus and penalty multipliers, and the exact boss-level estimate, which waits on Milestone 7's boss levels.
- Implement the PC team analyzer (design.md, "Team analyzer"). Open: the hypothetical opponents' attacking stats (for the defensive part of the grade); the score of a Pokémon with no top grades (avg of an empty set, assumed 0, giving score 1).
- Design the analyzer's move-suggestion extension: per-species best-move lists and a greedy/hill-climb search over box Pokémon and future learnable moves.
- Type-recolored sprites (design.md, "Type-recolored sprites"): the build tool for picked palette indices, the runtime recolor at palette load (battle, summary, Pokédex, party/box icons, followers),, the shiny shimmer, and visual tests to tune the bucket cutoffs. Party menu icons (6 Pokémon) fit in free OBJ palette slots. PC box icons: switch OBJ palettes between icon rows with HBlank DMA (rows don't overlap vertically; icons overlap only left/right). BG palettes are a separate bank of 16, so the box's backgrounds don't compete. Use one icon palette per type combination (6 per row). Vanilla `src/pokemon/pokemon_storage_system.c` already loads 15 OBJ palettes: 6 icon palettes (`PALTAG_MON_ICON_0-5`) plus `BOX_TITLE`, `DISPLAY_MON`, `MARKING_COMBO`, `MARKING_MENU`, `MISC_1`, `MISC_2`, and `ITEM_ICON_0-2` (item mode only). Those beside the icon rows (preview sprite, markings, cursor, item icons) plus a slot for the carried Pokémon (it crosses rows) stay reserved, leaving roughly 7-10 per row: 6 fits, 12 split-icon halves do not. Open: map which tags share scanlines with the icon rows and when each is loaded, and check box scrolling (the title slides across) and the carried icon.
- Implement type icons before Pokémon names in all text: pokeemerald-expansion is believed to already have text icons for each type (unverified; check before adding new glyphs); handling of name buffers and text width, and nickname/name-entry screens.
- Decide which expansion mechanics (Mega Evolution, Gigantamax, Z-Moves, Terastallization, etc.) and alternate forms to keep, and how procedural typing/move pools interact with them (e.g. Mega types, Tera types, form-specific moves).
- Biome art and terrain: decide which biomes share tilesets (palette variants) and which get unique sprites; design the per-biome terrain placement algorithms (trees, grass types, dirt, water, rocks, hills); map the 25 biomes to type-affinity tables and to Emerald's existing tilesets for the hardcoded stage.
- Find the latent memory-layout bug behind the Z-Move animation test crash seen when linking in directory order (see notes.md), so the basename-sorted link order can be dropped.
- Design the geography generator: outer-ring mountain/ocean stretches, inland mountain and ocean extensions, island formation, and humidity/temperature gradients at region-tile granularity that guarantee a non-mountain desert-like biome.
- Define the landform list (beach, volcano, farmland, sandbars, fjords, salt flats, mesas, river basin) and cave variants (underground river, bioluminescence, crystals, sinkholes, magma, ice), and how each biases tiles, encounters and layouts.
- Define the species metadata fields (habitat requirements/biases from sprite appearance, domestication, base evolution, level-range enum, commonness) and tag all ~1000 species; reconcile image-based habitat biases with the species-to-biome stride mapping that decides types (the mapping decides a species' types; habitat flags decide where it appears).
- Check why Emerald caps land encounters at 12 slots and fixed slot ratios, and whether the encounter code can take dynamic table sizes and ratios.
- Define the per-method encounter tables (eager: walk, surf, grass, dark grass; lazy: rods, rock smash, headbutt, berry trees, spots), time-of-day and weather filters, and table recalculation when time or weather changes.
- Hardcode estimated encounter levels for non-level evolutions (stones ~29, trade, friendship, etc.).
- Audit existing rules against the layering principle (design.md "Layered generation"): e.g. route-story-index rules must depend only on county-story-indices and connectivity, and species/encounter generation must not need generated map data.
- Define trainer team generation: the trainer classes (ordinary, boss, low-level grunt), how route-story-index maps to trainer and wild levels and hidden items, and tune the grunt catch-rate cutoff (starts at 255).
- Define the theme catalog (requirements, sprite sets, gym types, NPC texts) and how the region layer assigns themes to spread gym types; define how placement tendencies (Elite Four corner, start near it, capital far, coast/mountain city nudges) are implemented within the layering principle.
- Decide how strongly gym assignment follows the average gym-order table (soft bias strength, Elite Four preference for late types) alongside theme requirements.
- Content roadmap: Emerald rival arc and story arc first (preserving cutscene text), then the Kanto story arc and Red/Blue/Yellow rival arc, then other mainline games; subplots from Emerald, then RBY and others, and eventually TV-episode subplots.
- Check the dropped top tile row against the Emerald region map: confirm the 28x15 cells after dropping the top row and the first/last columns still hold the intended county contents, and that the city name and Nav overlays only cover unused tiles.
- Design chunked, per-frame generation (region/county/map and city plot points) and its CPU budget.
- Add the generator id to the save and the refuse-to-load check.
- Confirm the story-graph proposals in design.md (the layer-2 growth constraint, the start county always having a settlement, key items and HMs as the only keys) and check the first-place rule against Emerald's actual layout.
- Define the catch-rate floor curve (as a function of route-story-index/badge count), the full list of species excluded from encounter tables (babies, starters, legendaries, and any further category such as pseudo-legendaries or mythicals), and the legendary/starter acquisition mechanism (gift subplots, weekly legendary spawn, generation grouping of legendary locations).
- Non-blocking idea: shuffle which gym ordinal unlocks which HM. Constraints to check: Surf stays tied to the Surf-lock county's block and gym 5, Fly (and its gym) is never a callback and its block sets the callback distance bound, water HMs need terrain on the routes into the next county (or the blocker fallback), and an HM's badge must be held when the HM-gated route is entered.
- Review the exceptions list under "Main-story-index table (proposed)" in `Emerald.md`: decide each (rule change or table change), then renumber the county table to match. Candidates for rule changes: a callback gym sharing a block with the previous gym (G4 forced to step 0), an arc dictating its own callback host,.
- Ports (design.md, "Ports") are decided but not yet cross-checked: the layer-2 growth constraint for the ocean county (exempt, since it is reached by teleport), the interaction with the Surf land-count threshold, and giving the Surf-gated sea routes the Surf block's level.
- V2 readiness (several regions, as in Gold/Silver and FireRed/LeafGreen; design.md, "Regions and multiple regions"): the 24 county var slices, `GetRegionHeader()` and the tile table assume one region; give them a region index and a per-region seed when V2 starts.
