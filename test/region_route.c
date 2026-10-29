#include "global.h"
#include "test/test.h"
#include "region_route.h"

// Properties are written over the county accessors so they become seed-sweep tests later.

static bool32 SamePos(struct RegionTilePos a, struct RegionTilePos b)
{
    return a.x == b.x && a.y == b.y;
}

static u32 Distance(struct RegionTilePos a, struct RegionTilePos b)
{
    return abs(a.x - b.x) + abs(a.y - b.y);
}

static bool32 IsInEitherCounty(u32 countyA, u32 countyB, struct RegionTilePos pos)
{
    u32 id = GetCountyFromRegionCell(pos.x, pos.y);

    return id == countyA || id == countyB;
}

static struct RegionTilePos LocationOf(u32 countyId, u32 index)
{
    if (DoesCountyHaveCity(countyId))
    {
        if (index == 0)
            return GetCountyCityPos(countyId);
        index--;
    }
    return GetCountySpecialLocation(countyId, index).pos;
}

static u32 LocationCountOf(u32 countyId)
{
    return DoesCountyHaveCity(countyId) + GetCountySpecialLocationCount(countyId);
}

// Runs the following statement for each ordered pair of adjacent counties that both have a location.
#define FOR_EACH_ROUTE_PAIR(a, b, posA, posB) \
    for (a = 0; a < NUM_COUNTIES; a++) \
        for (b = 0; b < NUM_COUNTIES; b++) \
            if (AreCountiesAdjacent(a, b) && GetCountyPairRouteEndpoints(a, b, &posA, &posB))

TEST("RegionRoute: Endpoints are the same whichever county asks")
{
    u32 a, b;
    struct RegionTilePos posA, posB, rev1, rev2;

    FOR_EACH_ROUTE_PAIR(a, b, posA, posB)
    {
        ASSUME(GetCountyPairRouteEndpoints(b, a, &rev1, &rev2));
        EXPECT(SamePos(posA, rev2));
        EXPECT(SamePos(posB, rev1));
        EXPECT_EQ(GetCountyFromRegionCell(posA.x, posA.y), a);
        EXPECT_EQ(GetCountyFromRegionCell(posB.x, posB.y), b);
    }
}

TEST("RegionRoute: Endpoints are a closest pair of locations")
{
    u32 a, b, i, j;
    struct RegionTilePos posA, posB;

    FOR_EACH_ROUTE_PAIR(a, b, posA, posB)
    {
        for (i = 0; i < LocationCountOf(a); i++)
            for (j = 0; j < LocationCountOf(b); j++)
                EXPECT_LE(Distance(posA, posB), Distance(LocationOf(a, i), LocationOf(b, j)));
    }
}

TEST("RegionRoute: A route is symmetric, shortest, connected and inside its two counties")
{
    u32 a, b, i;
    struct RegionTilePos posA, posB;

    FOR_EACH_ROUTE_PAIR(a, b, posA, posB)
    {
        u32 count = GetRouteTileCount(posA, posB);
        struct RegionTilePos prev = posA;

        EXPECT_EQ(count, GetRouteTileCount(posB, posA));
        EXPECT_EQ(count, Distance(posA, posB) - 1);
        for (i = 0; i < count; i++)
        {
            struct RegionTilePos tile = GetRouteTile(posA, posB, i);

            EXPECT(SamePos(tile, GetRouteTile(posB, posA, i)));
            EXPECT(IsInEitherCounty(a, b, tile));
            EXPECT(!SamePos(tile, posA) && !SamePos(tile, posB));
            if (i > 0)
                EXPECT_EQ(Distance(prev, tile), 1);
            prev = tile;
        }
    }
}

TEST("RegionRoute: A route leaves and enters its locations in the neighbors' direction")
{
    u32 a, b;
    struct RegionTilePos posA, posB;

    FOR_EACH_ROUTE_PAIR(a, b, posA, posB)
    {
        bool32 horizontal = COUNTY_Y(a) == COUNTY_Y(b);
        bool32 swapped = horizontal ? COUNTY_X(a) > COUNTY_X(b) : COUNTY_Y(a) > COUNTY_Y(b);
        struct RegionTilePos start = swapped ? posB : posA;
        struct RegionTilePos end = swapped ? posA : posB;
        u32 count = GetRouteTileCount(start, end);
        struct RegionTilePos first, last;
        u32 gap = horizontal ? end.x - start.x : end.y - start.y;

        if (count == 0)
            continue;
        first = GetRouteTile(start, end, 0);
        last = GetRouteTile(start, end, count - 1);
        if (horizontal)
        {
            EXPECT_EQ(first.x, start.x + 1);
            EXPECT_EQ(first.y, start.y);
            if (gap > 1)
            {
                EXPECT_EQ(last.x, end.x - 1);
                EXPECT_EQ(last.y, end.y);
            }
        }
        else
        {
            EXPECT_EQ(first.y, start.y + 1);
            EXPECT_EQ(first.x, start.x);
            if (gap > 1)
            {
                EXPECT_EQ(last.y, end.y - 1);
                EXPECT_EQ(last.x, end.x);
            }
        }
    }
}

// First tile of the route leaving `from` toward `to`; FALSE if the route has no tiles.
static bool32 FirstTileFrom(u32 fromCounty, u32 toCounty, struct RegionTilePos from, struct RegionTilePos to, struct RegionTilePos *tile)
{
    u32 count = GetRouteTileCount(from, to);
    bool32 horizontal = COUNTY_Y(fromCounty) == COUNTY_Y(toCounty);
    bool32 fromIsStart = horizontal ? from.x < to.x : from.y < to.y;

    if (count == 0)
        return FALSE;
    *tile = GetRouteTile(from, to, fromIsStart ? 0 : count - 1);
    return TRUE;
}

// Only first tiles are guaranteed distinct. Routes sharing a location that head into the same
// quadrant can still overlap further out (see remaining_work.md, "Route overlap").
TEST("RegionRoute: Routes leaving one location toward different neighbors start on different tiles")
{
    u32 a, b, c, d;
    struct RegionTilePos posA, posB, posC, posD, first1, first2;

    FOR_EACH_ROUTE_PAIR(a, b, posA, posB)
    {
        FOR_EACH_ROUTE_PAIR(c, d, posC, posD)
        {
            if (a != c || b == d || !SamePos(posA, posC))
                continue;
            if (FirstTileFrom(a, b, posA, posB, &first1) && FirstTileFrom(c, d, posC, posD, &first2))
                EXPECT(!SamePos(first1, first2));
        }
    }
}
