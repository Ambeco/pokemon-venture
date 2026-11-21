#include "global.h"
#include "test/test.h"
#include "chapter.h"
#include "county_plan.h"
#include "county_chain.h"
#include "region_county.h"
#include "subplot.h"

#define MAUVILLE COUNTY_ID(2, 1)
#define SLATEPORT COUNTY_ID(2, 3)
#define LITTLEROOT COUNTY_ID(1, 3)
#define OLDALE COUNTY_ID(1, 2)

TEST("CountyPlan: A callback hosted by another county and an unlock step take no chapters")
{
    struct SubplotSlotRule rules[COUNTY_MAIN_SLOTS];

    GetCountySlotRules(OLDALE, rules);
    EXPECT_EQ(rules[1].max, 0);
    EXPECT_EQ(rules[3].max, 0);
    GetCountySlotRules(MAUVILLE, rules);
    EXPECT_EQ(rules[1].max, 0);
    EXPECT_EQ(rules[3].max, 0);
}

TEST("CountyPlan: Every Emerald county plans, and its chain and off-chain slots cover exactly its hosted chapters")
{
    u32 numHosts, county, i, hosted, covered;
    const struct ChapterHost *hosts = GetChapterHosts(&numHosts);

    for (county = 0; county < NUM_COUNTIES; county++)
    {
        struct CountyChainPlan plan;
        bool8 seen[CHAPTERS_PER_COUNTY] = {FALSE};

        hosted = 0;
        for (i = 0; i < numHosts; i++)
        {
            if (hosts[i].county == county)
                hosted++;
        }
        EXPECT(PlanEmeraldCountyChain(county, &plan));
        EXPECT_EQ(plan.problem, SUBPLOT_PLAN_OK);
        covered = plan.numEntries + plan.numOffChain;
        EXPECT_EQ(covered, hosted);
        for (i = 0; i < plan.numEntries; i++)
        {
            EXPECT(GetCountyChapter(county, plan.chain[i].chapterSlot) != NULL);
            EXPECT(!seen[plan.chain[i].chapterSlot]);
            seen[plan.chain[i].chapterSlot] = TRUE;
            EXPECT_EQ(plan.chain[i].numStages, GetCountyChapter(county, plan.chain[i].chapterSlot)->numStages);
            EXPECT(i == 0 || plan.chain[i].mainSlot >= plan.chain[i - 1].mainSlot);
        }
        for (i = 0; i < plan.numOffChain; i++)
        {
            EXPECT(GetCountyChapter(county, plan.offChainSlot[i]) != NULL);
            EXPECT(!seen[plan.offChainSlot[i]]);
            seen[plan.offChainSlot[i]] = TRUE;
        }
    }
}

TEST("CountyPlan: Mauville's Rock Smash and Rydel's Cycles are on the chain in slot 2; the optional Coin Case need not be")
{
    struct CountyChainPlan plan;
    u32 i;
    bool32 smash = FALSE, bike = FALSE;

    EXPECT(PlanEmeraldCountyChain(MAUVILLE, &plan));
    EXPECT_EQ(plan.numEntries, 2);
    EXPECT_EQ(plan.numOffChain, 1);
    EXPECT_EQ(plan.offChainSlot[0], 1);
    for (i = 0; i < plan.numEntries; i++)
    {
        EXPECT_EQ(plan.chain[i].mainSlot, 2);
        smash |= plan.chain[i].chapterSlot == 0;
        bike |= plan.chain[i].chapterSlot == 2;
    }
    EXPECT(smash);
    EXPECT(bike);
}

TEST("CountyPlan: Slateport's optional Harbor Mail has no slot with gets, so it runs beside the chain")
{
    struct CountyChainPlan plan;

    EXPECT(PlanEmeraldCountyChain(SLATEPORT, &plan));
    EXPECT_EQ(plan.numEntries, 0);
    EXPECT_EQ(plan.numOffChain, 1);
    EXPECT_EQ(plan.offChainSlot[0], 0);
}

TEST("CountyPlan: The Littleroot tutorial stays one ordered subplot, in slot 0 with slots kept")
{
    struct CountyChainPlan plan;

    EXPECT(PlanEmeraldCountyChain(LITTLEROOT, &plan));
    EXPECT_EQ(plan.numEntries, 2);
    EXPECT_EQ(plan.chain[0].chapterSlot, 0);
    EXPECT_EQ(plan.chain[1].chapterSlot, 1);
    EXPECT_EQ(plan.chain[0].mainSlot, 0);
    EXPECT_EQ(plan.chain[1].mainSlot, 0);
}

TEST("CountyPlan: The Old Rod and the Super Rod sit in the slot their utility gets are in")
{
    struct CountyChainPlan plan;

    EXPECT(PlanEmeraldCountyChain(COUNTY_ID(0, 3), &plan));
    EXPECT_EQ(plan.numEntries, 1);
    EXPECT_EQ(plan.chain[0].mainSlot, 0);
    EXPECT(PlanEmeraldCountyChain(COUNTY_ID(5, 1), &plan));
    EXPECT_EQ(plan.numEntries, 1);
    EXPECT_EQ(plan.chain[0].mainSlot, 0);
}
