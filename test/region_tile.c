#include "global.h"
#include "constants/region_map_sections.h"
#include "test/test.h"
#include "region_tile.h"
#include "region_route.h"

// Invariants are written over the accessors so they become seed-sweep tests once the generator exists.

#define EMPTY_COUNTY COUNTY_ID(3, 2)

static bool32 IsPosInCounty(u32 countyId, struct RegionTilePos pos)
{
    u32 left, top, right, bottom;

    GetCountyRegionBounds(countyId, &left, &top, &right, &bottom);
    return pos.x >= left && pos.x <= right && pos.y >= top && pos.y <= bottom;
}

TEST("RegionTile: A county has a city iff its population is above zero")
{
    u32 id;

    for (id = 0; id < NUM_COUNTIES; id++)
    {
        u32 left, top, right, bottom, x, y, cities = 0;

        EXPECT_EQ(DoesCountyHaveCity(id), GetCountyHeader(id)->population != 0);
        GetCountyRegionBounds(id, &left, &top, &right, &bottom);
        for (y = top; y <= bottom; y++)
            for (x = left; x <= right; x++)
                cities += (GetRegionTileFlags(x, y) & TILE_CITY) != 0;
        EXPECT_EQ(cities, DoesCountyHaveCity(id) ? 1 : 0);
    }
}

TEST("RegionTile: A city is not in its county's bottom row or rightmost column unless the county is on that edge")
{
    u32 id;

    for (id = 0; id < NUM_COUNTIES; id++)
    {
        u32 left, top, right, bottom;
        struct RegionTilePos pos;

        if (!DoesCountyHaveCity(id))
            continue;
        pos = GetCountyCityPos(id);
        GetCountyRegionBounds(id, &left, &top, &right, &bottom);
        if (COUNTY_Y(id) != COUNTY_ROWS - 1)
            EXPECT_LT(pos.y, bottom);
        if (COUNTY_X(id) != COUNTY_COLS - 1)
            EXPECT_LT(pos.x, right);
    }
}

TEST("RegionTile: City, special-location and route tiles lie inside their county")
{
    u32 id, i;

    for (id = 0; id < NUM_COUNTIES; id++)
    {
        if (DoesCountyHaveCity(id))
            EXPECT_EQ(IsPosInCounty(id, GetCountyCityPos(id)), TRUE);
        for (i = 0; i < GetCountySpecialLocationCount(id); i++)
            EXPECT_EQ(IsPosInCounty(id, GetCountySpecialLocation(id, i).pos), TRUE);
        for (i = 0; i < GetCountyRouteTileCount(id); i++)
            EXPECT_EQ(IsPosInCounty(id, GetCountyRouteTile(id, i)), TRUE);
    }
}

TEST("RegionTile: Every county has a special location except the deliberately empty one")
{
    u32 id;

    for (id = 0; id < NUM_COUNTIES; id++)
    {
        if (id == EMPTY_COUNTY)
        {
            EXPECT_EQ(GetCountySpecialLocationCount(id), 0);
            EXPECT_EQ(GetCountyRouteTileCount(id), 0);
        }
        else
        {
            EXPECT_GT(GetCountySpecialLocationCount(id), 0);
            EXPECT_LE(GetCountySpecialLocationCount(id), MAX_SPECIAL_LOCATIONS_PER_COUNTY);
        }
    }
}

TEST("RegionTile: Special locations are distinct tiles, and a route tile is shared only with an outside-city location")
{
    u32 id, i, j;

    for (id = 0; id < NUM_COUNTIES; id++)
    {
        for (i = 0; i < GetCountySpecialLocationCount(id); i++)
        {
            struct SpecialLocation a = GetCountySpecialLocation(id, i);
            u32 flags = GetRegionTileFlags(a.pos.x, a.pos.y);

            EXPECT(flags & TILE_SPECIAL);
            EXPECT_EQ((flags & TILE_ROUTE) != 0, !IsSpecialLocationKindInsideCity(a.kind) && a.locationClass != LOCATION_CLASS_ENCLAVE);
            for (j = i + 1; j < GetCountySpecialLocationCount(id); j++)
            {
                struct SpecialLocation b = GetCountySpecialLocation(id, j);
                EXPECT(a.pos.x != b.pos.x || a.pos.y != b.pos.y);
            }
        }
        for (i = 0; i < GetCountyRouteTileCount(id); i++)
        {
            struct RegionTilePos pos = GetCountyRouteTile(id, i);
            u32 flags = GetRegionTileFlags(pos.x, pos.y);

            EXPECT(flags == TILE_ROUTE || flags == (TILE_ROUTE | TILE_SPECIAL));
        }
    }
}

TEST("RegionTile: Route tiles are listed in row-major order without repeats")
{
    u32 id, i;

    for (id = 0; id < NUM_COUNTIES; id++)
    {
        for (i = 1; i < GetCountyRouteTileCount(id); i++)
        {
            struct RegionTilePos prev = GetCountyRouteTile(id, i - 1);
            struct RegionTilePos cur = GetCountyRouteTile(id, i);
            EXPECT(cur.y > prev.y || (cur.y == prev.y && cur.x > prev.x));
        }
    }
}

TEST("RegionTile: Whole-region tile counts match the per-county accessors")
{
    u32 x, y, id, cities = 0, specials = 0, routes = 0;
    u32 expectCities = 0, expectSpecials = 0, expectRoutes = 0;

    for (y = 0; y < REGION_H; y++)
    {
        for (x = 0; x < REGION_W; x++)
        {
            u32 flags = GetRegionTileFlags(x, y);
            cities += (flags & TILE_CITY) != 0;
            specials += (flags & TILE_SPECIAL) != 0;
            routes += (flags & TILE_ROUTE) != 0;
        }
    }
    for (id = 0; id < NUM_COUNTIES; id++)
    {
        expectCities += DoesCountyHaveCity(id);
        expectSpecials += GetCountySpecialLocationCount(id);
        expectRoutes += GetCountyRouteTileCount(id);
    }
    EXPECT_EQ(cities, expectCities);
    EXPECT_EQ(specials, expectSpecials);
    EXPECT_EQ(routes, expectRoutes);
}

TEST("RegionTile: Cities and special locations are named")
{
    u32 id, i;

    for (id = 0; id < NUM_COUNTIES; id++)
    {
        if (DoesCountyHaveCity(id))
            EXPECT_NE(GetCountyCityMapSec(id), MAPSEC_NONE);
        for (i = 0; i < GetCountySpecialLocationCount(id); i++)
            EXPECT_NE(GetCountySpecialLocation(id, i).mapSec, MAPSEC_NONE);
    }
}

TEST("RegionTile: Emerald cities sit in their Emerald.md counties")
{
    EXPECT_EQ(GetCountyCityMapSec(COUNTY_ID(0, 1)), MAPSEC_RUSTBORO_CITY);
    EXPECT_EQ(GetCountyCityMapSec(COUNTY_ID(4, 0)), MAPSEC_LILYCOVE_CITY);
    EXPECT_EQ(GetCountyCityMapSec(COUNTY_ID(1, 3)), MAPSEC_LITTLEROOT_TOWN);
    EXPECT_EQ(GetCountySpecialLocationCount(COUNTY_ID(2, 0)), 6);
}

TEST("RegionTile: The generated route halves are route tiles, and no route tile is a city tile")
{
    u32 a, b, i;

    for (a = 0; a < NUM_COUNTIES; a++)
    {
        for (b = 0; b < NUM_COUNTIES; b++)
        {
            if (!DoCountiesHaveRoute(a, b))
                continue;
            for (i = 0; i < GetCountyRouteHalfTileCount(a, b); i++)
            {
                struct RegionTilePos pos = GetCountyRouteHalfTile(a, b, i);
                u32 flags = GetRegionTileFlags(pos.x, pos.y);

                EXPECT_EQ((flags & TILE_ROUTE) != 0, (flags & TILE_CITY) == 0);
            }
        }
    }
}

TEST("RegionTile: A pitstop sits on a free route tile; every county with locations has a city or a routed place")
{
    u32 id, i;

    for (id = 0; id < NUM_COUNTIES; id++)
    {
        u32 routed = DoesCountyHaveCity(id);
        u32 enclaves = 0;

        for (i = 0; i < GetCountySpecialLocationCount(id); i++)
        {
            struct SpecialLocation s = GetCountySpecialLocation(id, i);

            routed += s.locationClass != LOCATION_CLASS_PITSTOP && s.locationClass != LOCATION_CLASS_ENCLAVE;
            enclaves += s.locationClass == LOCATION_CLASS_ENCLAVE;
            EXPECT_EQ(IsSpecialLocationKindInsideCity(s.kind), s.locationClass == LOCATION_CLASS_TOWN);
            if (s.locationClass == LOCATION_CLASS_PITSTOP)
                EXPECT_EQ(GetRegionTileFlags(s.pos.x, s.pos.y), TILE_ROUTE | TILE_SPECIAL);
            if (s.locationClass == LOCATION_CLASS_ENCLAVE)
                EXPECT_EQ(GetRegionTileFlags(s.pos.x, s.pos.y), TILE_SPECIAL);
        }
        if (IsCountyEnclaveOnly(id))
        {
            EXPECT_EQ(routed, 0);
            EXPECT_EQ(enclaves, GetCountySpecialLocationCount(id));
            EXPECT_EQ(GetCountyRouteTileCount(id), 0);
        }
        else if (GetCountySpecialLocationCount(id) != 0)
            EXPECT_GT(routed, 0); // the cityless-county pitstop fallback is not implemented
    }
}

TEST("RegionTile: Only landmarks and flagged enclaves are on the map")
{
    EXPECT_EQ(IsLocationOnMap(&(struct SpecialLocation){.locationClass = LOCATION_CLASS_LANDMARK}), TRUE);
    EXPECT_EQ(IsLocationOnMap(&(struct SpecialLocation){.locationClass = LOCATION_CLASS_DETOUR}), FALSE);
    EXPECT_EQ(IsLocationOnMap(&(struct SpecialLocation){.locationClass = LOCATION_CLASS_PITSTOP}), FALSE);
    EXPECT_EQ(IsLocationOnMap(&(struct SpecialLocation){.locationClass = LOCATION_CLASS_ENCLAVE}), FALSE);
    EXPECT_EQ(IsLocationOnMap(&(struct SpecialLocation){.locationClass = LOCATION_CLASS_ENCLAVE, .enclaveOnMap = TRUE}), TRUE);
}
