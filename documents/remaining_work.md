# Remaining Work

## Milestones

- The staged hardcode-first roadmap (design.md, "Development approach"); each milestone leaves the game playable and `make check` green.
- Code pointers below are starting points found while planning, not verified designs.

### Milestone 0: Foundations

- Prerequisites for everything else (design.md stages 1-3: hardcoded Emerald county division, region tiles, city/location metadata).
- Add salt constants (map, NPC, ...) to `include/constants/identity_salts.h` and `test/identity_salts.c` as callers appear; `IdentityRng` takes bare numbers.
- Routes (`region_route.c` gives each county its default half of every inter-county route):
  - Feed tile ownership to route-story-indices.
  - Emerald's Fallarbor unlock has no lock tile (`GetRouteLock` finds none), so its implicit key is unreachable from a lock.
  - Use the pool ids (`GetCountyMcGuffinId`) when a county's chapter gates an inner location.
  - Intra-county routes exist only in counties with a city (Emerald's cityless counties with several specials get none); nothing routes a cityless county's locations to each other.
  - When generation exists, keep generated locations off a county's outermost row and column, by construction (not testable on Emerald's table).
  - Add an accessor for the route id/MAPSEC between neighbors, for NPC text about adjacent cities (the city accessors already take any county id).
- Landmark/detour/pitstop split (design_region.md, "Special locations"):
  - Make the generator use the pitstop deal: `DealCountyPitstops` exists and is tested, but Emerald's table still assigns pitstops by hand and the tile accessors do not read the deal.
  - The stride deal leaves many counties with few or no pitstops, or reuses them, and the pool (17 entries for 24 counties) is small. Before turning it on, add stub landmarks, detours and pitstops reusing existing maps with no stateful content (houses with 1-2 babbling NPCs), then later pull in the FR/LG ones with their maps and chapters.
  - Cityless-county pitstop fallback: the test only flags a cityless county with no routed place; Emerald's Routes 119 and 117 were bumped to detours for it.
  - Pitstop capacity: the deal caps a county's pitstops by its free tiles; Emerald met it only by bumping Sealed Chamber, Mirage Tower and Desert Ruins to detours.
  - Show only landmarks on the region-map screen once the generated region replaces Emerald's map (`IsLocationOnMap`).
  - Review Emerald's class assignments (`emerald_tiles.h` comment lists them).
- Extend the region dump (`src/region/region_dump.c`) as more data appears; Milestones 1 and 2 consume the city accessors, and the gym scripts and NPC text are untouched so far.
- Story state storage (design_story.md, "Story state storage"): the first 32 vars are temp and object-GFX, so reservations are named regions in `0x4020`-`0x40FF`, not Var0-31.
  - Count authored-chapter stage vars (after Milestone 1a's bucketing) and the optional inter-county stories; the slice is 4 vars per county, 3 used by the main line (`Emerald.md`, "Per-county storage needs").
  - Retire Emerald vars as Milestones 1a, 2 and 3 land: the 44 legacy vars in `vars.h` shrink toward zero and county slices become usable (the first 21 are free now); trim `test/vars.c`'s table as they go. A removed var leaves its address unused until the block is renumbered (as `LEGACY_VARS_COUNT` shrinks, renumber the remaining offsets).

### Milestone 1a: Chapter abstraction groundwork

- Unblocks Milestone 2 and fixes the interfaces Milestone 1 builds on (design_chapters.md, "Chapter abstraction").
- Confirm the cross-host key/lock pairs in `Emerald_walkthrough.md` "Lock/key buckets" (135 entries bucketed), then treat them as planner unlock steps when paper-testing below.
- Implement the hard Flash limit (an unlit player cannot go more than about 6 tiles from the entrance of a `requires_flash` map; design_story.md, "Dark caves"), and make Flash a route-unlock McGuffin in the planner like the other HMs.
- Port more chapters to the descriptor (`Emerald_walkthrough.md`): no "city chapter (convert)" entries are left; the rest are the special-location chapters (Abandoned Ship, Trick House, Petalburg Woods and the flower shop). Briney's Wingull is main arc (Route 104, Rustboro and the Rusturf tunnel on the Devon Goods vars).
- New Mauville (a stateless adapter in Mauville, `sNewMauvilleChapter`): its hints (`GetCountyStatelessHint`) have no hinters yet, and the news never mentions it (no main-plot get): bind NPCs for Mauville, and give the other stateless chapters (gyms, Rydel's proxy) hints only if they need them.
- Re-analyze city locations vs building roles, then refactor: `BuildingRole` is a per-city bit mask (all 16 `u16` bits used); consider a role enum plus subroles, with a wider set type for `.roles`/`requiredRoles`. Anchors that are really locations should use `TownLocationKind`: the fossil revival (Devon Corp = Rustboro's `TOWN_LOCATION_INSTITUTION`, currently anchored on `BUILDING_SERVICE`) and the Lilycove contest hall (`BUILDING_CONTEST_HALL`; not a special location of Lilycove's tile yet). `BUILDING_SERVICE` lumps name rater, tutor, reminder, deleter.
- Save Birch: Birch's Lab is not a special location of Littleroot's tile entry, so every stage is anchored to the Oldale road; anchor the lab stages there once it is.
- Chain dealing for shuffled chapters (design_chapters.md, "Ordering when chapters are shuffled"; build it in Milestone 2):
  - Avoid a sort: number chapter ids in contiguous class blocks (outputs, neutral, inputs) and pick `base + (index * stride + offset) mod size` per class (stride coprime to the class size, as in `region_pitstop.c`); striding across all ids scrambles the class order.
  - Open: a host's candidates are filtered by roles, badges and pool, so a class's eligible members may have holes; keep it scan-free with fixed class sizes per slot or a per-county eligible list.
  - Add a static test that each id's class matches its block.
- Lock NPCs (twin, Birch's chase, the Oldale sketch artist): all three already gate on stage writes (`VAR_SAVE_BIRCH_BEAT` written by the rival-meeting scripts; the artist on `FLAG_ADVENTURE_STARTED`, set when the Pokédex is given). `IsCurrentChapterAvailable` adds nothing until a chapter can be moved or reordered; wire it when the first one is. Also make the cutscene runner (Milestone 2) walk a stage's `extraRoles` buildings.
- Location placeholders for plot strings: `{CUR_LOCATION}`, `{PREV_LOCATION}`, `{NEXT_LOCATION}` (city, landmark, detour or pitstop):
  - Resolve prev/next in another county through one county accessor, since that county must say which location it picks.
  - Write a remote county as "near X" with X its first city, landmark or detour (county names don't exist for cityless counties, and relative phrasing like "northeast of CERULEAN" is grammatically hard and not always adjacent to a city).
- Direction hint placeholder ("north of RUSTBORO"): needs county adjacency via the route accessor in Milestone 0 and the hinter's county in `ExpandHintTemplate`; only far chapter hinters need it, and none exist yet (design_city_npcs.md, "City planning"). `{HINT_CITY}` and `{HINT_PLACE}` exist.
- Far references (random NPC babble only, no story link): each county generates a few far-reference texts about its own places (e.g. the shopping center in Saffron), and an NPC may pull one from an adjacent county, so a Cerulean NPC can mention Saffron. Replaces referring to adjacent towns by gym leader, type or theme, which is hard to match.
- Check the placeholder planner ranges in `Emerald.md` ("Planner inputs").
- Place the generic blocker script (`route_blocker.inc`) as an NPC for the two planner blockers (indices 39, 67): the sea route into Mossdeep needs a placement decision, and check vanilla cannot reach either spot before the previous list is done. Grow the blocker pool toward about 96 with generated texts later.
- Main-plot descriptor follow-ups (`plot_stage.h`):
  - Add the stored accessor backend for main-line lists (the host county's chain, Milestone 3).
  - Add the full story-reference resolver with its build tool (only `{HINT_ANCHOR}` exists, via `ExpandHintTemplate`).
  - Bind the new ports' hinter NPCs for generated maps (Lilycove's and Rustboro's are bound by hand).
  - Check the blocking/reported-only classification in `src/data/region/emerald_plot_gets.h` (Emerald.md, "Step predicates"); it errs toward reported-only where unsure.
- Once the descriptor and McGuffin abstraction exist, also abstract Kanto's chapters and McGuffins (`data/maps/*_Frlg` scripts, `src/data/trainers_frlg.*`; notes.md) and add them to the chapter and McGuffin lists.
- County chain (design_chapters.md, "County chain"; `county_chain.h` is unused by game code yet):
  - Parallel subplots (design_chapters.md, "Parallel subplots (proxy entries)"): Mauville's Rydel is converted (`CHAPTER_PROXY`, `CHAPTER_SLOT_STATELESS`); convert other independent errands on a shared chain the same way, and `CheckProxySoftLock` checks the four soft-lock conditions for every proxy.
    - Var 0 of each county slice is reserved and unused; give it a job or renumber the slices when the legacy var block shrinks (`vars.h` rebuilds everything).
  - Use `IsCountyMainSlotDone` where main-slot completion is needed (gym chapters, the main-story-index advance); adapter chapters on a chain read legacy vars, so a county with one still needs it ported.
  - Port the other gyms (`GYM_CHAPTER` in `emerald_chapters.h`): the gym scripts and hints are still vanilla; Sootopolis's block-20 callback (Victory Road, Wally 2) hosts no chapter.
  - Convert the six hinted adapter get chapters (`PLOT_GET_HINTED_CHAPTER`: Pokénav, the Starter, Shipyard, the Elite Four, the Champion and the Ever Grande lobby) to real multi-stage chapters with scenes: they have one anchor and a before/after hint pair, but still two stages that mirror `emerald_plot_gets.h`'s predicates (the Starter reads `FLAG_SYS_POKEMON_GET`), and no hinters are bound. Rival 1, the Pokédex, Poké Balls and Running Shoes are views of `sRivalDexChapter`'s var; give them hints and anchors through its stages. The Starter should read Save Birch's stage once the debug menu writes it.
  - Petalburg's gym (`sPetalburgGymChapter`) anchors its Surf stage on the gym: Emerald hands Surf over in Wally's house, so give Petalburg a house role and anchor stages 3-5 of `VAR_PETALBURG_CITY_STATE` there (design_chapters.md, "One chapter, several buildings"). A callback list has no slot rules, so a blocking callback get with no chapter does not underfill its host's plan.
  - A chapter that supplies a get (`mainGet`, `mainHm`) but whose get is in another county's block or a slot without room asserts (`SUBPLOT_PLAN_BOUND_ELSEWHERE`); untested, since an `assertf` makes a test invalid. The tutorial chapters (Moving in, Save Birch, Rival and Pokédex) still declare no get, so they are first-fit.
  - Advance the global main-story-index when `DidChainJustFinishMainSlot` fires (no stored index exists until Milestone 3; `GetMainStoryIndex` is computed by the adapter).
- Scene runner (design_scenes.md, "Scene runner"; `src/story/scene.c`, first conversion is Littleroot's "need a Pokémon" girl):
  - Check the wild form of `battle` in a real game or `make e2e` (no ported scene uses it yet, and the headless tests cannot run battles); the trainer form and `ask_choice` are covered by `test/e2e/tests/route103_rival.json`.
  - Add a Lilycove rival e2e (decline, ask again, then win; needs a `lilycove_city_below_rival` recipe with the strong-party helper), likewise for Rival 2 on Route 104 and in Rustboro: those scenes are covered only by headless scene and chapter tests.
  - Give `give` a stage reward (a descriptor field) and `battle` a wild double or two-trainer form when a chapter needs them.
  - Route generation must guarantee the rescue's grass in view of the route entrance and an open 8-10 tile ring near it (design_scenes.md, "Scene runner") and keep the ring off the entrance-to-bag path.
  - Add the remaining triggers:
    - Add talk, enter a door and interact with an item; the proximity and door-tile triggers exist, but a map-edge door has no "on door", so scenes needing it must be refused at placement (design_scenes.md, "Door and off-screen approach rules").
    - Classify the entrance/exit border: `SCENE_TRIGGER_EXIT` takes its edge from the table, not from route locks.
    - Bind actors per map instead of by local ids in each `struct Scene`.
    - Add a stage-to-scenes field beside `actors` on `ChapterDesc` (triggers are still a hand-written table).
    - Take the door's facing from the map instead of "south".
  - Add the off-screen approach (search from the end tile for an off-screen tile on the right edge, over about 10 steps, then through objects, then a teleport animation).
  - Add building features (chair, TV, center-stage) and transient-actor placement with its test.
  - Convert the rest of the Route 101 tutorial (stage 5) and the other two ported chapters' scripts.
  - Replace the Super Rod fisherman's talk script with a "talk" trigger once it exists (his script still checks the stage and calls the scene).
- Scene arbitration (design_scenes.md, "Interleaving subplots"): when two subplots' scenes qualify for one trigger, run the chain entry before an off-chain one, then the lower chapter slot; nothing implements this yet.
- Actor placement: enforce the per-screen and per-map object budgets in the layout step (check the engine's spawn margin and the player and follower overhead first).
- County planning (design_chapters.md, "County planning and chapter placement"):
  - Add an attachment field (location, unique building or main beat) to the descriptor beside generic chapters.
  - Make slot 0 a county chain (stage position resolved to chapter and stage by cumulative stage counts).
  - Add declarative triggers on anchors with a proximity radius; the trigger list in the design doc is incomplete.
  - Add the fallback hint-then-give chapter.
  - Add the unique-building and generic-chapter stride deals.
- Hinter follow-ups (`city_hinters.h`):
  - Place hinters for Mossdeep's Super Rod chapter (`numHinters` is 0: Mossdeep binds no NPCs yet).
  - Bind hinters to NPCs for generated maps at map load (Emerald's Lilycove and Rustboro bind theirs by hand).
  - Give main-beat hints detail for non-gym kinds (`WriteMainBeatHint` names gym leader, type and HM, but only the first item's kind otherwise) and bind them with `ShowMainBeatHint` for generated maps.
  - Decide who places far hinters for chapters hosted in other counties.
- Check what Emerald and FRLG use for path surfaces in villages, towns and cities, and settle the paving, dirt and grass rule (design_city_npcs.md, "City planning" step 4).
- Place the hinters of the newest ports (Dewford's Old Rod, Mauville's Rock Smash and Coin Case, Slateport's Harbor Mail) once NPCs are bound for those cities; their e2e tests have no bag-full case, and Harbor Mail's does not buy through the Mart menu (the bag is pre-filled).
- Pokémon Center news object (design_city_npcs.md, "City planning"):
  - Place the news NPC in Pokémon Centers: `ShowPokemonCenterNews` is bound by hand only to Lilycove's Pokémon Center Boy, via `Common_EventScript_PokemonCenterNews`, and not yet checked in-game; bind it in the other Emerald centers and for generated maps.
  - Mark ported chapters that give a main-plot get with `producesMainGet` and `mainGet` as they are ported (only Mossdeep's Super Rod does so far).
  - Give a county details to hint at for a step-3 unlock that is no blocker (the Letter), so the news in that county is more than a vague line.
  - Write more generic news lines: `sGenericNews` has only 2, since Emerald has almost no standalone news text.

### Milestone 1: NPC texts shuffled, still hinting the current arcs and chapter

- Builds on Milestone 1a's accessor and descriptor, so it needs no redesign when chapters are ported.
- Grow the text pools in `src/story/npc_talk.c` (two lines per theme and role, one thank-you pool, three gym shapes): more lines per pool, theme-specific lines per building role, thank-yous that depend on the arc state, and far references (see Milestone 1a). Check the pool lines in-game once.
- Kanto text extraction (`Kanto.md`, "What the ROM can carry"): a build tool copies Kanto's generic texts out of `data/maps/*_Frlg/scripts.inc` into their own data file, which the pools above draw from alongside Hoenn's. City and island sign slogans (17) go into the city table as themes; building-role talk (fan club, name rater, school, bike shop, museum, ...) and generic NPC and sign texts need classifying by role and theme, which is this milestone's pool design. Only the Emerald-reusable roles are extracted; skip story texts and the deferred roles.
- Replace fixed `msgbox` scripts in `data/maps/*/scripts.inc` with `generic_npc_talk role` (design_city_npcs.md, "NPC dialogue"), via a conversion tool rather than by hand; only the fallbacks of Lilycove's and Rustboro's four hinters are converted. The tool must work out each NPC's building role, and keep NPCs that carry story state (flags, items, battles) as they are.
- Routes: hint NPCs there (cities already have an arc hinter in every four local ids); at map load for generated maps, bind hinters and make sure a map has at least four NPCs using the generic talk.
- Done when: every arc state is hinted from every city (true for the planner's data once the msgbox conversion covers every city's NPCs), and a seed sweep finds no city without a hint.

### Milestone 2: Chapters shuffled randomly

- Chapter data format (linear stages, hint texts, state texts, trigger place, rewards).
- Generic cutscene runner (NPC runs up, text, optional battle/item, runs off) implemented once as a parameterized script/special, using map-script hooks for "finished route" and "left building" triggers.
- Convert the remaining first Emerald chapters (Captain Stern; Devon Goods, Briney's Wingull and Flash are main arc or gates, see design_chapters.md, "Interface rules from paper tests") to the data format; assign a chapter per city at the region layer.
- Done when: for some seed, two city-bucket chapters whose role and progression requirements both hosts meet are swapped, both are completable, and NPCs in both cities hint correctly; and the same city gets different chapters for different seeds, each completable. After bucketing out main arc, gyms and gates, Emerald may not contain two qualifying city chapters; if so, author one small extra chapter for the test.

### Milestone 3: Rival and story-arc cutscenes no longer at fixed locations

- Implement route-story-index and county-story-index lookups over the hardcoded region (design_region.md, "Route story indices").
- Arc cutscene placement: main-story-index list (rival or arc items) to host county to route/city/building, without storing it; maintain the main-story-index and the per-county completion masks for the main arc.
- Rival team (design_pokemon.md, "Rival teams"): the swap-rule engine `rival_team.c` exists and is tested; still to do: candidate extraction from a route's encounter table (final form, types, catch rate; exclude legendaries, babies, starters), twelve stored vars (Wally and the secondary rival; reserve in `vars.md` first, `vars.h` edits rebuild everything so run one full `make check` after), the route-entry hook (Wally's fold first, the secondary after it), the battle-size mask from rival wins, and feeding the stored team into the battle (dynamic party, `RANDOMIZE_TRAINERS`).
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
- Enclaves (design_region.md, "Enclaves"): generate Enclaves and Enclave-only counties in layers 2/3 (the hardcoded Emerald data, the route and tile rules and tests exist; still to do: the at-least-one-non-Enclave rule when generating, and the region-map screen showing mapped Enclaves once unlocked), the port destination menu that checks it (should fail loudly if the region has no port), and a more specific per-Enclave access mechanism later; the Emerald tables still list Southern Island, Mirage Island and the Battle Frontier as routed.
- Reserved legendary locations: write the separate algorithm that picks which reserved county is the solo post-Elite-Four one and ties the other 2-3 into the gym order (gym order need not follow county-story-index, and reserved counties hold the last indices, after the Elite Four's). Also decide how a county without an index is reached, since it lacks the usual lower neighbor route, and what step 5's "about 19" is exactly (24 counties = 20 indices 0-19, Elite Four, and 3 reserved).

### Milestone 11: Procedural county-story-indices

- Layer-2 generator with McGuffin boundaries and the Elite Four/start/capital placement tendencies; completability must follow from the algorithm's construction, not from checking seeds.

### Milestone 12: Procedural terrain types and biomes

- Layer-1 generator: outer ring mountain/ocean, inland extensions, humidity/temperature gradients with a guaranteed non-mountain desert, 4x3 biome subset selection (one of 6).

### Cross-cutting

- Budget CPU and RAM for regeneration; decide what to cache (the route planner replans a county per lookup, with a one-county cache; a plan costs roughly 100k cycles by a rough estimate, from the test timeouts).
- Convert the region invariant tests (`test/region_*.c`) to `FOR_EACH_SWEEP_SEED` once the generator reads the seed.
- Rewrite `README.md` as the fork's own README.
- End-to-end tests: once map generation exists, add an in-ROM test (`Test_LoadMap` on a generated map, fixed identity seed, check hinters bound from the plan) and a fixed-seed e2e recipe for a generated region; also cover `addobject` scripts in-ROM.

## Unscheduled work

- Story mode (design_dev.md, "Story mode"): play it through in the emulator once and check the skip-to-field transition (fade), the corner popup placement against the map-name popup and the evolution scene hand-off; pooled trainer parties pay out from their first `partySize` entries only.
- Collect the overall plan for the mod from the user and record it in `design.md`.
- Emerald McGuffins (next session): read the user's Emerald walkthrough and list every McGuffin key and lock it shows (location, approximate story index, requirements); then read how a sample is implemented (flags, vars, badges, key items, HMs, chapter state) and use the code to complete the list in `Emerald_walkthrough.md`, so NPC dialogue can be keyed off them.
- Emerald county table (`Emerald.md`): assign population and keys/locks, and derive county-story-indices from the visit order.
- Budget CPU: profile/estimate the cost of regenerating region data, species mapping, encounter filtering, and move picks on GBA hardware; decide what to cache.
- Estimate ROM budget for chapter text/script data.
- Implement battle damage estimates (design_pokemon.md, "Battle damage estimates"): move menu, in-battle summary moves page, and party-menu switch grades.
- Implement the gym guide's party-specific advice (design_pokemon.md, "Gym guide advice"); depends on the encounter tables for the in-county species suggestions.
- Debug menu entry showing the current route's encounter tables (every method, time of day and weather); needed once Milestone 5 generates them.
- Turn on party-wide experience (`I_EXP_SHARE_FLAG`, always set) and add the underleveled bonus and overleveled penalty (design_pokemon.md, "Experience"); open: the bonus and penalty multipliers, and the exact boss-level estimate, which waits on Milestone 7's boss levels.
- Implement the PC team analyzer (design_pokemon.md, "Team analyzer"). Open: the hypothetical opponents' attacking stats (for the defensive part of the grade); the score of a Pokémon with no top grades (avg of an empty set, assumed 0, giving score 1).
- Type-recolored sprites (design_pokemon.md, "Type-recolored sprites"):
  - Build tool for picked palette indices.
  - Runtime recolor at palette load (battle, summary, Pokédex, party/box icons, followers).
  - Shiny shimmer.
  - Visual tests to tune the bucket cutoffs.
  - Party menu icons (6 Pokémon): fit in free OBJ palette slots.
  - PC box icons: switch OBJ palettes between icon rows with HBlank DMA (rows overlap only left/right); one icon palette per type combination (6 per row); BG palettes are a separate bank of 16, so box backgrounds don't compete.
  - PC box palette budget: vanilla `src/pokemon/pokemon_storage_system.c` loads 15 OBJ palettes (6 icon `PALTAG_MON_ICON_0-5`, plus `BOX_TITLE`, `DISPLAY_MON`, `MARKING_COMBO`, `MARKING_MENU`, `MISC_1`, `MISC_2`, `ITEM_ICON_0-2` item mode only); those beside the icon rows plus a slot for the carried Pokémon stay reserved, leaving roughly 7-10 per row: 6 fits, 12 split-icon halves do not.
  - Open: map which palette tags share scanlines with the icon rows and when each loads; check box scrolling (the title slides across) and the carried icon.
- Implement type icons before Pokémon names in all text: pokeemerald-expansion is believed to already have text icons for each type (unverified; check before adding new glyphs); handling of name buffers and text width, and nickname/name-entry screens.
- Find the latent memory-layout bug behind the Z-Move animation test crash seen when linking in directory order (see notes.md), so the basename-sorted link order can be dropped.
- Define the species metadata fields (habitat requirements/biases from sprite appearance, domestication, base evolution, level-range enum, commonness) and tag all ~1000 species; reconcile image-based habitat biases with the species-to-biome stride mapping that decides types (the mapping decides a species' types; habitat flags decide where it appears).
- Check why Emerald caps land encounters at 12 slots and fixed slot ratios, and whether the encounter code can take dynamic table sizes and ratios.
- Define the per-method encounter tables (eager: walk, surf, grass, dark grass; lazy: rods, rock smash, headbutt, berry trees, spots), time-of-day and weather filters, and table recalculation when time or weather changes.
- Hardcode estimated encounter levels for non-level evolutions (stones ~29, trade, friendship, etc.).
- Audit existing rules against the layering principle (design.md "Layered generation"): e.g. route-story-index rules must depend only on county-story-indices and connectivity, and species/encounter generation must not need generated map data.
- Design chunked, per-frame generation (region/county/map and city plot points) and its CPU budget.
- Add the generator id to the save and the refuse-to-load check.
- Decide each "Exceptions to the normal rules" entry in `Emerald.md` (rule change or table change), then renumber the counties.
- Cross-check the Ports rules (design_region.md, "Ports") against the Surf land-count threshold and the sea-route levels.
- Randomized starters (design_pokemon.md, "Randomized starters"): build the four tables and the bonus list, the stride roll and its saved stride (for the rivals), and the rolling bag.
- New Game configuration menu and Nuzlocke mode (design_dev.md, "New Game configuration and Nuzlocke mode").
- Pokédex habitat view and region-tile species view (design_pokemon.md, "Pokédex habitat view").
- Building classification for chapters:
  - Classify buildings into three kinds: city-locations with actors (unique maps), generic-fill buildings with generic-fill NPCs (immutable, per chapter not per stage; a city asks a chapter for actors and for a count of them), and generic-fill buildings with actors.
  - Let a later chapter refer back to an earlier chapter's building by (`chapter_id`, building index); the id is enough because each run uses a subset of chapters and never reuses one.
  - Re-run `tools/building_audit.py` with only mutations (reads ignored), key items only, and TMs only for Secret Power (TM43).
