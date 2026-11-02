#include "global.h"
#include "constants/region_map_sections.h"
#include "region_tile.h"
#include "region_route.h"

#include "data/region/emerald_tiles.h"

// Region cell (x, y) is grid tile (x + 1, y + 1); the off-limits ring is never stored.
static struct RegionTilePos LocalTileToPos(u32 countyId, u32 tile)
{
    struct RegionTilePos pos;

    AGB_ASSERT(tile < COUNTY_TILES_W * COUNTY_TILES_H);
    pos.x = COUNTY_X(countyId) * COUNTY_TILES_W + tile % COUNTY_TILES_W - 1;
    pos.y = COUNTY_Y(countyId) * COUNTY_TILES_H + tile / COUNTY_TILES_W - 1;
    return pos;
}

static const struct CountyTiles *GetCountyTiles(u32 countyId)
{
    AGB_ASSERT(countyId < NUM_COUNTIES);
    return &sEmeraldTiles[countyId];
}

static u32 LocalTileOfRegionCell(u32 x, u32 y)
{
    return (x + 1) % COUNTY_TILES_W + ((y + 1) % COUNTY_TILES_H) * COUNTY_TILES_W;
}

// The county's halves of its generated routes plus its outside-city locations. A road that
// crosses a city tile is absorbed by the city, so route tiles never carry TILE_CITY.
static u32 GetCountyRouteMask(u32 countyId)
{
    const struct CountyTiles *tiles = GetCountyTiles(countyId);
    u32 mask = 0;
    u32 neighbor, i;

    for (i = 0; i < tiles->numSpecials; i++)
    {
        if (!IsSpecialLocationKindInsideCity(tiles->specials[i].kind))
            mask |= 1u << tiles->specials[i].tile;
    }
    for (neighbor = 0; neighbor < NUM_COUNTIES; neighbor++)
    {
        if (!DoCountiesHaveRoute(countyId, neighbor))
            continue;
        for (i = 0; i < GetCountyRouteHalfTileCount(countyId, neighbor); i++)
        {
            struct RegionTilePos pos = GetCountyRouteHalfTile(countyId, neighbor, i);

            mask |= 1u << LocalTileOfRegionCell(pos.x, pos.y);
        }
    }
    if (tiles->cityTile != NO_TILE)
        mask &= ~(1u << tiles->cityTile);
    return mask;
}

u32 GetRegionTileFlags(u32 x, u32 y)
{
    u32 countyId = GetCountyFromRegionCell(x, y);
    u32 tile = LocalTileOfRegionCell(x, y);
    const struct CountyTiles *tiles = GetCountyTiles(countyId);
    u32 flags = 0;
    u32 i;

    if (tiles->cityTile == tile)
        flags |= TILE_CITY;
    for (i = 0; i < tiles->numSpecials; i++)
    {
        if (tiles->specials[i].tile == tile)
            flags |= TILE_SPECIAL;
    }
    if (GetCountyRouteMask(countyId) & (1u << tile))
        flags |= TILE_ROUTE;
    return flags;
}

bool32 DoesCountyHaveCity(u32 countyId)
{
    return GetCountyHeader(countyId)->population != 0;
}

struct RegionTilePos GetCountyCityPos(u32 countyId)
{
    AGB_ASSERT(DoesCountyHaveCity(countyId));
    return LocalTileToPos(countyId, GetCountyTiles(countyId)->cityTile);
}

u32 GetCountyCityMapSec(u32 countyId)
{
    AGB_ASSERT(DoesCountyHaveCity(countyId));
    return GetCountyTiles(countyId)->cityMapSec;
}

u32 GetCountySpecialLocationCount(u32 countyId)
{
    return GetCountyTiles(countyId)->numSpecials;
}

struct SpecialLocation GetCountySpecialLocation(u32 countyId, u32 index)
{
    const struct CountyTiles *tiles = GetCountyTiles(countyId);
    struct SpecialLocation location;

    AGB_ASSERT(index < tiles->numSpecials);
    location.pos = LocalTileToPos(countyId, tiles->specials[index].tile);
    location.mapSec = tiles->specials[index].mapSec;
    location.kind = tiles->specials[index].kind;
    return location;
}

u32 GetCountyRouteTileCount(u32 countyId)
{
    return __builtin_popcount(GetCountyRouteMask(countyId));
}

struct RegionTilePos GetCountyRouteTile(u32 countyId, u32 index)
{
    u32 mask = GetCountyRouteMask(countyId);
    u32 tile;

    AGB_ASSERT(index < __builtin_popcount(mask));
    for (tile = 0; tile < COUNTY_TILES_W * COUNTY_TILES_H; tile++)
    {
        if ((mask & (1u << tile)) && index-- == 0)
            break;
    }
    return LocalTileToPos(countyId, tile);
}
