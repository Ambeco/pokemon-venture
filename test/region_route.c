#include "global.h"
#include "test/test.h"
#include "region_route.h"
#include "test/seed_sweep.h"

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

// Routed locations: the city, then the landmarks and detours (pitstops are not routed).
static struct RegionTilePos LocationOf(u32 countyId, u32 index)
{
    u32 i;

    if (DoesCountyHaveCity(countyId))
    {
        if (index == 0)
            return GetCountyCityPos(countyId);
        index--;
    }
    for (i = 0; ; i++)
        if (GetCountySpecialLocationClass(countyId, i) != LOCATION_CLASS_PITSTOP && index-- == 0)
            return GetCountySpecialLocation(countyId, i).pos;
}

static u32 LocationCountOf(u32 countyId)
{
    u32 i, count = DoesCountyHaveCity(countyId);

    for (i = 0; i < GetCountySpecialLocationCount(countyId); i++)
        count += GetCountySpecialLocationClass(countyId, i) != LOCATION_CLASS_PITSTOP;
    return count;
}

// Runs the following statement for each ordered pair of counties joined by a route that both have a location.
#define FOR_EACH_ROUTE_PAIR(a, b, posA, posB) \
    for (a = 0; a < NUM_COUNTIES; a++) \
        for (b = 0; b < NUM_COUNTIES; b++) \
            if (DoCountiesHaveRoute(a, b) && GetCountyPairRouteEndpoints(a, b, &posA, &posB))

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
    u32 si, seed = 0, a, b, crossing, owner, crossingBack, ownerBack;
    (void)seed;
    struct RegionTilePos posA, posB;

    FOR_EACH_SWEEP_SEED(si, seed)
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
    u32 si, seed = 0, a, b, i, crossing, owner;
    (void)seed;
    struct RegionTilePos posA, posB;

    FOR_EACH_SWEEP_SEED(si, seed)
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
    u32 si, seed = 0, a, b;
    (void)seed;
    struct RegionTilePos posA, posB;

    // Alternating counties replans each time, so this sweeps fewer seeds than the others.
    FOR_EACH_SWEEP_SEED(si, seed)
    {
        if (si >= 8)
            break;
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
}

TEST("RegionRoute: The owner's half is straight and every half leaves toward its neighbor")
{
    u32 si, seed = 0, a, b, crossing, owner;
    (void)seed;
    struct RegionTilePos posA, posB;

    FOR_EACH_SWEEP_SEED(si, seed)
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

// An independent description of one half, with the bend as a parameter, to brute-force the planner.
struct TestHalf
{
    bool8 horizontal;
    bool8 isFree;
    s16 step, u0, v0, border, crossing, length;
};

#define TEST_MAX_HALVES 4
#define TEST_MAX_TILES 40

static struct RegionTilePos TestHalfTile(const struct TestHalf *h, s32 bend, u32 index)
{
    s32 across = abs(h->crossing - h->v0);
    s32 vStep = h->crossing >= h->v0 ? 1 : -1;
    s32 lead = (bend - h->u0) * h->step;
    s32 u, v;
    struct RegionTilePos pos;

    if (index < lead)
        u = h->u0 + h->step * (index + 1), v = h->v0;
    else if (index - lead < across)
        u = bend, v = h->v0 + vStep * (index - lead + 1);
    else
        u = bend + h->step * (index - lead - across + 1), v = h->crossing;
    pos.x = h->horizontal ? u : v;
    pos.y = h->horizontal ? v : u;
    return pos;
}

static u32 TestHalfTileCount(const struct TestHalf *h)
{
    return h->length + abs(h->crossing - h->v0);
}

static u32 CollectHalves(u32 county, struct TestHalf *halves, u32 *neighbors)
{
    u32 n, count = 0;

    for (n = 0; n < NUM_COUNTIES; n++)
    {
        struct RegionTilePos mine, theirs;
        u32 crossing, owner, left, top, right, bottom;
        struct TestHalf *h = &halves[count];

        if (!DoCountiesHaveRoute(county, n) || !GetCountyPairRouteEndpoints(county, n, &mine, &theirs))
            continue;
        GetRouteBorderCrossing(county, n, &crossing, &owner);
        GetCountyRegionBounds(county, &left, &top, &right, &bottom);
        h->horizontal = COUNTY_Y(county) == COUNTY_Y(n);
        h->step = (h->horizontal ? COUNTY_X(n) > COUNTY_X(county) : COUNTY_Y(n) > COUNTY_Y(county)) ? 1 : -1;
        h->u0 = h->horizontal ? mine.x : mine.y;
        h->v0 = h->horizontal ? mine.y : mine.x;
        h->border = h->horizontal ? (h->step > 0 ? right : left) : (h->step > 0 ? bottom : top);
        h->crossing = crossing;
        h->length = (h->border - h->u0) * h->step;
        h->isFree = owner != county && h->length > 0 && crossing != h->v0;
        neighbors[count++] = n;
    }
    return count;
}

// Directions (bit set) a half leaves a tile in: toward the previous tile (the location for the
// first) and the next (the border side for the last).
static u32 TestDirections(const struct TestHalf *h, s32 bend, u32 index)
{
    struct RegionTilePos t = TestHalfTile(h, bend, index);
    struct RegionTilePos prev = index == 0
        ? (struct RegionTilePos){h->horizontal ? h->u0 : h->v0, h->horizontal ? h->v0 : h->u0}
        : TestHalfTile(h, bend, index - 1);
    struct RegionTilePos next = index + 1 == TestHalfTileCount(h)
        ? (struct RegionTilePos){t.x + (h->horizontal ? h->step : 0), t.y + (h->horizontal ? 0 : h->step)}
        : TestHalfTile(h, bend, index + 1);
    u32 mask = 0;
    s32 dx[2] = {(s32)prev.x - t.x, (s32)next.x - t.x};
    s32 dy[2] = {(s32)prev.y - t.y, (s32)next.y - t.y};
    u32 k;

    for (k = 0; k < 2; k++)
        mask |= dx[k] > 0 ? 1 : dx[k] < 0 ? 2 : dy[k] > 0 ? 4 : 8;
    return mask;
}

// A conflict is a half through another location, or two halves sharing a tile without leaving it
// in a common direction (a crossing; sharing a run of road is fine).
static bool32 HasConflict(u32 county, const struct TestHalf *halves, const s32 *bends, u32 count)
{
    u32 i, j, a, b, loc;

    for (i = 0; i < count; i++)
    {
        for (a = 0; a < TestHalfTileCount(&halves[i]); a++)
        {
            struct RegionTilePos t = TestHalfTile(&halves[i], bends[i], a);

            for (loc = 0; loc < LocationCountOf(county); loc++)
            {
                struct RegionTilePos l = LocationOf(county, loc);
                bool32 isStart = (halves[i].horizontal ? l.x : l.y) == halves[i].u0
                    && (halves[i].horizontal ? l.y : l.x) == halves[i].v0;

                if (!isStart && SamePos(t, l))
                    return TRUE;
            }
            for (j = i + 1; j < count; j++)
                for (b = 0; b < TestHalfTileCount(&halves[j]); b++)
                    if (SamePos(t, TestHalfTile(&halves[j], bends[j], b))
                     && !(TestDirections(&halves[i], bends[i], a) & TestDirections(&halves[j], bends[j], b)))
                        return TRUE;
        }
    }
    return FALSE;
}

static bool32 AnyBendComboWorks(u32 county, const struct TestHalf *halves, s32 *bends, u32 count, u32 slot)
{
    s32 k;

    if (slot == count)
        return !HasConflict(county, halves, bends, count);
    if (!halves[slot].isFree)
        return AnyBendComboWorks(county, halves, bends, count, slot + 1);
    for (k = 1; k <= halves[slot].length; k++)
    {
        bends[slot] = halves[slot].u0 + halves[slot].step * k;
        if (AnyBendComboWorks(county, halves, bends, count, slot + 1))
            return TRUE;
    }
    return FALSE;
}

// A planned half's bend: the u coordinate of its first step across.
static s32 PlannedBend(u32 county, u32 neighbor, const struct TestHalf *h)
{
    u32 i;

    for (i = 0; i < GetCountyRouteHalfTileCount(county, neighbor); i++)
    {
        struct RegionTilePos t = GetCountyRouteHalfTile(county, neighbor, i);

        if ((h->horizontal ? t.y : t.x) != h->v0)
            return h->horizontal ? t.x : t.y;
    }
    return h->border;
}

TEST("RegionRoute: A county's halves never cross each other or pass through its locations whenever any bend choice can avoid it")
{
    static EWRAM_DATA struct TestHalf halves[TEST_MAX_HALVES];
    static EWRAM_DATA u32 neighbors[TEST_MAX_HALVES];
    s32 bends[TEST_MAX_HALVES], planned[TEST_MAX_HALVES];
    u32 si, seed = 0, county, i, count, solvable = 0, free = 0;
    (void)seed;

    FOR_EACH_SWEEP_SEED(si, seed)
    {
        for (county = 0; county < NUM_COUNTIES; county++)
        {
            count = CollectHalves(county, halves, neighbors);
            ASSUME(count <= TEST_MAX_HALVES);
            for (i = 0; i < count; i++)
            {
                bends[i] = planned[i] = halves[i].isFree ? PlannedBend(county, neighbors[i], &halves[i]) : halves[i].border;
                free += halves[i].isFree;
            }
            if (AnyBendComboWorks(county, halves, bends, count, 0))
            {
                solvable++;
                EXPECT(!HasConflict(county, halves, planned, count));
            }
        }
    }
    // Guards against the test passing vacuously.
    EXPECT_GT(free, 0);
    EXPECT_GT(solvable, 0);
}

TEST("RegionRoute: Bends vary with the seed")
{
    static EWRAM_DATA struct TestHalf halves[TEST_MAX_HALVES];
    static EWRAM_DATA u32 neighbors[TEST_MAX_HALVES];
    u32 si, seed = 0, county, i, count, distinct = 0;
    (void)seed;
    s32 first = 0;
    bool32 found = FALSE;

    for (county = 0; county < NUM_COUNTIES && !found; county++)
    {
        FOR_EACH_SWEEP_SEED(si, seed)
        {
            count = CollectHalves(county, halves, neighbors);
            for (i = 0; i < count; i++)
            {
                s32 bend;

                if (!halves[i].isFree || halves[i].length < 2)
                    continue;
                bend = PlannedBend(county, neighbors[i], &halves[i]);
                if (!found)
                    first = bend;
                found = TRUE;
                distinct += bend != first;
                break;
            }
            if (!found)
                break;
        }
    }
    ASSUME(found);
    EXPECT_GT(distinct, 0);
}

TEST("RegionRoute: DoCountiesHaveRoute is symmetric and only for adjacent counties")
{
    u32 a, b;

    for (a = 0; a < NUM_COUNTIES; a++)
    {
        for (b = 0; b < NUM_COUNTIES; b++)
        {
            EXPECT_EQ(DoCountiesHaveRoute(a, b), DoCountiesHaveRoute(b, a));
            if (!AreCountiesAdjacent(a, b))
                EXPECT_EQ(DoCountiesHaveRoute(a, b), FALSE);
        }
    }
}

TEST("RegionRoute: Every county routes to its capital neighbor, its story-index neighbors, and has at least one route")
{
    u32 a, b;
    const struct RegionHeader *region = GetRegionHeader();

    for (a = 0; a < NUM_COUNTIES; a++)
    {
        u32 routes = 0;

        for (b = 0; b < NUM_COUNTIES; b++)
        {
            if (!AreCountiesAdjacent(a, b))
                continue;
            if (b == region->capital || GetCountyHeader(a)->storyIdx + 1 == GetCountyHeader(b)->storyIdx
                || GetCountyHeader(b)->storyIdx + 1 == GetCountyHeader(a)->storyIdx)
                EXPECT_EQ(DoCountiesHaveRoute(a, b), !IsCountyEnclaveOnly(a) && !IsCountyEnclaveOnly(b));
            routes += DoCountiesHaveRoute(a, b);
        }
        if (IsCountyEnclaveOnly(a))
            EXPECT_EQ(routes, 0);
        else
            EXPECT_GT(routes, 0);
    }
}

TEST("RegionRoute: An outside-city special location is joined to its city by a shortest connected route")
{
    u32 si, seed = 0, county, i, j;
    (void)seed;

    FOR_EACH_SWEEP_SEED(si, seed)
    {
        for (county = 0; county < NUM_COUNTIES; county++)
        {
            for (i = 0; i < GetCountySpecialLocationCount(county); i++)
            {
                struct SpecialLocation location = GetCountySpecialLocation(county, i);
                u32 count = GetCountyIntraRouteTileCount(county, i);
                struct RegionTilePos city, prev = location.pos;

                if (!DoesCountyHaveCity(county) || IsSpecialLocationKindInsideCity(location.kind) || location.locationClass == LOCATION_CLASS_PITSTOP)
                {
                    EXPECT_EQ(count, 0);
                    continue;
                }
                city = GetCountyCityPos(county);
                EXPECT_EQ(count + 1, Distance(location.pos, city));
                for (j = 0; j < count; j++)
                {
                    struct RegionTilePos tile = GetCountyIntraRouteTile(county, i, j);

                    EXPECT(IsPosInCounty(county, tile));
                    EXPECT(!SamePos(tile, city));
                    EXPECT_EQ(Distance(prev, tile), 1);
                    prev = tile;
                }
                EXPECT_EQ(Distance(prev, city), 1);
            }
        }
    }
}

TEST("RegionRoute: Intra-county routes avoid other locations (an axis or bend always can in Emerald's table)")
{
    u32 county, i, j, k;

    for (county = 0; county < NUM_COUNTIES; county++)
    {
        for (i = 0; i < GetCountySpecialLocationCount(county); i++)
        {
            for (j = 0; j < GetCountyIntraRouteTileCount(county, i); j++)
            {
                struct RegionTilePos tile = GetCountyIntraRouteTile(county, i, j);

                for (k = 0; k < LocationCountOf(county); k++)
                    EXPECT(!SamePos(tile, LocationOf(county, k)) || SamePos(tile, GetCountySpecialLocation(county, i).pos));
            }
        }
    }
}

static bool32 RouteCovers(u32 county, u32 routeId, struct RegionTilePos pos)
{
    u32 i, count;

    if (routeId < NUM_COUNTIES)
    {
        count = DoCountiesHaveRoute(county, routeId) ? GetCountyRouteHalfTileCount(county, routeId) : 0;
        for (i = 0; i < count; i++)
            if (SamePos(GetCountyRouteHalfTile(county, routeId, i), pos))
                return TRUE;
        return FALSE;
    }
    count = GetCountyIntraRouteTileCount(county, routeId - NUM_COUNTIES);
    for (i = 0; i < count; i++)
        if (SamePos(GetCountyIntraRouteTile(county, routeId - NUM_COUNTIES, i), pos))
            return TRUE;
    return FALSE;
}

static u32 CountRoutesCovering(u32 county, struct RegionTilePos pos, u32 *only)
{
    u32 id, n = 0;

    for (id = 0; id < ROUTE_ID_TO_SPECIAL(MAX_SPECIAL_LOCATIONS_PER_COUNTY); id++)
    {
        if (RouteCovers(county, id, pos))
        {
            n++;
            *only = id;
        }
    }
    return n;
}

// A pitstop sits on a route tile that keeps its owner.
static bool32 IsPitstopTile(u32 county, struct RegionTilePos pos)
{
    u32 i;

    for (i = 0; i < GetCountySpecialLocationCount(county); i++)
        if (GetCountySpecialLocationClass(county, i) == LOCATION_CLASS_PITSTOP && SamePos(GetCountySpecialLocation(county, i).pos, pos))
            return TRUE;
    return FALSE;
}

TEST("RegionRoute: Every plain route tile has one owner that covers it, and other tiles have none")
{
    u32 si, seed = 0, county, x, y;
    (void)seed;

    FOR_EACH_SWEEP_SEED(si, seed)
    {
        if (si >= 2)
            break;
        for (y = 0; y < REGION_H; y++)
        {
            for (x = 0; x < REGION_W; x++)
            {
                u32 flags = GetRegionTileFlags(x, y);
                u32 id = GetRegionTileRouteId(x, y);
                struct RegionTilePos pos = {x, y};
                u32 only = 0;

                county = GetCountyFromRegionCell(x, y);
                if (flags == TILE_ROUTE || IsPitstopTile(county, pos))
                {
                    EXPECT_NE(id, ROUTE_ID_NONE);
                    EXPECT(RouteCovers(county, id, pos));
                    if (CountRoutesCovering(county, pos, &only) == 1)
                        EXPECT_EQ(id, only);
                }
                else
                {
                    EXPECT_EQ(id, ROUTE_ID_NONE);
                }
            }
        }
    }
}

TEST("RegionRoute: A lock is on its route, on no location, and past every tile its route shares")
{
    u32 a, b, i, locks = 0, missing = 0;
    struct RegionTilePos posA, posB;

    FOR_EACH_ROUTE_PAIR(a, b, posA, posB)
    {
        struct RegionTilePos lock;
        u32 gated, early, late, only, n;
        bool32 onEarly;

        if (a > b)
            continue;
        if (!GetRouteLock(a, b, &lock, &gated))
        {
            missing++;
            continue;
        }
        locks++;
        early = GetCountyHeader(a)->storyIdx < GetCountyHeader(b)->storyIdx ? a : b;
        late = early == a ? b : a;
        EXPECT_EQ(gated, late);
        onEarly = RouteCovers(early, late, lock);
        EXPECT(onEarly || RouteCovers(late, early, lock));
        for (i = 0; i < LocationCountOf(early); i++)
            EXPECT(!SamePos(lock, LocationOf(early, i)));
        for (i = 0; i < LocationCountOf(late); i++)
            EXPECT(!SamePos(lock, LocationOf(late, i)));
        if (onEarly)
        {
            u32 count = GetCountyRouteHalfTileCount(early, late);

            // Nothing from the lock to the later end is shared with another route of the earlier county.
            for (i = 0; i < count; i++)
            {
                struct RegionTilePos t = GetCountyRouteHalfTile(early, late, i);

                if (SamePos(t, lock))
                    break;
            }
            for (; i < count; i++)
            {
                n = CountRoutesCovering(early, GetCountyRouteHalfTile(early, late, i), &only);
                EXPECT_EQ(n, 1);
            }
        }
    }
    EXPECT_GT(locks, 0);
    // Emerald's table may leave some routes without a tile (locations adjacent across a border).
    EXPECT_LT(missing, locks);
}

TEST("RegionRoute: An intra-county lock is on its route and on no location")
{
    u32 county, i, k;

    for (county = 0; county < NUM_COUNTIES; county++)
    {
        for (i = 0; i < GetCountySpecialLocationCount(county); i++)
        {
            struct RegionTilePos lock;
            bool32 gatesCity;

            if (GetCountyIntraRouteTileCount(county, i) == 0 || !GetCountyIntraRouteLock(county, i, &lock, &gatesCity))
                continue;
            EXPECT(RouteCovers(county, ROUTE_ID_TO_SPECIAL(i), lock));
            for (k = 0; k < LocationCountOf(county); k++)
                EXPECT(!SamePos(lock, LocationOf(county, k)));
        }
    }
}
