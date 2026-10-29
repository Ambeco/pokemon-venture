#include "global.h"
#include "identity_seed.h"
#include "region_route.h"

// A route in axis space: u runs along the pair's axis from the west/north end, v across it.
struct RouteShape
{
    bool8 horizontal;
    s16 u1, v1, u2, v2;
};

static struct RouteShape MakeRouteShape(struct RegionTilePos a, struct RegionTilePos b)
{
    u32 countyA = GetCountyFromRegionCell(a.x, a.y);
    u32 countyB = GetCountyFromRegionCell(b.x, b.y);
    struct RouteShape shape;
    s16 ua, va, ub, vb;

    AGB_ASSERT(AreCountiesAdjacent(countyA, countyB));
    shape.horizontal = COUNTY_Y(countyA) == COUNTY_Y(countyB);
    ua = shape.horizontal ? a.x : a.y;
    va = shape.horizontal ? a.y : a.x;
    ub = shape.horizontal ? b.x : b.y;
    vb = shape.horizontal ? b.y : b.x;
    if (ua > ub)
    {
        s16 t = ua; ua = ub; ub = t;
        t = va; va = vb; vb = t;
    }
    AGB_ASSERT(ub > ua);
    shape.u1 = ua; shape.v1 = va; shape.u2 = ub; shape.v2 = vb;
    return shape;
}

static struct RegionTilePos ShapeTile(const struct RouteShape *shape, s32 u, s32 v)
{
    struct RegionTilePos pos;

    pos.x = shape->horizontal ? u : v;
    pos.y = shape->horizontal ? v : u;
    return pos;
}

static s32 BendOffset(const struct RouteShape *shape)
{
    u32 key = (shape->u1 << 24) | (shape->v1 << 16) | (shape->u2 << 8) | shape->v2;

    return IdentityHash(GetPlayerIdentitySeed(), REGION_SALT_ROUTE_BEND + shape->horizontal, key) % (shape->u2 - shape->u1 - 1);
}

u32 GetRouteTileCount(struct RegionTilePos a, struct RegionTilePos b)
{
    struct RouteShape shape = MakeRouteShape(a, b);
    s32 across = abs(shape.v2 - shape.v1);

    return shape.u2 - shape.u1 == 1 ? across : shape.u2 - shape.u1 - 1 + across;
}

struct RegionTilePos GetRouteTile(struct RegionTilePos a, struct RegionTilePos b, u32 index)
{
    struct RouteShape shape = MakeRouteShape(a, b);
    s32 step = shape.v2 >= shape.v1 ? 1 : -1;
    s32 across = abs(shape.v2 - shape.v1);
    s32 bend, lead;

    AGB_ASSERT(index < GetRouteTileCount(a, b));
    if (shape.u2 - shape.u1 == 1)
        return ShapeTile(&shape, shape.u2, shape.v1 + step * index);

    bend = shape.u1 + 1 + BendOffset(&shape);
    lead = bend - shape.u1;
    if (index < lead)
        return ShapeTile(&shape, shape.u1 + 1 + index, shape.v1);
    index -= lead;
    if (index < across)
        return ShapeTile(&shape, bend, shape.v1 + step * (index + 1));
    index -= across;
    return ShapeTile(&shape, bend + 1 + index, shape.v2);
}

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
