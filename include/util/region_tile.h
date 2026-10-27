#ifndef GUARD_REGION_TILE_H
#define GUARD_REGION_TILE_H

/**
 * Layer 5 region data: what each region tile (one cell of the 28x15 region map) holds.
 *
 * A county is 5x4 tiles (see region_county.h). Its city, special locations and route tiles are
 * read through the accessors below, which take county ids and region-map cells, never a table
 * index. Stage 2 (hardcoded Emerald) answers from a per-county ROM table; later the same functions
 * answer from per-county hashes, so callers do not change. Tiles are always asked about through
 * their county, because a county is generated from its own header and its neighbors' headers only.
 *
 * Roles are not exclusive: a special location inside a city shares the city's tile, so
 * GetRegionTileFlags() returns a bit set. A tile with no flags is empty (sea, mountain, unused).
 * Special locations that sit on a route in Emerald (Glass Workshop, Trick House) are named by their
 * route's MAPSEC; a special location inside a city without its own MAPSEC (Devon Corp) uses the
 * city's. Region cells never include the off-limits outer ring (see GetCountyRegionBounds).
 */

#include "global.h"
#include "region_county.h"

#define MAX_SPECIAL_LOCATIONS_PER_COUNTY 6

enum RegionTileFlags
{
    TILE_CITY    = 1 << 0,
    TILE_SPECIAL = 1 << 1,
    TILE_ROUTE   = 1 << 2,
};

struct RegionTilePos
{
    u8 x;
    u8 y;
};

struct SpecialLocation
{
    struct RegionTilePos pos;
    u8 mapSec;
};

// Bit set of enum RegionTileFlags for a region-map cell.
u32 GetRegionTileFlags(u32 x, u32 y);

// TRUE iff the county has population > 0.
bool32 DoesCountyHaveCity(u32 countyId);

// Region cell of the county's city. Asserts that the county has a city.
struct RegionTilePos GetCountyCityPos(u32 countyId);

// MAPSEC naming the county's city. Asserts that the county has a city.
u32 GetCountyCityMapSec(u32 countyId);

u32 GetCountySpecialLocationCount(u32 countyId);

struct SpecialLocation GetCountySpecialLocation(u32 countyId, u32 index);

u32 GetCountyRouteTileCount(u32 countyId);

// Region cell of the index-th route tile of the county, in row-major order.
struct RegionTilePos GetCountyRouteTile(u32 countyId, u32 index);

#endif // GUARD_REGION_TILE_H
