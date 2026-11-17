# Subplot design

Subplots as data that can be assigned to any place meeting their requirements, so they can be shuffled and NPC talk can be written once against final interfaces. Cutscene machinery is in `design_scenes.md`; city layout and hinters are in `design_city_npcs.md`.

## Subplots

- Each special location has a hardcoded, thin subplot.
- A number of city subplots are authored; each city randomly selects one.
- Subplots are modeled after Pokémon TV-show episodes. Ideally the player meets each gym leader during a subplot before battling them.
- Each subplot is linear. Each stage has ~2 NPC hints of the form "I heard something is happening at [city/location]", ~2 of the form "I saw something happening at [building]", plus story-state texts.

Initial subplot content comes from Emerald (`Emerald.md`). More follow from Red/Blue/Yellow, then other games. In the far future, more subplots are modeled after TV show episodes.

## Subplot abstraction

Goal: city and special-location subplots become data that can be assigned to any place that meets their requirements, so subplots can be shuffled (stage 5) and NPC talk can be written once against the final interfaces. Shuffling is done when, for some seed, two city-bucket subplots whose role and progression requirements both hosts meet are swapped, both are completable, and NPCs in both cities hint correctly.

### Scope

Only subplot beats need converting; trainer text, item pickups, trainer-defeated flags and NPC-hide flags stay as they are. The lock/key entries in `Emerald_walkthrough.md` fall into buckets:

- Main arc (rival, Aqua/Magma, Wally, and any storyline spanning several counties, such as Devon Goods): stays on Emerald's vars behind the adapter until the arc cutscenes are generalized (Milestone 3). A subplot takes place entirely within a city and the starts of its adjacent routes, so a cross-county storyline is main arc (owned by its first county) or is split into per-city subplots.
- City subplot and special-location subplot: converted, and the only buckets needed to unblock subplot shuffling.
- Gate (an HM, key item or blocking NPC that gates progress): one mechanism, a lock with a key.
- Gym: a city subplot in the end state (the gym leader is met during the subplot, as in the anime and Black/White). Temporarily a separate fixed beat per city, so the Emerald story can be abstracted first.

### Placement and requirements

- The end state picks a city's subplot early in city generation, so the city knows which story buildings to place.
- Each subplot declares requirements on the cities or locations that may host it: required buildings (by role, e.g. gym, shop, house with an NPC), required HMs, biome, and terrain (mountain, coast, island). These requirements are expected to stay long term.
- Progression requirements: the keys and badges a subplot consumes (Norman needs 4 badges; Petalburg Woods needs Cut), and the keys it produces. Keys a subplot produces may only open locks relative to its host (its outbound routes); between counties those locks are planner unlock steps, ordered by main-story-index (a key's index is below its lock's), so the rule holds after a move. A key that opens a fixed lock elsewhere (Cut opens Petalburg Woods and Rusturf) is a gate, not a subplot output. Consumed requirements are either relative to the host (the lower neighbor's subplot is done) or counted (badges >= N, so the host's step must come after the Nth gym step). A subplot may move only to a host where these hold, which is what makes a swapped subplot provably completable.
- In the hardcoded stage, buildings are fixed, so a subplot is assigned only to a city whose existing buildings satisfy its requirements, and subplots address buildings by role, not by map.
- NPC hosting: each host building keeps a few placeholder object events whose graphics come from a var (`OBJ_EVENT_GFX_VAR_0`, `VAR_OBJ_GFX_ID_0`..`F`), a generic script, and a temp hide flag (counts and spawning: "Building roles and NPC spawning"); a subplot fills them by setting the vars and flags in the host map's on-transition script. These vars are shared (the rival graphics, battle frontier and Route 117 already set them), so they are never persistent state. This is cheaper than spawning NPCs at runtime, which is kept for cutscene beats (an NPC running up to the player) via the generic runner.

### Interfaces

Defined for the end state, not for Emerald, so NPC talk never needs redesign:

- Subplot descriptor: linear stages; per stage the hint texts, story-state texts and trigger place (by role) (hints are the six distance-by-state entries of "City planning"); the requirements above; rewards; and the key/lock it produces. A subplot that unlocks a location describes its output as a key for the gates on the routes into that location, not as "unlock X". A subplot supplies its own blocker scripts (the person who stands in the way, with texts) for now.
- Stage accessor: reads and writes a subplot's stage by slot (the county's reserved vars, see "Story state storage" (design_story.md)), never by subplot identity, so moving a subplot moves no state. A subplot that is a main-plot step has no state of its own: not started or current comes from the main-story-index (see "Main plot planner" (design_story.md)), and done or partly done from the county's chain position (see "County chain"). Subplots internal to a county use that county's reserved vars. Callers cannot tell the difference. The main-story-index is not maintained until the main arc is ported (Milestone 3), so until then main-line stages come from the adapter over Emerald's vars.
- Built so far (`include/util/subplot.h`, `src/util/subplot.c`, hosts in `src/data/region/emerald_subplots.h`, tests in `test/subplot.c`): `SubplotDesc` (stages with the six hint entries relative to the stage, beat anchor, extra roles of a multi-building cutscene, hinter count and roles, a hint count, actors with their stage ranges, done-predicate for adapter stages; required roles, minimum badges, required keys and the produced key with its production stage), `GetSubplotStage`/`SetSubplotStage` by (county, slot) over the county slice (var 0 of a slice is the county chain position, `include/util/county_chain.h` with `ResolveChainPosition`, `IsChainMainSlotDone` and `DidChainJustFinishMainSlot` as pure functions and Get/Set/AdvanceCountyChainPosition over the var; vars 1-3 are slots; only counties whose slice is free of legacy vars can host), `GetCountySlotHint` as the NPC-talk query (distance plus a per-NPC selector, NULL means generic text), and the script specials `GetCurrentSubplotStage`/`SetCurrentSubplotStage`/`IsCurrentSubplotAvailable` (VAR_0x8004 = slot, county from the current map's MAPSEC, a city for now). Rustboro's Cutter's House (county (0,1), slot 0, one stage per HM given or not, anchored on the generic house), the Lilycove Museum exhibit hall (county (4,0), slot 0) and Mossdeep's Super Rod fisherman (county (5,1), slot 0, the generic house; the first port whose beat is a scene: his talk script checks the stage and runs `SCENE_MOSSDEEP_SUPER_ROD`, whose `advance` writes the slot var), Littleroot's Save Birch (slot 1) and Moving in (slot 0, anchored on the home) and Oldale's Rival and Pokédex (slot 0, anchored on the Oldale road) are ported; vanilla scripts advance the two tutorial subplots. Adapter subplots reuse `GetSubplotStage` and are read-only. A stage's hint entries describe that stage as a beat: before, current or after depending on whether the subplot is below, at or above it, so an NPC may recall any passed beat (`GetCountySlotHintAbout`). The last stage's close-after entry is never reached and may be empty. The consumers of requirements and keys are described under "Interface rules from paper tests".
- NPC-talk query: reads only descriptor, accessor and placement, and returns the text to say. It cannot tell a ported subplot from an adapter-backed one.
- Adapter: lets an unported Emerald subplot answer the accessor from its old vars (e.g. Petalburg gym "open" is `VAR_PETALBURG_GYM_STATE >= 6`). It is read-only, so NPCs can hint at unported subplots, but only ported subplots can be shuffled. Subplots are ported one at a time, each leaving `make check` green. The main-line half exists: `PlotGetDesc` (a done-predicate and a blocking or reported-only class per get), `GetPlotListStage(index)` (done and blocking masks over a list's gets) and `GetMainStoryIndex()` (the first list not done, scanning from 0) in `include/util/plot_stage.h`, `src/util/plot_stage.c`, with Emerald's predicates in `src/data/region/emerald_plot_gets.h` and tests in `test/region_plot_stage.c`. Only gets that vanilla forces are blocking, since a skippable blocking get would freeze the index; the rest are reported only (Emerald.md, "Step predicates for the read-only adapter").

### Text references

The script engine has no field-path variables. Text holds only the placeholders `{STR_VAR_1}`-`{STR_VAR_3}` (`gStringVar1-3`), a few fixed ones (`{PLAYER}`, `{RIVAL}`, `{AQUA}`...) expanded by `StringExpandPlaceholders` (`src/util/string_util.c`), and values that scripts put into the string vars with `buffer*` commands or a `special` function.

Authored text uses story references such as `$currentCity.gym.leader.name`, `$currentCity.nearbySmallCity.name`, `$stepCounty.city.theme.motto` or `$currentCity.pkmnCenter.nearBuildingName`. A reference is an anchor plus a path through the generated region data. A build tool turns each reference in a template into a compact reference id, and a single talk special expands the whole template in C into `gStringVar4` and shows it itself with `ShowFieldMessageFromBuffer` (not via the `message` command, which expands its source string into `gStringVar4` and would overlap it); this avoids the three-reference limit of `{STR_VAR_1}`-`{STR_VAR_3}`. Subplots are limited to a single county, so anchors are the speaker's own place (the NPC's city and building, for `$currentCity.pkmnCenter...`-style random talk), the subplot's own place, and, for hints about the main story, the responsible county of the current step (`$stepCounty`).

### Interface rules from paper tests

Checked against Petalburg gym/Surf, Devon Goods and Granite Cave/Flash in Emerald's scripts. The format absorbs all three with these rules, none needing a special case:

- Derived stages: a stage may be computed from requirements rather than stored. Petalburg gym's "locked, N badges short" and "ready" (`VAR_PETALBURG_GYM_STATE` 2-6) is just badges held, so a ported gym stores no counter and the other gyms stop writing to it (Emerald's four `addvar` calls plus `Common_EventScript_ReadyPetalburgGymForBattle` disappear). Badges held is the popcount of the badge flags (`GetBadgeCount`), not "gyms 1-4". A derived subplot is an adapter subplot whose stage predicates use `STORY_PRED_BADGES_GE`.
- Parameterized hints: a stage's `hintCount` names a number from a counted requirement (`SUBPLOT_COUNT_BADGES_SHORT`: badges still missing for `minBadges`). The hint special writes it into `{STR_VAR_1}`, which the template uses ("{STR_VAR_1} more badges").
- Stage predicates: an adapter-backed subplot declares its stages as an ordered list of flag or var predicates, highest first, and the first true one is the stage. Devon Goods is DELIVERED, then RETURNED, then RECOVERED, then STOLEN, else not started. `FLAG_DEVON_GOODS_STOLEN` is cleared when RECOVERED is set, so the order, not set-ness, defines the stage. `FLAG_DOCK_REJECTED_DEVON_GOODS` is a side branch that changes only the Slateport shipyard talk, not the stage.
- One subplot, several buildings: a stage's trigger place is a building role, and a cutscene may walk across maps. Norman's defeat triggers a walk to Wally's house (`VAR_PETALBURG_CITY_STATE` 4) where the dad hands over Surf, so the Petalburg gym host needs a "house with an NPC" role beside the gym, and the cutscene runner crosses both. Emerald keeps this beat in a second var (`VAR_PETALBURG_CITY_STATE`, six values); ported, it is stages 3-5 of the one gym subplot. Each stage anchors one building (`anchor.role`) and may list further buildings its cutscene visits in `extraRoles`; the host must have every role (`GetSubplotNeededRoles`).
- Keys: an HM key is a pair (HM given, badge held), and field-move checks read fixed badge flags (`src/field/field_move.c`), so shuffling which gym unlocks which HM must also change those. A key's production is a stage's reward, not a flag a gate reads directly: a descriptor's `producedKey` becomes held once the subplot reaches `producedKeyStage` (`IsMcGuffinProducedBy`), and `requiredKeys` must all be held before the subplot is available (`IsSubplotAvailable`, with the badge count). A key is named relative to the host (`SUBPLOT_KEY_HOST_POOL`, an index into the host county's McGuffin pool, so a moved subplot moves its key) or as a planner item (`SUBPLOT_KEY_ITEM`, which other hosts may consume). `CheckSubplotPlacement` is the static test for a move: the city has the roles, a subplot giving a main-plot get comes after the gym its badges wait for (any other waits at runtime), and every consumed key has a producer in an earlier county, or an earlier slot of the same one. `IsCurrentSubplotAvailable` is the script special.
- Cross-host effects: a subplot's completion may write another host's state. Beating Norman hides Wattson in his gym and shows him in Mauville, which is what starts New Mauville. That is a produced key ("gym 5 done") consumed by New Mauville's requirements, not a flag write.
- Main-arc steps are hosted by counties, not locations: a county places its steps anywhere (city, route location, plain route) as long as the player can and must take them in order. The Devon Goods letter delivery happens to sit in Granite Cave's 1F side room (`FLAG_DELIVERED_STEVEN_LETTER`), but its host is the Dewford county.
- Devon Goods stays main arc: it spans Rustboro (theft, return), the Rusturf tunnel (recovery), Dewford and the Granite Cave (letter) and Slateport (delivery), and its recovery key opens a lock back in the first county, so it is a callback, not four per-city subplots.
- Dark caves: Emerald has no script check for Flash. Dark floors (Granite Cave B1F and B2F are `requires_flash`) only dim the screen, and the engine lets the player walk them unlit. Flash is made a hard key by limiting an unlit player to about 6 tiles from the entrance of a `requires_flash` map. Steven's room is a warp off the lit 1F, so the letter delivery never needs Flash.

### Building roles and NPC spawning

- Roles are the `BuildingRole` bit set (`region_city.h`), with the list in `Emerald.md` ("Building roles"); the generic house, which the Petalburg gym/Surf paper test needs beside the gym, is `BUILDING_HOUSE`. A subplot addresses buildings by role only.
- Placeholder NPC objects per host map: 2 for each building role, 3 for a gym, 1 for a service building (its own NPC is fixed), 4 for a city's outdoor map, none for routes. The 16 shared `VAR_OBJ_GFX_ID_*` vars are set by the host map's on-transition script, so the limit is per map, not per game; a city map already spends 1-2 of them on the rival, so 4 is well inside it. Placeholders hide through temp flags (cleared on map load), so they cost no saved flags.
- Runtime NPC spawning is needed only for cutscene actors (an NPC runs up to the player and off again), because a generated route or city has no object in the right place. The generic cutscene runner builds an object template at runtime and spawns it (`SpawnSpecialObjectEvent`, as the player avatar does); to be confirmed when the runner is written. Subplot NPCs inside buildings never spawn at runtime: placeholders cover them, and the rejected alternative below explains why.
- Route hint NPCs (Milestone 1) are not decided here.

### County planning and subplot placement

A county plans its subplots algorithmically from the seed, in this order, and stores none of it (like the main-story order):

1. Unique buildings: a coprime-stride deal picks the unique buildings a county has beyond its generic ones.
2. Attached subplots: the county looks at the main-story beats it hosts, its locations and its unique buildings, and takes the subplots attached to them (Pokémon Tower: Fuji held by Rocket, a Marowak, and a Silph Scope as a consumed key). An attached subplot is never shuffled, so its requirements hold by construction.
3. Generic subplots: a second coprime-stride deal picks from the generic pool. A candidate that fails `CheckSubplotPlacement` for the county is skipped to the next stride step. The pool will be short once everything is randomized, so more are added from FRLG or custom.
4. McGuffins to provide: the county reads the main-story beats it hosts for the keys it must supply. The fallback NPC guarantees them: a generic one-role subplot whose first stage hints its requirement (badges, a key) and whose second gives the McGuffin. Completability then follows from construction.
5. Location order: the county reads which locations carry inter-county routes, and those counties' story indices, to order its locations.
6. Slot details: the county derives each subplot slot's details (route ends, building doorways) on demand.

### Plot kinds

- Location-tied (attached) subplots belong to a location or unique building and are never shuffled.
- City subplots are generic and dealt to cities by coprime stride.
- The main story is the cross-county plot: its steps are lists of gets hosted by counties, and the global main-story-index (a stored var, `4x + s` over counties and slots) says which step is current. Each county has 4 main-story slots, slot 1 usually being a callback hosted by another county.
- Every plot is a sequence of stages. It may require an intro McGuffin, and maps stage ranges, including "before" and "after" pseudo-stages, to hint texts. Every subplot ends with a reward: usually a McGuffin, sometimes a rare item (rare candy, mushroom), sometimes only story advancement.

### County chain

A county fulfills its main-story slots through its own subplots, and its progress through them is the county's var 0. The county picks, per main-story slot, which subplots supply the slot's gets and the intro McGuffins those subplots need; any surplus subplots are optional. Var 0 counts through the concatenated stages of the chain's subplots, in slot order, skipping each subplot's "before" and "after" pseudo-stages, and the accessor resolves a value to (subplot, stage) by cumulative stage counts.

Example: slot 0 provides a Silph Scope and a rival battle; the county assigns Nugget Bridge (1 stage, the rival battle), Copycat Girl (1 stage, a Rocket key, the intro McGuffin of the next subplot) and the Rocket hideout (3 stages, the Silph Scope), so var 0 runs 0 Nugget Bridge, 1 doll to Copycat Girl, 2 enter the hideout, 3 elevator key, 4 defeat the boss and take the scope. Completing the slot's last stage also advances the global main-story-index to the next slot (the callback in another county). When that resolves, the index returns to this county's slot 2, and var 0 continues from 5 through the subplots assigned to slot 2, then slot 3, whose completion opens access to the next county and advances the index there. A chain entry that needs an optional subplot done consumes a host-pool key the optional subplot produces. Optional subplots keep their own stage in vars 1-3.

## Alternatives considered

### Subplots bound to their original maps (shuffling only by swapping maps)

Keep each subplot's NPCs and events in its Emerald maps, and shuffle by swapping whole cities.

Pros:
- No new runtime NPC machinery.

Cons:
- Subplots cannot move between cities, which is what stage 5 needs.
- Conflicts with procedural buildings, where maps are not fixed.
