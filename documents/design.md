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

## Base codebase

- Built on [rh-hideout/pokeemerald-expansion](https://github.com/rh-hideout/pokeemerald-expansion) 1.16.1 (itself built on pret/pokeemerald). Remote `upstream` points there; `origin` is `Ambeco/pokemon-venture`.
- Feature toggles live in `include/config/*.h` (`battle.h`, `pokemon.h`, `overworld.h`, `general.h`, `item.h`, etc.); most mod-wide behavior switches should be made there before touching code.
- Expansion supports building FRLG as well as Emerald (`docs/tutorials/how_to_frlg.md`).

## Alternatives considered

_None yet._
