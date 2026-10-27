#include "global.h"
#include "constants/vars.h"
#include "region_county.h"

#include "data/region/emerald_region.h"

STATIC_ASSERT(NUM_COUNTIES == NUM_COUNTY_VAR_SLICES, CountyVarSlicesMatchCounties);

const struct RegionHeader *GetRegionHeader(void)
{
    return &sEmeraldRegion;
}

const struct CountyHeader *GetCountyHeader(u32 countyId)
{
    AGB_ASSERT(countyId < NUM_COUNTIES);
    return &GetRegionHeader()->counties[countyId];
}

u32 GetCountyBiome(u32 countyId)
{
    const struct RegionHeader *region = GetRegionHeader();
    u32 local = GetCountyHeader(countyId)->biome;

    return BIOME_GRID(region->biomeHumidityBase + local / BIOME_SUBSET_TEMPERATURE_LEVELS,
                      region->biomeTemperatureBase + local % BIOME_SUBSET_TEMPERATURE_LEVELS);
}

u32 GetCountyByStoryIdx(u32 idx)
{
    u32 i;

    for (i = 0; i < NUM_COUNTIES; i++)
    {
        if (GetCountyHeader(i)->storyIdx == idx)
            return i;
    }
    AGB_ASSERT(FALSE);
    return 0;
}

u32 GetCountyLockId(u32 countyId)
{
    u32 idx = GetCountyHeader(countyId)->storyIdx;

    return idx == 0 ? 0 : GetCountyHeader(GetCountyByStoryIdx(idx - 1))->keyId;
}

u32 GetEliteFourStoryIdx(void)
{
    return NUM_COUNTIES - 1 - GetRegionHeader()->reservedCount;
}

bool32 IsCountyReserved(u32 countyId)
{
    return GetCountyHeader(countyId)->storyIdx > GetEliteFourStoryIdx();
}

bool32 IsCountyEliteFour(u32 countyId)
{
    return GetCountyHeader(countyId)->storyIdx == GetEliteFourStoryIdx();
}

bool32 AreCountiesAdjacent(u32 a, u32 b)
{
    u32 dx = COUNTY_X(a) > COUNTY_X(b) ? COUNTY_X(a) - COUNTY_X(b) : COUNTY_X(b) - COUNTY_X(a);
    u32 dy = COUNTY_Y(a) > COUNTY_Y(b) ? COUNTY_Y(a) - COUNTY_Y(b) : COUNTY_Y(b) - COUNTY_Y(a);

    return dx + dy == 1;
}

// The 30x16 tile grid has its first and last columns and its first row off limits, so region cell
// (x, y) is grid tile (x + 1, y + 1).
void GetCountyRegionBounds(u32 countyId, u32 *left, u32 *top, u32 *right, u32 *bottom)
{
    u32 gridLeft = COUNTY_X(countyId) * COUNTY_TILES_W;
    u32 gridTop = COUNTY_Y(countyId) * COUNTY_TILES_H;
    u32 gridRight = gridLeft + COUNTY_TILES_W - 1;
    u32 gridBottom = gridTop + COUNTY_TILES_H - 1;

    *left = gridLeft == 0 ? 0 : gridLeft - 1;
    *top = gridTop == 0 ? 0 : gridTop - 1;
    *right = min(gridRight - 1, REGION_W - 1);
    *bottom = min(gridBottom - 1, REGION_H - 1);
}

u32 GetCountyFromRegionCell(u32 x, u32 y)
{
    AGB_ASSERT(x < REGION_W && y < REGION_H);
    return COUNTY_ID((x + 1) / COUNTY_TILES_W, (y + 1) / COUNTY_TILES_H);
}
