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
- If a county's population is above a threshold, it places a city at a random position within its middle 3x3 world tiles.
- Every county always has a special location, whether or not it has a city.
- With a city, the special location is placed either in or adjacent to the city (50%), or on a random non-adjacent world tile (50%).
- Without a city (population below the threshold), the special location is necessarily not in a city, and it is what routes attach to.

### Special locations

Special locations are either inside a city or outside one (mirroring mainline games, with Red/Blue as the reference).

- Outside cities: Viridian Forest, Mt. Moon, Bill's House, Dark Tunnel (Diglett's Cave), Power Plant, S.S. Anne, Safari Zone, Whirl Islands (Seafoam Islands), Victory Road tunnel.
- Inside cities: Museum, burgled house, Pokémon Tower, Game Corner, Celadon Mall, Mansion, Silph Co. tower.

### Route generation

Routes are placed after all cities and special locations are placed.

- If the special location is outside the city, a route always connects it to the city.
- If an adjacent county has an adjacent story index, a route is made: the shortest route connecting either of that county's locations to either of ours, biased toward cities.
- If an adjacent county is the capital, there is always a route from one of our locations to the capital city.
- If no adjacent county is the capital or has an adjacent story index, the county routes to whichever adjacent county has the highest story index.
- Counties only ever route to adjacent counties.

## Base codebase

- Built on [rh-hideout/pokeemerald-expansion](https://github.com/rh-hideout/pokeemerald-expansion) 1.16.1 (itself built on pret/pokeemerald). Remote `upstream` points there; `origin` is `Ambeco/pokemon-venture`.
- Feature toggles live in `include/config/*.h` (`battle.h`, `pokemon.h`, `overworld.h`, `general.h`, `item.h`, etc.); most mod-wide behavior switches should be made there before touching code.
- Expansion supports building FRLG as well as Emerald (`docs/tutorials/how_to_frlg.md`).

## Alternatives considered

_None yet._
