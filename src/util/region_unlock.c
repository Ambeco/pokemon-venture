#include "global.h"
#include "region_county.h"
#include "region_route.h"
#include "region_unlock.h"

#include "data/region/emerald_unlocks.h"

struct PlotUnlock GetPlotBlockUnlock(u32 block)
{
    AGB_ASSERT(block < ARRAY_COUNT(sEmeraldUnlocks)); // valid blocks are 0..E
    return sEmeraldUnlocks[block < ARRAY_COUNT(sEmeraldUnlocks) ? block : ARRAY_COUNT(sEmeraldUnlocks) - 1];
}

u32 GetCountyUnlockBlock(u32 countyId)
{
    u32 storyIdx = GetCountyHeader(countyId)->storyIdx;
    u32 elite = GetEliteFourStoryIdx();

    AGB_ASSERT(storyIdx != 0); // the starting county has no unlock: check GetCountyUnlockIndex first
    if (storyIdx == 0)
        return 0;
    return storyIdx - 1 < elite ? storyIdx - 1 : elite;
}

bool32 GetCountyUnlockIndex(u32 countyId, u32 *index)
{
    if (GetCountyHeader(countyId)->storyIdx == 0)
        return FALSE;
    *index = 4 * GetCountyUnlockBlock(countyId) + 3;
    return TRUE;
}

bool32 IsCountyUnlockedAt(u32 countyId, u32 mainStoryIndex)
{
    u32 unlockIndex;

    return !GetCountyUnlockIndex(countyId, &unlockIndex) || mainStoryIndex > unlockIndex;
}

bool32 GetRouteLockKey(u32 countyA, u32 countyB, struct RegionTilePos *pos, u32 *gatedCounty, struct PlotUnlock *key)
{
    if (!GetRouteLock(countyA, countyB, pos, gatedCounty))
        return FALSE;
    *key = GetPlotBlockUnlock(GetCountyUnlockBlock(*gatedCounty));
    return TRUE;
}

bool32 GetCountyIntraRouteLockKey(u32 county, u32 specialIndex, struct RegionTilePos *pos, bool32 *gatesCity, struct PlotUnlock *key)
{
    if (!GetCountyIntraRouteLock(county, specialIndex, pos, gatesCity))
        return FALSE;
    key->type = PLOT_UNLOCK_COUNTY_PRIVATE;
    key->numKeys = 0;
    return TRUE;
}
