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
    struct PlotSlot get;

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
            if (GetChapterSuppliedSlot(GetCountyChapter(county, plan.chain[i].chapterSlot), &get))
                EXPECT_EQ(plan.chain[i].mainSlot, get.step);
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

TEST("CountyPlan: The Cutter's House takes the slot of HM Cut, and Rock Smash the slot of HM Rock Smash")
{
    struct CountyChainPlan plan;
    u32 i, found = 0;

    EXPECT(PlanEmeraldCountyChain(COUNTY_ID(0, 1), &plan));
    for (i = 0; i < plan.numEntries; i++)
    {
        if (plan.chain[i].chapterSlot == 0)
        {
            EXPECT_EQ(plan.chain[i].mainSlot, GetVanillaHmSlot(PLOT_HM_CUT).step);
            found++;
        }
    }
    EXPECT_EQ(found, 1);
    EXPECT(PlanEmeraldCountyChain(MAUVILLE, &plan));
    for (i = 0; i < plan.numEntries; i++)
    {
        if (plan.chain[i].chapterSlot == 0)
            EXPECT_EQ(plan.chain[i].mainSlot, GetVanillaHmSlot(PLOT_HM_ROCK_SMASH).step);
    }
}

static const struct ChapterDesc sFillerChapter = {.numStages = 1};

// A county with a utility, scripted or rival get in a slot that is not its first slot with room, so first-fit would misplace
// the chapter supplying it.
static bool32 FindCountyWithLateGet(u32 *countyOut, struct PlotKeyRef *getOut, u32 *slotOut)
{
    static const u8 kinds[] = {PLOT_KIND_UTILITY, PLOT_KIND_SCRIPTED, PLOT_KIND_RIVAL};
    u32 county, k, ordinal, s;

    for (county = 0; county < NUM_COUNTIES; county++)
    {
        struct SubplotSlotRule rules[COUNTY_MAIN_SLOTS];

        GetCountySlotRules(county, rules);
        for (k = 0; k < ARRAY_COUNT(kinds); k++)
        {
            u32 first = kinds[k] == PLOT_KIND_RIVAL ? 1 : 0;

            for (ordinal = first; ordinal < first + GetPlotKindCount(kinds[k]); ordinal++)
            {
                struct PlotSlot get = GetVanillaPlotSlot(kinds[k], ordinal);

                if (get.block != GetCountyHeader(county)->storyIdx || get.step >= COUNTY_MAIN_SLOTS || rules[get.step].max == 0)
                    continue;
                for (s = 0; s < get.step; s++)
                {
                    if (rules[s].max > 0)
                    {
                        *countyOut = county;
                        getOut->kind = kinds[k];
                        getOut->ordinal = ordinal;
                        *slotOut = get.step;
                        return TRUE;
                    }
                }
            }
        }
    }
    return FALSE;
}

TEST("CountyPlan: An entry supplying a get is bound to the get's slot, whether its subplot comes before or after an unbound one")
{
    u32 county = 0, slot = 0, order;
    struct PlotKeyRef get = {0};
    struct ChapterDesc getChapter = {.numStages = 1};
    static const struct SubplotEntry fillerEntries[] = {{&sFillerChapter, FALSE}};
    static const struct SubplotDesc fillerSubplot = {1, fillerEntries};
    struct SubplotEntry hmEntries[1];
    struct SubplotDesc hmSubplot = {1, hmEntries};

    EXPECT(FindCountyWithLateGet(&county, &get, &slot));
    getChapter.producesMainGet = TRUE;
    getChapter.mainGet = get;
    hmEntries[0].chapter = &getChapter;
    hmEntries[0].optional = FALSE;
    for (order = 0; order < 2; order++)
    {
        struct ChapterHost hosts[2];
        struct SubplotHost subplots[2];
        struct CountyChainPlan plan;
        u32 i, hmMainSlot = 0xFF;

        hosts[0] = (struct ChapterHost){county, 0, &sFillerChapter};
        hosts[1] = (struct ChapterHost){county, 1, &getChapter};
        subplots[order] = (struct SubplotHost){county, &fillerSubplot};
        subplots[1 - order] = (struct SubplotHost){county, &hmSubplot};
        EXPECT(PlanCountyChain(county, subplots, 2, hosts, 2, &plan));
        for (i = 0; i < plan.numEntries; i++)
        {
            if (plan.chain[i].chapterSlot == 1)
                hmMainSlot = plan.chain[i].mainSlot;
        }
        EXPECT_EQ(hmMainSlot, slot);
    }
}
