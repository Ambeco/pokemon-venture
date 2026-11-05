#include "global.h"
#include "test/test.h"
#include "test/seed_sweep.h"
#include "plot_stage.h"
#include "region_county.h"
#include "region_route.h"
#include "region_unlock.h"

TEST("RegionUnlock: Every key is a vanilla item of its own block, before the unlock step")
{
    u32 block, i;

    for (block = 0; block <= GetEliteFourStoryIdx(); block++)
    {
        struct PlotUnlock unlock = GetPlotBlockUnlock(block);

        EXPECT_EQ(unlock.type == PLOT_UNLOCK_KEYS, unlock.numKeys != 0);
        EXPECT_LE(unlock.numKeys, PLOT_MAX_UNLOCK_KEYS);
        for (i = 0; i < unlock.numKeys; i++)
        {
            struct PlotSlot slot = GetVanillaPlotSlot(unlock.keys[i].kind, unlock.keys[i].ordinal);

            EXPECT_EQ(slot.block, block);
            EXPECT_LT(4 * slot.block + slot.step, 4 * block + 3);
        }
    }
}

TEST("RegionUnlock: Emerald's blockers are blocks 9 and 16, and Fallarbor is implicit")
{
    u32 block;

    for (block = 0; block <= GetEliteFourStoryIdx(); block++)
    {
        u32 type = GetPlotBlockUnlock(block).type;

        if (block == 9 || block == 16)
            EXPECT_EQ(type, PLOT_UNLOCK_BLOCKER);
        else if (block == 8)
        {
            EXPECT_EQ(type, PLOT_UNLOCK_IMPLICIT);
            EXPECT_EQ(GetPlotBlockUnlock(block).implicit, PLOT_IMPLICIT_FIERY_PATH_ENTERED);
        }
        else
            EXPECT_EQ(type, PLOT_UNLOCK_KEYS);
    }
}

TEST("RegionUnlock: A county unlocks at 4 * (storyIdx - 1) + 3; reserved counties at 4E + 3")
{
    u32 id, index;
    u32 elite = GetEliteFourStoryIdx();

    for (id = 0; id < NUM_COUNTIES; id++)
    {
        u32 storyIdx = GetCountyHeader(id)->storyIdx;

        if (storyIdx == 0)
        {
            EXPECT(!GetCountyUnlockIndex(id, &index));
            EXPECT(IsCountyUnlockedAt(id, 0));
            continue;
        }
        EXPECT(GetCountyUnlockIndex(id, &index));
        EXPECT_EQ(index, 4 * (storyIdx <= elite ? storyIdx - 1 : elite) + 3);
        EXPECT(!IsCountyUnlockedAt(id, index));
        EXPECT(IsCountyUnlockedAt(id, index + 1));
    }
}

TEST("RegionUnlock: Every Enclave unlocks before the post-game index, so the story can pass it")
{
    u32 id, i, enclaves = 0;

    for (id = 0; id < NUM_COUNTIES; id++)
    {
        u32 index;

        for (i = 0; i < GetCountySpecialLocationCount(id); i++)
        {
            if (GetCountySpecialLocationClass(id, i) != LOCATION_CLASS_ENCLAVE)
                continue;
            enclaves++;
            if (GetCountyUnlockIndex(id, &index))
                EXPECT_LT(index, MAIN_STORY_INDEX_POSTGAME);
            EXPECT(IsCountyUnlockedAt(id, MAIN_STORY_INDEX_POSTGAME));
        }
    }
    EXPECT_EQ(enclaves, 4);
}

TEST("RegionUnlock: A route lock carries the unlock of the county it gates")
{
    u32 a, b, locks = 0, open = 0, blockers = 0;

    for (a = 0; a < NUM_COUNTIES; a++)
    {
        for (b = a + 1; b < NUM_COUNTIES; b++)
        {
            struct RegionTilePos pos, plain;
            struct PlotUnlock key;
            u32 gated, plainGated;

            if (!GetRouteLockKey(a, b, &pos, &gated, &key))
            {
                EXPECT(!GetRouteLock(a, b, &plain, &plainGated));
                continue;
            }
            locks++;
            EXPECT(GetRouteLock(a, b, &plain, &plainGated));
            EXPECT_EQ(plain.x, pos.x);
            EXPECT_EQ(plain.y, pos.y);
            EXPECT_EQ(plainGated, gated);
            EXPECT_EQ(key.type, GetPlotBlockUnlock(GetCountyHeader(gated)->storyIdx - 1).type);
            EXPECT_NE(key.type, PLOT_UNLOCK_COUNTY_PRIVATE);
            open += key.type == PLOT_UNLOCK_OPEN;
            blockers += key.type == PLOT_UNLOCK_BLOCKER;
        }
    }
    EXPECT_GT(locks, 0);
    EXPECT_GT(open + blockers, 0);
    (void)open;
}

TEST("RegionUnlock: Intra-county locks are county-private")
{
    u32 id, i, found = 0;

    for (id = 0; id < NUM_COUNTIES; id++)
    {
        for (i = 0; i < GetCountySpecialLocationCount(id); i++)
        {
            struct RegionTilePos pos;
            bool32 gatesCity;
            struct PlotUnlock key;

            if (GetCountyIntraRouteTileCount(id, i) == 0 || !GetCountyIntraRouteLockKey(id, i, &pos, &gatesCity, &key))
                continue;
            found++;
            EXPECT_EQ(key.type, PLOT_UNLOCK_COUNTY_PRIVATE);
            EXPECT_EQ(key.numKeys, 0);
            EXPECT_LT(key.mcguffinId, NUM_COUNTY_MCGUFFIN_IDS);
        }
    }
    EXPECT_GT(found, 0);
}

TEST("RegionUnlock: County McGuffin pools are disjoint, in range, and clear of cross-county ids")
{
    u32 i, seed;

    FOR_EACH_SWEEP_SEED(i, seed)
    {
        u8 used[NUM_COUNTY_MCGUFFIN_IDS] = {0};
        u32 county, k, kind;

        for (county = 0; county < NUM_COUNTIES; county++)
        {
            for (k = 0; k < MCGUFFIN_IDS_PER_COUNTY; k++)
            {
                u32 id = GetCountyMcGuffinId(county, k);

                EXPECT_LT(id, NUM_COUNTY_MCGUFFIN_IDS);
                EXPECT_EQ(used[id], 0);
                used[id] = 1;
            }
        }
        for (kind = 0; kind < NUM_PLOT_KINDS; kind++)
        {
            u32 id = GetPlotItemMcGuffinId(kind, 3);

            EXPECT_GE(id, NUM_COUNTY_MCGUFFIN_IDS);
            EXPECT_LT(id, NUM_COUNTY_MCGUFFIN_IDS + NUM_CROSS_MCGUFFIN_IDS);
            EXPECT_EQ(id, GetPlotItemMcGuffinId(kind, 3));
        }
    }
}

TEST("RegionUnlock: A county's pool varies with the seed")
{
    u32 i, seed, first = 0, different = 0;

    FOR_EACH_SWEEP_SEED(i, seed)
    {
        u32 id = GetCountyMcGuffinId(5, 2);

        if (i == 0)
            first = id;
        else
            different += id != first;
    }
    EXPECT_GT(different, 0);
}
