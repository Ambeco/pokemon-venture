#include "global.h"
#include "test/test.h"
#include "constants/flags.h"
#include "constants/vars.h"
#include "event_data.h"
#include "region_city.h"
#include "chapter.h"
#include "county_plan.h"
#include "county_chain.h"
#include "region_county.h"
#include "region_plot.h"
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
        bool8 seen[COUNTY_MAX_CHAPTER_SLOTS] = {FALSE};

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
            ASSUME(plan.chainDesc[i] != NULL);
            if (plan.chain[i].chapterSlot == CHAPTER_SLOT_STATELESS)
            {
                EXPECT(plan.chainDesc[i]->backend == CHAPTER_PROXY || plan.chainDesc[i]->backend == CHAPTER_ADAPTER);
            }
            else
            {
                EXPECT(GetCountyChapter(county, plan.chain[i].chapterSlot) == plan.chainDesc[i]);
                EXPECT(!seen[plan.chain[i].chapterSlot]);
                seen[plan.chain[i].chapterSlot] = TRUE;
            }
            EXPECT_EQ(plan.chain[i].numAdvances, plan.chainDesc[i]->numStages - 1);
            EXPECT(i == 0 || plan.chain[i].mainSlot >= plan.chain[i - 1].mainSlot);
            if (GetChapterSuppliedSlot(plan.chainDesc[i], &get))
            {
                bool32 isCallback = get.block != (u32)GetCountyHeader(county)->storyIdx;

                EXPECT_EQ(plan.chain[i].mainSlot, isCallback ? CHAPTER_MAIN_SLOT_CALLBACK(get.block) : get.step);
            }
        }
        for (i = 0; i < plan.numOffChain; i++)
        {
            if (plan.offChainSlot[i] == CHAPTER_SLOT_STATELESS)
                continue;
            EXPECT(GetCountyChapter(county, plan.offChainSlot[i]) != NULL);
            EXPECT(!seen[plan.offChainSlot[i]]);
            seen[plan.offChainSlot[i]] = TRUE;
        }
    }
}

TEST("CountyPlan: Mauville's chain holds Rock Smash, Rydel's proxy and the gym in slot 2; the Coin Case, Rydel's Cycles and New Mauville run beside it")
{
    struct CountyChainPlan plan;
    u32 i;
    bool32 smash = FALSE, proxy = FALSE, gym = FALSE;

    EXPECT(PlanEmeraldCountyChain(MAUVILLE, &plan));
    EXPECT_EQ(plan.numEntries, 3);
    EXPECT_EQ(plan.numOffChain, 3);
    EXPECT_EQ(plan.offChainSlot[0], 1);
    EXPECT_EQ(plan.offChainSlot[1], 2);
    EXPECT_EQ(plan.offChainSlot[2], CHAPTER_SLOT_STATELESS); // New Mauville
    for (i = 0; i < plan.numEntries; i++)
    {
        EXPECT_EQ(plan.chain[i].mainSlot, 2);
        smash |= plan.chain[i].chapterSlot == 0;
        proxy |= plan.chain[i].chapterSlot == CHAPTER_SLOT_STATELESS && plan.chainDesc[i]->backend == CHAPTER_PROXY;
        gym |= plan.chain[i].chapterSlot == CHAPTER_SLOT_STATELESS && plan.chainDesc[i]->backend == CHAPTER_ADAPTER;
    }
    EXPECT(gym);
    EXPECT(smash);
    EXPECT(proxy);
}

TEST("CountyPlan: Slateport's optional Harbor Mail has no slot with gets, so it runs beside the chain, and only the Shipyard get is on it")
{
    struct CountyChainPlan plan;

    EXPECT(PlanEmeraldCountyChain(SLATEPORT, &plan));
    EXPECT_EQ(plan.numEntries, 1);
    EXPECT_EQ(plan.numOffChain, 1);
    EXPECT_EQ(plan.offChainSlot[0], 0);
}

TEST("CountyPlan: The Littleroot tutorial stays one ordered subplot, in slot 0 with slots kept, then the Starter and block 1's three callback gets")
{
    struct CountyChainPlan plan;
    u32 i;

    EXPECT(PlanEmeraldCountyChain(LITTLEROOT, &plan));
    EXPECT_EQ(plan.numEntries, 6);
    EXPECT_EQ(plan.chain[0].chapterSlot, 0);
    EXPECT_EQ(plan.chain[1].chapterSlot, 1);
    EXPECT_EQ(plan.chain[0].mainSlot, 0);
    EXPECT_EQ(plan.chain[1].mainSlot, 0);
    EXPECT_EQ(plan.chain[2].mainSlot, 0);
    for (i = 3; i < 6; i++)
        EXPECT_EQ(plan.chain[i].mainSlot, CHAPTER_MAIN_SLOT_CALLBACK(1));
}

TEST("CountyPlan: The Old Rod and the Super Rod sit in the slot their utility gets are in, beside their gyms")
{
    struct CountyChainPlan plan;

    EXPECT(PlanEmeraldCountyChain(COUNTY_ID(0, 3), &plan));
    EXPECT_EQ(plan.numEntries, 2);
    EXPECT_EQ(plan.chain[0].mainSlot, 0);
    EXPECT_EQ(plan.chain[1].mainSlot, 0);
    EXPECT(PlanEmeraldCountyChain(COUNTY_ID(5, 1), &plan));
    EXPECT_EQ(plan.numEntries, 2);
    EXPECT_EQ(plan.chain[0].mainSlot, 0);
    EXPECT_EQ(plan.chain[1].mainSlot, 0);
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

static const struct ChapterDesc sFillerChapter = {.numStages = 2};

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
    struct ChapterDesc getChapter = {.numStages = 2};
    static const struct SubplotEntry fillerEntries[] = {{&sFillerChapter, FALSE}};
    static const struct SubplotDesc fillerSubplot = {1, fillerEntries};
    struct SubplotEntry hmEntries[1];
    struct SubplotDesc hmSubplot = {1, hmEntries};
    struct ChapterDesc gymChapter = {.numStages = 2};
    struct SubplotEntry gymEntries[1] = {{&gymChapter, FALSE}};
    struct SubplotDesc gymSubplot = {1, gymEntries};
    u32 ordinal = 0;

    EXPECT(FindCountyWithLateGet(&county, &get, &slot));
    ordinal = GetCountyGymOrdinal(county);
    gymChapter.producesMainGet = TRUE;
    gymChapter.mainGet = (struct PlotKeyRef){PLOT_KIND_GYM, ordinal};
    getChapter.producesMainGet = TRUE;
    getChapter.mainGet = get;
    hmEntries[0].chapter = &getChapter;
    hmEntries[0].optional = FALSE;
    for (order = 0; order < 2; order++)
    {
        struct ChapterHost hosts[3];
        struct SubplotHost subplots[3];
        struct CountyChainPlan plan;
        u32 i, hmMainSlot = 0xFF, num = 2; // a gym county's gym slot needs its own gym chapter

        hosts[0] = (struct ChapterHost){county, 0, &sFillerChapter};
        hosts[1] = (struct ChapterHost){county, 1, &getChapter};
        subplots[order] = (struct SubplotHost){county, &fillerSubplot};
        subplots[1 - order] = (struct SubplotHost){county, &hmSubplot};
        if (ordinal >= 1 && ordinal < ELITE_FOUR_ORDINAL)
        {
            hosts[2] = (struct ChapterHost){county, CHAPTER_SLOT_STATELESS, &gymChapter};
            subplots[2] = (struct SubplotHost){county, &gymSubplot};
            num = 3;
        }
        EXPECT(PlanCountyChain(county, subplots, num, hosts, num, &plan));
        for (i = 0; i < plan.numEntries; i++)
        {
            if (plan.chain[i].chapterSlot == 1)
                hmMainSlot = plan.chain[i].mainSlot;
        }
        EXPECT_EQ(hmMainSlot, slot);
    }
}

static const struct ChapterDesc sParallelChapters[COUNTY_MAX_CHAPTER_SLOTS + 1] = {[0 ... COUNTY_MAX_CHAPTER_SLOTS] = {.numStages = 2}};

#define PARALLEL_ENTRY(i) static const struct SubplotEntry sParallelEntries##i[] = {{&sParallelChapters[i], FALSE}};
PARALLEL_ENTRY(0)
PARALLEL_ENTRY(1)
PARALLEL_ENTRY(2)
PARALLEL_ENTRY(3)

TEST("CountyPlan: Slot capacity is per subplot: more subplots than the slot's max all take the same slot, and its minimum stays shared")
{
    static const struct SubplotDesc subplots[] =
    {
        {1, sParallelEntries0}, {1, sParallelEntries1}, {1, sParallelEntries2}, {1, sParallelEntries3},
    };
    struct SubplotHost subplotHosts[4];
    struct ChapterHost hosts[4];
    struct CountyChainPlan plan;
    u32 county, s, i, firstSlot = COUNTY_MAIN_SLOTS;

    for (county = 0; county < NUM_COUNTIES && firstSlot == COUNTY_MAIN_SLOTS; county++)
    {
        struct SubplotSlotRule rules[COUNTY_MAIN_SLOTS];
        u32 minimums = 0;

        GetCountySlotRules(county, rules);
        for (s = 0; s < COUNTY_MAIN_SLOTS; s++)
            minimums += rules[s].min;
        for (s = 0; s < COUNTY_MAIN_SLOTS && minimums == 0; s++)
        {
            if (rules[s].max > 0)
            {
                firstSlot = s;
                break;
            }
        }
        if (firstSlot != COUNTY_MAIN_SLOTS)
            break;
    }
    ASSUME(firstSlot != COUNTY_MAIN_SLOTS);
    EXPECT_GT(ARRAY_COUNT(subplots), GetCountyChapterSlotCount());
    for (i = 0; i < ARRAY_COUNT(subplots); i++)
    {
        hosts[i] = (struct ChapterHost){county, CHAPTER_SLOT_STATELESS, &sParallelChapters[i]};
        subplotHosts[i] = (struct SubplotHost){county, &subplots[i]};
    }
    EXPECT(PlanCountyChain(county, subplotHosts, ARRAY_COUNT(subplots), hosts, ARRAY_COUNT(hosts), &plan));
    EXPECT_EQ(plan.numEntries, ARRAY_COUNT(subplots));
    for (i = 0; i < plan.numEntries; i++)
        EXPECT_EQ(plan.chain[i].mainSlot, firstSlot);
}

TEST("CountyPlan: A main-story slot is done when all its on-chain entries are, in any order, and a proxy is done with its subplot")
{
    EXPECT(!IsCountyMainSlotDone(MAUVILLE, 2));
    SetCountySlotStage(MAUVILLE, 2, 1);
    EXPECT(IsCountyChainEntryDone(MAUVILLE, 2));
    EXPECT(!IsCountyMainSlotDone(MAUVILLE, 2));
    SetCountySlotStage(MAUVILLE, 0, 1);
    EXPECT(!IsCountyMainSlotDone(MAUVILLE, 2));
    FlagSet(FLAG_BADGE03_GET);
    EXPECT(IsCountyMainSlotDone(MAUVILLE, 2));
}

TEST("CountyPlan: Rock Smash alone does not finish Mauville's slot 2 while Rydel's Cycles is open")
{
    SetCountySlotStage(MAUVILLE, 0, 1);
    EXPECT(!IsCountyMainSlotDone(MAUVILLE, 2));
}

TEST("CountyPlan: Finishing a county's current main-story slot through SetCountySlotStage advances the index once")
{
    u32 current = GetCountyHeader(MAUVILLE)->storyIdx * COUNTY_MAIN_SLOTS + 2;

    VarSet(VAR_MAIN_STORY_INDEX, current);
    FlagSet(FLAG_BADGE03_GET);
    SetCountySlotStage(MAUVILLE, 2, 1);
    EXPECT_EQ(VarGet(VAR_MAIN_STORY_INDEX), current);
    SetCountySlotStage(MAUVILLE, 0, 1);
    EXPECT_EQ(VarGet(VAR_MAIN_STORY_INDEX), current + 1);
    SetCountySlotStage(MAUVILLE, 0, 1);
    EXPECT_EQ(VarGet(VAR_MAIN_STORY_INDEX), current + 1);
}

TEST("CountyPlan: Finishing a main-story slot that is not current, or an off-chain chapter, leaves the index alone")
{
    u32 later = GetCountyHeader(MAUVILLE)->storyIdx * COUNTY_MAIN_SLOTS;

    VarSet(VAR_MAIN_STORY_INDEX, later);
    FlagSet(FLAG_BADGE03_GET);
    SetCountySlotStage(MAUVILLE, 2, 1);
    SetCountySlotStage(MAUVILLE, 0, 1);
    EXPECT_EQ(VarGet(VAR_MAIN_STORY_INDEX), later);
    EXPECT_EQ(GetCountyChapterMainSlot(MAUVILLE, 1), CHAPTER_MAIN_SLOT_OFF_CHAIN);
    SetCountySlotStage(MAUVILLE, 1, 1);
    EXPECT_EQ(VarGet(VAR_MAIN_STORY_INDEX), later);
}

TEST("CountyPlan: A slot with no chain entries is never done")
{
    EXPECT(!IsCountyMainSlotDone(MAUVILLE, 1));
    EXPECT(!IsCountyMainSlotDone(SLATEPORT, 0));
}

TEST("CountyPlan: Petalburg's gym is block 11's callback, so it joins Petalburg's chain after its own slots at the callback block")
{
    struct CountyChainPlan plan;
    struct ChapterHost hosts[48];
    u32 numHosts = GetPlannedChapterHosts(hosts, ARRAY_COUNT(hosts)), i, found = 0;
    u32 petalburg = COUNTY_ID(0, 2);

    EXPECT_EQ(GetPlotCallbackHost(11), (u32)GetCountyHeader(petalburg)->storyIdx);
    EXPECT_EQ(GetVanillaPlotSlot(PLOT_KIND_GYM, 5).block, 11);
    EXPECT_EQ(GetVanillaPlotSlot(PLOT_KIND_GYM, 5).step, 1);
    EXPECT(PlanEmeraldCountyChain(petalburg, &plan));
    EXPECT_EQ(plan.numEntries, 2);
    EXPECT_EQ(plan.chain[0].mainSlot, 0);
    EXPECT_EQ(plan.chain[1].mainSlot, CHAPTER_MAIN_SLOT_CALLBACK(11));
    EXPECT_EQ(plan.chain[1].chapterSlot, CHAPTER_SLOT_STATELESS);
    EXPECT_EQ(plan.chain[1].numAdvances, 2);
    EXPECT_EQ(plan.numOffChain, 0);
    for (i = 0; i < numHosts; i++)
    {
        if (hosts[i].county == petalburg && hosts[i].mainSlot == CHAPTER_MAIN_SLOT_CALLBACK(11))
            found++;
    }
    EXPECT_EQ(found, 1);
}

TEST("CountyPlan: Each ported gym is on its county's chain in its badge's slot, and done once the badge is held")
{
    static const struct { u16 county; u8 ordinal; u16 flag; } gyms[] =
    {
        {COUNTY_ID(0, 1), 1, FLAG_BADGE01_GET}, {COUNTY_ID(0, 3), 2, FLAG_BADGE02_GET},
        {MAUVILLE, 3, FLAG_BADGE03_GET}, {COUNTY_ID(2, 0), 4, FLAG_BADGE04_GET},
        {COUNTY_ID(3, 1), 6, FLAG_BADGE06_GET}, {COUNTY_ID(5, 1), 7, FLAG_BADGE07_GET},
    };
    u32 g, i;

    for (g = 0; g < ARRAY_COUNT(gyms); g++)
    {
        struct CountyChainPlan plan;
        u32 found = 0, slot = GetGymSlot(gyms[g].ordinal).step;

        EXPECT_EQ(GetCountyGymOrdinal(gyms[g].county), gyms[g].ordinal);
        EXPECT(PlanEmeraldCountyChain(gyms[g].county, &plan));
        for (i = 0; i < plan.numEntries; i++)
        {
            if (plan.chainDesc[i]->producesMainGet && plan.chainDesc[i]->mainGet.kind == PLOT_KIND_GYM)
            {
                found++;
                EXPECT_EQ(plan.chain[i].mainSlot, slot);
                EXPECT_EQ(plan.chain[i].chapterSlot, CHAPTER_SLOT_STATELESS);
                EXPECT_EQ(GetChapterStage(gyms[g].county, plan.chain[i].chapterSlot, plan.chainDesc[i]), 0);
            }
        }
        EXPECT_EQ(found, 1);
        FlagSet(gyms[g].flag);
        for (i = 0; i < plan.numEntries; i++)
        {
            if (plan.chainDesc[i]->producesMainGet && plan.chainDesc[i]->mainGet.kind == PLOT_KIND_GYM)
                EXPECT_EQ(GetChapterStage(gyms[g].county, plan.chain[i].chapterSlot, plan.chainDesc[i]), 1);
        }
        FlagClear(gyms[g].flag);
    }
}
