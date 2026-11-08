# Pokémon Venture — Design

Current state of the design for the mod. Organized by topic; describes what the design *is*, not how it evolved (history lives in `notes.md`, open items in `remaining_work.md`).

## Overview

The mod procedurally generates a Pokémon region rather than using a fixed map. (The user describes the idea as "overly ambitious"; the full concept is still being collected.)

The goal of the region generation below is a game region map with cities, routes, and special locations that mirrors what a normal mainline Pokémon game looks like.

## Layered generation

Generation is a stack of layers, each computed only from the layers above it, never from its own neighbors or from later layers:

1. Region map: terrain types and biomes per county, from the PRNG seeded from the region seed (today the identity seed itself; see "Regions and multiple regions") (see "Seeds" below).
2. Region map: county-story-indices, from layer 1 plus a couple of McGuffin boundaries.
3. Region map: county populations, city locations and names, and special location IDs and positions, from layers 1-2.
4. Region map: which counties connect to which, from layers 1-3 (using only county-story-index adjacency and the capital, never planner data).

    The main plot planner (see "Main plot planner") needs no routes: its items come from layers 1-3, and layer 4's route rules already connect each county to the next county-story-index. Layer 3 runs in this order: (a) the settlement quota: counties 0 and 1 and the Elite Four county always have a settlement, plus a random 8 to 13 more; (b) the gym hosts (the Surf gym's host is the Surf-lock county or its callback county), each picked from its hardcoded range, which gives the gym counties their settlements first; (c) the remaining settlements, distributed among the other counties, then populations and special locations (with arc-beat reservations); (d) the other items' hosts from their masks.
5. Each county: the details of its own region tiles (including exact route placement) and its route-story-indices, from region layers 1-4 only. No county layer depends on any other county's data, including an adjacent county's route-story-indices. Counties may have sub-layers like the region map.
6. Field maps: generated from region layers 1-4 and the county they are inside, never from another county's data or from adjacent field maps. The exception is inter-county routes, which may use the data of the two counties they connect.
7. Further layers continue the same pattern.

This forces single-pass generation with no recursion or transitive operations, and keeps memory use minimal. Any new rule must be checked against it: a rule that needs data from a later layer, or from a sibling's same-layer output, breaks the design.

## Region generation

- The region map is a grid of 6 columns x 4 rows of "counties" (roughly the 5-6 columns and 4 rows of cities in Kanto).
- Each county is procedurally assigned a terrain type, a biome, a population, and a special location.
- A "story order" over the counties is generated, which sometimes designates McGuffin "keys" and the "lock" counties they open.
- One county's city is picked as the "capital".

### County layout

- Each county is 5x4 "region tiles" (5 wide, 4 tall). A region tile is one cell of the Hoenn region map, which is 28x15 cells (`MAP_WIDTH`/`MAP_HEIGHT` in `src/field/region_map.c`; cell contents in `src/data/region_map/region_map_layout.h`).
- 6x4 counties of 5x4 tiles is 30x16, so the first tile column of the first county column, the last tile column of the last county column, and the top tile row of the top county row are off limits, leaving 28x15. The top row is dropped (not the bottom) because the city name overlay covers the top-right 13x1 tiles and the Nav covers the bottom-right 13x2 tiles, so this leaves both the top and bottom counties 2 usable rows in those corners, where dropping the bottom would leave the bottom-right county only 1. The 6x4 grid of 5x4-tile counties was chosen over the inverse (5x4 counties of 6x4 tiles) because it yields more locations on the map and shorter routes between them.
- If a county's population is above zero, it places a settlement at a random region tile, except that the county's bottom row and rightmost column are excluded (unless the county is in the bottom row or rightmost column of counties), which keeps space between cities. Population is 0-3 (2 bits); population 0 never generates a city.
- Every county always has a special location (a landmark or detour; a pitstop only as the emergency fallback below), whether or not it has a city. A reserved end-game county may instead hold only an Enclave (see Enclaves).
- Some counties instead hold a reserved end-game special location (see Reserved legendary locations). Layer 2 reserves them while assigning county-story-indices, so they are known before layer 3 places anything.
- With a city, the special location is placed either in or adjacent to the city (50%), or on a random non-adjacent region tile (50%).
- Without a city (population 0), the special location is necessarily not in a city, and it is what routes attach to.

### Reserved legendary locations

A few special locations are reserved for legendaries: 2-3 after roughly the 7th gym (each holding a pair or trio of legendaries), and 1 solo location after the Elite Four. They are reserved on the region map, as part of the initial county-story-index determination, and relax the usual special-location rules:

- If the county has a city, routes are generated for the city as if the reserved location did not exist, then one extra route runs from the city to the reserved location. That route and the location get end-game route-story-indices (100+), above every ordinary index.
- If the county has no city, routes generate as normal, and the reserved location is the county's ordinary special location and what routes attach to.
- In a county with a city, McGuffins gating progress out of the county require only the city's subplot, never the reserved location's. The reserved location is an optional dead end.

### Enclaves

An Enclave is a special location with no routes and no route-story-index, reached only by an NPC script (a boat, a ticket), such as Southern Island. It is a third place class alongside landmark/detour/pitstop (below), and not a route tile.

- A reserved end-game county may hold only Enclaves (a layer-2 header flag, `enclaveOnly`). Whether an Enclave appears on the region-map screen is a per-place flag (`enclaveOnMap`), like the landmark class: a mapped Enclave shows once its unlock point has passed, and is Fly-selectable only if it has a safe heal location; a hidden Enclave is reached only through the port menu. Emerald: Southern Island and the Battle Frontier are mapped, Mirage Island and Artisan Cave hidden. That county then has no city and no routes at all, and no county-story-index neighbor rule applies to it. It is unlocked like any county: the main plot planner places its unlock at step 3 of another county's block. Every port city lists the Enclave as a destination once the story has passed that unlock point, so the destination menu is the only access.
- Any other county may also hold Enclaves, but must have at least one non-Enclave special location (or a city), so it keeps something routable.
- An Enclave never counts as the county's ordinary special location, and McGuffins never depend on it.

### Settlement types

Population selects one of three generation algorithms:

- 1 = village: buildings never touch each other, and paths run everywhere intrinsically.
- 2 = town: a few specific paths run through it; buildings often touch each other, and the layout is not very organized.
- 3 = city: paths are mostly straight horizontal or vertical lines, and buildings mostly fill the grid.

### City themes

Some towns and cities have a theme.

- Themes often have requirements, such as being on an island, being far from the capital, being low level, or being ocean-adjacent.
- A theme affects which sprites the settlement uses and its gym type, and carries its own NPC texts so NPCs talk about their town appropriately.
- Themes are picked at the region layer(s), so gym types can be distributed well across the map.

### Gym type order

Gym types follow a general but subtle pattern in the mainline games: some types tend to be early gyms and some late, and the Elite Four generally prefers types toward the late end. Gym assignment (including theme-driven types) should respect this as a soft bias, not a rule.

Average gym order in the mainline games (earliest first):

| Type | Average order |
|---|---|
| Bug | 2.0 |
| Rock | 2.2 |
| Grass | 2.4 |
| Normal | 2.8 |
| Fighting | 3.4 |
| Poison | 3.5 |
| Fire | 3.8 |
| Water | 4.2 |
| Flying | 4.3 |
| Ghost | 4.3 |
| Electric | 4.6 |
| Fairy | 5.5 |
| Steel | 6.0 |
| Ground | 6.5 |
| Psychic | 6.7 |
| Ice | 7.0 |
| Dark | 7.0 |
| Dragon | 8.0 |

Each type's order also has a standard deviation in the mainline games:

- Bug through Poison (Bug, Rock, Grass, Normal, Fighting, Poison): 0.82-1.94.
- Fire, Water, Flying: 2.17-2.85.
- Electric: 1.85. Ground: 1.5.
- All others: 0.63 or less. Dragon: 0 (always last).

The algorithm only needs to feel like the mainline games; the exact details are barely user-visible. Pick 8 types at random, then give each a sort key of its average order plus random noise, and sort. Every type has some chance to be first or last, but the result stays near the ideal order. Noise is scaled by each type's deviation (so Dragon stays last and Bug/Fire/Water/Flying move around), using ~0.5 where only an upper bound is known. A simpler fallback is the same noise range (0 to ~9) for every type.

### Special locations

Special locations are either inside a city or outside one (mirroring mainline games, with Red/Blue as the reference).

- Outside cities: Viridian Forest, Mt. Moon, Bill's House, Dark Tunnel (Rock Tunnel), Power Plant, S.S. Anne, Safari Zone, Whirlpool Islands (Seafoam Islands), Victory Road tunnel.
- Inside cities: Museum, burgled house, Pokémon Tower, Game Corner, Celadon Mall, Mansion, Silph Co. tower.

Outside-city places come in three classes, a property of the individual place rather than of its kind (Rock Tunnel and Dark Tunnel are both tunnels):

- Landmark: anything with a map icon. Has coordinates and is routed like any location, and also appears on the region-map screen (Mt. Chimney, Mt. Pyre, Viridian Forest, Mt. Moon, Rock Tunnel, Power Plant, Seafoam Islands, Victory Road).
- Detour: has coordinates and is routed identically, but does not appear on the map (Dark Tunnel, Bill's House).
- Pitstop: not a location as far as the headers and route placement are concerned. Pitstops are placed after the routes are final, on route region tiles that are otherwise empty (no location, no other pitstop), and are dealt out to counties by a coprime stride over the pitstop list, so none repeats (code: `include/util/region_pitstop.h`, `src/util/region_pitstop.c`: slot j is pool entry `(j * stride + offset) mod L`, and county ranks come from a second coprime stride over story indices; a county's window of `floor(L * r / N)` slots depends on its story index alone, so counties never read each other's route plans, and capacity only truncates a window, never moves a slot). Because they sit on existing routes, they need no routes of their own, which keeps the number of intra-county routes (and so route de-overlapping) down: roadside stops and similar small places should be pitstops rather than detours.
- Classifying a place: anything with a map icon is a landmark. A route end that is not a city, landmark or another route is always a detour, and so is a place where a route ends at another route. The remaining places are mostly pitstops, and a few of the more interesting ones are bumped up to detours by hand.
- Emergency fallback: a county with no city that cannot place a landmark or detour places a pitstop as if it were a detour, so the player can still route through it. This should never be needed.

The design is intended to be able, in principle, to generate the Red/Blue region map. How to represent the underground tunnels around Saffron (the Underground Path, not Diglett's Cave) is TBD.

### County-story-index assignment

Layer 2 walks the 24 counties in a snaking path from the starting county, using a temporary table of how many neighbors of each county still lack a county-story-index. A county may hold several McGuffin keys and locks.

First pick R, the number of reserved legendary counties: 3 or 4 (the 2-3 post-7th-gym counties plus the 1 post-Elite-Four solo). The walk indexes the other 24 - R - 1 counties (19 or 20, indices 0-18 or 0-19), the Elite Four takes the next index, and the R reserved counties take the last R indices.

1. Pick a mountainous edge county, preferring a corner, as the Elite Four. It is set aside until the walk ends.
2. Pick a land county toward an edge as the starting town. It gets index 0 and becomes the walk's current position.
3. From the current position, look at the neighbors without an index. If any has exactly 1 unindexed neighbor, pick randomly among those; otherwise pick a random unindexed neighbor. Random choice makes the path snake, and preferring 1 fills corners first, which reduces dead ends. The pick gets the next index and becomes the current position.
4. At a dead end (no unindexed neighbors), the current county gets a McGuffin key. The next county is the closest keyless county that still has 1+ unindexed non-ocean neighbors (before Surf is placed), and it gets the matching McGuffin lock. The walk continues from there.
5. If the walk dead-ends with every remaining unindexed county having 0 indexed neighbors before the target count is reached, the next indices are assigned to those counties at random until it is.
6. The Elite Four county takes the next index, then the R counties still unindexed take the last R indices, and become the reserved legendary counties (see Reserved legendary locations). Which of them is the solo post-Elite-Four county, and how the others tie into the gym order, is decided by a separate algorithm.

Ocean counties are skipped in steps 3-5 until Surf is placed, as in mainline games where Surf opens the sea late. In steps 3-4 a "dead end" means no unindexed non-ocean neighbor. At a dead end, count the unindexed non-ocean counties (not counting the set-aside Elite Four and reserved counties). If at least about 3 remain, step 4 applies as written, with an ordinary key and lock. If fewer remain, this is where Surf is placed instead: the current county gets the Surf McGuffin key, and the next county is the closest unindexed ocean county adjacent to an indexed county, which gets the Surf lock. From then on ocean counties are eligible like any other. The count threshold, not a fixed index, makes Surf arrive near the end of the land counties however many ocean counties the terrain produces. If only ocean counties remain as candidates, Surf is placed immediately. Surf is always placed in time: if no qualifying dead end has occurred by four indices below the Elite Four county's index, it is forced there, so no ocean county can end up unreachable and gyms 6-8 still fit after it. The land count excludes only the Elite Four county, since the reserved counties are whatever is left when the walk ends. Surf is never placed before index 5 (gym 5 is the Surf gym, see "Main plot planner", and gyms 1-4 need four distinct time blocks after the scripted block 0; any county, including the first two, may host a gym); the dead-end test checks the index explicitly. The terrain rules do not guarantee enough land by themselves: ocean extends toward the middle of the map, and a land-starved seed would force the walk onto an ocean county at a dead end. Layer 1 therefore requires at least about 10 non-ocean counties (mountain counts as non-ocean): the Elite Four county, the six or so counties up to Surf, and the roughly 3 left when Surf is placed. A seed-sweep test checks both.
#### Ports

Before the walk, pick a random target T from 5 to 8. The first county the walk indexes at T or above that is a coast gets a port, and the next county indexed is an ocean county, which also gets a port. Then pick U from 4 to 7, add the current county-story-index, and continue the land-only walk from the first port county, so the ocean county is a side visit (its step-3 unlock of the following county is a jump, like a McGuffin lock). The first coastal county indexed at or above that sum gets a third port.

Ports are layer-2 county flags. A ferry is a teleport between two port cities, not a route: the sea route between them still exists under the normal route rules, but the player cannot traverse it until Surf. The ferry is open exactly when both port cities are unlocked, so the ferry to the third port opens only after that county is unlocked normally. The ocean county reached by a port is available before Surf; the Surf rule below still governs every other ocean county.

Because the ocean county is reached by teleport, it needs no lower-index neighbor for the growth constraint. Its sea routes are Surf-gated, and their wild Pokémon, trainers and items are leveled for the block that gives Surf, not for the cities they connect.

#### Kanto reference pattern

Story points in Kanto, as `county-story-index - content` (an index may hold several things, and the order circles back):

0 start / professor's lab; 1 first city, with a McGuffin to return to the lab for the Pokedex, Poke Balls and store use; 2 forest maze special location; 3 first gym; 4 first tunnel (usually 4 or 6); 5 second gym, and the solo legendary; 7 third gym, Cut, Flash; 8 dark tunnel; 9 city; 10 fourth gym, Fly; 14 fifth gym; 17 sixth gym, Surf, Strength; 18 legendary bird; 20 seventh gym; 21 legendary bird; 22 Victory Road; 23 Elite Four. The eighth gym sat in county 1.

So gym order does not have to follow county-story-index, and the story can circle back, provided the McGuffins are careful.

### Route generation

Routes are placed after all cities and special locations are placed.

- If the special location is outside the city, a route always connects it to the city.
- A reserved legendary location in a county with a city is excluded from all other route rules, and gets only its own route from the city (see Reserved legendary locations).
- If an adjacent county has an adjacent story index, a route is made: the shortest route connecting either of that county's locations to either of ours, biased toward cities.
- If an adjacent county is the capital, there is always a route from one of our locations to the capital city.
- If no adjacent county is the capital or has an adjacent story index, the county routes to whichever adjacent county has the highest story index.
- Counties only ever route to adjacent counties. Ferries are teleports between port cities, not routes (see "Ports").

Code: `include/util/region_route.h`, `src/util/region_route.c`. `DoCountiesHaveRoute` applies the rules above (a pure function of the headers, symmetric). The endpoints and the border crossing are pure functions of the unordered county pair, so both counties agree without storing anything. The endpoints are the closest pair of locations (Manhattan distance), ties preferring cities, then the lower location index, scored over the lower-id county first. The crossing is one row (column, for north-south pairs) taken from one of the two locations by hash. The county owning that location runs a straight route along its row to its border column. The other county joins its location to the crossing tile on its own border column with an L or Z shape (the bend is a hash), and bends the Z to avoid crossing its own other routes and passing through its locations (a backtracking search over its bendable halves: hashed bend first, then bends toward the border, then toward the location; the hashed bends are kept when no crossing-free set exists), since it knows them all. Each county edge carries exactly one inter-county route, so border runs never collide. Two halves may share a run of tiles, for example when a location sits on the border column, but may not cross: at every shared tile they must leave in a common direction. A shared run is one road with a single owning route per tile: the sharing route that opens first (the later of its two counties' story indices; an intra-county route opens with its county), ties to the lower route id. A McGuffin lock sits on the earlier-unlocking county's half, on the first tile after the last tile that half shares with another route (so the road cannot be joined after the lock), not at the location; if that half is empty or all shared, on the later half's border tile. Each outside-city special location also gets an intra-county route to the county's city (a hashed axis, Z-bent around the county's other routes, planned after the inter-county halves so a crowded county never costs the halves their crossing-free plan); a county without a city has none. Each half is decided by one county alone, so no two counties ever coordinate past the crossing row, and routes cannot collide across a border. Generated locations never sit on a county's outermost row or column (the same border rule as cities); routes may cross them. The hardcoded Emerald table does not obey this and is left as it is: the abstractions exist because Emerald only roughly fits the constraints, and its placeholder positions are treated as if they obeyed them. A location outside a city is itself a route tile (`TILE_ROUTE | TILE_SPECIAL`), so the route reaches it without a gap and "adjacent" locations still have a route involved; a location inside a city shares the city's tile instead.

### Route story indices

Each route, city, and special location also gets a route-story-index, derived from the county-story-indices of the places a route connects. Route-story-indices start at 4 * county-story-index, which leaves room for several steps inside one county.

Within a county, the "first" city/location is the one the main route into the county connects to. That route comes from the adjacent county with the highest story index (below this county's); any other city/location in the county is the "second". Indices are then:

- 4 * county-story-index + 0: the main route into this county, and the first city or location.
- +1: a special location inside the first, if the first is a city.
- +2: the second city or location (if any), and the route connecting the first and second (if any).
- +3: a special location inside the second, if the second is a city.

Between counties:

- The route gets its index from the higher county's city or location, and that place gets the same index.
- Whether and when the route opens is not decided by the index: a route is gated by the unlock condition of its later end location (see "Main plot planner"), and the main plot planner places each county's first-location unlock at step 3 of the previous county's block (index 4(x-1)+3), after that block's key (if any) was given.

#### Route-story-index edge cases

- "Adjacent" counties are orthogonal grid neighbors. County-story-indices are unique, so "highest lower neighbor" never ties.
- Layer-2 constraint (growth order): every county with story index above 0 has at least one adjacent county with a lower index. The story order therefore grows outward from the start, and every county has a main route into it. Non-adjacent story jumps (the McGuffin-lock case) are still allowed, because some other lower neighbor always exists to enter from.
- The main route into county c comes from its adjacent lower county with the highest story index. If c-1 is adjacent, that is c-1. Other routes from lower neighbors into c are "routes to lower-index counties" and open when the higher county's entry McGuffin triggers (see Between counties).
- Starting county (index 0): it has no lower neighbor and no main route into it. It always has a settlement (population at least 1). Its first place is the city, and the player starts there, with +0 = the city.
- A county without a city (population 0) has only its special location; that is its "first", and +2 and +3 are unused.
- A reserved legendary location in a county with a city does not take part in the +0 to +3 scheme; it and its route get end-game indices (100+), and do not count as "the county's special location" for the between-counties locks.
- Which place of c is "first" is the place its main route connects to. That follows from the route rules (shortest route between either of the two counties' places, biased toward cities), which depend only on layers 1-3, so layer 4 can decide it without any county data from layer 5.

## Geography

County-level terrain is laid out like a typical Pokémon region:

- The outer ring of counties is bordered by mountains on 1-2 sides and by ocean on 2-3 sides: first mark a continuous stretch of the outer ring as mountainous, and most of the rest as ocean.
- A stretch of mountain and a stretch of ocean each extend toward the middle.
- Cities and special locations placed in the ocean automatically form islands.

Placement tendencies:

- The Elite Four is generally in a mountainous corner of the map.
- County-story-index 0 (where the player starts) is vaguely near the Elite Four, and the capital is vaguely far from it.
- Within a county, mountains and coasts nudge city placement toward them, since most cities are on coasts and mountain passes.
- The capital always has the maximum population (3).

Climate gradients:

- Counties near the outer ocean are very humid, so humidity needs a rapid gradient across the map to guarantee some low-humidity zones at all.
- Temperature is assumed to be mostly a north-south (or south-north) gradient.
- The gradients must be designed so that the most desert-like biome is likely to occur at least once on non-mountain terrain. This is expected to be tricky.

Biome granularity:

- Routes and cities are not locked to their county's biome. The gradients extend to the region-tile level, and each route or city is based on its own region tile.
- A single route or city has a single biome, with no gradient within it.

### Landforms and cave variants

A route can additionally be marked as a landform within its biome, which also breaks up the monotony of routes sharing a biome: beach, volcano, farmland, sandbars, fjords, salt flats, mesas, river basin.

Caves and tunnels get distinct characteristics too: underground river, bioluminescence, crystals, sinkholes, magma, ice.

For reference, the Pokémon Snap courses were beach, tunnel, volcano, river, cave, valley, clouds, meadow, jungle, forest, desert, reef, ocean, snow, tundra, and ruins. The biome grid already covers meadow, jungle, forest, desert, ocean, snow and tundra; tunnel, cave and ruins are special locations; the rest are landforms.

## Biomes

- There are 25 possible biomes, forming a 5x5 grid: one axis is precipitation, the other is temperature. Every species is associated with one of all 25 biomes.
- A given region/region map is limited to one of the six 4x3 subsets of that grid (4 adjacent humidity levels of 5, and 3 adjacent temperature levels of 5), chosen randomly per game: 12 biomes. This keeps biomes coherent at small scale and makes separate games feel distinct. Temperature spans only 3 levels because 4 made the hot and cold ends of one region too far apart.
- Every evolutionary line maps to one of the region's 12 biomes (see "Species <-> biome mapping"), so all ~541 lines are wild-encounterable, about 45 per biome. That exceeds Pokémon storage (420 slots, 14 boxes of 30); if it feels like too many, the mapping widens to mod 13+, and lines landing on residue 12+ do not spawn in the wild that run (NPCs can still have and trade them).

The 25 biomes. Columns run from hottest (left) to coldest (right); rows are humidity levels.

| Humidity | Hottest | | | | Coldest |
|---|---|---|---|---|---|
| 4 | Tropical rainforest | Tropical fresh-water swamp forest | Temperate giant rainforest | Montane rainforest | Bog |
| 3 | Tropical seasonal rainforest | Mangrove swamp | Temperate fresh-water swamp forest | Temperate evergreen forest | Elfin woodland |
| 2 | Temperate deciduous forest | Temperate woodland | Temperate shrublands | Salt marsh | Taiga |
| 1 | Thorn forest | Savanna | Temperate grassland | Alpine grasslands | Tundra |
| 0 | Tropical desert | Thorn scrub | Warm-temperate desert | Cool temperate desert scrub | Arctic-alpine desert |

Biome terrain and art:

- To keep graphics size down, many biomes share tile sprites and differ by palette tint; ideally each biome still has some unique sprites.
- Each biome has its own algorithm for the number and placement of trees, dark/tall/short grass, dirt, water, rocks, hills, etc.

## Data representation and determinism

- GBA memory is scarce, so per-county data is bitpacked: biome type in 4 bits (an index into the region's 4x3 subset of the 25 biomes), population in 2 bits (roughly logarithmic).
- Map generation uses a PRNG seeded from the region seed (today the identity seed itself; see "Regions and multiple regions"). The generated region therefore does not need to be stored in the save file, and possibly not even in RAM (it can be regenerated on demand). Every distinct identity yields a completely different map.

- Saves record a generator id, and the game refuses to open a save whose generator id does not match. Players upgrading mid-playthrough are not supported.
- Seeds: everything (region, types, moves) derives from one identity seed, `SeedFromIdentity(otId, otName)` in `src/util/identity_seed.c`: a hash of the full 32-bit OT ID (public and secret halves) and the 7-byte OT name hashed as a fixed block, which is why every name writer EOS-pads it (`PadOtName`: the naming screen, quickstart, `SetBoxMonData`); a debug assert catches unpadded names. Trading is only supported between Venture ROMs, so incoming mons are trusted to be padded. These are the only inputs that are stable, always in RAM, and attached to traded Pokémon, so a traded Pokémon's types and moves come from its own OT's seed. `IdentityRng(seed, salt, ordinal)` returns an independent SFC32 state per purpose and instance, never touching `gRngValue`; the player's own seed is `GetPlayerIdentitySeed()`.
- Incremental generation goal: region, county, and map generation should be light enough to run in tiny chunks per frame as the player approaches a new area. On entering a city, plot points are picked a little per frame so the next cutscene location is known before the player has moved about 5 tiles.

### County headers

Code: `include/util/region_county.h`, `src/util/region_county.c`, Emerald data in `src/data/region/emerald_region.h`.

The 24 county headers plus a small region header are cached for the whole session (24 x 3 bytes = 72 bytes), and they carry everything a county needs to know about its neighbors, so a county is generated from its own header and its neighbors' headers and never from a neighbor's generated content. Because they are always resident, a field is stored only if it is the output of a global constraint (the terrain shape, the story-index walk, the settlement quota) and so cannot be recomputed from the other headers plus a per-county hash.

Per county (`struct CountyHeader`, packed, 17 bits used of 24):

| Field | Bits | Why stored |
|---|---|---|
| terrain (land, mountain, ocean) | 2 | layer 1 shape |
| biome, as an index into the region's 4x3 subset | 4 | layer 1 gradients |
| county-story-index | 5 | layer 2 walk |
| population | 2 | layer 3 settlement quota |
| key id: jump ordinal 1-7, 0 = none | 3 | layer 2 dead ends; the lock is always in the next story index, so lock ids are derived |
| port flag | 1 | layer 2 |

Per region (`struct RegionHeader`): the capital county, R (the reserved legendary count, 3 or 4), the biome subset (humidity base and temperature base) and the story-arc id. These are one-per-region facts, so they are not per-county flags; the Elite Four and reserved counties follow from R and the story indices.

Derived, never stored: island (a land county whose neighbors are ocean), the city name (table lookup from the seed and county id), the special locations (kind, tile and routes, picked per county by a coprime stride over the kind list like the city name, so kinds are not reused; the reserved legendary counties are known from the story index), the gym, HM and arc-beat hosts and the gym types (all planner hashes), and whether a route exists to a neighbor (county-story-index adjacency, capital, highest index). A key's kind and cross-county McGuffin id come from the planner's item hash, so the header holds only the ordinal that pairs a key county with its lock county. A county holds at most one key and one lock (it can be both a lock's target and a key's holder). The city theme is not stored yet; if its eligibility rules turn out to need a global choice it gets spare bits.

### Region tiles

Code: `include/util/region_tile.h`, `src/util/region_tile.c`, Emerald data in `src/data/region/emerald_tiles.h`.

Tile positions and names are not in the headers. They are read through accessors keyed by county id or region-map cell: `GetRegionTileFlags(x, y)`, the city (`DoesCountyHaveCity`, `GetCountyCityPos`, `GetCountyCityMapSec`), the special locations (count, then position and MAPSEC per index) and the route tiles (count, then position per index). In the hardcoded Emerald stage they answer from a per-county ROM table (city tile and up to 6 special locations in county-local coordinates); route tiles come from the route generator: the county's halves toward every neighbor `DoCountiesHaveRoute` joins, plus its outside-city locations, with a road crossing a city tile absorbed by the city; later they answer from per-county hashes, so callers do not change. Everything is asked through the county, so a county is still generated from its own header and its neighbors' headers only.

- Roles are a bit set, not an enum. A special location inside a city (Devon Corp, Mauville Game Corner) shares the city's tile (`TILE_CITY | TILE_SPECIAL`). A special location outside a city is itself a route tile (`TILE_ROUTE | TILE_SPECIAL`) and is counted in `GetCountyRouteTileCount`. Which of the two it is follows from its kind alone (`IsSpecialLocationKindInsideCity`): `struct SpecialLocation` carries a `kind` and a `locationClass` (`enum LocationClass`: town, landmark, detour, pitstop; the Emerald table sets it per place, and a pitstop has no stored tile: `GetPitstopLocalTile` deals pitstops onto the county's free route tiles by hash, after the routes are planned). The kind is from two enums sharing one value space, `RouteLocationKind` (outside) and `TownLocationKind` (inside; catalog in `Emerald.md`). Special locations are otherwise distinct tiles, and a plain route tile shares with nothing.
- Names are MAPSEC ids. A special location that lies on a route in Emerald (Glass Workshop, Trick House) takes the route's MAPSEC, and an in-city one without its own MAPSEC takes the city's.
- The table is per county rather than a 28x15 grid because Emerald.md fudges places into counties: for example Fallarbor's real cell (3, 0) falls in county (0, 0) geometrically but belongs to county (1, 0). The region-map screen still shows the real Hoenn layout; the two are reconciled only when the generated region replaces Emerald.
- The empty county (3, 2) has no city, special location or route tile, as a one-off Emerald exception; every other county has at least one special location.
- Positions inside a county are placeholders nudged toward Hoenn; routes are short unconnected stubs until route placement and route-story-indices are generated.

### City data

Code: `include/util/region_city.h`, `src/util/region_city.c`, Emerald data in `src/data/region/emerald_cities.h`.

Read-only per-county metadata behind accessors, like the tiles: `GetCountyCityTheme`, `DoesCountyCityHaveBuilding(countyId, role)`, `GetCountyGymOrdinal` (0 = none, 1-8 gyms, 9 the Elite Four county), `GetCountyGymType`, `GetCountyGymLeader` (a trainer id, so the name comes from trainer data) and `GetCountyCityName` (the region map's name for the city's MAPSEC). The hardcoded Emerald stage answers from a ROM table; the generator later answers from hashes and the main plot planner, so NPC text and gym scripts do not change. Everything but the gym ordinal asserts that the county has a city.

- Building roles are a bit set (`BuildingRole`). The Elite Four's building is `BUILDING_LEAGUE`, not `BUILDING_GYM`; it has no gym type, and its leader is the Champion. Every city except the starting town (story index 0) has a Pokémon Center; the start has home healing and a bedroom PC instead. A building supplied by an inside-city special location is not a role.
- `GetGymSlot(ordinal)` is a planner input: the time block and step that give gym k. Slots strictly increase with the ordinal; a step-1 slot is a callback whose gym county lies in an earlier block, and any other slot's county is the block. Emerald's Surf exception is encoded here (it is an Emerald-only exception to the Surf rule in "County-story-index assignment"): gyms 4 and 5 share block 11, Lavaridge's own as step 0 and Petalburg's as the step-1 callback, and the block's unlock leads to a land county, not an ocean-lock county. Lavaridge's gym cannot move earlier, because the walkthrough opens the Weather Institute county from Lavaridge's block.
- Tests over the accessors are written as seed-sweep-ready invariants (`test/region_city.c`). The test runner has no real trainer data, so the Emerald-only check that each gym leader's party matches the gym type was run once offline. It must not become a generator invariant: mainline games have exceptions (FireRed's Elite Four has at least 40% off-type).

### Region dump

`DumpRegion()` / `PrintRegionToLog()` (`include/util/region_dump.h`, debug menu: Utilities, "Print region (log)") print an ASCII map of the 28x15 cells (city, special location, route, empty; county separators) and three lines per county (header fields with capital/Elite Four/reserved; the city's name, MAPSEC, tile, theme, building roles, gym ordinal, type and leader; route count and special locations with their kinds) through the accessors, so it shows whatever the generator later produces. After the county lines it prints one line per route lock (tile, gated county and its story index, and the planner key bound to it), one per Enclave (unlock index and map flag), and one per main-story-index through the Elite Four block (the vanilla list's items, with the callback host at step 1, or the step-3 unlock key). Lines are emitted one at a time to keep stack use small.

### Regions and multiple regions

Gold/Silver and FireRed/LeafGreen have several region maps the player moves between. This mod is built for one region now but keeps the seams so a later version can host more: nothing region-shaped may assume a single instance in new code.

- Per region: the region header, the 24 county headers, the tile data, the county var slices (sized for one region today) and a region seed. A region's seed would be `IdentityRng(identitySeed, SALT_REGION, regionIndex)` or similar, never the identity seed itself.
- Per identity, shared by all regions: the identity seed, species types and move pools (which must agree across trades and regions).
- Not implemented until V2: travel between regions, a region index in the save, per-region var slices.


## Procedural species types and wild encounters

Species types and move pools are procedurally generated, and wild Pokémon on a route should match the route's biome, like a real game.

### Biome type affinity

Each biome has a hardcoded table of how often each of the 18 Pokémon types occurs there. Possible encoding: 3 bits per type on a log scale, 54 bits (7 bytes) per biome.

### Species <-> biome mapping

A species' biome must be findable quickly without walking the generated map data, and so must the species list of a biome. This uses a two-way coprime-stride (modular multiplicative) mapping between species and biomes:

- The mapping operates on the ~541 evolutionary lines (via each species' base evolution), and associates every line with one of the region's 12 biomes.
- A stride coprime to 12 (not divisible by 2 or 3), and its modular inverse, are hardcoded.
- Species to biome: `(baseSpeciesIndex * stride + playerID) mod 12`, indexing the region's biome subset.
- The species' two types are then chosen by random selection seeded from species + player ID, weighted by that biome's type affinity.
- Biome to species: compute the first species of the biome via the inverse, then the rest follow at a fixed index distance from each other (derived from the inverse).
- Each biome ends up with ~45 evolutionary lines.
- A Pokémon whose trainer ID differs from the player's maps through that trainer ID, so it may land in a different biome and get different types.

### Species metadata

Per-species fields used for encounters and trading (field widths are tentative):

- Minimum requirements: flags for a required habitat. Fish-looking species require water (surfing or fishing) and are almost certainly Water type; land-looking species such as Pikachu (a mouse) require land. These two flags may be the only ones.
- Habitat bias: flags biasing a species toward or away from habitats. They are based on the species' image, never its type: types are randomized, each biome should have a general mix of types, and a habitat such as volcano should be neither only fire nor the only home of fire Pokémon. Example: Pikachu looks like a mouse, so it prefers tunnel, cave, valley, forest and ruins, and avoids beach, clouds, reef, ocean, snow and tundra.
- Domestication (2 bits, 0-3): 0 is fully wild (Pinsir, Kangaskhan, Tauros, Dratini, legendaries), biased toward the map edges and Safari Zones; 3 is fully domestic (Hitmonlee, Hitmonchan, Meowth, Porygon).
- Base evolution: a reference to the species' base form, so that all evolutions share the base's type generation.
- Normal level range: a 3-bit enum for the low end (2, 7, 10, 17, 25, 34, 43, 55); the high end comes from the evolution data.
- Commonness (2 bits): 0 = legendary (never wild; NPCs never have or trade them); 1 = baby, trade-evolved, item-evolved and fossil Pokémon (never wild; NPCs rarely have them, but trade normally); 2 = starters and other super-rare Pokémon such as Chansey and Tauros (super rare in the wild; NPCs rarely have them, but trade regularly); 3 = normal (encounter ratios based on catch rate; NPCs regularly have and trade them).

### Route encounter selection

The species-to-biome stride mapping only determines types. A route's encounters are built separately from the evolutionary lines correlated with the route's biome (~45 per biome). Many routes share a biome but sit at very different levels, so the species are filtered, then weighted:

- by commonness: legendaries, babies, fossils, and trade/item-evolved Pokémon are never wild, and starters and other super-rare Pokémon are very rare,
- by domestication, relative to how close the route is to the map edge or a Safari Zone,
- by habitat: minimum requirements (e.g. water for fish) and image-based habitat biases,
- by level: species whose evolve-to/evolve-from levels make them unlikely at the route's level are dropped,
- by time of day and weather,
- by a random filter that drops some of the rest.

Most of the ~45 lines are expected to be filtered out, leaving roughly 10-15.

Super-common species: a hardcoded list of very common species (catch rate >= 235; Pidgey, Rattata, Zubat, Magikarp, etc.) that, as in the mainline games, can show up on routes all over the region. When a route's table has too few species after filtering, these are also considered, bypassing the biome filter but not the others (habitat still applies: Magikarp needs water, Pidgey avoids caves, Zubat only spawns in caves and ruins). Still under consideration. The survivors' encounter ratios are proportional to their catch rates.

All of this, including the randomness, is derived from the trainer ID, so each play-through is fully deterministic. Tables are deterministic, not fixed: they are recalculated when the time of day or weather changes.

A species whose habitat strongly opposes the biome may not appear at all in a play-through. This is intended, like Sandshrew, Vulpix, Meowth, Bellsprout, Magmar, and Pinsir never appearing in Pokémon Red.

Table size and ratios:

- Tables are built on the fly and never stored in ROM, so Emerald's 12-slot land cap (probably there to keep ROM tables small) can be dropped, as can its hardcoded slot ratios, if the encounter code allows dynamic sizes and ratios.
- If a table still needs to be capped, species are dropped randomly until it fits.

Encounter methods:

- Each encounter method gets its own table (PokeAPI's `encounter_methods.csv` lists many: walk, surf, old/good/super rod, rock smash, dark grass, headbutt, spots, roaming, and others).
- Most are not needed. Walk, surf, grass, dark grass, and other common methods are calculated eagerly; item and object interactions (rods, headbutt, berry trees, "spots") are calculated lazily.

Evolution levels:

- Level-based evolutions define a line's level range, which normally keeps multiple stages of one line off the same route. Occasional overlap is fine, as in mainline games.
- Non-level evolutions use hardcoded estimated levels for encounter-table purposes only (e.g. stone evolutions at ~29, since Red's Celadon sells stones and its gym leader's ace is level 29). The Pokémon still evolves only by its real method.

### Trainer teams and levels

Route-story-indices determine wild Pokémon levels, trainer levels, hidden item choices, and similar progression-scaled content. Trainer species are chosen as follows:

- Trainers in counties with story index 1-2 generally use only species passing that county's encounter filters and biome.
- Trainers in counties with story index 3+ loosen the filters: the species may match the biome or any adjacent biome (orthogonal neighbors in the 5x5 grid, so up to 4; edge biomes have fewer), must fit the level, and must be within 1 domestication step (of the 0-3 domestication value).
- Gym members, the rival, the Elite 4, and other "boss" trainers are not limited by domestication, habitat, or biome.
- Low-level "bad guy" (Team Rocket-style) members are also not limited by domestication, habitat, or biome, but only use species whose base evolution has a catch rate of 255 (the starting cutoff, to be tuned by feel) (Rattata, Zubat, Caterpie, Pidgey, Spearow, Ekans, Oddish, Diglett, Meowth, Poliwag, Bellsprout, Geodude, Magikarp, etc.).

### Experience

- The whole party gains experience after every battle, as in Sword/Shield (`I_EXP_SHARE_FLAG` in `include/config/item.h`, set permanently rather than toggled by a key item).
- Level-based multipliers keep the party's levels together:
  - Underleveled bonus: a Pokémon below the current route's lowest wild level gets significantly more experience.
  - Overleveled penalty: a Pokémon above the next boss's estimated highest level (gym leader, villain leader, champion, etc.) gets significantly less.
- The next boss's highest level is estimated from the route-story-index alone (about `route-story-index + 6`, with boss levels about `route-story-index + 3`), not read from the actual boss team.
- Invariant: before every boss, at least one route with encounters lets the player train up to the boss's level + 1 without penalty. This holds by construction, so it needs no seed-sweep test: every boss is in a city or special location, each of which is reached by an actual route (which always has wild encounters) with a route-story-index 0-2 lower. Whenever the boss level formula changes, retune the penalty threshold against that 0-2 gap.

### Catch-rate floor by progression

Wild encounters are limited by a minimum catch rate that falls as the story progresses, so players can catch what they meet with the balls they can buy:

- The first route's encounters are generally limited to catch rate 250+, and each subsequent route lowers the threshold.
- Most Pokémon before the second gym should have catch rate 152+, because Great Balls are not sold until after the second badge. (At catch rate 192, a Pokémon at 50% health has a 50% chance of breaking out of a basic Poké Ball.)
- Ultra Balls are sold after the fifth badge, so from then on wild Pokémon should generally be above ~85. (At catch rate 85, a Pokémon at 30% health has a 60% chance of breaking out of a Great Ball.)
- The minimum falls to ~0 around the time of the seventh gym.
- Babies, starters, and legendaries (and possibly another excluded category) never occur in encounter tables.

### Legendaries and starters

- Around the seventh to eighth gym, McGuffins unlock a pair or trio of reserved special locations, each holding a pair or trio of legendaries from a single game generation.
- After the Elite Four, another reserved special location unlocks, holding a solo legendary.
- Ideally every legendary and starter is obtainable, but the mechanism is undecided. Starters might come as gift Pokémon after a (possibly weekly repeatable) subplot, as in Pokémon Yellow.
- The post-Elite Four solo legendary location might spawn a new legendary each week: 50% a legendary the player does not own, 50% fully random.

### Move pools

Moves are generated procedurally per species, from per-species anatomy and per-move requirements.

- Each species has a bitfield of appendages (e.g. mouth). Each move has a field of required appendages and an associated stat. "Roar" needs a mouth; "Tackle" needs nothing.
- The ~1000 moves are roughly sorted by power, with ~20 moves associated with each level.
- Learn levels are generated per species (a procedurally generated list of levels at which it learns a move). This guarantees no Pokémon goes long without learning something, which a per-level chance would not.
- At a learn level, the candidate range is wider than one level's ~20 moves, and exactly one move is guaranteed to be picked.
- Candidates are filtered out if the species lacks the required appendages. Moves whose type does not match the species' types are dropped with high probability, as are moves whose associated stat does not match the species' stat ratios.
- Levels 1 and 6 are always learn levels (at least one move each).
- Balance is deliberately loose: moves arrive in general level ranges, and as long as Pokémon get reasonably typed moves at roughly normal frequency and power level, each is roughly balanced. Players can catch 6+ Pokémon before the first gym, so type advantage and leveling cover the rest.
- TMs and tutors use the same validity filter as level-up, except instead of randomly picking one valid move, they only check whether a given move is valid for the species at all.
- Egg moves (tentative, pending research into how breeding works): pick a move the child species could learn by TM that a different-species parent learns naturally, chosen randomly.

## Resource budget

- ROM space is not expected to be a problem. The extra per-species and per-move metadata costs tens of thousands of bytes, but the hand-authored learnset/move tables, most maps, and encounter tables are dropped.
- RAM is scarce (hence the bitpacking and regeneration from the player-ID seed).
- CPU time is the bigger hurdle: on-demand regeneration of the region, species mapping, encounter filtering, and learn-level move picks must be cheap enough for a GBA.

## Story and subplots

The story is expected to be the biggest hurdle, and most of the ROM space saved by dropping maps is expected to go to subplot content.

### Main story arcs

- A handful of hand-designed story arcs; each game picks one at random based on trainer ID.
- Reference points: Red/Blue (take down Rocket bases, then the leader) and Black/White (N and Ghetsis). The arcs should be deeper than Red/Blue.
- Story points are sprinkled across the counties according to their generated story indices.

### Arc progression

The progression mechanism is adapted from a design for far more powerful hardware, so it may need rework to fit GBA CPU and memory limits.

- Game start picks the rival arc and the story arc; map generation may insert some McGuffins (usually HMs).
- The start of the game is driven entirely by the rival arc.
- Rival and story-arc beats are steps of the main plot planner (e.g. "places Rival Battle 2"); the responsible county picks the specific route or city (and, for a city, building) within its own places.
- When the player finishes that route or leaves that building, the cutscene triggers. Usually an NPC runs up, says something, sometimes triggers a battle or gives an item, then runs off screen. The arc then advances to its next stage.
- When a city is generated or loaded, the code checks whether it (or an adjacent route) is the right place for each arc's current state, and uses that to pick "near" or "far" NPC texts for that arc. At least one NPC per arc gets an arc-state hint text. When the state advances, those NPCs switch to an arc-state-specific thank-you text.
- Subplots work the same way but take place entirely within a city and the starts of its adjacent routes.
- Arc and subplot state use the game's existing flag/var storage for small story state.
- Cutscene placement is deterministic, so it is never stored: each rival and story-arc beat has its own planner slot (slots 5 and 6), so beats cannot land on the same main-story-index, and the responsible county's private layer picks the place.
- Each route checks whether a rival or story cutscene belongs to it and plays it when the player enters. Each city or location likewise checks whether a cutscene belongs to its area and may place it anywhere inside its subplot, since the player cannot advance without completing the subplot anyway.

### Possible future extensions

In the far future:

- City subplots could involve the gym leaders more, as in Pokémon Black and White.
- Specific named grunts could take a bigger part in story arcs, like Jessie and James in Pokémon Yellow.

### Rival arcs

The first version always uses the Emerald rival arc (`Emerald.md`), preserving the cutscene text as much as possible.

Later versions add the Red/Blue/Yellow rival arc as an alternative the game can pick, then rival arcs from the other mainline games as far as possible.

### Story arc content

The first version always uses the Emerald story arc (`Emerald.md`).

Next comes the Kanto arc: Team Rocket pushes human exploitation of nature versus harmony with it, and tragically creates a man-made legendary it cannot control. They treat Pokémon as commodities, tools, and weapons. Beats include a Mt. Moon heist, a Lavender Town tragedy, and a corporate takeover. Giovanni is the 8th gym leader, and the player dismantles Rocket.

Arcs from the other mainline games are added as far as possible.

### Main plot planner

The main story is an ordered sequence of steps, counted by one var, the main-story-index. The sequence is computed from hardcoded ranges and a few small masks, never stored as a step list, and advancing is always a single increment. It uses only layers 1-3 and the McGuffin links of layer 2, not the routes: the route rules already guarantee a route from each county to the next county-story-index, and the planner follows that order.

Steps per county. County-story-index x owns four main-story-indices, `4x + s`, so the county is `index >> 2` and the step is `index & 3`. Level is x, so levels map directly from the index.

| Step | What happens |
|---|---|
| 4x+0 | County x gives a list of McGuffins |
| 4x+1 | Optional callback: county y(x) < x gives a list of McGuffins |
| 4x+2 | County x gives a list of McGuffins |
| 4x+3 | County x unlocks county x+1 (its first location, and so every route into it), via a McGuffin |

A McGuffin is anything the player is given or achieves: an HM, a key item, a guaranteed utility item, a gym, a rival or arc beat. Main-story advancement is itself a McGuffin, so a list may be empty. An empty step is passed at once on the same advance, and when there is no callback steps 0 and 2 merge into one group. The gets within a list are unordered; the county decides in what order to do them, using its subplot var, several vars, or flags. A list is complete when every get in it is done, tracked by a completion mask in the county's reserved vars, and only then does the index move to the next step.

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

Code: `include/util/region_unlock.h`, `src/util/region_unlock.c`, Emerald data in `src/data/region/emerald_unlocks.h`. `GetRouteLockKey` binds a route lock tile to the unlock of the county it gates (block `storyIdx - 1`; the reserved counties and so their Enclaves all use block E, unlocking at `4E + 3` below `MAIN_STORY_INDEX_POSTGAME`), `GetCountyUnlockIndex`/`IsCountyUnlockedAt` give a county's unlock point, an unlock with no get to name is implicit (a vanilla event stands in: Fallarbor's is Fiery Path entered), and intra-county locks come back as county-private with an id from the county's pool: the u8 id space splits into 24 pools of 8 by a coprime stride, with 64 cross-county ids hashed above them. Salts live in `include/constants/identity_salts.h`. A key names a planner item by (kind, ordinal), an HM through its gym, and every key is an item of the unlock's own block.

Unlock gating. Every "county x unlocks county x+1" step names an explicit McGuffin, so that non-adjacent counties can reference their key and so the main story, which manages the main McGuffin list, can ensure none is reused. The step is gated by every traversal McGuffin obtained in block x's lists (each an obstacle on the routes into x+1), and a jump key from layer 2 is satisfied by them when present; otherwise one key item or a blocker McGuffin from the global pool is used. All of them are obtained in block x's lists, never later. A gym enables one HM (8 gyms, 8 HMs; in the fixed Emerald order for this stage: gym 1 Cut, 2 Flash, 3 Rock Smash, 4 Strength, 5 Surf, 6 Fly, 7 Dive, 8 Waterfall) and an HM has no range of its own: it takes its gym's time. Both are items of the same host and step (except in the Surf block), so the player does the city's subplot (which hands out the HM) and the gym in any order, then uses the HM on the obstacle on the route into the next county. A gym and HM placed as a callback gate the unlock of county x+1 of that block, not of their host's next county. An obstacle must be buildable on the routes into x+1. The land obstacles (a Cut tree, a Strength boulder, a Rock Smash rock, a short dark tunnel for Flash) are buildable on any route, so only the water HMs (Surf, Dive, Waterfall) depend on the layer-1 terrain of x and x+1, which their block mask checks; if no block in a water HM's gym range qualifies, the HM is still given at its gym's time, and that unlock uses a blocker McGuffin instead. The Elite Four unlock is the end-game exception: it needs all 8 badges, not a McGuffin from the previous block.

Surf. Layer 2 places the Surf lock at step 4n+3 of its dead-end county n, so block n must give Surf and gym 5, the Surf gym (gyms 1-4 are in counties before n and gyms 6-8 after it). Both are items with time n. Either, both or neither may be in block n's callback, independently: a callback item is hosted by the callback county y(n), so it may be acquired elsewhere and brought back, and a non-callback item is hosted by n. The host of the Surf gym is a gym county and has a settlement. n is at least 5, so gyms 1-4 fit in four distinct time blocks 1 to n-1 (block 0 is the scripted tutorial; the starting county can host a gym only as a callback in a later block), and at most four below the Elite Four county's index, so gyms 6-8 fit after it. Step 4n+3 then unlocks the ocean county that holds the Surf lock, whose routes are gated by Surf. Counties indexed before Surf's lock never rely on water crossings; ocean counties' route generation can assume the player holds Surf, with no bridge special cases (ferries are teleports between ports, see "Ports"). A jump key is likewise an item of the key county n with time n.

Settlements. The number of settlements is not drawn independently per county: counties 0 and 1 and the Elite Four county always have one, plus a random 8 to 13 more. The gyms are placed first and each gym's county takes a settlement from that random count (unless it is county 0 or 1, which already have one; the Surf gym's county is a gym county like any other), then the rest are distributed among the other counties. The capital always has the maximum population.

Traversal McGuffins. An HM (other than Fly) is only ever used as a route-unlock McGuffin, since it unlocks a new way to traverse the map; subplot gates and blockers never require one. A few other special items are limited the same way (the Wailmer Pail, Go-Goggles, the Devon Scope; the full list comes from the McGuffin bucketing). Optional items hidden behind an HM obstacle on a route are still allowed, as route traversal. The planner guarantees that every HM is given before the end game, since a game with missing HMs would feel wrong. In the Emerald stage the special-item and HM McGuffins are fixed.

Blocker McGuffins. A blocker is a person who stands in the way until a boolean is set; talking to them or walking past triggers a script, which may also move the player back one square. Emerald has a limited number of these locks, which Milestone 1a enumerates. The global pool of blocker McGuffins is meant to grow to about 96 (24 x 4, so every main-story-index could have one), each with generated text. For now subplots provide their own blocker scripts; later the global list is extended and subplots may draw generic ones from it.

Guaranteed utility items. Some items appear in every game in a typical window but gate nothing: the Bicycle, the Old, Good and Super Rods, the Itemfinder, the Coin Case, the Exp. Share, the Town Map and the PokéNav. Each is an item with a host range (the Bicycle in the first half, the rods early, middle and late). They are not locks, so they never affect the soft-lock proof, and a county may still use one as an optional gate (a ledge or Cycling Road that needs the Bicycle). Items that do gate something in Emerald (Go-Goggles for the desert, the Devon Scope) are ordinary McGuffins. The set is open: any other item every game should hand out belongs here.

Rules:

- Locks live on routes, never on locations. A location's own tiles and data never change when it unlocks; what changes is every route into it. The route generator therefore enforces the unlock condition of the route's later-unlocking end location (its key, e.g. the McGuffin id) as a gate placed at the route's earlier-unlocking end, so the player cannot reach the later location until the key is held. Conceptually a route is open exactly when both its end locations are unlocked, so a county's unlock step (of its first location) opens every route into it at once, and a county's private unlock of another location opens every route into that one; a route between two already-unlocked locations has no gate. Which end is earlier comes from the county-story-index order for first locations, and from the counties' private order inside a county; inter-county routes may read both counties' data (layer 6), so this is known where the route is built.
- A list's gets are hosted in the host county's own places, or on an inter-county route bordering that county (layer 6 allows this). A beat that plays in Petalburg is a Petalburg step, never a Littleroot one; a callback is the way an earlier county gives its gets later. Unlock step 3 of block x is hosted by county x.
- Unlocking inside a county, and which place hosts a get, is entirely that county's private choice. When an inter-county route unlocks is fixed by the step order; how it is blocked is usually left to the county, but a step may specify it (HM Cut).
- Optional content: an inner location (not a route) may gate on a key from anywhere earlier in the story, as long as the player can finish the main story without ever entering it. Such a gate never affects the main-story-index or the soft-lock proof. Because it is private to its county, NPCs elsewhere cannot hint at it unless it has an optional-story index (see `remaining_work.md`).
- A key's index is strictly below the index of the lock it opens: the unlock's McGuffin comes from block x's lists, which precede step 4x+3.
- Cross-county McGuffin ids derive from the item (a hash of the seed, kind and ordinal), so both counties on a route agree on its lock without reading each other's private data. County-internal McGuffins draw from per-county reserved pools, partitioned by a coprime-stride mapping like the species-to-biome one. The two partitions are disjoint, so a hash never collides with a county's reserved id.
- NPC hint text asks the planner for the list at the current main-story-index, so the planner is the top layer of the subplot descriptor (see "Subplot abstraction").
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

Vanilla stage overrides. Vanilla Emerald cannot come out of the hash formulas, since the step bit, the callback hash and the chance are random. While the planner's randomization bit is off, each item instead reads a fixed `{block, step}` from a ROM table, with a fixed host for each callback block (`Emerald.md`, "Planner inputs"; `GetGymSlot` is the gym part). This is not the rejected explicit step table: it only replaces the hash outcome for the hardcoded stage, it stores no step list at runtime, and the generic ranges and masks stay in force. A test checks that every vanilla slot lies inside its kind's range and mask, so the exceptions are exactly the numbered ones in `Emerald.md`. Code: `include/util/region_plot.h`, `src/util/region_plot.c`, Emerald data in `src/data/region/emerald_plot.h`, test `test/region_plot.c`. A gym item carries its HM (`PlotHm`); an HM displaced from its gym's slot (Strength) also gets an `hmOnly` entry, and the vanilla-script gets (tutorial, Wally's Ralts, Shipyard, Elite Four, Champion) are `PLOT_KIND_SCRIPTED` slots. Wally's mask includes the Elite Four block so Wally 2 passes; an arc beat's mask covers its block only (its callback host is authored).

Open: the exact callback chance and distance bound (tuned from play); the hardcoded ranges for each kind's ordinals (placeholders at first; each is listed for the user to check); the end-game blocks (the Elite Four steps, the reserved legendary counties); whether an HM obstacle on a route counts as the unlock of the location behind it; the size of the McGuffin id pool.

### Story state storage

Emerald stores all story state as flags and vars in `SaveBlock1` (`include/global.h`), accessed through `src/minigames/event_data.c` (`FlagGet`/`FlagSet`, `VarGet`/`VarSet`) and driven by map scripts (`data/maps/*/scripts.inc`, `data/scripts/`) using `setflag`, `setvar`, `addvar`, `goto_if_*`, `switch` and `map_script_2 VAR, value, handler` (which fires a handler when a var holds a value, the usual way a stage triggers a cutscene).

- Flags are single bits (`u8 flags[]`, 2400 flags / 300 bytes). IDs run `0x000`-`0x95F`: temp (cleared on map load), trainer-defeated (`0x500`+), system (`0x860`+, includes the badge flags `FLAG_BADGE01_GET`..`08`), and daily. About 380 are already unused. `FLAG_HIDE_*` flags control whether an NPC exists.
- Vars are 16-bit (`u16 vars[]`, `0x4000`-`0x40FF` = 256 vars / 512 bytes, the first 16 temp). About 29 are already unused. A var is a stage counter for one storyline.
- Limits: the saved ranges are fixed by `FLAGS_COUNT` and `VARS_END`, and growing them costs `SaveBlock1` (and so save sector) space. The ID space allows far more: flags up to `0x3FFF` and vars up to `0x7FFF` before the unsaved special ranges (`SPECIAL_FLAGS_START` `0x4000`, `SPECIAL_VARS_START` `0x8000`). IDs `>= SPECIAL_*_START` are RAM scratch and are not saved.
- Emerald's own dependencies are implicit counters. Example: Norman's gym requires 4 badges because each of the first four gyms does `addvar VAR_PETALBURG_GYM_STATE, 1`, and Norman `switch`es on it. These are replaced by explicit lock/key checks when McGuffins are abstracted.

The full var inventory, with each var's use and what could be merged, is in `vars.md`. Only `0x4020`-`0x40FF` (224 vars) can hold persistent state: `0x4000`-`0x400F` are temp vars cleared on map load and `0x4010`-`0x401F` are shared object-graphics vars. So reservations are named regions of constants, not positions at the start of the var space.

Regions (badge and HM flags stay as they are):

- Main story: one var, the main-story-index, a position `4x + s` over the counties (under 100 values, plus the end game) that always advances by one (see "Main plot planner").
- Optional inter-county stories: about 4 vars (`VAR_OPTIONAL_STORY_0..`), each a linear stage index, so optional stories run in parallel with the main story and with each other, and NPCs in other counties can hint at them. Only content with a real optional story takes one; the Trick House uses inner-county indexing.
- List completion: the completion mask of the current list of gets lives in the host county's reserved vars, alongside that county's own subplot stages, and the accessor reads it. Nothing else about the main line is stored.
- County-internal state: each county gets one contiguous slice of 4 whole vars, which no other county reads; no bit packing at this level (a county may pack inside its own slice). A county needs at most a completion mask plus 2 chain vars, leaving one for authored subplots. The 24 slices are contiguous with each other, at the start of the range (`COUNTY_VARS_START`, 96 vars).
- Minigames and facilities (Trick House, Battle Frontier, lottery, contests, size records, cycling records, Glass Workshop, ...; 65 vars): each gets its own hardcoded contiguous range in the minigame block, after the county slices and the reserve, since each appears at most once per run and has custom code. Their Emerald vars are scattered through `0x4020`-`0x40FF` today, so they are renumbered into this block.
  - What gets a dedicated range: content with a lot of non-script code (wanted in nearly every run, hard to fork), or many steps doable out of order (fits neither the main story nor a linear subplot). Repeatable content starts with a dedicated range; repeatable subplots may absorb it later. Fossils may be found in several counties but are revived in one place, biased toward the gym 5-8 timeline.
- Flags for county-internal McGuffins come from per-county reserved pools (see "Main plot planner").
- Engine vars (`vars.md`, "Keep") (41 vars) are renumbered into a contiguous block at the end of the range. Positions that look fixed are not: the ROM header's Dex var is `VAR_NATIONAL_DEX - VARS_START` (`rom_header_gf.c:118`); only struct-cast and 32-bit pairs must stay adjacent. Renumbering breaks old saves, which is acceptable.
- Reserve: 22 vars between the county slices and the minigame block hold the main-story-index and about 4 optional-story vars, with the rest spare.
- `VARS_END` does not grow: 224 vars = 96 county + 22 reserve + 65 minigame + 41 engine. Five per county does not fit (224 - 106 keep - 5 story = 113 < 120).
- Legacy block: Emerald vars not yet retired (61: the main-arc and subplot vars plus the trivially freeable ones) sit in the last 40 county vars and the reserve. Their slices are unusable until the vars are retired, so only the first 14 county slices are free at first. A test checks that the blocks do not overlap and that the legacy block stays inside the county tail and reserve.

Supply: 56 vars are unreferenced today and 10 more are freeable with trivial edits, so the regions can be allocated before any Emerald var is retired. Emerald's main-arc vars (about 40) are retired when the main arc is ported, and city and special-location vars (about 25) one subplot at a time, each leaving `make check` green. A test checks that regions do not overlap and stay in range.

### Subplots

- Each special location has a hardcoded, thin subplot.
- A number of city subplots are authored; each city randomly selects one.
- Subplots are modeled after Pokémon TV-show episodes. Ideally the player meets each gym leader during a subplot before battling them.
- Each subplot is linear. Each stage has ~2 NPC hints of the form "I heard something is happening at [city/location]", ~2 of the form "I saw something happening at [building]", plus story-state texts.

Initial subplot content comes from Emerald (`Emerald.md`). More follow from Red/Blue/Yellow, then other games. In the far future, more subplots are modeled after TV show episodes.

### Subplot abstraction

Goal: city and special-location subplots become data that can be assigned to any place that meets their requirements, so subplots can be shuffled (stage 5) and NPC talk can be written once against the final interfaces. Shuffling is done when, for some seed, two city-bucket subplots whose role and progression requirements both hosts meet are swapped, both are completable, and NPCs in both cities hint correctly.

#### Scope

Only subplot beats need converting; trainer text, item pickups, trainer-defeated flags and NPC-hide flags stay as they are. The lock/key entries in `Emerald.md` fall into buckets:

- Main arc (rival, Aqua/Magma, Wally, and any storyline spanning several counties, such as Devon Goods): stays on Emerald's vars behind the adapter until the arc cutscenes are generalized (Milestone 3). A subplot takes place entirely within a city and the starts of its adjacent routes, so a cross-county storyline is main arc (owned by its first county) or is split into per-city subplots.
- City subplot and special-location subplot: converted, and the only buckets needed to unblock subplot shuffling.
- Gate (an HM, key item or blocking NPC that gates progress): one mechanism, a lock with a key.
- Gym: a city subplot in the end state (the gym leader is met during the subplot, as in the anime and Black/White). Temporarily a separate fixed beat per city, so the Emerald story can be abstracted first.

#### Placement and requirements

- The end state picks a city's subplot early in city generation, so the city knows which story buildings to place.
- Each subplot declares requirements on the cities or locations that may host it: required buildings (by role, e.g. gym, shop, house with an NPC), required HMs, biome, and terrain (mountain, coast, island). These requirements are expected to stay long term.
- Progression requirements: the keys and badges a subplot consumes (Norman needs 4 badges; Petalburg Woods needs Cut), and the keys it produces. Keys a subplot produces may only open locks relative to its host (its outbound routes); between counties those locks are planner unlock steps, ordered by main-story-index (a key's index is below its lock's), so the rule holds after a move. A key that opens a fixed lock elsewhere (Cut opens Petalburg Woods and Rusturf) is a gate, not a subplot output. Consumed requirements are either relative to the host (the lower neighbor's subplot is done) or counted (badges >= N, so the host's step must come after the Nth gym step). A subplot may move only to a host where these hold, which is what makes a swapped subplot provably completable.
- In the hardcoded stage, buildings are fixed, so a subplot is assigned only to a city whose existing buildings satisfy its requirements, and subplots address buildings by role, not by map.
- NPC hosting: each host building keeps a few placeholder object events whose graphics come from a var (`OBJ_EVENT_GFX_VAR_0`, `VAR_OBJ_GFX_ID_0`..`F`), a generic script, and a temp hide flag (counts and spawning: "Building roles and NPC spawning"); a subplot fills them by setting the vars and flags in the host map's on-transition script. These vars are shared (the rival graphics, battle frontier and Route 117 already set them), so they are never persistent state. This is cheaper than spawning NPCs at runtime, which is kept for cutscene beats (an NPC running up to the player) via the generic runner.

#### Interfaces

Defined for the end state, not for Emerald, so NPC talk never needs redesign:

- Subplot descriptor: linear stages; per stage the hint texts, story-state texts and trigger place (by role) (hints are the six distance-by-state entries of "City planning"); the requirements above; rewards; and the key/lock it produces. A subplot that unlocks a location describes its output as a key for the gates on the routes into that location, not as "unlock X". A subplot supplies its own blocker scripts (the person who stands in the way, with texts) for now.
- Stage accessor: reads and writes a subplot's stage by slot (the county's reserved vars, see "Story state storage"), never by subplot identity, so moving a subplot moves no state. A subplot that is a main-plot step has no state of its own: not started or current comes from the main-story-index (see "Main plot planner"), and done or partly done from the county's completion mask. Subplots internal to a county use that county's reserved vars. Callers cannot tell the difference. The main-story-index is not maintained until the main arc is ported (Milestone 3), so until then main-line stages come from the adapter over Emerald's vars.
- Built so far (, , hosts in , tests in ):  (stages with the six hint entries relative to the stage, beat anchor, hinter count and roles, actors with their stage ranges, done-predicate for adapter stages; required roles, minimum badges and produced key), / by (county, slot) over the county slice (var 0 of a slice is the completion mask, vars 1-3 are slots; only counties whose slice is free of legacy vars can host),  as the NPC-talk query (`GetCountySlotHint`: distance plus a per-NPC selector, NULL means generic text), and the script specials / (VAR_0x8004 = slot, county from the current map's MAPSEC, a city for now). The Lilycove Museum exhibit hall (county (4,0), slot 0) is the first ported subplot. Adapter subplots reuse  and are read-only. Requirements, produced keys and parameterized hints are declared but not yet consumed. A stage's hint entries describe that stage as a beat: before, current or after depending on whether the subplot is below, at or above it, so an NPC may recall any passed beat (`GetCountySlotHintAbout`). The last stage's close-after entry is never reached and may be empty.
- NPC-talk query: reads only descriptor, accessor and placement, and returns the text to say. It cannot tell a ported subplot from an adapter-backed one.
- Adapter: lets an unported Emerald subplot answer the accessor from its old vars (e.g. Petalburg gym "open" is `VAR_PETALBURG_GYM_STATE >= 6`). It is read-only, so NPCs can hint at unported subplots, but only ported subplots can be shuffled. Subplots are ported one at a time, each leaving `make check` green. The main-line half exists: `PlotGetDesc` (a done-predicate and a blocking or reported-only class per get), `GetPlotListStage(index)` (done and blocking masks over a list's gets) and `GetMainStoryIndex()` (the first list not done, scanning from 0) in `include/util/plot_stage.h`, `src/util/plot_stage.c`, with Emerald's predicates in `src/data/region/emerald_plot_gets.h` and tests in `test/region_plot_stage.c`. Only gets that vanilla forces are blocking, since a skippable blocking get would freeze the index; the rest are reported only (Emerald.md, "Step predicates for the read-only adapter").

#### Text references

The script engine has no field-path variables. Text holds only the placeholders `{STR_VAR_1}`-`{STR_VAR_3}` (`gStringVar1-3`), a few fixed ones (`{PLAYER}`, `{RIVAL}`, `{AQUA}`...) expanded by `StringExpandPlaceholders` (`src/util/string_util.c`), and values that scripts put into the string vars with `buffer*` commands or a `special` function.

Authored text uses story references such as `$currentCity.gym.leader.name`, `$currentCity.nearbySmallCity.name`, `$stepCounty.city.theme.motto` or `$currentCity.pkmnCenter.nearBuildingName`. A reference is an anchor plus a path through the generated region data. A build tool turns each reference in a template into a compact reference id, and a single talk special expands the whole template in C into `gStringVar4` and shows it itself with `ShowFieldMessageFromBuffer` (not via the `message` command, which expands its source string into `gStringVar4` and would overlap it); this avoids the three-reference limit of `{STR_VAR_1}`-`{STR_VAR_3}`. Subplots are limited to a single county, so anchors are the speaker's own place (the NPC's city and building, for `$currentCity.pkmnCenter...`-style random talk), the subplot's own place, and, for hints about the main story, the responsible county of the current step (`$stepCounty`).

#### Interface rules from paper tests

Checked against Petalburg gym/Surf, Devon Goods and Granite Cave/Flash in Emerald's scripts. The format absorbs all three with these rules, none needing a special case:

- Derived stages: a stage may be computed from requirements rather than stored. Petalburg gym's "locked, N badges short" and "ready" (`VAR_PETALBURG_GYM_STATE` 2-6) is just badges held, so a ported gym stores no counter and the other gyms stop writing to it (Emerald's four `addvar` calls plus `Common_EventScript_ReadyPetalburgGymForBattle` disappear). Badges held is the popcount of the badge flags, not "gyms 1-4".
- Parameterized hints: a stage's hint text may take a number from a counted requirement ("{n} more badges"), supplied by the accessor.
- Stage predicates: an adapter-backed subplot declares its stages as an ordered list of flag or var predicates, highest first, and the first true one is the stage. Devon Goods is DELIVERED, then RETURNED, then RECOVERED, then STOLEN, else not started. `FLAG_DEVON_GOODS_STOLEN` is cleared when RECOVERED is set, so the order, not set-ness, defines the stage. `FLAG_DOCK_REJECTED_DEVON_GOODS` is a side branch that changes only the Slateport shipyard talk, not the stage.
- One subplot, several buildings: a stage's trigger place is a building role, and a cutscene may walk across maps. Norman's defeat triggers a walk to Wally's house (`VAR_PETALBURG_CITY_STATE` 4) where the dad hands over Surf, so the Petalburg gym host needs a "house with an NPC" role beside the gym, and the cutscene runner crosses both. Emerald keeps this beat in a second var (`VAR_PETALBURG_CITY_STATE`, six values); ported, it is stages 3-5 of the one gym subplot.
- Keys: an HM key is a pair (HM given, badge held), and field-move checks read fixed badge flags (`src/field/field_move.c`), so shuffling which gym unlocks which HM must also change those. A key's production is a stage's reward, not a flag a gate reads directly.
- Cross-host effects: a subplot's completion may write another host's state. Beating Norman hides Wattson in his gym and shows him in Mauville, which is what starts New Mauville. That is a produced key ("gym 5 done") consumed by New Mauville's requirements, not a flag write.
- Main-arc steps are hosted by counties, not locations: a county places its steps anywhere (city, route location, plain route) as long as the player can and must take them in order. The Devon Goods letter delivery happens to sit in Granite Cave's 1F side room (`FLAG_DELIVERED_STEVEN_LETTER`), but its host is the Dewford county.
- Devon Goods stays main arc: it spans Rustboro (theft, return), the Rusturf tunnel (recovery), Dewford and the Granite Cave (letter) and Slateport (delivery), and its recovery key opens a lock back in the first county, so it is a callback, not four per-city subplots.
- Dark caves: Emerald has no script check for Flash. Dark floors (Granite Cave B1F and B2F are `requires_flash`) only dim the screen, and the engine lets the player walk them unlit. Flash is made a hard key by limiting an unlit player to about 6 tiles from the entrance of a `requires_flash` map. Steven's room is a warp off the lit 1F, so the letter delivery never needs Flash.

#### Building roles and NPC spawning

- Roles are the `BuildingRole` bit set (`region_city.h`), with the list in `Emerald.md` ("Building roles"); the generic house, which the Petalburg gym/Surf paper test needs beside the gym, is `BUILDING_HOUSE`. A subplot addresses buildings by role only.
- Placeholder NPC objects per host map: 2 for each building role, 3 for a gym, 1 for a service building (its own NPC is fixed), 4 for a city's outdoor map, none for routes. The 16 shared `VAR_OBJ_GFX_ID_*` vars are set by the host map's on-transition script, so the limit is per map, not per game; a city map already spends 1-2 of them on the rival, so 4 is well inside it. Placeholders hide through temp flags (cleared on map load), so they cost no saved flags.
- Runtime NPC spawning is needed only for cutscene actors (an NPC runs up to the player and off again), because a generated route or city has no object in the right place. The generic cutscene runner builds an object template at runtime and spawns it (`SpawnSpecialObjectEvent`, as the player avatar does); to be confirmed when the runner is written. Subplot NPCs inside buildings never spawn at runtime: placeholders cover them, and the rejected alternative below explains why.
- Route hint NPCs (Milestone 1) are not decided here.

### City planning

A city is laid out by its county (layer 5/6), in this order. Steps 5-8 are what the subplot descriptor's hint and actor fields serve. The hard part will be building and path placement (steps 2-3), not the story hooks.

1. Districts (V2 feature; V1 always uses one district, and locks sit only at doors and route ends): usually one district covers the whole city. The only reason for the split is to let a subplot put a lock mid-city. A subplot may ask for two, separated by obstacles and one narrow choke point closed by a lock (usually an NPC, or an HM or item). The planner must know which routes the player can enter yet (from the story indices), and every route entrance must be in the first district. The second district either touches no route, or guards routes the player cannot reach yet: the inner-county routes when the city is entered from an inter-county route, or the way on to the next county when leaving.
2. Paths connect the routes. Large cities get rectangular city blocks, possibly several tiles wide; small towns get meandering paths one tile wide.
3. Buildings (including city special locations) are placed against paths without covering one, and the path must reach their door. Buildings may shift paths slightly if connectivity is kept, unless that costs too much time. Placing a building also decides how many generic NPCs it holds.
4. Decoration fills the space between buildings: paving, dirt, grass, flowers, rocks. Path surface follows settlement type: paved in cities, dirt in towns, grass in villages (to be checked against what Emerald and FRLG do).
5. Main-plot hinters: for each main-story beat hosted in the city, place one hinter NPC for the next main-story beat (looked up through the planner), so a player who finishes a county's subplots always finds at least one NPC there who says where to go next, even when that is the same city.
6. Subplot hinters: a county has a few subplots, each a list of beats like the main story, and the player may interleave them in any order. A subplot asks the county for hinter NPCs, each bound to one beat index (`numHinters` per stage) and each knowing whether that beat is in the NPC's own city (close) or not (far). A subplot outside the city may want none, and one inside may want hinters for a few of its beats.
7. More generic NPCs fill out the population.
8. Each subplot hosted by this city may place actor NPCs, which the owning subplot controls entirely.

Generic NPCs get context about where they are and say a random matching line ("This is my brother!", "The flowers in this city smell wonderful!").

Hint texts: every main-story slot and subplot has six hint entries, by distance (far: the NPC is in another city or county; close: the NPC is at the place) and by story state (before, current, after):

- Far and before: a list, possibly empty.
- Far and current: a required string ("I heard Team Rocket took over Celadon...").
- Far and after: a list, possibly empty ("I heard Team Rocket was kicked out of Celadon!").
- Close and before: an optional string.
- Close and current: a required string ("What are those guys doing there?").
- Close and after: a required string ("You saved us!").

Beat placement: a beat's trigger is almost always an immobile NPC, an immobile item, or a chokepoint (route-to-route transition, route-to-location transition, or a door). The descriptor therefore records a specific anchor per stage (kind plus the route, location or building role involved), not just a role, and hint texts can say where it is: "near route x", "near the <building type>", or the city. Close or far then follows from whether the hinter stands in the anchor's city (`struct BeatAnchor`, `GetBeatDistance` and `WriteBeatAnchorReference` in `subplot.h`; every anchor of a subplot belongs to its host county, so the host county's city is the anchor's city, and a route anchor is "near the road to X").

Planning (`PlanCountyHinters`, `city_hinters.h`) is derived on demand and stores nothing. Subplot hinters stand in the host city, so they are always close; far records exist only for main-plot hinters, one per county-hosted main beat, bound to the next beat that has an item or a blocker unlock. Hinters use the placeholder NPC budget of "Building roles and NPC spawning" and fall back to any building with room. Hint texts embed `{HINT_ANCHOR}` (charmap `FD 0F`), which `ExpandHintTemplate` replaces with the anchor reference before placeholders expand; `ShowCurrentSubplotHint` puts the result in `gStringVar4`. An NPC script binds itself with the `subplot_hinter slot, beat, selector` macro (`data/scripts/subplot_hinter.inc`; values from its `HinterPlacement`), which shows the hint and leaves VAR_RESULT TRUE, else the script falls through to its generic text. Emerald's static maps bind by hand (Lilycove: Man3 and RichBoy, pinned by a test); generated maps will bind from the plan at map load.

Main-beat hints are generated, not authored: `WriteMainBeatHint` (`city_hinters.h`) opens with "I heard" (far) or "Word around town is" (close), then a phrase picked by the plot kind of the beat's first item (a step-3 beat is a blocker), the host city's anchor reference, and a tail. The `ShowMainBeatHint` special (VAR_0x8004 = the hinter's own index, 0x8005 = its target) speaks only while the main-story-index is past the hinter's own beat and not past the target; else VAR_RESULT is FALSE and the NPC says generic text.

Route blockers: `data/scripts/route_blocker.inc` holds one generic script per push-back direction; it uses `IsMainStoryIndexPast` (VAR_0x8004 = s) and lets the player through once the main-story-index is past s.

A hinter bound to beat b says the before entry while the subplot's current index is below b, current while equal, and after once above; the query takes the distance from the placement, never from the stage.

News: each Pokémon Center has a newspaper, TV or reading NPC giving "the news" (`WritePokemonCenterNews`, special `ShowPokemonCenterNews`, `city_hinters.h`). It is about the first list at or after the main-story-index that has a host (an empty list is passed at once). If the player is not in the host county, it says that beat's far main-beat hint. In the host county it names the blocking subplot: the first blocking get still undone in the list (`GetFirstUndonePlotSlot`), matched to a host-county subplot that declares it produces that planner item (`SubplotDesc.producesMainGet` and `mainGet`, tested by `DoesSubplotProduce`), and says that subplot's current close hint, so the next step is easy to find without nagging. With no producing subplot (a gym, a rival beat, a blocker: the blocker is a location rather than a subplot) it says the close main-beat hint instead. A step-3 unlock that is no blocker (which `GetMainStoryIndex` can rest on, such as the Letter) has no beat but still has block x's county: away from it the news says "There's a story about something unfolding near <place>", and in it a vague "right here" line until the county is given details to hint at. The place is the county's city, else its first landmark or detour (or Enclave, for an Enclave-only county), and a beat hosted in a cityless county gets the same vague news. In the postgame, or when the county has none of those (only Emerald's empty county), it says one generic line from a small pool of copied Emerald TV lines, picked by county and player seed. Beats get no separate news strings: the far main-beat text is generated, and a subplot's current entries are required by the descriptor, so the news cannot lack one.

Where an entry is missing or an empty list, the NPC falls back to generic NPC text. An NPC says the same thing every time while the state holds. The far lists let NPCs in different cities give different hints.

### NPC dialogue

- Each named building has a short list of NPC talking points.
- Each gym type has a short list of NPC talking points, parameterized by city name and gym leader name.

### Shops and trading

- Poké Marts: villages and possibly towns keep a general Mart; cities instead have a separate store per bag category (balls, healing items, stat items, etc.).
- Towns and cities have the Pokémon Center trade room as a separate building (not the second floor).
- Each trade building has 0-3 random NPC traders per day; higher population makes more traders likely.
- For each trader, a route or special location is randomly chosen, then a random Pokémon that spawns there; that is the Pokémon the trader asks for, which guarantees the player can obtain it.
- The Pokémon the trader offers is a random species with the same commonness and a similar catch rate, whose evolution level range overlaps the chosen route's level. It may be the same species as the requested one.
- Traded Pokémon carry a different trainer ID, so their types and moves differ from the player's own (see the other-trainer-ID biome subset above).

### Type-recolored sprites

A species' sprite is recolored to suggest its randomized types: the sprite colors that stand for its original types are swapped for its new types' colors.

Type colors: 16 hues evenly spaced 22.5° apart, plus 3 achromatic types.

| Hue | Type | RGB | | Hue | Type | RGB |
| --- | --- | --- | --- | --- | --- | --- |
| 0° | Fire | #FF5555 | | 22.5° | Ground | #AA3F00 |
| 45° | Electric | #FFD455 | | 67.5° | Rock | #94AA00 |
| 90° | Bug | #AAFF55 | | 112.5° | Dragon | #15AA00 |
| 135° | Grass | #55FF7F | | 157.5° | Stellar | #00AA6A |
| 180° | Ice | #55FFFF | | 202.5° | Flying | #006AAA |
| 225° | Water | #557FFF | | 247.5° | Poison | #1500AA |
| 270° | Psychic | #AA55FF | | 292.5° | Ghost | #9400AA |
| 315° | Fairy | #FF55D4 | | 337.5° | Fighting | #AA003F |

Normal is white (#FFFFFF), Dark black (#000000), Steel gray (#7F7F7F). The bright set has S=0.67, V=1; the dark set S=1, V=0.67. The hues follow the mainline games' associations where possible (fire red, electric yellow, water blue, ghost dark purple, fairy pink, ground brown, etc.). Stellar is not a real type, so its color need not be distinct.

Color buckets define "close": a color with low saturation falls in an achromatic bucket by V (Normal, Steel, Dark); otherwise its bucket is its hue rounded to the nearest 22.5°, i.e. one of the 16 type hues. S and V never choose between hued buckets. Starting cutoffs, to be tuned by visual tests: saturation below 1/3 is achromatic (likely tuned down); achromatic V below 1/3 is Dark, above 2/3 Normal, between them Steel (the middle may need more room). A whole shading ramp of one color shares a hue, so it lands in one bucket and recolors together.

Algorithm:
1. For each original type, pick the sprite palette colors in that type's bucket.
2. If the bucket is empty, pick the most-used color that is not near black or white and (with two types) not near the other type's pick.
3. Recolor every color in a picked bucket by rotating its hue to the new type's hue (keeping its offset within the bucket) and scaling its saturation toward the new type's, while keeping each color's own V. Keeping V preserves the sprite's shading, so only the type colors' hues matter, not their brightness.
4. Achromatic types: recoloring to Normal, Dark or Steel desaturates instead of shifting hue; recoloring from them gives the picked gray a hue.

Shinies:
- A shiny's palette is its vanilla shiny palette, recolored at the same indices. Two-type shinies swap the two types' colors (a Fairy/Fire is pink+red, its shiny red+pink).
- Shimmer: when a shiny enters battle, at the start of its turn, and every ~10 s while idle, its recolored hues rotate 360° over 1 s, an iridescent ripple. This is the main shiny tell, since single-type shinies have no swap. It is a per-frame palette update of a few colors, like the existing palette fades.

Cost: GBA sprites use 16-color palettes, so recoloring touches 16 colors per Pokémon, not pixels. Steps 1-2 depend only on the original sprite, so a build tool precomputes the picked palette indices per species; only step 3 runs at runtime, when the palette loads.

### Type icons in names

Players strongly associate Pokémon with their types, so random types are a mental hurdle. To ease it, whenever a Pokémon's name is displayed it is prefixed with icons for its types, e.g. "You encountered a wild 🐉❄️Rattata!".

### Battle damage estimates

Random types and moves make matchups hard to remember, so battle menus show a damage grade.

- **Where:** the move selection menu, and the in-battle Pokémon > summary > moves page, show a grade per move. Status moves show no grade.
- **Multiple opponents:** a move's grade is the highest over all opponents; the target selection screen shows each target's own grade.
- **Level-agnostic:** grades compare stats and multipliers, not levels or HP, so a grade means the same thing anywhere in the game.
- **What the player knows decides the estimate's accuracy,** keyed on the opponent species' Pokédex state:
  - Not seen: defense base stats are assumed to be 100 (average; species base stats are mostly 50-150), and type effectiveness is skipped (treated as 1x).
  - Seen, not owned: real defense base stats, rounded to the nearest 25 (about a quarter of the 100 average, deliberately coarse); type effectiveness applied.
  - Owned: real defense base stats.
  - Temporary battle stat stages (both sides) are applied.
  - Hidden information (ability, held item, crits, the random roll) is never used.
- **Grade scale:** a letter from Z to A, with a red-to-green color gradient.
  - Z is reserved for a damaging move that the type matchup makes deal nothing (e.g. Normal vs Ghost).
  - Y is the worst possible case: move power 10, own attacking base stat 20 with 0 IVs and EVs, opposing defense base stat 170, no STAB, and 4x resisted.
  - A is the best possible case: the opposite of every Y assumption (attacking base stat 170 with max IVs and EVs, defense base stat 20, STAB, 4x super effective, move power 150).
  - The 20-170 range covers non-Mega, non-joke species; values outside it clamp to Y or A.
  - Power 150 sits just above the strongest downside-free move (140). Power 10 sits well below the weakest common single-hit moves (40, e.g. Tackle) so those don't grade as terrible early on; multi-hit moves grade by their listed per-hit power, which is no more misleading than the power the game shows.
  - X-B are evenly spaced between Y and A on a log scale, since the factors multiply, so each letter is the same damage multiplier apart.
  - Being level-agnostic, the Y-A thresholds are compile-time constants: a 24-entry threshold table searched in at most 5 comparisons.
- **Switch hint:** in the in-battle Pokémon menu, each party member gets a grade too: its best move's estimated damage times that move's accuracy squared ("if it isn't 100%, it's 0%": 50% accuracy weighs 0.25x), scaled by its defensive type matchup against the opponent's types, against whichever opponent it hits best (the same Pokédex rules apply to what the player knows about those types).
- **Implementation:** reuse the AI's damage calculation (`src/battle/ai/battle_ai_util.c`) where it fits, with the estimated defenses substituted, rather than a new formula.

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
- Art: every encountered species needs a map sprite, so type-recolored sprites ("Type-recolored sprites") must also recolor OW and follower sprites, and `OW_SUBSTITUTE_PLACEHOLDER` covers missing ones. `OW_PKMN_OBJECTS_SHARE_PALETTES` stays off, so palettes compete with the 16 OBJ palettes. To sidestep that, overworld encounter sprites use one palette per first type (a recolor of the species' map sprite by its first type), not one per species.
- Object budget: spawned Pokémon, the follower, placeholder NPCs and runtime cutscene actors share the map's object-event table. A cutscene that needs more objects aggressively despawns overworld encounters to make room. The placeholder counts in "Building roles and NPC spawning" assume the table has room; check `OBJECT_EVENTS_COUNT` before the runner is written.

Other changes:

- Shiny: `SHINY_ODDS` 80 (about 1/820, 10x vanilla), Shiny Charm adds 2 rolls, fishing chain on. OWE sparkle makes shinies visible before battle, so the higher rate is noticeable.
- Fishing boosts (follower friendship, proximity, time of day) are on.
- Battle UI: `B_SHOW_TYPES` and `B_SHOW_EFFECTIVENESS` are always on. With randomized types this is what lets a player read a matchup, and it overlaps "Battle damage estimates"; that design must not duplicate it.
- Summary: IV/EV info with exact values, move relearners (egg, TM, tutor), pre-evolution moves and reusable TMs are on. Move relearners and pre-evolution moves must be kept in mind for Milestone 6's learnset replacement; reusable TMs make TMs non-scarce, which suits generated moves.
- Field moves: Defog and Rock Climb are enabled. They act like HMs but are not gym HMs, and need no planner support as long as the main story is completable without them: they, like Dig, Teleport, Soft-Boiled, Whirlpool, Waterfall, Rock Smash, Headbutt, Sweet Scent, Milk Drink, Secret Power and Chatter, are non-HM field moves that only open optional content.
- Possible later change, as in Gen 7+ (Let's Go's "Chop Down", "Sky Dash", "Strong Push", "Sea Skim", "Light Up"): an NPC teaches "your Pokémon" a field move by setting a flag instead of giving an HM item, and the field move works while any party member could theoretically learn the move (`CanLearnTeachableMove`), even if it does not know it. This looks cheap to add: expansion already routes every field move through `gFieldMoveInfo[].isUnlockedFunc`, which today reads badge flags (`field_move.c`), but the "has a mon with the move" check is `MonKnowsMove` in places such as `PartyHasMonWithSurf` (`field_player_avatar.c`), so each move's party check also needs replacing. Not decided; it would turn the planner's HM items into flag-setting NPC lessons and drop the badge/HM pairing in "Vanilla stage overrides".
- NPC followers (`FNPC_ENABLE_NPC_FOLLOWERS`) are on, usable by the cutscene runner; it costs SaveBlock3 space.
- Instant text is on (`TEXT_SPEED_INSTANT`). `FLAG_TEXT_SPEED_INSTANT` stays 0: a nonzero value must be a free saved flag, never a temp flag such as `FLAG_TEMP_1`.
- Held evolution items are usable from the bag, and ambient item descriptions show the first time an item is found.

## Development approach: hardcode first, then randomize

The region starts as a hardcoded Pokémon Emerald region and is progressively abstracted and randomized, rather than building the generator first. Hardcoded content exercises the same metadata the generator will later produce, and keeps the story testable throughout.

Why this order: if NPCs don't talk about subplots and story arcs, testers can't tell how to progress, which would cripple testing; and subplot babble requires city/route/location metadata anyway. NPC talk exists to support subplot shuffling, so its interfaces (descriptor, stage accessor, text references; see "Subplot abstraction") are defined first, for the end state, and NPC talk is built on them before any subplot is ported.

Stages, each leaving the game playable:

1. Divide the Emerald region map(s) into the 6x4 counties with hardcoded terrain type (mountainous, ocean, flat), biome, population, special location, story order, McGuffin keys and locks, and capital (Lilycove City). A key/lock is basically anywhere story advancement moves from one county to a non-adjacent county. The story-arc id is also hardcoded.
2. Hardcode the region tiles within each county: which are cities, routes, and special locations.
3. For each city and special location, hardcode its named buildings, subplot McGuffins, and city "theme"; hardcode which cities have gyms, the leader's name, and the gym type.
4. Define the subplot interfaces (see "Subplot abstraction"), then implement the "random" NPC talking on them: some NPCs give story-arc state text, some give city/location subplot McGuffins, some talk about named buildings, the gym type, gym leader, or the city's theme. Remaining NPCs talk about their immediate vicinity or random text.
5. Abstract the subplots per city/location, add more, and randomize them; likewise abstract story-arc advancement (less visible, since region tiles keep hardcoded story indices).
6. With subplots no longer fixed, building locations are no longer fixed: randomize city layouts within a region tile (except special buildings).
7. Abstract McGuffins and make them randomizable; until then, shuffling is limited by McGuffins and encounter tables.
8. Implement randomized encounter tables by the route's biome.
9. Shuffle city locations within a county, and which special locations are in each county.
10. Shuffle the counties' story indices.
11. Randomize terrain types and biomes (the last randomizable map element).

Random Pokémon types and move sets can be implemented at virtually any point in this order.

The work for each step is broken into milestones 0-12 in `remaining_work.md`. Milestone 0 covers the hardcoded foundation (stages 1-3); Milestone 1a defines the subplot interfaces and ports the first subplot; the milestones then follow the order: shuffled NPC texts, shuffled subplots, non-fixed arc cutscenes, random types, encounters, moves, trainers, procedural buildings and terrain, shuffled McGuffins, randomized city placement, procedural story indices, procedural terrain and biomes.

### Randomization switches

Each randomization sits behind one `RANDOMIZE_*` bit (`include/constants/randomization.h`), read with `IsRandomized()` (`src/util/randomization.c`); with the bit off, that path uses vanilla Emerald data. The mask is a `u16` in `SaveBlock2` (the former `padding2`), so it survives save and load and can later become a New Game setting; a new game copies `RANDOMIZATION_NEW_GAME_DEFAULT` (`include/config/randomization.h`). A randomization is built with its bit off, so existing tests keep passing; one commit then turns it on in the default and updates the tests' expected data, with the vanilla path kept.

Tests: the runner zeroes the save blocks before each test, so every bit is off and the player identity is fixed (OT ID 0, all-zero name), which makes the randomized path reproducible. A test opts in with `SetRandomized(bit, TRUE)`. Seed sweeps (`include/test/seed_sweep.h`) loop `FOR_EACH_SWEEP_SEED(i, seed)`, which writes a fixed, per-index OT ID and EOS-padded name into the save; a failing sweep test prints its index, and `SetSweepIdentity(i)` reproduces it.

### Emerald county assignment (stage 1)

Hoenn is fudged into the 6x4 county grid for the hardcoded stage; the county, terrain and biome tables are in `Emerald.md`.

### McGuffins

A McGuffin key is whatever gates progress until a condition is met: an NPC blocking the way until a subplot reaches a certain state, an HM needed to pass an obstacle, or a special plot item such as the Poké Flute to wake a sleeping Pokémon.

### Story graph

The set of places the player can reach is an implicit "story graph", ideally never held in memory.

- Reachable area only ever grows. The player gains route entrances/exits and cities, and never loses access.
- Exception: the initial rival battle in the lab temporarily reduces the reachable area, but the battle restores the prior reachability, win or lose.
- The reachable area is bounded by McGuffin locks, and at least one of those has a McGuffin key inside the reachable area.
- Usual pattern: within a county, a lock blocking access to route-story-index N has its key at N-1; between counties, a lock's key is given at a lower main-story-index (see "Main plot planner"). As long as the reachable area only grows, this makes the story completable.
- Soft-locks are prevented by construction, not detected afterwards: the generation algorithm must make them impossible, with no seed-sweep or solver check. The rules above (key at N-1, only-growing reachability) are the means.
- Soft-lock protection: if the route from a city to a higher route-story-index is always locked on the city's subplot being resolved, and a subplot's resolution only requires McGuffins in the city or in adjacent routes with a lower route-story-index, accidental soft-locks become much less likely.

The route-story-index parts of this section apply within a county (and to the starts of its adjacent routes); between counties the induction over planner steps in "Main plot planner" applies instead.

How accessibility is evaluated without a stored graph: every lock is checked locally where the player meets it (a blocking NPC, an HM obstacle, a gated route entrance). A lock on a place with route-story-index r opens when the flag of its key is set, and the key's location is derived deterministically from r (at index r-1, or the lower city's subplot completion). No global graph or reachability computation is ever needed.

Why soft-locks are impossible by construction, as an induction on route-story-index:

1. Claim: a player who has completed everything at indices below r can reach every place of index r and obtain its key.
2. Base: index 0 is the start county's city, where the player starts.
3. Step: a place of index r is entered from a place of lower index (the layer-2 growth constraint guarantees a lower neighbor; inside a county the +0, +1, +2, +3 chain enters each place from an earlier one). The only gate on the way requires keys found at index r-1 or at the lower city's subplot, and subplots require only McGuffins from their own city or adjacent routes of lower index. So nothing needed is at index r or above.
4. Monotonic reachability: reachable area never shrinks (apart from the initial rival battle, which restores it, win or lose), so no key can be lost by moving on.
5. Hence the claim holds for every r, and the story is always completable.

The argument depends on these rules, each of which must be kept:

- Keys are things that cannot be permanently lost. Key items and HM moves already cannot be sold, tossed, or forgotten in Pokémon games, so they are safe; ordinary items and ordinary moves must never be keys.
- No lock may be placed on a place at index r unless its key is placed at index r-1.
- Cutscenes of different arcs have distinct indices, and a cutscene never gates progress other than inside its subplot.
- A battle loss may block progress, provided the player can still reach wild encounters, shops that sell Poké Balls, and money for balls, so they can catch more Pokémon or level up. A loss only teleports the player to the last Pokémon Center, so battle McGuffins are rarely soft-locks. The initial rival battle is the exception that must unlock on a loss too, because the player cannot yet reach grass to level up.

Emerald's opening as an example:

- After the initial rival cutscenes, once the player has a Pokémon and has battled the rival, Littleroot Town, Route 101 and Oldale Town are accessible.
- Route 102 is locked: a sketch artist blocks its entrance.
- Route 103 is locked in the middle by Surf.
- Route 103 contains a McGuffin: the rival battle, which unlocks the next rival cutscene at Birch's Lab.
- Birch's Lab contains a McGuffin: the Pokédex and Poké Balls, which unlock the Route 102 artist lock.


### Team analyzer

A PC menu option that suggests one party/box swap, using the battle damage grades.

1. Hypothetical opponents: for each of the 18 types, one Pokémon with physical/special defense base stats 130/70, and one with 70/130 (36 total). Milder than the grade extremes, to resemble real species.
2. Each party Pokémon is graded against each opponent with the switch-hint grade (best move, accuracy squared, defensive matchup).
3. Top grade is decided by the raw damage estimate, not the letter, so ties are rare (if the raw estimate is hard to expose, tied Pokémon all count as top). Team score of a Pokémon = (opponents where it has the top grade) + 1 - avg(second place / its grade, over those opponents). It rewards covering many opponents, and covering them by a wide margin.
4. With a full party, the lowest team score is the "replaceable" Pokémon.
5. Each box Pokémon is scored as if it took the replaceable one's place; the best is the "replacement".
6. If the replacement scores strictly higher than the replaceable Pokémon, the analyzer suggests the swap; ties keep the player's existing choice.
7. With fewer than 6 party Pokémon, it instead suggests at least 1 box Pokémon to add (ideally the top 2-4, letting the player pick one).
8. After the player accepts a swap or addition, it offers to reanalyze or quit, so repeated runs iterate the team toward better coverage.

Known and accepted bias: scoring favors types whose same-type attacks are super effective against many types. Steel/Flying (e.g. Corviknight) gets STAB super effectiveness against 6 types, pure Normal against none, so the analyzer will often pick the former and rarely pure Normal types.

The player chooses to run it and waits, so it may take several seconds, behind a progress bar; run it in chunks across frames.

Future extension: a hardcoded list of 4-8 "best moves" per species (covering types and physical/special), scored the same way, to suggest move replacements. Combining it with the box scan (which box Pokémon would be best with moves they can learn later) is a combinatorial explosion (70 four-move sets per Pokémon from 8 candidates), so it would need a greedy or hill-climbing approximation; not designed yet.

### Gym guide advice

The existing tips man at each gym entrance also gives party-specific advice from the gym's type:
- Which party Pokémon have attacking moves strong against the gym type.
- Which party Pokémon have types weak defensively against the gym type.
- Species found in the current county, at an equal or lower route-story-index, with a type advantage over the gym type.

## End-to-end tests

Two tiers. `make check` (the headless `mgba-rom-test` ROM) covers logic and game state, including real maps and NPC scripts through `Test_LoadMap` (`include/test/map_loaded.h`); it has no VBlank, so nothing reaches VRAM or OAM, and no real key handling. `test/e2e` covers what needs those: UI as drawn, real input and multi-screen flows. Use the e2e tier only when the in-ROM tier cannot cover a scenario.

`test/e2e` drives the built `pokeemerald.gba` headlessly through libmgba using node-mgba (pinned), at about 1000x real time. Tests are JSON (`test/e2e/README.md`): a snapshot recipe, key presses with a configurable delay, expectations on memory (by symbol name from `pokeemerald.map`, with game text decoded through `charmap.txt`), and an optional screenshot region compared against a committed golden image.

Snapshots are built from boot rather than committed: a recipe presses through New Game, then `teleport()` writes the save block's location and position and points `gMain.callback2` at `CB2_LoadMap`, which is what a warp does. Savestates embed ROM addresses, so they are cached per ROM hash and recipe code and rebuilt when either changes. The runner refuses a `.gba` and `.map` that are not from the same link. Animated parts of a screen (the message prompt) are masked in waits and in golden comparisons. A failing test writes its screenshot, diff, RAM dumps and a savestate to a gitignored `out/` folder, which is how a failure is investigated. It stays out of `make check` until it is stable and cheap enough.

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

### Subplots bound to their original maps (shuffling only by swapping maps)

Keep each subplot's NPCs and events in its Emerald maps, and shuffle by swapping whole cities.

Pros:
- No new runtime NPC machinery.

Cons:
- Subplots cannot move between cities, which is what stage 5 needs.
- Conflicts with procedural buildings, where maps are not fixed.

### Runtime-spawned NPCs for every subplot beat

The generic cutscene runner creates every subplot NPC and event at runtime, instead of placeholder object events in each building.

Pros:
- A subplot is independent of the host city's buildings.

Cons:
- Subplots tied to a building (a gym, a shop, a house) lose their interiors.
- Large rewrite of every subplot up front.

### Generator-first development

Build the procedural region generator first and hand-author content to fit it.

Pros:
- Matches the end goal directly; no throwaway hardcoded layer.
- Finds generator performance problems early.

Cons:
- Story progression can't be tested without NPC hints, which need subplot/city metadata the generator would have to produce first.
- Everything must work before anything is playable.
- Random layouts make bugs hard to reproduce.

### Two biomes per species

Associate each species with two biomes: only the first determines types, but the species can spawn in either.

Pros:
- Players could encounter ~470 of the 541 lines, so far fewer species are locked out of a playthrough.

Cons:
- 470 exceeds the 420 box slots.
- Needs a second mapping and more bookkeeping for the species-to-biome lookup.

Not planned; may be revisited depending on player feedback.

### 4x4 biome subsets with a mod-25 mapping

Each region uses 16 biomes (4 humidity x 4 temperature levels), and species map over all 25 biomes, so lines landing outside the subset are not wild.

Pros:
- Fewer wild lines (~346), closer to the box capacity.
- More biome variety within one region.

Cons:
- 4 temperature levels made a region's hot and cold ends too far apart (Hoenn fits in 3).
- Locks ~195 lines out of each run's wild encounters.

### 5x4 counties of 6x4 tiles

Use 5 columns x 4 rows of counties, each 6 wide x 4 tall.

Pros:
- Matches Kanto's 5 columns of cities.
- Larger counties leave more room within each county.

Cons:
- Fewer locations on the map, and longer routes between them.

### Damage grades in the overworld party menu inside gyms

Showing the switch-hint grades in the field party menu while inside a gym, graded against the gym type.
- Pros: helps plan a gym's battles before they start.
- Cons: a rare case for extra UI; the gym guide's advice covers the same need.

### Per-county flags for capital and Elite Four

Store an `isCapital` and `isEliteFour` bit in every county header.

Pros:
- A county reads its own header only.

Cons:
- Spends 2 bits x 24 on one-per-region facts; the region header holds the capital id, and R plus the story index identify the Elite Four.

### A global 28x15 table of tile roles

Store one grid of cells for the whole Emerald region instead of per-county tables.

Pros:
- Direct lookup by region cell; simplest to read.

Cons:
- Hides the county as the unit of generation, so the generator's per-county hashes would need a different shape.
- Emerald.md fudges places into counties, so the grid would disagree with the real Hoenn region-map layout and invite using the wrong one.
- 420 bytes of ROM for mostly empty cells.

### Tile positions and names in the headers

Store each county's city tile, special-location tile and city name index in its header.

Pros:
- No hashing at read time.

Cons:
- Roughly doubles the always-resident data for values a per-county hash gives for free.

### A separate required news string per main beat

Each beat would carry its own authored news line for the Pokémon Center, instead of reusing the hint entries.

Pros:
- The news could be worded as news rather than as an NPC's hearsay.

Cons:
- Unneeded: the far main-beat text is generated, so it exists for every beat, and a subplot's `farCurrent` and `closeCurrent` are already required by the descriptor.
- One more string per beat and per subplot stage, to author and keep consistent with the hints.

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
