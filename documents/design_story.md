# Main story design

The main story: arcs, the main plot planner, story state storage, McGuffins and the story graph with its soft-lock argument. The main-story-index is the only stored story position between counties. Chapters are in `design_chapters.md`.

## Story and chapters

The story is expected to be the biggest hurdle, and most of the ROM space saved by dropping maps is expected to go to chapter content.

## Main story arcs

- A handful of hand-designed story arcs; each game picks one at random based on trainer ID.
- Reference points: Red/Blue (take down Rocket bases, then the leader) and Black/White (N and Ghetsis). The arcs should be deeper than Red/Blue.
- Story points are sprinkled across the counties according to their generated story indices.

## Arc progression

The progression mechanism is adapted from a design for far more powerful hardware, so it may need rework to fit GBA CPU and memory limits.

- Game start picks the rival arc and the story arc; map generation may insert some McGuffins (usually HMs).
- The start of the game is driven entirely by the rival arc.
- Rival and story-arc beats are steps of the main plot planner (e.g. "places Rival Battle 2"); the responsible county picks the specific route or city (and, for a city, building) within its own places.
- When the player finishes that route or leaves that building, the cutscene triggers. Usually an NPC runs up, says something, sometimes triggers a battle or gives an item, then runs off screen. The arc then advances to its next stage.
- When a city is generated or loaded, the code checks whether it (or an adjacent route) is the right place for each arc's current state, and uses that to pick "near" or "far" NPC texts for that arc. At least one NPC per arc gets an arc-state hint text. When the state advances, those NPCs switch to an arc-state-specific thank-you text.
- Chapters work the same way but take place entirely within a city and the starts of its adjacent routes.
- Arc and chapter state use the game's existing flag/var storage for small story state.
- Cutscene placement is deterministic, so it is never stored: each rival and story-arc beat has its own planner slot (slots 5 and 6), so beats cannot land on the same main-story-index, and the responsible county's private layer picks the place.
- Each route checks whether a rival or story cutscene belongs to it and plays it when the player enters. Each city or location likewise checks whether a cutscene belongs to its area and may place it anywhere inside its chapter, since the player cannot advance without completing the chapter anyway.

## Possible future extensions

In the far future:

- City chapters could involve the gym leaders more, as in Pokémon Black and White.
- Specific named grunts could take a bigger part in story arcs, like Jessie and James in Pokémon Yellow.

## Rival arcs

The first version always uses the Emerald rival arc (`Emerald.md`), preserving the cutscene text as much as possible.

Later versions add the Red/Blue/Yellow rival arc as an alternative the game can pick, then rival arcs from the other mainline games as far as possible.

## Story arc content

The first version always uses the Emerald story arc (`Emerald.md`).

Next comes the Kanto arc: Team Rocket pushes human exploitation of nature versus harmony with it, and tragically creates a man-made legendary it cannot control. They treat Pokémon as commodities, tools, and weapons. Beats include a Mt. Moon heist, a Lavender Town tragedy, and a corporate takeover. Giovanni is the 8th gym leader, and the player dismantles Rocket.

Arcs from the other mainline games are added as far as possible.

## Main plot planner

The main story is an ordered sequence of steps, counted by one var, the main-story-index. The sequence is computed from hardcoded ranges and a few small masks, never stored as a step list, and advancing is always a single increment. It uses only layers 1-3 and the McGuffin links of layer 2, not the routes: the route rules already guarantee a route from each county to the next county-story-index, and the planner follows that order.

Steps per county. County-story-index x owns four main-story-indices, `4x + s`, so the county is `index >> 2` and the step is `index & 3`. Level is x, so levels map directly from the index.

| Step | What happens |
|---|---|
| 4x+0 | County x gives a list of McGuffins |
| 4x+1 | Optional callback: county y(x) < x gives a list of McGuffins |
| 4x+2 | County x gives a list of McGuffins |
| 4x+3 | County x unlocks county x+1 (its first location, and so every route into it), via a McGuffin |

A McGuffin is anything the player is given or achieves: an HM, a key item, a guaranteed utility item, a gym, a rival or arc beat. Main-story advancement is itself a McGuffin, so a list may be empty. An empty step is passed at once on the same advance, and when there is no callback steps 0 and 2 merge into one group. The gets within a list are unordered; the county decides in what order to do them, using its chapter var, several vars, or flags. A list is complete when every get in it is done, tracked by the host county's var 0 position (see "Story state storage"), and only then does the index move to the next step.

Items and placement. Each item (a kind and an ordinal: the 8 gyms, the rival beats, the arc beats, the HMs, the utility items, the key items) has a hardcoded range of time blocks. Ranges of one kind are disjoint and ordered, so gym k precedes gym k+1, and each range must contain a valid block. Gym ranges are expressed relative to the Surf-lock county n (gyms 1-4 before it, gyms 6-8 between n and the Elite Four) and the Elite Four county's index (19 or 20, depending on the reserved county count). A per-kind mask records which blocks may take the kind (an arc beat only where the host has the special location it needs, a gym in any county except the Elite Four and the reserved ones (county 0's own block is the hardcoded tutorial, so it hosts a gym only as a callback, as Red/Blue's 8th gym is in county 1); a traversal item only where the routes into the next county can hold its obstacle; gym counties receive their settlements afterwards). From those, an item's placement is a pure function of the seed:

- Time block x: `hash(seed, kind, ordinal)` over the valid blocks in its range.
- Host: x itself, or, if block x has a callback and the item qualifies, the callback county y(x) (step 1). Each block has a callback with a configured chance, `hash(seed, x) < p`, so a block has at most one callback host and no item coordinates with another. An item falls back to host x if y(x) fails the kind's mask or already hosts a gym (evaluated over the gyms in ordinal order); if no prior county lies within the distance bound, the block has no callback; a gym and its HM go together (both callbacks or neither) except in the Surf block; Fly, and so gym 6, never moves to a callback.
- Callback county: `y(x)` is a hash over the prior counties within a grid-step distance bound of x (layers 1-2 supply the geometry): 3 county steps up to and including the block that gives Fly and unbounded after it, since before Fly the player must walk back. Fly's block is a pure function of its range.
- Callback chance p: roughly how often a normal Emerald run must revisit a prior town, about 2 of 23 blocks, raised to about 20%. The tutorial's callback at block 1 satisfies the minimum of one callback in every region.
- Step: a hash bit chooses step 0 or 2 for a non-callback item, and step 1 for a callback.

A callback is hosted in county y, which is already unlocked, and may hold a gym, a rival battle and a Rocket beat together.

- The next step after an index is index + 1, and its list is the items whose time and step match: evaluate every item (about 30 hashes) and cache the current list in EWRAM.
- A county lists everything it owns by evaluating the items whose time range includes its block, plus the callback blocks after it whose y(x) is this county (at most the bounded number of blocks before Fly, all later blocks after). It never reads another county's private data.
- Nothing is stored beyond the masks (about 12 bytes per kind), the county-story-indices in the county headers (5 bits each), and the cached current list.

Code: `include/region/region_unlock.h`, `src/region/region_unlock.c`, Emerald data in `src/data/region/emerald_unlocks.h`. `GetRouteLockKey` binds a route lock tile to the unlock of the county it gates (block `storyIdx - 1`; the reserved counties and so their Enclaves all use block E, unlocking at `4E + 3` below `MAIN_STORY_INDEX_POSTGAME`), `GetCountyUnlockIndex`/`IsCountyUnlockedAt` give a county's unlock point, an unlock with no get to name is implicit (a vanilla event stands in: Fallarbor's is Fiery Path entered), and intra-county locks come back as county-private with an id from the county's pool: the u8 id space splits into 24 pools of 8 by a coprime stride, with 64 cross-county ids hashed above them. Salts live in `include/constants/identity_salts.h`. A key names a planner item by (kind, ordinal), an HM through its gym, and every key is an item of the unlock's own block.

Unlock gating. Every "county x unlocks county x+1" step names an explicit McGuffin, so that non-adjacent counties can reference their key and so the main story, which manages the main McGuffin list, can ensure none is reused. The step is gated by every traversal McGuffin obtained in block x's lists (each an obstacle on the routes into x+1), and a jump key from layer 2 is satisfied by them when present; otherwise one key item or a blocker McGuffin from the global pool is used. All of them are obtained in block x's lists, never later. A gym enables one HM (8 gyms, 8 HMs; in the fixed Emerald order for this stage: gym 1 Cut, 2 Flash, 3 Rock Smash, 4 Strength, 5 Surf, 6 Fly, 7 Dive, 8 Waterfall) and an HM has no range of its own: it takes its gym's time. Both are items of the same host and step (except in the Surf block), so the player does the city's chapter (which hands out the HM) and the gym in any order, then uses the HM on the obstacle on the route into the next county. A gym and HM placed as a callback gate the unlock of county x+1 of that block, not of their host's next county. An obstacle must be buildable on the routes into x+1. The land obstacles (a Cut tree, a Strength boulder, a Rock Smash rock, a short dark tunnel for Flash) are buildable on any route, so only the water HMs (Surf, Dive, Waterfall) depend on the layer-1 terrain of x and x+1, which their block mask checks; if no block in a water HM's gym range qualifies, the HM is still given at its gym's time, and that unlock uses a blocker McGuffin instead. The Elite Four unlock is the end-game exception: it needs all 8 badges, not a McGuffin from the previous block.

Surf. Layer 2 places the Surf lock at step 4n+3 of its dead-end county n, so block n must give Surf and gym 5, the Surf gym (gyms 1-4 are in counties before n and gyms 6-8 after it). Both are items with time n. Either, both or neither may be in block n's callback, independently: a callback item is hosted by the callback county y(n), so it may be acquired elsewhere and brought back, and a non-callback item is hosted by n. The host of the Surf gym is a gym county and has a settlement. n is at least 5, so gyms 1-4 fit in four distinct time blocks 1 to n-1 (block 0 is the scripted tutorial; the starting county can host a gym only as a callback in a later block), and at most four below the Elite Four county's index, so gyms 6-8 fit after it. Step 4n+3 then unlocks the ocean county that holds the Surf lock, whose routes are gated by Surf. Counties indexed before Surf's lock never rely on water crossings; ocean counties' route generation can assume the player holds Surf, with no bridge special cases (ferries are teleports between ports, see "Ports" (design_region.md)). A jump key is likewise an item of the key county n with time n.

Settlements. The number of settlements is not drawn independently per county: counties 0 and 1 and the Elite Four county always have one, plus a random 8 to 13 more. The gyms are placed first and each gym's county takes a settlement from that random count (unless it is county 0 or 1, which already have one; the Surf gym's county is a gym county like any other), then the rest are distributed among the other counties. The capital always has the maximum population.

Traversal McGuffins. An HM (other than Fly) is only ever used as a route-unlock McGuffin, since it unlocks a new way to traverse the map; chapter gates and blockers never require one. A few other special items are limited the same way (the Wailmer Pail, Go-Goggles, the Devon Scope; the full list comes from the McGuffin bucketing). Optional items hidden behind an HM obstacle on a route are still allowed, as route traversal. The planner guarantees that every HM is given before the end game, since a game with missing HMs would feel wrong. In the Emerald stage the special-item and HM McGuffins are fixed.

Blocker McGuffins. A blocker is a person who stands in the way until a boolean is set; talking to them or walking past triggers a script, which may also move the player back one square. Emerald has a limited number of these locks, which Milestone 1a enumerates. The global pool of blocker McGuffins is meant to grow to about 96 (24 x 4, so every main-story-index could have one), each with generated text. For now chapters provide their own blocker scripts; later the global list is extended and chapters may draw generic ones from it.

Guaranteed utility items. Some items appear in every game in a typical window but gate nothing: the Bicycle, the Old, Good and Super Rods, the Itemfinder, the Coin Case, the Exp. Share, the Town Map and the PokéNav. Each is an item with a host range (the Bicycle in the first half, the rods early, middle and late). They are not locks, so they never affect the soft-lock proof, and a county may still use one as an optional gate (a ledge or Cycling Road that needs the Bicycle). Items that do gate something in Emerald (Go-Goggles for the desert, the Devon Scope) are ordinary McGuffins. The set is open: any other item every game should hand out belongs here.

Rules:

- Locks live on routes, never on locations. A location's own tiles and data never change when it unlocks; what changes is every route into it. The route generator therefore enforces the unlock condition of the route's later-unlocking end location (its key, e.g. the McGuffin id) as a gate placed at the route's earlier-unlocking end, so the player cannot reach the later location until the key is held. Conceptually a route is open exactly when both its end locations are unlocked, so a county's unlock step (of its first location) opens every route into it at once, and a county's private unlock of another location opens every route into that one; a route between two already-unlocked locations has no gate. Which end is earlier comes from the county-story-index order for first locations, and from the counties' private order inside a county; inter-county routes may read both counties' data (layer 6), so this is known where the route is built.
- A list's gets are hosted in the host county's own places, or on an inter-county route bordering that county (layer 6 allows this). A beat that plays in Petalburg is a Petalburg step, never a Littleroot one; a callback is the way an earlier county gives its gets later. Unlock step 3 of block x is hosted by county x.
- Unlocking inside a county, and which place hosts a get, is entirely that county's private choice. When an inter-county route unlocks is fixed by the step order; how it is blocked is usually left to the county, but a step may specify it (HM Cut).
- Optional content: an inner location (not a route) may gate on a key from anywhere earlier in the story, as long as the player can finish the main story without ever entering it. Such a gate never affects the main-story-index or the soft-lock proof. Because it is private to its county, NPCs elsewhere cannot hint at it unless it has an optional-story index (see `remaining_work.md`).
- A key's index is strictly below the index of the lock it opens: the unlock's McGuffin comes from block x's lists, which precede step 4x+3.
- Cross-county McGuffin ids derive from the item (a hash of the seed, kind and ordinal), so both counties on a route agree on its lock without reading each other's private data. County-internal McGuffins draw from per-county reserved pools, partitioned by a coprime-stride mapping like the species-to-biome one. The two partitions are disjoint, so a hash never collides with a county's reserved id.
- NPC hint text asks the planner for the list at the current main-story-index, so the planner is the top layer of the chapter descriptor (see "Chapter abstraction" (design_chapters.md)).
- Dark maps. A `requires_flash` map is only fully usable with Flash (see "Dark caves"). The planner knows no maps, so this binds the county's private placement: a get, a lock's key, or the path to a later unlock that lies beyond the unlit limit (about 6 tiles from the entrance) of a `requires_flash` map must sit at a main-story-index after the one that completes Flash and its badge (gym 2 in Emerald). A get in the same list as Flash is allowed only if the county's script orders Flash first. It holds when Flash is a callback: a callback at step 1 of block x completes at `4x+1`, so a dark get may use `4x+2` onward but not `4x`. A county that cannot satisfy it (Flash's block is later than every step it could use) keeps its dark map's contents inside the lit part; the part beyond the limit is optional content.

Guarantees the earlier layers must provide:

- Counties 0 and 1 both have a city and are adjacent, with their cities close to each other and any special location further away, so a direct route between them exists.
- Every gym's county has a settlement (the Surf gym's host is a gym county; see Settlements).
- Story-arc beats claim special locations or cities in layer 3, as reserved legendary locations do, so an arc beat's mask always has a valid host.

Relation to the route-story-index. The main-story-index governs everything between counties; the route-story-index orders places only within a county (and as a level and difficulty hint). The soft-lock proof of "Story graph" is an induction over main-story-indices:

1. Block x is reached only after step 4(x-1)+3, which unlocks county x's first location from a lower neighbor (the layer-2 growth constraint guarantees one, except for the counties that layer-2 step 5 indexes with no indexed neighbor, tracked in Milestone 10 of `remaining_work.md`), so every list in block x is hosted in a county that is already reachable (a callback's host is lower than x, so already unlocked).
2. A list's gets become available exactly when the main-story-index reaches its step, and the host county produces its keys within places already reachable; the next step is reached only when all the list's gets are done. A key's index is below its lock's index.
3. The index only grows, so reachability only grows, and every step is reachable and completable when reached, provided a county's private dependencies between its own gets are either absent (any order works) or form a strict order, and either way the whole list is completable, so that no order of play strands the player.

Completion is level-triggered: every get's content is gated on the index being at least its list's step (a gym cannot be beaten before its county's turn), and on every advance and on map load the current list's completion mask is re-checked, so no completed get is ever lost. A counted requirement such as "4 badges" is re-expressed as "after the 4th gym item". A gym that Emerald revisits later (Petalburg's gym 5) is reproduced by a callback.

Opening. The tutorial is an ordinary instance of the model, with hardcoded lists (the first ~8 indices; normal items may spawn in the remaining steps of block 1 once the tutorial ends):

```
0   Littleroot gives [Starter]
1   (no callback)
2   (no McGuffins given)
3   Littleroot unlocks Oldale via [Starter]
4   Oldale gives [Rival Battle 1]   (the rival's battle is the trigger that sends the player back to Birch)
5   callback: Littleroot gives [Pokédex]
6   Oldale gives (generated as normal)
7   Oldale unlocks Petalburg via [Pokédex]   (the sketch-artist lock)
```

Vanilla stage overrides. Vanilla Emerald cannot come out of the hash formulas, since the step bit, the callback hash and the chance are random. While the planner's randomization bit is off, each item instead reads a fixed `{block, step}` from a ROM table, with a fixed host for each callback block (`Emerald.md`, "Planner inputs"; `GetGymSlot` is the gym part). This is not the rejected explicit step table: it only replaces the hash outcome for the hardcoded stage, it stores no step list at runtime, and the generic ranges and masks stay in force. A test checks that every vanilla slot lies inside its kind's range and mask, so the exceptions are exactly the numbered ones in `Emerald.md`. Code: `include/region/region_plot.h`, `src/region/region_plot.c`, Emerald data in `src/data/region/emerald_plot.h`, test `test/region_plot.c`. A gym item carries its HM (`PlotHm`); an HM displaced from its gym's slot (Strength) also gets an `hmOnly` entry, and the vanilla-script gets (tutorial, Wally's Ralts, Shipyard, Elite Four, Champion) are `PLOT_KIND_SCRIPTED` slots. Wally's mask includes the Elite Four block so Wally 2 passes; an arc beat's mask covers its block only (its callback host is authored).

Open: the exact callback chance and distance bound (tuned from play); the hardcoded ranges for each kind's ordinals (placeholders at first; each is listed for the user to check); the end-game blocks (the Elite Four steps, the reserved legendary counties); whether an HM obstacle on a route counts as the unlock of the location behind it; the size of the McGuffin id pool.

## Story state storage

Emerald stores all story state as flags and vars in `SaveBlock1` (`include/global.h`), accessed through `src/minigames/event_data.c` (`FlagGet`/`FlagSet`, `VarGet`/`VarSet`) and driven by map scripts (`data/maps/*/scripts.inc`, `data/scripts/`) using `setflag`, `setvar`, `addvar`, `goto_if_*`, `switch` and `map_script_2 VAR, value, handler` (which fires a handler when a var holds a value, the usual way a stage triggers a cutscene).

- Flags are single bits (`u8 flags[]`, 2400 flags / 300 bytes). IDs run `0x000`-`0x95F`: temp (cleared on map load), trainer-defeated (`0x500`+), system (`0x860`+, includes the badge flags `FLAG_BADGE01_GET`..`08`), and daily. About 380 are already unused. `FLAG_HIDE_*` flags control whether an NPC exists.
- Vars are 16-bit (`u16 vars[]`, `0x4000`-`0x40FF` = 256 vars / 512 bytes, the first 16 temp). About 29 are already unused. A var is a stage counter for one storyline.
- Limits: the saved ranges are fixed by `FLAGS_COUNT` and `VARS_END`, and growing them costs `SaveBlock1` (and so save sector) space. The ID space allows far more: flags up to `0x3FFF` and vars up to `0x7FFF` before the unsaved special ranges (`SPECIAL_FLAGS_START` `0x4000`, `SPECIAL_VARS_START` `0x8000`). IDs `>= SPECIAL_*_START` are RAM scratch and are not saved.
- Emerald's own dependencies are implicit counters. Example: Norman's gym requires 4 badges because each of the first four gyms does `addvar VAR_PETALBURG_GYM_STATE, 1`, and Norman `switch`es on it. These are replaced by explicit lock/key checks when McGuffins are abstracted.

The full var inventory, with each var's use and what could be merged, is in `vars.md`. Only `0x4020`-`0x40FF` (224 vars) can hold persistent state: `0x4000`-`0x400F` are temp vars cleared on map load and `0x4010`-`0x401F` are shared object-graphics vars. So reservations are named regions of constants, not positions at the start of the var space.

Regions (badge and HM flags stay as they are):

- Main story: one var, the main-story-index, a position `4x + s` over the counties (under 100 values, plus the end game) that always advances by one (see "Main plot planner").
- Optional inter-county stories: about 4 vars (`VAR_OPTIONAL_STORY_0..`), each a linear stage index, so optional stories run in parallel with the main story and with each other, and NPCs in other counties can hint at them. Only content with a real optional story takes one; the Trick House uses inner-county indexing.
- List completion: a list is done when the stored main-story-index has moved past it. Within the current list, a get is done when the hosting county's var 0 position (below) is past the chapter and stage that produces it. Nothing else about the main line is stored (see "County chain" (design_chapters.md)).
- County-internal state: each county gets one contiguous slice of 4 whole vars, which no other county reads. The 24 slices are contiguous with each other, at the start of the range (`COUNTY_VARS_START`, 96 vars). This section is the only place the layout is specified; other documents and comments link here.
  - Var 0 is the county's position in its current main-story slot, a bit field: bits 15-12 the chapter index (0-15, so at most 16 required chapters per slot), bits 11-8 the stage of that chapter (0-15, at most 16 stages), bits 7-0 chapter state, which the current chapter may use freely and which is cleared whenever the chapter index advances. A chapter's last stage does not mean "done": finishing the chapter advances the chapter index.
  - The global main-story-index (a global var, not part of any slice) says which slot is current, so a county's var 0 only tracks the county-required-chapter-order inside it. Incrementing the main-story-index resets the var 0 of the county that hosts the new slot to "the first required chapter, stage 0, state 0". That county is found from the new index (`index >> 2` is the county-story-index, mapped to the county id, then `GetCountyVarSlice` gives its slice), or is the callback's host for a step-1 slot; it is not necessarily the previous host. The chapter state (bits 7-0) is also cleared whenever the chapter index advances within a slot.
  - Chapter order. The required chapters of a slot have a county-chosen order; a few chapters are optional and sit off the order. Bits 15-12 hold the chapter's index in the slot's unsorted list (declaration order), not its place in the order, so reads never need the sorted order in memory: the county sorts only when the chapter index advances, writing the unsorted index of the next chapter in order. The reset on a main-story advance is the same operation, so it writes the unsorted index of the order's first chapter (not necessarily 0).
  - Skip-ahead. Chapters may request bits for state that is not the current position (see below). An early-completable chapter (Aqua can be fought before Magma, say) sets its bit when done. When the chain advances onto a chapter whose "already done" bit is set, it advances again, repeating until it reaches a chapter that is not done or the slot's end. So Magma then Aqua and Aqua then Magma both land after both, and the order only matters when the player follows it.
  - Bit masks. A subplot (usually an optional one, very rarely a required chapter) may request N bits. A cursor over vars 1-3 hands out masks in order; a subplot that does not fit in the rest of the current var starts at the next var (no repacking or other optimization, since lookups must be fast). Once every subplot has its range the cursor never moves again. Vars 1-3 hold 48 bits, so a county's off-chain subplots are limited by bits, not by slot count.
  - A finished slot's position is overwritten by the next reset, so nothing in var 0 outlives its slot. Anything a later list, hint or script needs stays in a flag, a requested mask or the main-story-index: "this beat is past" is answered by the index being past that slot.
  - The main-story-index is stored (the first reserve var, `VAR_MAIN_STORY_INDEX`), because var 0 resets and so cannot be re-derived from it.
- Minigames and facilities (Trick House, Battle Frontier, lottery, contests, size records, cycling records, Glass Workshop, ...; 59 vars): each gets its own hardcoded contiguous range in the minigame block, after the county slices and the reserve, since each appears at most once per run and has custom code. Their Emerald vars are scattered through `0x4020`-`0x40FF` today, so they are renumbered into this block.
  - What gets a dedicated range: content with a lot of non-script code (wanted in nearly every run, hard to fork), or many steps doable out of order (fits neither the main story nor a linear chapter). Repeatable content starts with a dedicated range; repeatable chapters may absorb it later. Fossils may be found in several counties but are revived in one place, biased toward the gym 5-8 timeline.
- Flags for county-internal McGuffins come from per-county reserved pools (see "Main plot planner").
- Engine vars (`vars.md`, "Keep") (37 vars) are renumbered into a contiguous block at the end of the range. Positions that look fixed are not: the ROM header's Dex var is `VAR_NATIONAL_DEX - VARS_START` (`rom_header_gf.c:118`); only struct-cast and 32-bit pairs must stay adjacent. Renumbering breaks old saves, which is acceptable.
- Arc vars: the story arc (the Team Rocket-style plot) gets one global var of its own in the reserve for its stage, so it never takes county bits. The rival arc has no var of its own: it lives in the twelve rival-team vars (six per rival, design_pokemon.md, "Rival teams"), each holding a species id in its bottom 11 bits (up to 2047) and whatever the rival story wants in its top 5 bits. A county's chapters read an arc's stage from there, and only the cutscene's place is county-private.
- Reserve: 32 vars between the county slices and the minigame block hold the main-story-index, the story-arc var, the twelve rival-team vars and about 4 optional-story vars (18 in all), with 14 spare.
- `VARS_END` does not grow: 224 vars = 96 county + 32 reserve + 59 minigame + 37 engine. Five per county would fit (224 - 96 keep - 5 story = 123 >= 120), but is not planned.
- Legacy block: Emerald vars not yet retired (44: the main-arc and chapter vars) sit in the last 12 county vars and the whole reserve. Their slices are unusable until the vars are retired, so only the first 21 county slices are free at first. A county's slice is its id except that Littleroot and Oldale (ids 19 and 14) swap slices with counties 0 and 1 (`GetCountyVarSlice`), so the tutorial chapters can be stored before the legacy block shrinks that far; the swap goes away once the slices are free. A test checks that the blocks do not overlap and that the legacy block stays inside the county tail and reserve.

Supply: Emerald left 56 vars unreferenced, and 20 more were removed or moved to flags (`vars.md`, "Moved to flags or removed"), so the regions are allocated before any Emerald var is retired. Emerald's main-arc vars (about 40) are retired when the main arc is ported, and city and special-location vars (about 25) one chapter at a time, each leaving `make check` green. A test checks that regions do not overlap and stay in range.

## McGuffins

A McGuffin key is whatever gates progress until a condition is met: an NPC blocking the way until a chapter reaches a certain state, an HM needed to pass an obstacle, or a special plot item such as the Poké Flute to wake a sleeping Pokémon.

## Story graph

The set of places the player can reach is an implicit "story graph", ideally never held in memory.

- Reachable area only ever grows. The player gains route entrances/exits and cities, and never loses access.
- Exception: the initial rival battle in the lab temporarily reduces the reachable area, but the battle restores the prior reachability, win or lose.
- The reachable area is bounded by McGuffin locks, and at least one of those has a McGuffin key inside the reachable area.
- Usual pattern: within a county, a lock blocking access to route-story-index N has its key at N-1; between counties, a lock's key is given at a lower main-story-index (see "Main plot planner"). As long as the reachable area only grows, this makes the story completable.
- Soft-locks are prevented by construction, not detected afterwards: the generation algorithm must make them impossible, with no seed-sweep or solver check. The rules above (key at N-1, only-growing reachability) are the means.
- Soft-lock protection: if the route from a city to a higher route-story-index is always locked on the city's chapter being resolved, and a chapter's resolution only requires McGuffins in the city or in adjacent routes with a lower route-story-index, accidental soft-locks become much less likely.

The route-story-index parts of this section apply within a county (and to the starts of its adjacent routes); between counties the induction over planner steps in "Main plot planner" applies instead.

How accessibility is evaluated without a stored graph: every lock is checked locally where the player meets it (a blocking NPC, an HM obstacle, a gated route entrance). A lock on a place with route-story-index r opens when the flag of its key is set, and the key's location is derived deterministically from r (at index r-1, or the lower city's chapter completion). No global graph or reachability computation is ever needed.

Why soft-locks are impossible by construction, as an induction on route-story-index:

1. Claim: a player who has completed everything at indices below r can reach every place of index r and obtain its key.
2. Base: index 0 is the start county's city, where the player starts.
3. Step: a place of index r is entered from a place of lower index (the layer-2 growth constraint guarantees a lower neighbor; inside a county the +0, +1, +2, +3 chain enters each place from an earlier one). The only gate on the way requires keys found at index r-1 or at the lower city's chapter, and chapters require only McGuffins from their own city or adjacent routes of lower index. So nothing needed is at index r or above.
4. Monotonic reachability: reachable area never shrinks (apart from the initial rival battle, which restores it, win or lose), so no key can be lost by moving on.
5. Hence the claim holds for every r, and the story is always completable.

The argument depends on these rules, each of which must be kept:

- Keys are things that cannot be permanently lost. Key items and HM moves already cannot be sold, tossed, or forgotten in Pokémon games, so they are safe; ordinary items and ordinary moves must never be keys.
- No lock may be placed on a place at index r unless its key is placed at index r-1.
- Cutscenes of different arcs have distinct indices, and a cutscene never gates progress other than inside its chapter.
- A battle loss may block progress, provided the player can still reach wild encounters, shops that sell Poké Balls, and money for balls, so they can catch more Pokémon or level up. A loss only teleports the player to the last Pokémon Center, so battle McGuffins are rarely soft-locks. The initial rival battle is the exception that must unlock on a loss too, because the player cannot yet reach grass to level up.

Emerald's opening as an example:

- After the initial rival cutscenes, once the player has a Pokémon and has battled the rival, Littleroot Town, Route 101 and Oldale Town are accessible.
- Route 102 is locked: a sketch artist blocks its entrance.
- Route 103 is locked in the middle by Surf.
- Route 103 contains a McGuffin: the rival battle, which unlocks the next rival cutscene at Birch's Lab.
- Birch's Lab contains a McGuffin: the Pokédex and Poké Balls, which unlock the Route 102 artist lock.

## Open questions

- The hand-designed main story arcs: how many (a handful), how much deeper than Red/Blue, and how their beats map onto county story indices.
- The chapter data format (linear stages, hint texts, story-state texts) is settled for the ported Emerald chapters (`design_chapters.md`); authoring the special-location and city chapters (TV-episode style, introducing each gym leader before their battle) is not designed.
- **Content roadmap:** the intended order is the Emerald rival and story arc first (preserving cutscene text), then the Kanto story arc and the Red/Blue/Yellow rival arc, then other mainline games; chapters come from Emerald, then RBY and others, and eventually TV-episode chapters.
- **Gym-type chapters:** a gym type (Psychic for Mossdeep, Rock for Rustboro) might carry its own chapter so the leader meets the player beforehand, as in Pokémon X/Y; undecided.
- **Story-graph proposals:** the layer-2 growth constraint, the start county always having a settlement, and key items and HMs as the only keys are proposals not yet confirmed; the first-place rule is not yet checked against Emerald's actual layout.

## Alternatives considered

### Reserving the first 32 vars by position

Reserve Var0 for the main story, Var1-5 for optional stories and Var6-31 for per-county state.

Pros:
- Simple numbering, easy to remember.

Cons:
- `0x4000`-`0x401F` are temp and object-graphics vars, referenced throughout the map scripts and not persistent.
- Moving them would mean rewriting every script that uses them for no gain; named regions in the persistent range give the same layout.

### Surf at a fixed county-story-index

Make ocean counties eligible only from index 15, with Surf at the first ocean county.

Pros:
- Predictable and mainline-like.

Cons:
- Misaligned whenever the land count is not about 15, with Surf arriving while many land counties remain or never.
- Fragile if terrain generation changes the number of ocean counties.

### Chapters and 32-slot blocks with kind strides

Index = chapter x 8 + slot kind over 0-511, with each county assigned a chapter and steps placed in hash-chosen chapters inside windows computed by a left-to-right scan; and the later variant of 32 slots per county, each slot's kind its index mod 8, with callbacks at every 17th index.

Pros:
- Fine-grained ordering of individual steps within a county.
- Chapter variant: unlock times are flexible; steps can land anywhere between constraints.

Cons:
- The kind-slot arithmetic is unnecessary once a list can hold any mix of McGuffins, and spends 768 indices where about 100 suffice.
- Chapter variant: the scan produces a resolved list of placed steps that must be held in memory, a county cannot enumerate its own steps without evaluating every kind and ordinal, and unlock timing is not fixed, so levels do not map directly from the index.

### Explicit main-plot step table

Generate the main plot as a list of steps (county and action per step) and keep it in memory.

Pros:
- Any step order is expressible, with no slot constraints.

Cons:
- Each entry is a full (county, action) record (100+ of them), instead of a few bytes of mask per kind with the slot derived from a hash.
- Nothing structural keeps rival, arc, gym and HM steps from landing on the same index.

### Main story owned by one county at a time

A var holds the county owning the main storyline and a second var the stage within it; a storyline may span counties while its first county still owns it.

Pros:
- Very little state.

Cons:
- A county's story would inject elements into foreign counties, which would have to be decided in the county headers and could not be deferred to private details, breaking independent county generation.
- The story circles back to counties (Petalburg's tutorial and gym 5), which a single owner cannot express.

### One whole var per chapter (var 0 reserved)

Each stateful chapter owns a slot var (vars 1-3, so 3 chapters per county), var 0 is unused, and chain position is modelled from the chapters' own stages and never stored.

Pros:
- Scripts compare a plain var against a stage number (`coord_event`, `map_script_2`, `call_if_eq`), with no accessor.
- Chapters of one slot can be played in any order with no extra mechanism.

Cons:
- At most 3 stateful chapters per county, and a parallel errand costs a proxy chapter.
- A main-story advance cannot reset anything, so "which beats are past" is re-derived from every chapter's stage forever, and the main-story-index has to be derived by a scan.
- Stages need a whole var each, although most chapters have 2-8 stages.

### More vars per county

Raise `VARS_PER_COUNTY`.

Pros:
- More chapters per county.

Cons:
- Needs the legacy vars retired first, so it blocks progress.
- The packed position makes it unnecessary.
