# Pokémon Venture — Design

Current state of the design for the mod. Organized by topic; describes what the design *is*, not how it evolved (history lives in `notes.md`, open items in `remaining_work.md`).

## Overview

The mod procedurally generates a Pokémon world rather than using a fixed map. (The user describes the idea as "overly ambitious"; the full concept is still being collected.)

The goal of the world generation below is a game world map with cities, routes, and special locations that mirrors what a normal mainline Pokémon game looks like.

## Layered generation

Generation is a stack of layers, each computed only from the layers above it, never from its own neighbors or from later layers:

1. World map: terrain types and biomes per county, from the PRNG seeded on the player ID alone.
2. World map: county-story-indices, from layer 1 plus a couple of McGuffin boundaries.
3. World map: county populations, city locations and names, and special location IDs and positions, from layers 1-2.
4. World map: which counties connect to which, from layers 1-3 (using only county-story-index adjacency and the capital, never planner data).

    The main plot planner (see "Main plot planner") needs no routes: its items come from layers 1-3, and layer 4's route rules already connect each county to the next county-story-index. Layer 3 runs in this order: (a) the settlement quota: counties 0 and 1 and the Elite Four county always have a settlement, plus a random 8 to 13 more; (b) the gym hosts (the Surf gym's host is the Surf-lock county or its callback county), each picked from its hardcoded range, which gives the gym counties their settlements first; (c) the remaining settlements, distributed among the other counties, then populations and special locations (with arc-beat reservations); (d) the other items' hosts from their masks.
5. Each county: the details of its own world tiles (including exact route placement) and its route-story-indices, from world layers 1-4 only. No county layer depends on any other county's data, including an adjacent county's route-story-indices. Counties may have sub-layers like the world map.
6. Field maps: generated from world layers 1-4 and the county they are inside, never from another county's data or from adjacent field maps. The exception is inter-county routes, which may use the data of the two counties they connect.
7. Further layers continue the same pattern.

This forces single-pass generation with no recursion or transitive operations, and keeps memory use minimal. Any new rule must be checked against it: a rule that needs data from a later layer, or from a sibling's same-layer output, breaks the design.

## World generation

- The world map is a grid of 6 columns x 4 rows of "counties" (roughly the 5-6 columns and 4 rows of cities in Kanto).
- Each county is procedurally assigned a terrain type, a biome, a population, and a special location.
- A "story order" over the counties is generated, which sometimes designates McGuffin "keys" and the "lock" counties they open.
- One county's city is picked as the "capital".

### County layout

- Each county is 5x4 "world tiles" (5 wide, 4 tall). A world tile is one cell of the Hoenn region map, which is 28x15 cells (`MAP_WIDTH`/`MAP_HEIGHT` in `src/field/region_map.c`; cell contents in `src/data/region_map/region_map_layout.h`).
- 6x4 counties of 5x4 tiles is 30x16, so the first tile column of the first county column, the last tile column of the last county column, and the top tile row of the top county row are off limits, leaving 28x15. The top row is dropped (not the bottom) because the city name overlay covers the top-right 13x1 tiles and the Nav covers the bottom-right 13x2 tiles, so this leaves both the top and bottom counties 2 usable rows in those corners, where dropping the bottom would leave the bottom-right county only 1. The 6x4 grid of 5x4-tile counties was chosen over the inverse (5x4 counties of 6x4 tiles) because it yields more locations on the map and shorter routes between them.
- If a county's population is above zero, it places a settlement at a random world tile, except that the county's bottom row and rightmost column are excluded (unless the county is in the bottom row or rightmost column of counties), which keeps space between cities. Population is 0-3 (2 bits); population 0 never generates a city.
- Every county always has a special location, whether or not it has a city.
- Some counties instead hold a reserved end-game special location (see Reserved legendary locations). Layer 2 reserves them while assigning county-story-indices, so they are known before layer 3 places anything.
- With a city, the special location is placed either in or adjacent to the city (50%), or on a random non-adjacent world tile (50%).
- Without a city (population 0), the special location is necessarily not in a city, and it is what routes attach to.

### Reserved legendary locations

A few special locations are reserved for legendaries: 2-3 after roughly the 7th gym (each holding a pair or trio of legendaries), and 1 solo location after the Elite Four. They are reserved on the world map, as part of the initial county-story-index determination, and relax the usual special-location rules:

- If the county has a city, routes are generated for the city as if the reserved location did not exist, then one extra route runs from the city to the reserved location. That route and the location get end-game route-story-indices (100+), above every ordinary index.
- If the county has no city, routes generate as normal, and the reserved location is the county's ordinary special location and what routes attach to.
- In a county with a city, McGuffins gating progress out of the county require only the city's subplot, never the reserved location's. The reserved location is an optional dead end.

### Settlement types

Population selects one of three generation algorithms:

- 1 = village: buildings never touch each other, and paths run everywhere intrinsically.
- 2 = town: a few specific paths run through it; buildings often touch each other, and the layout is not very organized.
- 3 = city: paths are mostly straight horizontal or vertical lines, and buildings mostly fill the grid.

### City themes

Some towns and cities have a theme.

- Themes often have requirements, such as being on an island, being far from the capital, being low level, or being ocean-adjacent.
- A theme affects which sprites the settlement uses and its gym type, and carries its own NPC texts so NPCs talk about their town appropriately.
- Themes are picked at the world layer(s), so gym types can be distributed well across the map.

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

The design is intended to be able, in principle, to generate the Red/Blue world map. How to represent the underground tunnels around Saffron (the Underground Path, not Diglett's Cave) is TBD.

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

Ports are layer-2 county flags. A ferry is a route between two port counties of any distance, and it is the main route into the port's ocean county (the adjacency rule for routes does not apply to ferries). It follows the usual lock rule, open exactly when both ends are unlocked, so the ferry to the third port opens only after that county is unlocked normally. The ocean county reached by a port is available before Surf; the Surf rule below still governs every other ocean county.

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
- Counties only ever route to adjacent counties, except ferries between port counties (see "Ports").

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

- Routes and cities are not locked to their county's biome. The gradients extend to the world-tile level, and each route or city is based on its own world tile.
- A single route or city has a single biome, with no gradient within it.

### Landforms and cave variants

A route can additionally be marked as a landform within its biome, which also breaks up the monotony of routes sharing a biome: beach, volcano, farmland, sandbars, fjords, salt flats, mesas, river basin.

Caves and tunnels get distinct characteristics too: underground river, bioluminescence, crystals, sinkholes, magma, ice.

For reference, the Pokémon Snap courses were beach, tunnel, volcano, river, cave, valley, clouds, meadow, jungle, forest, desert, reef, ocean, snow, tundra, and ruins. The biome grid already covers meadow, jungle, forest, desert, ocean, snow and tundra; tunnel, cave and ruins are special locations; the rest are landforms.

## Biomes

- There are 25 possible biomes, forming a 5x5 grid: one axis is precipitation, the other is temperature. Every species is associated with one of all 25 biomes.
- A given region/world map is limited to one of the six 4x3 subsets of that grid (4 adjacent humidity levels of 5, and 3 adjacent temperature levels of 5), chosen randomly per game: 12 biomes. This keeps biomes coherent at small scale and makes separate games feel distinct. Temperature spans only 3 levels because 4 made the hot and cold ends of one region too far apart.
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

- GBA memory is scarce, so per-county data is bitpacked: biome type in 4 bits (a world uses only 16 of the 25 biomes), population in 2 bits (roughly logarithmic).
- Map generation uses a PRNG seeded from the player ID. The generated world therefore does not need to be stored in the save file, and possibly not even in RAM (it can be regenerated on demand). Every player ID yields a completely different map.

- Saves record a generator id, and the game refuses to open a save whose generator id does not match. Players upgrading mid-playthrough are not supported.
- Seeds: terrain and world generation use the player ID plus the secret ID. Pokémon types and moves, which affect battles, use only information that travels with a Pokémon (the original trainer's ID and name), so battles and traded Pokémon are handled correctly. The 16-bit player ID alone gives only 65535 worlds, which is considered acceptable; the player name adds entropy for types and moves.
- Incremental generation goal: world, county, and map generation should be light enough to run in tiny chunks per frame as the player approaches a new area. On entering a city, plot points are picked a little per frame so the next cutscene location is known before the player has moved about 5 tiles.

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
- CPU time is the bigger hurdle: on-demand regeneration of the world, species mapping, encounter filtering, and learn-level move picks must be cheap enough for a GBA.

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
- Callback chance p: roughly how often a normal Emerald run must revisit a prior town, about 2 of 23 blocks, raised slightly (about 12%). The tutorial's callback at block 1 satisfies the minimum of one callback in every world.
- Step: a hash bit chooses step 0 or 2 for a non-callback item, and step 1 for a callback.

A callback is hosted in county y, which is already unlocked, and may hold a gym, a rival battle and a Rocket beat together.

- The next step after an index is index + 1, and its list is the items whose time and step match: evaluate every item (about 30 hashes) and cache the current list in EWRAM.
- A county lists everything it owns by evaluating the items whose time range includes its block, plus the callback blocks after it whose y(x) is this county (at most the bounded number of blocks before Fly, all later blocks after). It never reads another county's private data.
- Nothing is stored beyond the masks (about 12 bytes per kind), the county-story-indices in the county headers (5 bits each), and the cached current list.

Unlock gating. Every "county x unlocks county x+1" step names an explicit McGuffin, so that non-adjacent counties can reference their key and so the main story, which manages the main McGuffin list, can ensure none is reused. The step is gated by every traversal McGuffin obtained in block x's lists (each an obstacle on the routes into x+1), and a jump key from layer 2 is satisfied by them when present; otherwise one key item or a blocker McGuffin from the global pool is used. All of them are obtained in block x's lists, never later. A gym enables one HM (8 gyms, 8 HMs; in the fixed Emerald order for this stage: gym 1 Cut, 2 Flash, 3 Rock Smash, 4 Strength, 5 Surf, 6 Fly, 7 Dive, 8 Waterfall) and an HM has no range of its own: it takes its gym's time. Both are items of the same host and step (except in the Surf block), so the player does the city's subplot (which hands out the HM) and the gym in any order, then uses the HM on the obstacle on the route into the next county. A gym and HM placed as a callback gate the unlock of county x+1 of that block, not of their host's next county. An obstacle must be buildable on the routes into x+1. The land obstacles (a Cut tree, a Strength boulder, a Rock Smash rock, a short dark tunnel for Flash) are buildable on any route, so only the water HMs (Surf, Dive, Waterfall) depend on the layer-1 terrain of x and x+1, which their block mask checks; if no block in a water HM's gym range qualifies, the HM is still given at its gym's time, and that unlock uses a blocker McGuffin instead. The Elite Four unlock is the end-game exception: it needs all 8 badges, not a McGuffin from the previous block.

Surf. Layer 2 places the Surf lock at step 4n+3 of its dead-end county n, so block n must give Surf and gym 5, the Surf gym (gyms 1-4 are in counties before n and gyms 6-8 after it). Both are items with time n. Either, both or neither may be in block n's callback, independently: a callback item is hosted by the callback county y(n), so it may be acquired elsewhere and brought back, and a non-callback item is hosted by n. The host of the Surf gym is a gym county and has a settlement. n is at least 5, so gyms 1-4 fit in four distinct time blocks 1 to n-1 (block 0 is the scripted tutorial; the starting county can host a gym only as a callback in a later block), and at most four below the Elite Four county's index, so gyms 6-8 fit after it. Step 4n+3 then unlocks the ocean county that holds the Surf lock, whose routes are gated by Surf. Counties indexed before Surf's lock never rely on water crossings; ocean counties' route generation can assume the player holds Surf, with no bridge special cases (ferries are the port routes). A jump key is likewise an item of the key county n with time n.

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

Open: the exact callback chance and distance bound (tuned from play); the hardcoded ranges for each kind's ordinals (placeholders at first; each is listed for the user to check); the end-game blocks (the Elite Four steps, the reserved legendary counties); whether an HM obstacle on a route counts as the unlock of the location behind it; the size of the McGuffin id pool.

### Story state storage

Emerald stores all story state as flags and vars in `SaveBlock1` (`include/global.h`), accessed through `src/minigames/event_data.c` (`FlagGet`/`FlagSet`, `VarGet`/`VarSet`) and driven by map scripts (`data/maps/*/scripts.inc`, `data/scripts/`) using `setflag`, `setvar`, `addvar`, `goto_if_*`, `switch` and `map_script_2 VAR, value, handler` (which fires a handler when a var holds a value, the usual way a stage triggers a cutscene).

- Flags are single bits (`u8 flags[]`, 2400 flags / 300 bytes). IDs run `0x000`-`0x95F`: temp (cleared on map load), trainer-defeated (`0x500`+), system (`0x860`+, includes the badge flags `FLAG_BADGE01_GET`..`08`), and daily. About 380 are already unused. `FLAG_HIDE_*` flags control whether an NPC exists.
- Vars are 16-bit (`u16 vars[]`, `0x4000`-`0x40FF` = 256 vars / 512 bytes, the first 16 temp). About 29 are already unused. A var is a stage counter for one storyline.
- Limits: the saved ranges are fixed by `FLAGS_COUNT` and `VARS_END`, and growing them costs `SaveBlock1` (and so save sector) space. The ID space allows far more: flags up to `0x3FFF` and vars up to `0x7FFF` before the unsaved special ranges (`SPECIAL_FLAGS_START` `0x4000`, `SPECIAL_VARS_START` `0x8000`). IDs `>= SPECIAL_*_START` are RAM scratch and are not saved.
- Emerald's own dependencies are implicit counters. Example: Norman's gym requires 4 badges because each of the first four gyms does `addvar VAR_PETALBURG_GYM_STATE, 1`, and Norman `switch`es on it. These are replaced by explicit lock/key checks when McGuffins are abstracted.

Planned layout, once the Emerald vars are no longer needed (badge and HM flags stay as they are):

- Main story: one var, the main-story-index, a position `4x + s` over the counties (under 100 values, plus the end game) that always advances by one (see "Main plot planner").
- List completion: the completion mask of the current list of gets lives in the host county's reserved vars, alongside that county's own subplot stages, and the accessor reads it. Nothing else about the main line is stored.
- County-internal state: each county's own subplots keep their stage in vars reserved for that county, which no other county reads. 4-bit slots would need at most 18 vars (3 subplots x 24 counties x 4 bits, 4 slots per var); once Emerald's shufflable story vars are retired (map scripts reference ~220 vars in all, not all story; the reservable count is not yet measured), a whole var per city or special location (about 48) is affordable, and is simpler than packing. Vars are allocated in named regions as Emerald's vars are retired. A var per route (fewer than 79) would also fit, and is left for a later version.
- Flags for county-internal McGuffins come from per-county reserved pools (see "Main plot planner").

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
- NPC hosting: each host building keeps a few placeholder object events whose graphics come from a var (`OBJ_EVENT_GFX_VAR_0`, `VAR_OBJ_GFX_ID_0`..`F`), a generic script, and a hide flag; a subplot fills them by setting the vars and flags in the host map's on-transition script. These vars are shared (the rival graphics, battle frontier and Route 117 already set them), so they are never persistent state. This is cheaper than spawning NPCs at runtime, which is kept for cutscene beats (an NPC running up to the player) via the generic runner.

#### Interfaces

Defined for the end state, not for Emerald, so NPC talk never needs redesign:

- Subplot descriptor: linear stages; per stage the hint texts, story-state texts and trigger place (by role); the requirements above; rewards; and the key/lock it produces. A subplot that unlocks a location describes its output as a key for the gates on the routes into that location, not as "unlock X". A subplot supplies its own blocker scripts (the person who stands in the way, with texts) for now.
- Stage accessor: reads and writes a subplot's stage by slot (the county's reserved vars, see "Story state storage"), never by subplot identity, so moving a subplot moves no state. A subplot that is a main-plot step has no state of its own: not started or current comes from the main-story-index (see "Main plot planner"), and done or partly done from the county's completion mask. Subplots internal to a county use that county's reserved vars. Callers cannot tell the difference. The main-story-index is not maintained until the main arc is ported (Milestone 3), so until then main-line stages come from the adapter over Emerald's vars.
- NPC-talk query: reads only descriptor, accessor and placement, and returns the text to say. It cannot tell a ported subplot from an adapter-backed one.
- Adapter: lets an unported Emerald subplot answer the accessor from its old vars (e.g. Petalburg gym "open" is `VAR_PETALBURG_GYM_STATE >= 6`). It is read-only, so NPCs can hint at unported subplots, but only ported subplots can be shuffled. Subplots are ported one at a time, each leaving `make check` green.

#### Text references

The script engine has no field-path variables. Text holds only the placeholders `{STR_VAR_1}`-`{STR_VAR_3}` (`gStringVar1-3`), a few fixed ones (`{PLAYER}`, `{RIVAL}`, `{AQUA}`...) expanded by `StringExpandPlaceholders` (`src/util/string_util.c`), and values that scripts put into the string vars with `buffer*` commands or a `special` function.

Authored text uses story references such as `$currentCity.gym.leader.name`, `$currentCity.nearbySmallCity.name`, `$stepCounty.city.theme.motto` or `$currentCity.pkmnCenter.nearBuildingName`. A reference is an anchor plus a path through the generated world data. A build tool turns each reference in a template into a compact reference id, and a single talk special expands the whole template in C into `gStringVar4` and shows it itself with `ShowFieldMessageFromBuffer` (not via the `message` command, which expands its source string into `gStringVar4` and would overlap it); this avoids the three-reference limit of `{STR_VAR_1}`-`{STR_VAR_3}`. Subplots are limited to a single county, so anchors are the speaker's own place (the NPC's city and building, for `$currentCity.pkmnCenter...`-style random talk), the subplot's own place, and, for hints about the main story, the responsible county of the current step (`$stepCounty`).

#### Open

- Which building roles exist, and how many placeholder NPC objects each building needs.

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

## Development approach: hardcode first, then randomize

The world starts as a hardcoded Pokémon Emerald world and is progressively abstracted and randomized, rather than building the generator first. Hardcoded content exercises the same metadata the generator will later produce, and keeps the story testable throughout.

Why this order: if NPCs don't talk about subplots and story arcs, testers can't tell how to progress, which would cripple testing; and subplot babble requires city/route/location metadata anyway. NPC talk exists to support subplot shuffling, so its interfaces (descriptor, stage accessor, text references; see "Subplot abstraction") are defined first, for the end state, and NPC talk is built on them before any subplot is ported.

Stages, each leaving the game playable:

1. Divide the Emerald world map(s) into the 6x4 counties with hardcoded terrain type (mountainous, ocean, flat), biome, population, special location, story order, McGuffin keys and locks, and capital (Lilycove City). A key/lock is basically anywhere story advancement moves from one county to a non-adjacent county. The story-arc id is also hardcoded.
2. Hardcode the world tiles within each county: which are cities, routes, and special locations.
3. For each city and special location, hardcode its named buildings, subplot McGuffins, and city "theme"; hardcode which cities have gyms, the leader's name, and the gym type.
4. Define the subplot interfaces (see "Subplot abstraction"), then implement the "random" NPC talking on them: some NPCs give story-arc state text, some give city/location subplot McGuffins, some talk about named buildings, the gym type, gym leader, or the city's theme. Remaining NPCs talk about their immediate vicinity or random text.
5. Abstract the subplots per city/location, add more, and randomize them; likewise abstract story-arc advancement (less visible, since world tiles keep hardcoded story indices).
6. With subplots no longer fixed, building locations are no longer fixed: randomize city layouts within a world tile (except special buildings).
7. Abstract McGuffins and make them randomizable; until then, shuffling is limited by McGuffins and encounter tables.
8. Implement randomized encounter tables by the route's biome.
9. Shuffle city locations within a county, and which special locations are in each county.
10. Shuffle the counties' story indices.
11. Randomize terrain types and biomes (the last randomizable map element).

Random Pokémon types and move sets can be implemented at virtually any point in this order.

The work for each step is broken into milestones 0-12 in `remaining_work.md`. Milestone 0 covers the hardcoded foundation (stages 1-3); Milestone 1a defines the subplot interfaces and ports the first subplot; the milestones then follow the order: shuffled NPC texts, shuffled subplots, non-fixed arc cutscenes, random types, encounters, moves, trainers, procedural buildings and terrain, shuffled McGuffins, randomized city placement, procedural story indices, procedural terrain and biomes.

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

## Alternatives considered

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

Build the procedural world generator first and hand-author content to fit it.

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
