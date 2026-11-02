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
 * Roles are not exclusive, so GetRegionTileFlags() returns a bit set. A special location inside a
 * city shares the city's tile (TILE_CITY | TILE_SPECIAL). A special location outside a city is
 * itself a route tile (TILE_ROUTE | TILE_SPECIAL), counted by GetCountyRouteTileCount(), so the
 * route reaches it without a gap. Which of the two a location is follows from its kind alone
 * (IsSpecialLocationKindInsideCity). A tile with no flags is empty (sea, mountain, unused).
 * Special locations that sit on a route in Emerald (Glass Workshop, Trick House) are named by their
 * route's MAPSEC; a special location inside a city without its own MAPSEC (Devon Corp) uses the
 * city's. Region cells never include the off-limits outer ring (see GetCountyRegionBounds).
 */

#include "global.h"
#include "region_county.h"

#define MAX_SPECIAL_LOCATIONS_PER_COUNTY 6

#define REGION_SALT_PITSTOP 7

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

// Two enums in one value space, so a kind says whether the location is inside a city (design.md,
// "Special locations"; the Emerald assignment is in Emerald.md).
enum RouteLocationKind
{
    ROUTE_LOCATION_CAVE,
    ROUTE_LOCATION_TUNNEL,
    ROUTE_LOCATION_VOLCANO,
    ROUTE_LOCATION_MOUNTAIN,
    ROUTE_LOCATION_FOREST,
    ROUTE_LOCATION_RUINS,
    ROUTE_LOCATION_TOWER,
    ROUTE_LOCATION_HIDEOUT,
    ROUTE_LOCATION_ISLAND,
    ROUTE_LOCATION_UNDERWATER,
    ROUTE_LOCATION_WRECK,
    ROUTE_LOCATION_PARK,
    ROUTE_LOCATION_FACILITY,
    ROUTE_LOCATION_ROADSIDE_BUILDING,
    ROUTE_LOCATION_ROUTE_FEATURE,
    NUM_ROUTE_LOCATION_KINDS,
};

enum TownLocationKind
{
    TOWN_LOCATION_MUSEUM = NUM_ROUTE_LOCATION_KINDS,
    TOWN_LOCATION_ENTERTAINMENT,
    TOWN_LOCATION_STORE,
    TOWN_LOCATION_HARBOR,
    TOWN_LOCATION_INSTITUTION,
    TOWN_LOCATION_SACRED_CAVE,
    NUM_LOCATION_KINDS,
};

// A property of the place, not of its kind (design.md, "Special locations"). Landmarks and detours are
// routed; only landmarks appear on the region-map screen. A pitstop is placed on a free route tile
// after routes are final, so routes never see it.
enum LocationClass
{
    LOCATION_CLASS_TOWN, // inside a city
    LOCATION_CLASS_LANDMARK,
    LOCATION_CLASS_DETOUR,
    LOCATION_CLASS_PITSTOP,
};

struct SpecialLocation
{
    struct RegionTilePos pos;
    u8 mapSec;
    u8 kind; // enum RouteLocationKind or enum TownLocationKind
    u8 locationClass; // enum LocationClass
};

static inline bool32 IsLocationOnMap(const struct SpecialLocation *location)
{
    return location->locationClass == LOCATION_CLASS_LANDMARK;
}

static inline bool32 IsSpecialLocationKindInsideCity(u32 kind)
{
    return kind >= NUM_ROUTE_LOCATION_KINDS;
}

// Bit set of enum RegionTileFlags for a region-map cell.
u32 GetRegionTileFlags(u32 x, u32 y);

// TRUE iff the county has population > 0.
bool32 DoesCountyHaveCity(u32 countyId);

// Region cell of the county's city. Asserts that the county has a city.
struct RegionTilePos GetCountyCityPos(u32 countyId);

// MAPSEC naming the county's city. Asserts that the county has a city.
u32 GetCountyCityMapSec(u32 countyId);

u32 GetCountySpecialLocationCount(u32 countyId);

// A pitstop's position is a free route tile, so asking for it plans the county's routes.
struct SpecialLocation GetCountySpecialLocation(u32 countyId, u32 index);

// enum LocationClass, without the position lookup (safe inside route planning).
u32 GetCountySpecialLocationClass(u32 countyId, u32 index);

// Includes the tiles of the county's outside-city special locations.
u32 GetCountyRouteTileCount(u32 countyId);

// Region cell of the index-th route tile of the county, in row-major order.
struct RegionTilePos GetCountyRouteTile(u32 countyId, u32 index);

#endif // GUARD_REGION_TILE_H
