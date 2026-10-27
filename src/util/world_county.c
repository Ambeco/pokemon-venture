#include "global.h"
#include "constants/vars.h"
#include "world_county.h"

#include "data/world/emerald_world.h"

STATIC_ASSERT(NUM_COUNTIES == NUM_COUNTY_VAR_SLICES, CountyVarSlicesMatchCounties);

// Region map size (MAP_WIDTH/MAP_HEIGHT in region_map.c).
#define REGION_W 28
#define REGION_H 15

const struct WorldHeader *GetWorldHeader(void)
{
    return &sEmeraldWorld;
}

const struct CountyHeader *GetCountyHeader(u32 countyId)
{
    AGB_ASSERT(countyId < NUM_COUNTIES);
    return &GetWorldHeader()->counties[countyId];
}

u32 GetCountyBiome(u32 countyId)
{
    const struct WorldHeader *world = GetWorldHeader();
    u32 local = GetCountyHeader(countyId)->biome;

    return BIOME_GRID(world->biomeHumidityBase + local / BIOME_SUBSET_TEMPERATURE_LEVELS,
                      world->biomeTemperatureBase + local % BIOME_SUBSET_TEMPERATURE_LEVELS);
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

u32 GetEliteFourStoryIdx(void)
{
    return NUM_COUNTIES - 1 - GetWorldHeader()->reservedCount;
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
