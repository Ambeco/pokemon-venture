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
4. World map: which counties connect to which, from layers 1-3.
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
4. At a dead end (no unindexed neighbors), the current county gets a McGuffin key. The next county is the closest keyless county that still has 1+ unindexed neighbors, and it gets the matching McGuffin lock. The walk continues from there.
5. If the walk dead-ends with every remaining unindexed county having 0 indexed neighbors before the target count is reached, the next indices are assigned to those counties at random until it is.
6. The Elite Four county takes the next index, then the R counties still unindexed take the last R indices, and become the reserved legendary counties (see Reserved legendary locations). Which of them is the solo post-Elite-Four county, and how the others tie into the gym order, is decided by a separate algorithm.

Ocean counties are skipped in steps 3-5 until the next index is 15 or higher, as in mainline games where Surf opens the sea late. The first ocean county to be indexed gets a Surf McGuffin lock, and the county with the previous index (its index minus 1) gets the Surf McGuffin key. If only ocean counties remain as candidates before index 15, the first one is taken anyway, with the same lock and key.

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
- Counties only ever route to adjacent counties.

### Route story indices

Each route, city, and special location also gets a route-story-index, derived from the county-story-indices of the places a route connects. Route-story-indices start at 4 * county-story-index, which leaves room for several steps inside one county.

Within a county, the "first" city/location is the one the main route into the county connects to. That route comes from the adjacent county with the highest story index (below this county's); any other city/location in the county is the "second". Indices are then:

- 4 * county-story-index + 0: the main route into this county, and the first city or location.
- +1: a special location inside the first, if the first is a city.
- +2: the second city or location (if any), and the route connecting the first and second (if any).
- +3: a special location inside the second, if the second is a city.

Between counties:

- Adjacent county-story-indices: the route gets its index from the higher county. By default it is McGuffin-locked on completing the subplots of BOTH the lower county's city and special location. The higher county's city or location gets the same index, so completing the subplot at the lower location lets the player proceed along the route to the next one.
- Non-adjacent county-story-indices: the route gets its index from the higher county's city or location. If county generation determined there is a McGuffin lock into the higher county, the route is blocked on that McGuffin. Otherwise, by default, entering the higher-index city or location triggers a new McGuffin that unlocks all of its routes to lower-index counties.

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
- Each arc determines where its next cutscene should be: a county-story-index, then a specific route or city (and, for a city, a specific building).
- When the player finishes that route or leaves that building, the cutscene triggers. Usually an NPC runs up, says something, sometimes triggers a battle or gives an item, then runs off screen. The arc then advances to its next stage.
- When a city is generated or loaded, the code checks whether it (or an adjacent route) is the right place for each arc's current state, and uses that to pick "near" or "far" NPC texts for that arc. At least one NPC per arc gets an arc-state hint text. When the state advances, those NPCs switch to an arc-state-specific thank-you text.
- Subplots work the same way but take place entirely within a city and the starts of its adjacent routes.
- Arc and subplot state use the game's existing flag/var storage for small story state.
- Cutscene placement is deterministic, so it is never stored: the rival arc procedurally selects route-story-indices for its state-advancing cutscenes, then the story arc selects route-story-indices for its own, avoiding the rival arc's. With 50-100 route-story-indices to choose from, collisions are not a concern.
- Each route checks whether a rival or story cutscene belongs to it and plays it when the player enters. Each city or location likewise checks whether a cutscene belongs to its area and may place it anywhere inside its subplot, since the player cannot advance without completing the subplot anyway.

### Possible future extensions

In the far future:

- City subplots could involve the gym leaders more, as in Pokémon Black and White.
- Specific named grunts could take a bigger part in story arcs, like Jessie and James in Pokémon Yellow.

### Rival arcs

The first version always uses the Emerald rival arc, preserving the cutscene text as much as possible:

- You scare off a Pokémon, the professor gives you two Pokémon, then May/Brendan battles you.
- On a route just before the first gym, the rival shows up and battles again, then again on a route between the second and third gyms, on a route between the fourth and fifth gyms, and in some side area between the sixth and seventh gyms.
- A secondary rival, Wally, challenges you just before the third gym and as the Victory Road champion.

Later versions add the Red/Blue/Yellow rival arc as an alternative the game can pick, then rival arcs from the other mainline games as far as possible.

### Story arc content

The first version always uses the Emerald story arc: two eco-terrorist factions, Magma and Aqua, argue industrial progress versus environmental preservation. They awaken the Ground and Sea legendary Pokémon and lose control of them. Then a Flying legendary arrives from a tower landmark and pacifies them, and Magma and Aqua learn the error of their ways.

Next comes the Kanto arc: Team Rocket pushes human exploitation of nature versus harmony with it, and tragically creates a man-made legendary it cannot control. They treat Pokémon as commodities, tools, and weapons. Beats include a Mt. Moon heist, a Lavender Town tragedy, and a corporate takeover. Giovanni is the 8th gym leader, and the player dismantles Rocket.

Arcs from the other mainline games are added as far as possible.

### Subplots

- Each special location has a hardcoded, thin subplot.
- A number of city subplots are authored; each city randomly selects one.
- Subplots are modeled after Pokémon TV-show episodes. Ideally the player meets each gym leader during a subplot before battling them.
- Each subplot is linear. Each stage has ~2 NPC hints of the form "I heard something is happening at [city/location]", ~2 of the form "I saw something happening at [building]", plus story-state texts.

Initial subplot content comes from Emerald, for example:

- A man in sunglasses teaches you Cut.
- Mr. Briney's Wingull is stolen.
- You can't enter Devon Corp until you reclaim the Devon Goods from a tunnel.
- Answering questions earns the Old Rod.
- Granite Cave requires Flash.
- Delivering the Devon Goods to Captain Stern.

More follow from Red/Blue/Yellow, then other games. In the far future, more subplots are modeled after TV show episodes.

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

Why this order: if NPCs don't talk about subplots and story arcs, testers can't tell how to progress, which would cripple testing; and subplot babble requires city/route/location metadata anyway.

Stages, each leaving the game playable:

1. Divide the Emerald world map(s) into the 6x4 counties with hardcoded terrain type (mountainous, ocean, flat), biome, population, special location, story order, McGuffin keys and locks, and capital (Lilycove City). A key/lock is basically anywhere story advancement moves from one county to a non-adjacent county. The story-arc id is also hardcoded.
2. Hardcode the world tiles within each county: which are cities, routes, and special locations.
3. For each city and special location, hardcode its named buildings, subplot McGuffins, and city "theme"; hardcode which cities have gyms, the leader's name, and the gym type.
4. Implement the "random" NPC talking: some NPCs give story-arc state text, some give city/location subplot McGuffins, some talk about named buildings, the gym type, gym leader, or the city's theme. Remaining NPCs talk about their immediate vicinity or random text.
5. Abstract the subplots per city/location, add more, and randomize them; likewise abstract story-arc advancement (less visible, since world tiles keep hardcoded story indices).
6. With subplots no longer fixed, building locations are no longer fixed: randomize city layouts within a world tile (except special buildings).
7. Abstract McGuffins and make them randomizable; until then, shuffling is limited by McGuffins and encounter tables.
8. Implement randomized encounter tables by the route's biome.
9. Shuffle city locations within a county, and which special locations are in each county.
10. Shuffle the counties' story indices.
11. Randomize terrain types and biomes (the last randomizable map element).

Random Pokémon types and move sets can be implemented at virtually any point in this order.

The work for each step is broken into milestones 0-12 in `remaining_work.md`. Milestone 0 covers the hardcoded foundation (stages 1-4); the milestones then follow the order: shuffled NPC texts, shuffled subplots, non-fixed arc cutscenes, random types, encounters, moves, trainers, procedural buildings and terrain, shuffled McGuffins, randomized city placement, procedural story indices, procedural terrain and biomes.

### Emerald county assignment (stage 1)

Hoenn does not fit the 6x4 county grid cleanly, so for the hardcoded stage places are fudged into counties, with everything very slightly stretched apart. Since most content is hardcoded at this stage, the stretching does not matter, and once the world is fully generated there is no fakery left.

Each county holds at most one city, with the caves and other special locations spread around it. Each cell starts with the order in which the player first visits it in Emerald, in steps of 4 (leaving room for routes between), then its city, special locations, gym (with its badge order), rival battles and HM/McGuffin acquisitions. "Requires" names what unlocks a location. County (3,2) is deliberately empty: Emerald has nothing reasonable to put there, which is accepted for the hardcoded stage.

| Row | x=0 | x=1 | x=2 | x=3 | x=4 | x=5 |
| --- | --- | --- | --- | --- | --- | --- |
| y=0 | 40. Meteor Falls (requires Waterfall); Desert Underpass (requires Elite Four) | 36. Fallarbor; Glass Workshop | 42. Lavaridge + Gym 4; Mt. Chimney; Fiery Path; Jagged Pass (requires 52/Mt. Pyre); Magma Hideout (requires 52/Mt. Pyre); Mirage Tower; Desert Ruins | 44. Fortree + Gym 6; Rival Battle 4; HM Fly; Ancient Tomb (requires Flash) | 56. Lilycove; Scorched Slab; Ancient Ruins; Safari Zone; Dept. Store; Contest Hall; Rival Battle 5; Aqua Hideout | 60. Shoal Cave (requires Surf); Treasure Hunter's House |
| y=1 | 12. Rustboro + Gym 1; Devon Corporation; Rival Battle 2; HM Cut | 32. Verdanturf; Rusturf Tunnel (requires Cut); HM Strength | 28. Mauville + Gym 3; Rival Battle 3; Game Corner; HM Rock Smash; New Mauville (requires Surf); Trainer Hill (requires Elite Four) | 48. Berry Master; Weather Institute | 52. Mt. Pyre | 64. Mossdeep + Gym 7; Game Corner; Space Center; HM Dive |
| y=2 | 8. Petalburg + Gym 5; Petalburg Woods; Pretty Petal Flower Shop; HM Surf (requires Gym 5) | 4. Oldale; Rival Battle 1; Altering Cave (requires Elite Four) | 24. Cycling Road; Trick House; Pokémon Daycare | (empty) | 72. Sootopolis + Gym 8; HM Waterfall; Cave of Origin | 68. Ever Grande + Elite Four; Seafloor Cavern |
| y=3 | 16. Dewford + Gym 2; HM Flash; Granite Cave (requires Flash); Island Cave (requires Surf) | 0. Littleroot; Birch's Lab; Abandoned Ship (requires Surf and Dive) | 20. Slateport; Shipyard (requires Devon Goods); Museum | 84. Sealed Chamber; Southern Island (requires Eon Ticket) | 80. Pacifidlog; Sky Pillar | 76. Mirage Island; Battle Frontier (requires Elite Four); Artisan Cave (requires Elite Four) |

Lavaridge is 42 (between Meteor Falls and Fortree) to keep the existing numbering.

Terrain and biome per county. Hoenn is mostly rainforest, so it uses the warm, wet biome subset (humidity 1-4, the three hottest temperature columns). Temperature falls from the top row (hottest column) downward; the desert north of Mauville is faked with Thorn forest, and the smaller one at (0,0) with Savanna. Ocean counties get a biome too, for surf, dive and fishing encounters: the maximum-humidity row, from Tropical rainforest in the top row down to Temperate giant rainforest in the bottom two rows. Dewford's island is too small for its own biome, so (0,3) is Ocean.

| Row | x=0 | x=1 | x=2 | x=3 | x=4 | x=5 |
| --- | --- | --- | --- | --- | --- | --- |
| y=0 | Mountain, Savanna | Mountain, Savanna | Mountain, Thorn forest | Land, Tropical rainforest | Land, Tropical seasonal rainforest | Ocean, Tropical rainforest |
| y=1 | Land, Temperate woodland | Mountain, Savanna | Land, Mangrove swamp | Land, Mangrove swamp | Land, Tropical fresh-water swamp forest | Ocean, Tropical fresh-water swamp forest |
| y=2 | Land, Temperate woodland | Land, Temperate shrublands | Land, Temperate shrublands | Land, Temperate fresh-water swamp forest | Ocean, Temperate giant rainforest | Land (island), Temperate giant rainforest |
| y=3 | Ocean, Temperate giant rainforest | Land, Temperate fresh-water swamp forest | Land, Temperate grassland | Ocean, Temperate giant rainforest | Ocean, Temperate giant rainforest | Ocean, Temperate giant rainforest |

### McGuffins

A McGuffin key is whatever gates progress until a condition is met: an NPC blocking the way until a subplot reaches a certain state, an HM needed to pass an obstacle, or a special plot item such as the Poké Flute to wake a sleeping Pokémon.

### Story graph

The set of places the player can reach is an implicit "story graph", ideally never held in memory.

- Reachable area only ever grows. The player gains route entrances/exits and cities, and never loses access.
- Exception: the initial rival battle in the lab temporarily reduces the reachable area, but the battle restores the prior reachability, win or lose.
- The reachable area is bounded by McGuffin locks, and at least one of those has a McGuffin key inside the reachable area.
- Usual pattern: a lock blocking access to story index N has its key at story index N-1. As long as the reachable area only grows, this makes the story completable.
- Soft-locks are prevented by construction, not detected afterwards: the generation algorithm must make them impossible, with no seed-sweep or solver check. The rules above (key at N-1, only-growing reachability) are the means.
- Soft-lock protection: if the route from a city to a higher route-story-index is always locked on the city's subplot being resolved, and a subplot's resolution only requires McGuffins in the city or in adjacent routes with a lower route-story-index, accidental soft-locks become much less likely.

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

_None yet._

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
