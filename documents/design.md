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
5. Each county: the details of its own world tiles (including exact route placement) and its route-story-indices, from layers 1-4. Counties are independent and never need an adjacent county's route-story-indices. Counties may have sub-layers like the world map.
6. Field maps: generated from the layers above, without referring to adjacent field maps.
7. Further layers continue the same pattern.

This forces single-pass generation with no recursion or transitive operations, and keeps memory use minimal. Any new rule must be checked against it: a rule that needs data from a later layer, or from a sibling's same-layer output, breaks the design.

## World generation

- The world map is a grid of 5 columns x 4 rows of "counties" (matching roughly the 5 columns and 4 rows of cities in Kanto).
- Each county is procedurally assigned a terrain type, a biome, a population, and a special location.
- A "story order" over the counties is generated, which sometimes designates McGuffin "keys" and the "lock" counties they open.
- One county's city is picked as the "capital".

### County layout

- Each county is 6x4 "world tiles". A world tile is one cell of the Hoenn region map, which is 28x15 cells (`MAP_WIDTH`/`MAP_HEIGHT` in `src/field/region_map.c`; cell contents in `src/data/region_map/region_map_layout.h`).
- 5x4 counties of 6x4 tiles is 30x16, so the first tile column of the first county column, the last tile column of the last county column, and the last tile row of the last county row are off limits, leaving 28x15.
- If a county's population is above zero, it places a settlement at a random position within its middle 3x3 world tiles. Population is 0-3 (2 bits); population 0 never generates a city.
- Every county always has a special location, whether or not it has a city.
- With a city, the special location is placed either in or adjacent to the city (50%), or on a random non-adjacent world tile (50%).
- Without a city (population 0), the special location is necessarily not in a city, and it is what routes attach to.

### Settlement types

Population selects one of three generation algorithms:

- 1 = village: buildings never touch each other, and paths run everywhere intrinsically.
- 2 = town: a few specific paths run through it; buildings often touch each other, and the layout is not very organized.
- 3 = city: paths are mostly straight horizontal or vertical lines, and buildings mostly fill the grid.

### Special locations

Special locations are either inside a city or outside one (mirroring mainline games, with Red/Blue as the reference).

- Outside cities: Viridian Forest, Mt. Moon, Bill's House, Dark Tunnel (Rock Tunnel), Power Plant, S.S. Anne, Safari Zone, Whirlpool Islands (Seafoam Islands), Victory Road tunnel.
- Inside cities: Museum, burgled house, Pokémon Tower, Game Corner, Celadon Mall, Mansion, Silph Co. tower.

The design is intended to be able, in principle, to generate the Red/Blue world map. How to represent the underground tunnels around Saffron (the Underground Path, not Diglett's Cave) is TBD.

### Route generation

Routes are placed after all cities and special locations are placed.

- If the special location is outside the city, a route always connects it to the city.
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

## Geography

County-level terrain is laid out like a typical Pokémon region:

- The outer ring of counties is bordered by mountains on 1-2 sides and by ocean on 2-3 sides: first mark a continuous stretch of the outer ring as mountainous, and most of the rest as ocean.
- A stretch of mountain and a stretch of ocean each extend toward the middle.
- Cities and special locations placed in the ocean automatically form islands.

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
- A given region/world map is limited to one of the four 4x4 subsets of that grid, chosen randomly per game: 16 biomes. This keeps biomes coherent at small scale and makes separate games feel distinct.
- Species associated with the other 9 biomes cannot be encountered in the wild in that playthrough, but NPCs can still have and trade them as normal.
- Players can therefore expect to encounter about 346 of the ~541 evolutionary lines in the wild (16/25). Pokémon storage has 420 slots (14 boxes of 30), so this is already a lot.

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

## Procedural species types and wild encounters

Species types and move pools are procedurally generated, and wild Pokémon on a route should match the route's biome, like a real game.

### Biome type affinity

Each biome has a hardcoded table of how often each of the 18 Pokémon types occurs there. Possible encoding: 3 bits per type on a log scale, 54 bits (7 bytes) per biome.

### Species <-> biome mapping

A species' biome must be findable quickly without walking the generated map data, and so must the species list of a biome. This uses a two-way coprime-stride (modular multiplicative) mapping between species and biomes:

- The mapping operates on the ~541 evolutionary lines (via each species' base evolution), and associates every line with one of all 25 biomes, not just a game's 16.
- A stride coprime to 25 (not divisible by 5), and its modular inverse, are hardcoded.
- Species to biome: `(baseSpeciesIndex * stride + playerID) mod 25`.
- The species' two types are then chosen by random selection seeded from species + player ID, weighted by that biome's type affinity.
- Biome to species: compute the first species of the biome via the inverse, then the rest follow at a fixed index distance from each other (derived from the inverse).
- Each biome ends up with ~22 evolutionary lines.
- A Pokémon whose trainer ID differs from the player's maps through that trainer ID, so it may land in a different biome and get different types.

### Species metadata

Per-species fields used for encounters and trading (field widths are tentative):

- Minimum requirements: flags for a required habitat. Fish-looking species require water (surfing or fishing) and are almost certainly Water type; land-looking species such as Pikachu (a mouse) require land. These two flags may be the only ones.
- Habitat bias: flags biasing a species toward or away from habitats. They are based on the species' image, never its type: types are randomized, each biome should have a general mix of types, and a habitat such as volcano should be neither only fire nor the only home of fire Pokémon. Example: Pikachu looks like a mouse, so it prefers tunnel, cave, valley, forest and ruins, and avoids beach, clouds, reef, ocean, snow and tundra.
- Domestication (2-3 bits): 0 is fully wild (Pinsir, Kangaskhan, Tauros, Dratini, legendaries), biased toward the map edges and Safari Zones; the maximum is fully domestic (Hitmonlee, Hitmonchan, Meowth, Porygon).
- Base evolution: a reference to the species' base form, so that all evolutions share the base's type generation.
- Normal level range: a 3-bit enum for the low end (2, 7, 10, 17, 25, 34, 43, 55); the high end comes from the evolution data.
- Commonness (2 bits): 0 = legendary (never wild; NPCs never have or trade them); 1 = baby, trade-evolved, item-evolved and fossil Pokémon (never wild; NPCs rarely have them, but trade normally); 2 = starters and other super-rare Pokémon such as Chansey and Tauros (super rare in the wild; NPCs rarely have them, but trade regularly); 3 = normal (encounter ratios based on catch rate; NPCs regularly have and trade them).

### Route encounter selection

The species-to-biome stride mapping only determines types. A route's encounters are built separately from the evolutionary lines correlated with the route's biome (~22 per biome). Many routes share a biome but sit at very different levels, so the species are filtered, then weighted:

- by commonness: legendaries, babies, fossils, and trade/item-evolved Pokémon are never wild, and starters and other super-rare Pokémon are very rare,
- by domestication, relative to how close the route is to the map edge or a Safari Zone,
- by habitat: minimum requirements (e.g. water for fish) and image-based habitat biases,
- by level: species whose evolve-to/evolve-from levels make them unlikely at the route's level are dropped,
- by time of day and weather,
- by a random filter that drops some of the rest.

Roughly half of the ~22 lines are expected to be filtered out, leaving ~11. The survivors' encounter ratios are proportional to their catch rates.

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

### Move pools

Moves are generated procedurally per species, from per-species anatomy and per-move requirements.

- Each species has a bitfield of appendages (e.g. mouth). Each move has a field of required appendages and an associated stat. "Roar" needs a mouth; "Tackle" needs nothing.
- The ~1000 moves are roughly sorted by power, with ~20 moves associated with each level.
- Learn levels are generated per species (a procedurally generated list of levels at which it learns a move). This guarantees no Pokémon goes long without learning something, which a per-level chance would not.
- At a learn level, the candidate range is wider than one level's ~20 moves, and exactly one move is guaranteed to be picked.
- Candidates are filtered out if the species lacks the required appendages. Moves whose type does not match the species' types are dropped with high probability, as are moves whose associated stat does not match the species' stat ratios.
- Levels 1 and 6 are always learn levels (at least one move each).
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

### Subplots

- Each special location has a hardcoded, thin subplot.
- A number of city subplots are authored; each city randomly selects one.
- Subplots are modeled after Pokémon TV-show episodes. Ideally the player meets each gym leader during a subplot before battling them.
- Each subplot is linear. Each stage has ~2 NPC hints of the form "I heard something is happening at [city/location]", ~2 of the form "I saw something happening at [building]", plus story-state texts.

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

### Type icons in names

Players strongly associate Pokémon with their types, so random types are a mental hurdle. To ease it, whenever a Pokémon's name is displayed it is prefixed with icons for its types, e.g. "You encountered a wild 🐉❄️Rattata!".

## Base codebase

- Built on [rh-hideout/pokeemerald-expansion](https://github.com/rh-hideout/pokeemerald-expansion) 1.16.1 (itself built on pret/pokeemerald). Remote `upstream` points there; `origin` is `Ambeco/pokemon-venture`.
- Feature toggles live in `include/config/*.h` (`battle.h`, `pokemon.h`, `overworld.h`, `general.h`, `item.h`, etc.); most mod-wide behavior switches should be made there before touching code.
- Expansion already contains ~1000 Pokémon and ~1000 moves, plus mechanics such as Mega Evolution and Gigantamax. How these fit the mod is undecided; they could make fun mechanics.
- Expansion supports building FRLG as well as Emerald (`docs/tutorials/how_to_frlg.md`).

## Development approach: hardcode first, then randomize

The world starts as a hardcoded Pokémon Emerald world and is progressively abstracted and randomized, rather than building the generator first. Hardcoded content exercises the same metadata the generator will later produce, and keeps the story testable throughout.

Why this order: if NPCs don't talk about subplots and story arcs, testers can't tell how to progress, which would cripple testing; and subplot babble requires city/route/location metadata anyway.

Stages, each leaving the game playable:

1. Divide the Emerald world map(s) into the 5x4 counties with hardcoded terrain type (mountainous, ocean, flat), biome, population, special location, story order, McGuffin keys and locks, and capital (Lilycove City). A key/lock is basically anywhere story advancement moves from one county to a non-adjacent county. The story-arc id is also hardcoded.
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

### McGuffins

A McGuffin key is whatever gates progress until a condition is met: an NPC blocking the way until a subplot reaches a certain state, an HM needed to pass an obstacle, or a special plot item such as the Poké Flute to wake a sleeping Pokémon.

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
- 470 exceeds the 420 box slots, and even 346 already seems like a lot to collect.
- Needs a second mapping and more bookkeeping for the species-to-biome lookup.

Not planned; may be revisited depending on player feedback.
