#include "global.h"
#include "identity_seed.h"
#include "region_route.h"

// Location 0 is the city when the county has one, then the special locations in order.
static u32 GetLocationCount(u32 countyId)
{
    return DoesCountyHaveCity(countyId) + GetCountySpecialLocationCount(countyId);
}

static bool32 IsCityLocation(u32 countyId, u32 index)
{
    return index == 0 && DoesCountyHaveCity(countyId);
}

static struct RegionTilePos GetLocationPos(u32 countyId, u32 index)
{
    if (DoesCountyHaveCity(countyId))
    {
        if (index == 0)
            return GetCountyCityPos(countyId);
        index--;
    }
    return GetCountySpecialLocation(countyId, index).pos;
}

bool32 GetCountyPairRouteEndpoints(u32 countyA, u32 countyB, struct RegionTilePos *posA, struct RegionTilePos *posB)
{
    // Scored over the lower-id county first, so the winner does not depend on argument order.
    u32 lo = countyA < countyB ? countyA : countyB;
    u32 hi = countyA < countyB ? countyB : countyA;
    struct RegionTilePos bestLo = {0, 0}, bestHi = {0, 0};
    u32 best = 0xFFFFFFFF;
    u32 i, j;

    AGB_ASSERT(AreCountiesAdjacent(countyA, countyB));
    for (i = 0; i < GetLocationCount(lo); i++)
    {
        for (j = 0; j < GetLocationCount(hi); j++)
        {
            struct RegionTilePos p = GetLocationPos(lo, i);
            struct RegionTilePos q = GetLocationPos(hi, j);
            u32 distance = abs(p.x - q.x) + abs(p.y - q.y);
            u32 score = distance * 3 + 2 - IsCityLocation(lo, i) - IsCityLocation(hi, j);

            if (score < best)
            {
                best = score;
                bestLo = p;
                bestHi = q;
            }
        }
    }
    if (best == 0xFFFFFFFF)
        return FALSE;
    *posA = countyA == lo ? bestLo : bestHi;
    *posB = countyA == lo ? bestHi : bestLo;
    return TRUE;
}

static bool32 IsHorizontalPair(u32 countyA, u32 countyB)
{
    return COUNTY_Y(countyA) == COUNTY_Y(countyB);
}

bool32 GetRouteBorderCrossing(u32 countyA, u32 countyB, u32 *crossing, u32 *owner)
{
    u32 lo = countyA < countyB ? countyA : countyB;
    u32 hi = countyA < countyB ? countyB : countyA;
    struct RegionTilePos posLo, posHi;
    bool32 horizontal = IsHorizontalPair(lo, hi);
    bool32 pickHi;

    if (!GetCountyPairRouteEndpoints(lo, hi, &posLo, &posHi))
        return FALSE;
    pickHi = IdentityHash(GetPlayerIdentitySeed(), REGION_SALT_ROUTE_CROSSING + horizontal, lo << 8 | hi) & 1;
    *owner = pickHi ? hi : lo;
    if (horizontal)
        *crossing = pickHi ? posHi.y : posLo.y;
    else
        *crossing = pickHi ? posHi.x : posLo.x;
    return TRUE;
}

// A county's half of a route in axis space: u runs along the pair's axis toward the neighbor,
// v across it. The half goes from (u0, v0) to (border, crossing) as a run along v0 to `bend`, a run
// across to the crossing, and a run along the crossing row to the border.
struct HalfShape
{
    bool8 horizontal;
    s16 step;
    s16 u0, v0, border, crossing, bend;
};

static bool32 GetHalfShape(u32 county, u32 neighbor, struct HalfShape *shape)
{
    struct RegionTilePos mine, theirs;
    u32 crossing, owner, left, top, right, bottom;
    s32 length;

    if (!GetCountyPairRouteEndpoints(county, neighbor, &mine, &theirs))
        return FALSE;
    GetRouteBorderCrossing(county, neighbor, &crossing, &owner);
    GetCountyRegionBounds(county, &left, &top, &right, &bottom);
    shape->horizontal = IsHorizontalPair(county, neighbor);
    if (shape->horizontal)
    {
        shape->step = COUNTY_X(neighbor) > COUNTY_X(county) ? 1 : -1;
        shape->u0 = mine.x;
        shape->v0 = mine.y;
        shape->border = shape->step > 0 ? right : left;
    }
    else
    {
        shape->step = COUNTY_Y(neighbor) > COUNTY_Y(county) ? 1 : -1;
        shape->u0 = mine.y;
        shape->v0 = mine.x;
        shape->border = shape->step > 0 ? bottom : top;
    }
    shape->crossing = crossing;
    length = (shape->border - shape->u0) * shape->step;
    AGB_ASSERT(length >= 0);
    // The owner's crossing is its own row, so its half is straight whatever the bend is.
    shape->bend = length == 0 ? shape->border
        : shape->u0 + shape->step * (1 + IdentityHash(GetPlayerIdentitySeed(), REGION_SALT_ROUTE_BEND + shape->horizontal,
              county << 24 | shape->u0 << 16 | shape->v0 << 8 | shape->crossing) % length);
    return TRUE;
}

static struct RegionTilePos ShapeTile(const struct HalfShape *shape, s32 u, s32 v)
{
    struct RegionTilePos pos;

    pos.x = shape->horizontal ? u : v;
    pos.y = shape->horizontal ? v : u;
    return pos;
}

u32 GetCountyRouteHalfTileCount(u32 county, u32 neighbor)
{
    struct HalfShape shape;

    if (!GetHalfShape(county, neighbor, &shape))
        return 0;
    return (shape.border - shape.u0) * shape.step + abs(shape.crossing - shape.v0);
}

struct RegionTilePos GetCountyRouteHalfTile(u32 county, u32 neighbor, u32 index)
{
    struct HalfShape shape;
    s32 across, lead, vStep;

    AGB_ASSERT(index < GetCountyRouteHalfTileCount(county, neighbor));
    GetHalfShape(county, neighbor, &shape);
    across = abs(shape.crossing - shape.v0);
    vStep = shape.crossing >= shape.v0 ? 1 : -1;
    lead = (shape.bend - shape.u0) * shape.step;
    if (index < lead)
        return ShapeTile(&shape, shape.u0 + shape.step * (index + 1), shape.v0);
    index -= lead;
    if (index < across)
        return ShapeTile(&shape, shape.bend, shape.v0 + vStep * (index + 1));
    index -= across;
    return ShapeTile(&shape, shape.bend + shape.step * (index + 1), shape.crossing);
}
