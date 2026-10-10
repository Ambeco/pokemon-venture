# Pokémon Venture — Design

Current state of the design for the mod. Organized by topic; describes what the design *is*, not how it evolved (history lives in `notes.md`, open items in `remaining_work.md`).

## Overview

The mod procedurally generates a Pokémon world rather than using a fixed map. (The user describes the idea as "overly ambitious"; the full concept is still being collected.)

The goal of the world generation below is a game world map with cities, routes, and special locations that mirrors what a normal mainline Pokémon game looks like.

## World generation

- The world map is a grid of roughly 6x4 "counties".
- Each county is procedurally assigned a terrain type, a biome, a population, and a special location.
- A "story order" over the counties is generated, which sometimes designates McGuffin "keys" and the "lock" counties they open.
- One county's city is picked as the "capital".

### County layout

- Each county is ~5x5 "world tiles". A world tile is one GBA tile on the world map; the map looks to be ~30x20 tiles today (user's eyeball estimate), so the county grid must be adjusted to match the actual world-map tile grid.
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

## Biomes

- Biomes form a 5x5 grid: one axis is precipitation, the other is temperature.
- Each game randomly selects one of the four 4x4 subsets of that grid and uses those 16 biomes. This keeps biomes coherent at small scale and makes separate games feel distinct.

## Data representation and determinism

- GBA memory is scarce, so per-county data is bitpacked: biome type in 4 bits (16 biomes), population in 2 bits (roughly logarithmic).
- Map generation uses a PRNG seeded from the player ID. The generated world therefore does not need to be stored in the save file, and possibly not even in RAM (it can be regenerated on demand). Every player ID yields a completely different map.

## Procedural species types and wild encounters

Species types and move pools are procedurally generated, and wild Pokémon on a route should match the route's biome, like a real game.

### Biome type affinity

Each biome has a hardcoded table of how often each of the 18 Pokémon types occurs there. Possible encoding: 3 bits per type on a log scale, 54 bits (7 bytes) per biome.

### Species <-> biome mapping

A species' biome must be findable quickly without walking the generated map data, and so must the species list of a biome. This uses a two-way coprime-stride (modular multiplicative) mapping between species and biomes:

- A stride coprime to the species count, and its modular inverse, are hardcoded.
- Species to biome: `(baseSpeciesIndex * stride + playerID) mod 16`.
- The species' two types are then chosen by random selection seeded from species + player ID, weighted by that biome's type affinity.
- Biome to species: compute the first species of the biome via the inverse, then the rest follow at a fixed index distance from each other (derived from the inverse).
- Each biome ends up with ~63 species.
- A Pokémon whose trainer ID differs from the player's may use a different 16 of the 25 biomes, so the mapping depends on which 4x4 subset applies to that Pokémon's trainer ID.

### Route encounter selection

Many routes share a biome but sit at very different levels. To pick a route's species, iterate over the biome's ~63 species and:

- filter out species whose evolve-to/evolve-from levels make them unlikely at the route's level,
- filter out starters, legendaries, and pseudo-legendaries,
- randomly filter out some more, seeded from player ID.

Each species has a "commonness" score that sets the encounter ratios among the survivors.

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

## Base codebase

- Built on [rh-hideout/pokeemerald-expansion](https://github.com/rh-hideout/pokeemerald-expansion) 1.16.1 (itself built on pret/pokeemerald). Remote `upstream` points there; `origin` is `Ambeco/pokemon-venture`.
- Feature toggles live in `include/config/*.h` (`battle.h`, `pokemon.h`, `overworld.h`, `general.h`, `item.h`, etc.); most mod-wide behavior switches should be made there before touching code.
- Expansion supports building FRLG as well as Emerald (`docs/tutorials/how_to_frlg.md`).

## Alternatives considered

_None yet._
