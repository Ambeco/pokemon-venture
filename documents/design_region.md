# Region generation design

The generated region's structure: the county grid, special locations, county-story-indices, routes, route-story-indices, terrain and biomes. Storage and accessors are in `design_data.md`; the main story that rides on county-story-indices is in `design_story.md`. The layering rule that governs all of it is in `design.md`.

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
- In a county with a city, McGuffins gating progress out of the county require only the city's chapter, never the reserved location's. The reserved location is an optional dead end.

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
- Pitstop: not a location as far as the headers and route placement are concerned. Pitstops are placed after the routes are final, on route region tiles that are otherwise empty (no location, no other pitstop), and are dealt out to counties by a coprime stride over the pitstop list, so none repeats (code: `include/region/region_pitstop.h`, `src/region/region_pitstop.c`: slot j is pool entry `(j * stride + offset) mod L`, and county ranks come from a second coprime stride over story indices; a county's window of `floor(L * r / N)` slots depends on its story index alone, so counties never read each other's route plans, and capacity only truncates a window, never moves a slot). Because they sit on existing routes, they need no routes of their own, which keeps the number of intra-county routes (and so route de-overlapping) down: roadside stops and similar small places should be pitstops rather than detours.
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

Ocean counties are skipped in steps 3-5 until Surf is placed, as in mainline games where Surf opens the sea late. In steps 3-4 a "dead end" means no unindexed non-ocean neighbor. At a dead end, count the unindexed non-ocean counties (not counting the set-aside Elite Four and reserved counties). If at least about 3 remain, step 4 applies as written, with an ordinary key and lock. If fewer remain, this is where Surf is placed instead: the current county gets the Surf McGuffin key, and the next county is the closest unindexed ocean county adjacent to an indexed county, which gets the Surf lock. From then on ocean counties are eligible like any other. The count threshold, not a fixed index, makes Surf arrive near the end of the land counties however many ocean counties the terrain produces. If only ocean counties remain as candidates, Surf is placed immediately. Surf is always placed in time: if no qualifying dead end has occurred by four indices below the Elite Four county's index, it is forced there, so no ocean county can end up unreachable and gyms 6-8 still fit after it. The land count excludes only the Elite Four county, since the reserved counties are whatever is left when the walk ends. Surf is never placed before index 5 (gym 5 is the Surf gym, see "Main plot planner" (design_story.md), and gyms 1-4 need four distinct time blocks after the scripted block 0; any county, including the first two, may host a gym); the dead-end test checks the index explicitly. The terrain rules do not guarantee enough land by themselves: ocean extends toward the middle of the map, and a land-starved seed would force the walk onto an ocean county at a dead end. Layer 1 therefore requires at least about 10 non-ocean counties (mountain counts as non-ocean): the Elite Four county, the six or so counties up to Surf, and the roughly 3 left when Surf is placed. A seed-sweep test checks both.
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

Code: `include/region/region_route.h`, `src/region/region_route.c`. `DoCountiesHaveRoute` applies the rules above (a pure function of the headers, symmetric). The endpoints and the border crossing are pure functions of the unordered county pair, so both counties agree without storing anything. The endpoints are the closest pair of locations (Manhattan distance), ties preferring cities, then the lower location index, scored over the lower-id county first. The crossing is one row (column, for north-south pairs) taken from one of the two locations by hash. The county owning that location runs a straight route along its row to its border column. The other county joins its location to the crossing tile on its own border column with an L or Z shape (the bend is a hash), and bends the Z to avoid crossing its own other routes and passing through its locations (a backtracking search over its bendable halves: hashed bend first, then bends toward the border, then toward the location; the hashed bends are kept when no crossing-free set exists), since it knows them all. Each county edge carries exactly one inter-county route, so border runs never collide. Two halves may share a run of tiles, for example when a location sits on the border column, but may not cross: at every shared tile they must leave in a common direction. A shared run is one road with a single owning route per tile: the sharing route that opens first (the later of its two counties' story indices; an intra-county route opens with its county), ties to the lower route id. A McGuffin lock sits on the earlier-unlocking county's half, on the first tile after the last tile that half shares with another route (so the road cannot be joined after the lock), not at the location; if that half is empty or all shared, on the later half's border tile. Each outside-city special location also gets an intra-county route to the county's city (a hashed axis, Z-bent around the county's other routes, planned after the inter-county halves so a crowded county never costs the halves their crossing-free plan); a county without a city has none. Each half is decided by one county alone, so no two counties ever coordinate past the crossing row, and routes cannot collide across a border. Generated locations never sit on a county's outermost row or column (the same border rule as cities); routes may cross them. The hardcoded Emerald table does not obey this and is left as it is: the abstractions exist because Emerald only roughly fits the constraints, and its placeholder positions are treated as if they obeyed them. A location outside a city is itself a route tile (`TILE_ROUTE | TILE_SPECIAL`), so the route reaches it without a gap and "adjacent" locations still have a route involved; a location inside a city shares the city's tile instead.

### Route story indices

Each route, city, and special location also gets a route-story-index, derived from the county-story-indices of the places a route connects. Route-story-indices start at 4 * county-story-index, which leaves room for several steps inside one county.

Within a county, the "first" city/location is the one the main route into the county connects to. That route comes from the adjacent county with the highest story index (below this county's); any other city/location in the county is the "second". Indices are then:

- 4 * county-story-index + 0: the main route into this county, and the first city or location.
- +1: a special location inside the first, if the first is a city.
- +2: the second city or location (if any), and the route connecting the first and second (if any).
- +3: a special location inside the second, if the second is a city.

Between counties:

- The route gets its index from the higher county's city or location, and that place gets the same index.
- Whether and when the route opens is not decided by the index: a route is gated by the unlock condition of its later end location (see "Main plot planner" (design_story.md)), and the main plot planner places each county's first-location unlock at step 3 of the previous county's block (index 4(x-1)+3), after that block's key (if any) was given.

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
- Every evolutionary line maps to one of the region's 12 biomes (see "Species <-> biome mapping" (design_pokemon.md)), so all ~541 lines are wild-encounterable, about 45 per biome. That exceeds Pokémon storage (420 slots, 14 boxes of 30); if it feels like too many, the mapping widens to mod 13+, and lines landing on residue 12+ do not spawn in the wild that run (NPCs can still have and trade them).

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

## Open questions

Undecided points; none blocks the generator work in progress.

- Region generation details still open: terrain, biome, population and special-location types, the story-order algorithm, key/lock rules, capital selection, and how generated data maps onto pokeemerald maps, warps and encounters.
- Whether a city's count of inside-city special locations equals its population (1-3); Emerald averages about 0.6 per city (about 1.2 counting gyms), see `Emerald.md`.
- Route rules: tie-breaking and duplicate-route handling when several adjacency rules apply to the same county pair.
- The special-location catalog: inside-city vs outside-city lists, how a location is chosen for a county, and its relation to terrain, biome and story order.
- How the underground tunnels around Saffron (Underground Path) fit the region model; Red/Blue map reproducibility is a design check.
- The 5x5 biome grid's axis values and which 12 biomes each 4x3 subset yields; the meaning of each 2-bit population level (0 = none, 1 = village, 2 = town, 3 = city) and how it relates to terrain and biome; whether the region is regenerated on demand or cached in RAM (cost of regeneration).
- The three settlement generation algorithms (village, town, city): building footprints, path layout, mapping onto GBA map layouts and metatiles, and where gyms, Pokémon Centers and Marts go in each.
- Biome art and terrain: which biomes share tilesets (palette variants) and which get unique sprites; the per-biome terrain placement algorithms (trees, grass types, dirt, water, rocks, hills); mapping the 25 biomes to type-affinity tables and to Emerald's existing tilesets for the hardcoded stage.
- The geography generator: outer-ring mountain and ocean stretches, inland mountain and ocean extensions, island formation, and humidity and temperature gradients at region-tile granularity that guarantee a non-mountain desert-like biome.
- How each landform and cave variant (see "Landforms and cave variants") biases tiles, encounters and layouts.
- The theme catalog (requirements, sprite sets, gym types, NPC texts), how the region layer assigns themes to spread gym types, and how placement tendencies (Elite Four corner, start near it, capital far, coast and mountain city nudges) are implemented within the layering principle.
- How strongly gym assignment follows the average gym-order table (soft bias strength, Elite Four preference for late types) alongside theme requirements.
- Whether the dropped top tile row leaves the 28x15 cells (after dropping the first and last columns too) holding the intended county contents, and whether the city name and Nav overlays cover only unused tiles; to be checked against the Emerald region map.
- Shuffling which gym ordinal unlocks which HM (non-blocking). Constraints: Surf stays tied to the Surf-lock county's block and gym 5, Fly (and its gym) is never a callback and its block sets the callback distance bound, water HMs need terrain on the routes into the next county (or the blocker fallback), and an HM's badge must be held when the HM-gated route is entered.

## Alternatives considered

### 5x4 counties of 6x4 tiles

Use 5 columns x 4 rows of counties, each 6 wide x 4 tall.

Pros:
- Matches Kanto's 5 columns of cities.
- Larger counties leave more room within each county.

Cons:
- Fewer locations on the map, and longer routes between them.
