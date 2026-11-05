#include "global.h"
#include "identity_seed.h"
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
    key->implicit = PLOT_IMPLICIT_NONE;
    key->mcguffinId = GetCountyMcGuffinId(county, specialIndex);
    return TRUE;
}

static u32 Gcd(u32 a, u32 b)
{
    while (b != 0)
    {
        u32 t = a % b;

        a = b;
        b = t;
    }
    return a;
}

u32 GetCountyMcGuffinId(u32 countyId, u32 index)
{
    u32 stride = 1 + IdentityHash(GetPlayerIdentitySeed(), REGION_SALT_MCGUFFIN_STRIDE, 0) % (NUM_COUNTY_MCGUFFIN_IDS - 1);

    AGB_ASSERT(countyId < NUM_COUNTIES && index < MCGUFFIN_IDS_PER_COUNTY);
    while (Gcd(stride, NUM_COUNTY_MCGUFFIN_IDS) != 1)
        stride = stride % (NUM_COUNTY_MCGUFFIN_IDS - 1) + 1;
    return (countyId * MCGUFFIN_IDS_PER_COUNTY + index) * stride % NUM_COUNTY_MCGUFFIN_IDS;
}

u32 GetPlotItemMcGuffinId(u32 kind, u32 ordinal)
{
    return NUM_COUNTY_MCGUFFIN_IDS
        + IdentityHash(GetPlayerIdentitySeed(), REGION_SALT_MCGUFFIN_CROSS, kind << 8 | ordinal) % NUM_CROSS_MCGUFFIN_IDS;
}
