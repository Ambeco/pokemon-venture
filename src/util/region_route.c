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

// Counties that `county` itself routes to: the capital and adjacent story indices, else the
// highest-indexed neighbor.
static bool32 CountyRoutesTo(u32 county, u32 neighbor)
{
    const struct RegionHeader *region = GetRegionHeader();
    u32 storyIdx = GetCountyHeader(county)->storyIdx;
    u32 other, best = 0, bestIdx = 0;
    bool32 any = FALSE;

    for (other = 0; other < NUM_COUNTIES; other++)
    {
        u32 otherIdx = GetCountyHeader(other)->storyIdx;

        if (!AreCountiesAdjacent(county, other))
            continue;
        if (other == region->capital || otherIdx + 1 == storyIdx || otherIdx == storyIdx + 1)
        {
            if (other == neighbor)
                return TRUE;
            any = TRUE;
        }
        if (otherIdx >= bestIdx)
        {
            bestIdx = otherIdx;
            best = other;
        }
    }
    return !any && best == neighbor;
}

bool32 DoCountiesHaveRoute(u32 countyA, u32 countyB)
{
    return AreCountiesAdjacent(countyA, countyB) && (CountyRoutesTo(countyA, countyB) || CountyRoutesTo(countyB, countyA));
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
    bool8 isFree; // the bend can change the shape; only a non-owner half that has to turn
    bool8 omitLast; // an intra-county route ends at its city, which is not listed
    s16 step;
    s16 u0, v0, border, crossing, bend;
    s16 length, defaultOffset;
};

#define MAX_COUNTY_NEIGHBORS 4
#define MAX_COUNTY_ROUTES    (MAX_COUNTY_NEIGHBORS + MAX_SPECIAL_LOCATIONS_PER_COUNTY)

// Shapes [0, interCount) are halves of inter-county routes (routeId = neighbor county); the rest run
// from an outside-city special location to the county's city (routeId = ROUTE_ID_TO_SPECIAL(index)).
struct CountyPlan
{
    u8 count;
    u8 interCount;
    u8 routeId[MAX_COUNTY_ROUTES];
    u8 opensAt[MAX_COUNTY_ROUTES]; // story index at which the route opens, which decides tile ownership
    u8 owner[COUNTY_TILES_W * COUNTY_TILES_H]; // slot owning each tile (x - left, y - top), 0xFF = none
    bool8 placed[MAX_COUNTY_ROUTES];
    struct HalfShape shape[MAX_COUNTY_ROUTES];
};

static bool32 GetHalfShape(u32 county, u32 neighbor, struct HalfShape *shape)
{
    struct RegionTilePos mine, theirs;
    u32 crossing, owner, left, top, right, bottom;

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
    shape->length = (shape->border - shape->u0) * shape->step;
    AGB_ASSERT(shape->length >= 0);
    // The owner's crossing is its own row, so its half is straight whatever the bend is.
    shape->defaultOffset = shape->length == 0 ? 0
        : IdentityHash(GetPlayerIdentitySeed(), REGION_SALT_ROUTE_BEND + shape->horizontal,
              county << 24 | shape->u0 << 16 | shape->v0 << 8 | shape->crossing) % shape->length;
    shape->bend = shape->length == 0 ? shape->border : shape->u0 + shape->step * (1 + shape->defaultOffset);
    shape->isFree = owner != county && shape->length > 0 && shape->crossing != shape->v0;
    shape->omitLast = FALSE;
    return TRUE;
}

static void BuildIntraShape(u32 county, struct RegionTilePos from, struct RegionTilePos city, bool32 horizontal, struct HalfShape *shape)
{
    shape->horizontal = horizontal;
    shape->u0 = horizontal ? from.x : from.y;
    shape->v0 = horizontal ? from.y : from.x;
    shape->border = horizontal ? city.x : city.y;
    shape->crossing = horizontal ? city.y : city.x;
    shape->step = shape->border >= shape->u0 ? 1 : -1;
    shape->length = (shape->border - shape->u0) * shape->step;
    shape->defaultOffset = shape->length == 0 ? 0
        : IdentityHash(GetPlayerIdentitySeed(), REGION_SALT_ROUTE_BEND + horizontal,
              (county << 24 | shape->u0 << 16 | shape->v0 << 8 | shape->crossing) ^ 0x80000000) % shape->length;
    shape->bend = shape->length == 0 ? shape->border : shape->u0 + shape->step * (1 + shape->defaultOffset);
    shape->isFree = shape->length > 0 && shape->crossing != shape->v0;
    shape->omitLast = TRUE;
}

static bool32 CanAvoidLocations(u32 county, struct HalfShape *shape);

// The route from an outside-city special location to the city, in the same axis space as a half:
// the run along u ends at the city, whose coordinates play the part of the border and crossing.
// The axis is a hash, so some routes leave their location sideways and some vertically, unless
// that axis would force the route through another location.
static bool32 GetIntraShape(u32 county, u32 specialIndex, struct HalfShape *shape)
{
    struct SpecialLocation location;
    struct RegionTilePos city;
    bool32 horizontal;

    if (!DoesCountyHaveCity(county))
        return FALSE;
    location = GetCountySpecialLocation(county, specialIndex);
    if (IsSpecialLocationKindInsideCity(location.kind))
        return FALSE;
    city = GetCountyCityPos(county);
    horizontal = IdentityHash(GetPlayerIdentitySeed(), REGION_SALT_ROUTE_INTRA, county << 8 | specialIndex) & 1;
    BuildIntraShape(county, location.pos, city, horizontal, shape);
    if (!CanAvoidLocations(county, shape))
    {
        struct HalfShape other;

        BuildIntraShape(county, location.pos, city, !horizontal, &other);
        if (CanAvoidLocations(county, &other))
            *shape = other;
    }
    return TRUE;
}

static struct RegionTilePos ShapeTile(const struct HalfShape *shape, s32 u, s32 v)
{
    struct RegionTilePos pos;

    pos.x = shape->horizontal ? u : v;
    pos.y = shape->horizontal ? v : u;
    return pos;
}

static u32 ShapeFullCount(const struct HalfShape *shape)
{
    return shape->length + abs(shape->crossing - shape->v0);
}

static u32 ShapeTileCount(const struct HalfShape *shape)
{
    return ShapeFullCount(shape) - shape->omitLast;
}

static struct RegionTilePos ShapeHalfTile(const struct HalfShape *shape, u32 index)
{
    s32 across = abs(shape->crossing - shape->v0);
    s32 vStep = shape->crossing >= shape->v0 ? 1 : -1;
    s32 lead = (shape->bend - shape->u0) * shape->step;

    if (index < lead)
        return ShapeTile(shape, shape->u0 + shape->step * (index + 1), shape->v0);
    index -= lead;
    if (index < across)
        return ShapeTile(shape, shape->bend, shape->v0 + vStep * (index + 1));
    index -= across;
    return ShapeTile(shape, shape->bend + shape->step * (index + 1), shape->crossing);
}

static bool32 HalfHitsPos(const struct HalfShape *shape, struct RegionTilePos pos)
{
    u32 i, count = ShapeTileCount(shape);

    for (i = 0; i < count; i++)
    {
        struct RegionTilePos tile = ShapeHalfTile(shape, i);

        if (tile.x == pos.x && tile.y == pos.y)
            return TRUE;
    }
    return FALSE;
}

// Bit set of the directions a half's route leaves the tile, toward its neighbors in the half. The
// first tile's previous neighbor is the half's location; the last tile continues toward the border.
static u32 HalfTileDirections(const struct HalfShape *shape, u32 index)
{
    struct RegionTilePos tile = ShapeHalfTile(shape, index);
    struct RegionTilePos prev = index == 0 ? ShapeTile(shape, shape->u0, shape->v0) : ShapeHalfTile(shape, index - 1);
    struct RegionTilePos next = index + 1 == ShapeFullCount(shape)
        ? ShapeTile(shape, (shape->horizontal ? tile.x : tile.y) + shape->step, shape->horizontal ? tile.y : tile.x)
        : ShapeHalfTile(shape, index + 1);
    u32 mask = 0;

    if (prev.x != tile.x)
        mask |= prev.x > tile.x ? 1 : 2;
    else
        mask |= prev.y > tile.y ? 4 : 8;
    if (next.x != tile.x)
        mask |= next.x > tile.x ? 1 : 2;
    else
        mask |= next.y > tile.y ? 4 : 8;
    return mask;
}

// Halves may share a run of tiles (the route is then one road for that stretch), but may not cross:
// at every shared tile the two halves must leave in at least one common direction.
static bool32 HalvesCross(const struct HalfShape *a, const struct HalfShape *b)
{
    u32 i, j, countA = ShapeTileCount(a), countB = ShapeTileCount(b);

    for (i = 0; i < countA; i++)
    {
        struct RegionTilePos tile = ShapeHalfTile(a, i);

        for (j = 0; j < countB; j++)
        {
            struct RegionTilePos other = ShapeHalfTile(b, j);

            if (tile.x == other.x && tile.y == other.y && !(HalfTileDirections(a, i) & HalfTileDirections(b, j)))
                return TRUE;
        }
    }
    return FALSE;
}

static bool32 HalfHitsOtherLocation(u32 county, const struct HalfShape *shape)
{
    struct RegionTilePos start = ShapeTile(shape, shape->u0, shape->v0);
    u32 i;

    for (i = 0; i < GetLocationCount(county); i++)
    {
        struct RegionTilePos pos = GetLocationPos(county, i);

        if (shape->omitLast && IsCityLocation(county, i))
            continue;
        if ((pos.x != start.x || pos.y != start.y) && HalfHitsPos(shape, pos))
            return TRUE;
    }
    return FALSE;
}

// Whether some bend keeps the shape off every other location.
static bool32 CanAvoidLocations(u32 county, struct HalfShape *shape)
{
    s16 saved = shape->bend;
    u32 k;

    for (k = 0; k < (shape->isFree ? shape->length : 1); k++)
    {
        if (shape->isFree)
            shape->bend = shape->u0 + shape->step * (1 + k);
        if (!HalfHitsOtherLocation(county, shape))
        {
            shape->bend = saved;
            return TRUE;
        }
    }
    shape->bend = saved;
    return FALSE;
}

static bool32 CanPlaceHalf(u32 county, const struct CountyPlan *plan, u32 slot)
{
    u32 i;

    if (HalfHitsOtherLocation(county, &plan->shape[slot]))
        return FALSE;
    for (i = 0; i < plan->count; i++)
        if (i != slot && plan->placed[i] && HalvesCross(&plan->shape[slot], &plan->shape[i]))
            return FALSE;
    return TRUE;
}

// Backtracks over the free shapes in [slot, end) in plan order. Each tries its hashed bend first,
// then bends toward the border (which hugs the county edge, where no other route runs), then toward
// its location. Finds a crossing-free set whenever one exists.
static bool32 ChooseBends(u32 county, struct CountyPlan *plan, u32 slot, u32 end)
{
    struct HalfShape *shape;
    u32 k;

    while (slot < end && !plan->shape[slot].isFree)
        slot++;
    if (slot == end)
        return TRUE;
    shape = &plan->shape[slot];
    for (k = 0; k < shape->length; k++)
    {
        u32 toBorder = shape->length - shape->defaultOffset;
        u32 offset = k < toBorder ? shape->defaultOffset + k : shape->defaultOffset - 1 - (k - toBorder);

        shape->bend = shape->u0 + shape->step * (1 + offset);
        if (!CanPlaceHalf(county, plan, slot))
            continue;
        plan->placed[slot] = TRUE;
        if (ChooseBends(county, plan, slot + 1, end))
            return TRUE;
        plan->placed[slot] = FALSE;
    }
    shape->bend = shape->u0 + shape->step * (1 + shape->defaultOffset);
    return FALSE;
}

static void RestoreDefaultBends(struct CountyPlan *plan, u32 from, u32 end)
{
    u32 i;

    for (i = from; i < end; i++)
    {
        struct HalfShape *shape = &plan->shape[i];

        if (shape->isFree)
            shape->bend = shape->u0 + shape->step * (1 + shape->defaultOffset);
    }
}

// A shared run belongs to the sharing route that opens first, ties to the lower route id, so the
// road is walkable before any later route's lock.
static void AssignTileOwners(u32 county, struct CountyPlan *plan)
{
    u32 left, top, right, bottom, slot, i;

    GetCountyRegionBounds(county, &left, &top, &right, &bottom);
    memset(plan->owner, 0xFF, sizeof(plan->owner));
    for (slot = 0; slot < plan->count; slot++)
    {
        for (i = 0; i < ShapeTileCount(&plan->shape[slot]); i++)
        {
            struct RegionTilePos tile = ShapeHalfTile(&plan->shape[slot], i);
            u8 *owner = &plan->owner[tile.x - left + (tile.y - top) * COUNTY_TILES_W];

            if (*owner == 0xFF || plan->opensAt[slot] < plan->opensAt[*owner]
                || (plan->opensAt[slot] == plan->opensAt[*owner] && plan->routeId[slot] < plan->routeId[*owner]))
                *owner = slot;
        }
    }
}

// Inter-county halves are planned first, as a set. Then the intra-county routes are fitted around
// them as a second set, so a crowded county can never make the halves give up their crossing-free
// solution. When a set has no crossing-free solution (shapes with no choice already cross, as in
// Emerald's table), its shapes keep their hashed defaults.
static void PlanCounty(u32 county, struct CountyPlan *plan)
{
    u32 neighbor, i;

    plan->count = 0;
    for (neighbor = 0; neighbor < NUM_COUNTIES && plan->count < MAX_COUNTY_NEIGHBORS; neighbor++)
    {
        if (DoCountiesHaveRoute(county, neighbor) && GetHalfShape(county, neighbor, &plan->shape[plan->count]))
        {
            plan->routeId[plan->count] = neighbor;
            plan->opensAt[plan->count] = max(GetCountyHeader(county)->storyIdx, GetCountyHeader(neighbor)->storyIdx);
            plan->placed[plan->count] = !plan->shape[plan->count].isFree;
            plan->count++;
        }
    }
    plan->interCount = plan->count;
    if (!ChooseBends(county, plan, 0, plan->interCount))
        RestoreDefaultBends(plan, 0, plan->interCount);
    for (i = 0; i < GetCountySpecialLocationCount(county) && plan->count < MAX_COUNTY_ROUTES; i++)
    {
        if (GetIntraShape(county, i, &plan->shape[plan->count]))
        {
            plan->routeId[plan->count] = ROUTE_ID_TO_SPECIAL(i);
            plan->opensAt[plan->count] = GetCountyHeader(county)->storyIdx;
            plan->count++;
        }
    }
    for (i = 0; i < plan->count; i++)
        plan->placed[i] = i < plan->interCount || !plan->shape[i].isFree;
    if (!ChooseBends(county, plan, plan->interCount, plan->count))
        RestoreDefaultBends(plan, plan->interCount, plan->count);
    AssignTileOwners(county, plan);
}

u32 GetCountyRouteHalfTileCount(u32 county, u32 neighbor)
{
    struct HalfShape shape;

    if (!DoCountiesHaveRoute(county, neighbor) || !GetHalfShape(county, neighbor, &shape))
        return 0;
    return ShapeTileCount(&shape);
}

// A walk over one route calls this once per tile, so the last county's plan is kept.
static const struct CountyPlan *GetCachedPlan(u32 county)
{
    static EWRAM_DATA struct CountyPlan sPlan;
    static EWRAM_DATA u32 sPlanCounty; // county + 1, so zero means empty
    static EWRAM_DATA u32 sPlanSeed;
    u32 seed = GetPlayerIdentitySeed();

    if (sPlanCounty != county + 1 || sPlanSeed != seed)
    {
        PlanCounty(county, &sPlan);
        sPlanCounty = county + 1;
        sPlanSeed = seed;
    }
    return &sPlan;
}

static const struct HalfShape *FindShape(u32 county, u32 routeId)
{
    const struct CountyPlan *plan = GetCachedPlan(county);
    u32 i;

    for (i = 0; i < plan->count; i++)
        if (plan->routeId[i] == routeId)
            return &plan->shape[i];
    return NULL;
}

struct RegionTilePos GetCountyRouteHalfTile(u32 county, u32 neighbor, u32 index)
{
    const struct HalfShape *shape = FindShape(county, neighbor);

    AGB_ASSERT(shape != NULL && index < ShapeTileCount(shape));
    if (shape == NULL)
        return GetLocationPos(county, 0);
    return ShapeHalfTile(shape, index);
}

u32 GetCountyIntraRouteTileCount(u32 county, u32 specialIndex)
{
    const struct HalfShape *shape = FindShape(county, ROUTE_ID_TO_SPECIAL(specialIndex));

    return shape == NULL ? 0 : ShapeTileCount(shape);
}

struct RegionTilePos GetCountyIntraRouteTile(u32 county, u32 specialIndex, u32 index)
{
    const struct HalfShape *shape = FindShape(county, ROUTE_ID_TO_SPECIAL(specialIndex));

    AGB_ASSERT(shape != NULL && index < ShapeTileCount(shape));
    if (shape == NULL)
        return GetLocationPos(county, 0);
    return ShapeHalfTile(shape, index);
}

u32 GetRegionTileRouteId(u32 x, u32 y)
{
    u32 county = GetCountyFromRegionCell(x, y);
    const struct CountyPlan *plan = GetCachedPlan(county);
    u32 left, top, right, bottom, i, slot;

    for (i = 0; i < GetLocationCount(county); i++)
    {
        struct RegionTilePos pos = GetLocationPos(county, i);

        if (pos.x == x && pos.y == y)
            return ROUTE_ID_NONE;
    }
    GetCountyRegionBounds(county, &left, &top, &right, &bottom);
    slot = plan->owner[x - left + (y - top) * COUNTY_TILES_W];
    return slot == 0xFF ? ROUTE_ID_NONE : plan->routeId[slot];
}
