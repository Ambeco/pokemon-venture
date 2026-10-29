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

static bool32 IsPosInCounty(u32 countyId, struct RegionTilePos pos)
{
    return GetCountyFromRegionCell(pos.x, pos.y) == countyId;
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
        EXPECT(IsPosInCounty(a, posA));
        EXPECT(IsPosInCounty(b, posB));
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

TEST("RegionRoute: Both counties agree on the border crossing, which is one location's row or column")
{
    u32 a, b, crossing, owner, crossingBack, ownerBack;
    struct RegionTilePos posA, posB;

    FOR_EACH_ROUTE_PAIR(a, b, posA, posB)
    {
        bool32 horizontal = COUNTY_Y(a) == COUNTY_Y(b);
        struct RegionTilePos ownerPos;

        ASSUME(GetRouteBorderCrossing(a, b, &crossing, &owner));
        ASSUME(GetRouteBorderCrossing(b, a, &crossingBack, &ownerBack));
        EXPECT_EQ(crossing, crossingBack);
        EXPECT_EQ(owner, ownerBack);
        EXPECT(owner == a || owner == b);
        ownerPos = owner == a ? posA : posB;
        EXPECT_EQ(crossing, horizontal ? ownerPos.y : ownerPos.x);
    }
}

TEST("RegionRoute: Each half is connected, inside its county, and ends at the crossing on the border")
{
    u32 a, b, i, crossing, owner;
    struct RegionTilePos posA, posB;

    FOR_EACH_ROUTE_PAIR(a, b, posA, posB)
    {
        bool32 horizontal = COUNTY_Y(a) == COUNTY_Y(b);
        u32 count = GetCountyRouteHalfTileCount(a, b);
        u32 left, top, right, bottom;
        struct RegionTilePos prev = posA, last;

        GetRouteBorderCrossing(a, b, &crossing, &owner);
        for (i = 0; i < count; i++)
        {
            struct RegionTilePos tile = GetCountyRouteHalfTile(a, b, i);

            EXPECT(IsPosInCounty(a, tile));
            EXPECT_EQ(Distance(prev, tile), 1);
            prev = tile;
        }
        last = count > 0 ? prev : posA;
        GetCountyRegionBounds(a, &left, &top, &right, &bottom);
        if (horizontal)
        {
            EXPECT_EQ(last.x, COUNTY_X(b) > COUNTY_X(a) ? right : left);
            EXPECT_EQ(last.y, crossing);
        }
        else
        {
            EXPECT_EQ(last.y, COUNTY_Y(b) > COUNTY_Y(a) ? bottom : top);
            EXPECT_EQ(last.x, crossing);
        }
    }
}

TEST("RegionRoute: The two halves meet across the border and total a shortest route")
{
    u32 a, b;
    struct RegionTilePos posA, posB;

    FOR_EACH_ROUTE_PAIR(a, b, posA, posB)
    {
        u32 countA = GetCountyRouteHalfTileCount(a, b);
        u32 countB = GetCountyRouteHalfTileCount(b, a);
        struct RegionTilePos endA = countA > 0 ? GetCountyRouteHalfTile(a, b, countA - 1) : posA;
        struct RegionTilePos endB = countB > 0 ? GetCountyRouteHalfTile(b, a, countB - 1) : posB;

        EXPECT_EQ(Distance(endA, endB), 1);
        EXPECT_EQ(countA + countB + 1, Distance(posA, posB));
    }
}

TEST("RegionRoute: The owner's half is straight and every half leaves toward its neighbor")
{
    u32 a, b, crossing, owner;
    struct RegionTilePos posA, posB;

    FOR_EACH_ROUTE_PAIR(a, b, posA, posB)
    {
        bool32 horizontal = COUNTY_Y(a) == COUNTY_Y(b);
        s32 step = horizontal ? (COUNTY_X(b) > COUNTY_X(a) ? 1 : -1) : (COUNTY_Y(b) > COUNTY_Y(a) ? 1 : -1);
        u32 count = GetCountyRouteHalfTileCount(a, b);
        struct RegionTilePos first, last;

        GetRouteBorderCrossing(a, b, &crossing, &owner);
        if (count == 0)
            continue;
        first = GetCountyRouteHalfTile(a, b, 0);
        last = GetCountyRouteHalfTile(a, b, count - 1);
        if (horizontal)
        {
            // A location already on the border column can only go across (Emerald's table has some).
            if (last.x != posA.x)
                EXPECT_EQ((s32)first.x - (s32)posA.x, step);
            if (owner == a)
                EXPECT_EQ(last.y, posA.y);
        }
        else
        {
            if (last.y != posA.y)
                EXPECT_EQ((s32)first.y - (s32)posA.y, step);
            if (owner == a)
                EXPECT_EQ(last.x, posA.x);
        }
    }
}
