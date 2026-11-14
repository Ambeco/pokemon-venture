# Region data representation design

How the generated region is stored and read: determinism and seeds, the county headers, region tiles, city data and the region dump. Everything is read through accessors keyed by county, so the hardcoded Emerald stage and the generator can answer the same calls.

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
- `GetGymSlot(ordinal)` is a planner input: the time block and step that give gym k. Slots strictly increase with the ordinal; a step-1 slot is a callback whose gym county lies in an earlier block, and any other slot's county is the block. Emerald's Surf exception is encoded here (it is an Emerald-only exception to the Surf rule in "County-story-index assignment" (design_region.md)): gyms 4 and 5 share block 11, Lavaridge's own as step 0 and Petalburg's as the step-1 callback, and the block's unlock leads to a land county, not an ocean-lock county. Lavaridge's gym cannot move earlier, because the walkthrough opens the Weather Institute county from Lavaridge's block.
- Tests over the accessors are written as seed-sweep-ready invariants (`test/region_city.c`). The test runner has no real trainer data, so the Emerald-only check that each gym leader's party matches the gym type was run once offline. It must not become a generator invariant: mainline games have exceptions (FireRed's Elite Four has at least 40% off-type).

### Region dump

`DumpRegion()` / `PrintRegionToLog()` (`include/util/region_dump.h`, debug menu: Utilities, "Print region (log)") print an ASCII map of the 28x15 cells (city, special location, route, empty; county separators) and three lines per county (header fields with capital/Elite Four/reserved; the city's name, MAPSEC, tile, theme, building roles, gym ordinal, type and leader; route count and special locations with their kinds) through the accessors, so it shows whatever the generator later produces. After the county lines it prints one line per route lock (tile, gated county and its story index, and the planner key bound to it), one per Enclave (unlock index and map flag), and one per main-story-index through the Elite Four block (the vanilla list's items, with the callback host at step 1, or the step-3 unlock key). Lines are emitted one at a time to keep stack use small.

### Regions and multiple regions

Gold/Silver and FireRed/LeafGreen have several region maps the player moves between. This mod is built for one region now but keeps the seams so a later version can host more: nothing region-shaped may assume a single instance in new code.

- Per region: the region header, the 24 county headers, the tile data, the county var slices (sized for one region today) and a region seed. A region's seed would be `IdentityRng(identitySeed, SALT_REGION, regionIndex)` or similar, never the identity seed itself.
- Per identity, shared by all regions: the identity seed, species types and move pools (which must agree across trades and regions).
- Not implemented until V2: travel between regions, a region index in the save, per-region var slices.

## Alternatives considered

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
