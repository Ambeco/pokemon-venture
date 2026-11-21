#include "global.h"
#include "test/test.h"
#include "chapter.h"
#include "county_chain.h"
#include "region_county.h"
#include "event_data.h"
#include "subplot.h"

static const struct ChapterDesc sOneStage = {.numStages = 1};
static const struct ChapterDesc sThreeStages = {.numStages = 3};

#define REQ(c) {&c, FALSE}
#define OPT(c) {&c, TRUE}
#define SUBPLOT(entries) {ARRAY_COUNT(entries), entries}

// Slot 0 takes 0-2 chapters, slot 1 is a callback hosted elsewhere, slot 2 takes 1-2, slot 3 none.
static const struct SubplotSlotRule sRules[COUNTY_MAIN_SLOTS] = {{0, 2}, {0, 0}, {1, 2}, {0, 0}};

TEST("Subplot: Required entries fill slot 0, skip the callback slot, and leave one for slot 2's minimum")
{
    static const struct SubplotEntry entries[] = {REQ(sOneStage), REQ(sOneStage), REQ(sThreeStages)};
    static const struct SubplotDesc subplot = SUBPLOT(entries);
    struct SubplotPlan plan = PlanSubplot(&subplot, sRules);

    EXPECT_EQ(plan.problem, SUBPLOT_PLAN_OK);
    EXPECT_EQ(plan.mainSlot[0], 0);
    EXPECT_EQ(plan.mainSlot[1], 0);
    EXPECT_EQ(plan.mainSlot[2], 2);
    EXPECT_EQ(plan.count[0], 2);
    EXPECT_EQ(plan.count[1], 0);
    EXPECT_EQ(plan.count[2], 1);
}

TEST("Subplot: Two required entries put the second in slot 2 so its minimum holds, though slot 0 has room")
{
    static const struct SubplotEntry entries[] = {REQ(sOneStage), REQ(sOneStage)};
    static const struct SubplotDesc subplot = SUBPLOT(entries);
    struct SubplotPlan plan = PlanSubplot(&subplot, sRules);

    EXPECT_EQ(plan.problem, SUBPLOT_PLAN_OK);
    EXPECT_EQ(plan.mainSlot[0], 0);
    EXPECT_EQ(plan.mainSlot[1], 2);
}

TEST("Subplot: An optional entry is dropped when its slot is full, and the next required entry spills over")
{
    static const struct SubplotSlotRule rules[COUNTY_MAIN_SLOTS] = {{0, 1}, {0, 0}, {1, 2}, {0, 0}};
    static const struct SubplotEntry entries[] = {REQ(sOneStage), OPT(sOneStage), REQ(sOneStage)};
    static const struct SubplotDesc subplot = SUBPLOT(entries);
    struct SubplotPlan plan = PlanSubplot(&subplot, rules);

    EXPECT_EQ(plan.problem, SUBPLOT_PLAN_OK);
    EXPECT_EQ(plan.mainSlot[0], 0);
    EXPECT_EQ(plan.mainSlot[1], SUBPLOT_DROPPED);
    EXPECT_EQ(plan.mainSlot[2], 2);
    EXPECT_EQ(GetSubplotPlacedCount(&subplot, &plan), 2);
}

TEST("Subplot: An optional entry that would leave a later minimum unguaranteed is dropped")
{
    static const struct SubplotEntry entries[] = {OPT(sOneStage)};
    static const struct SubplotDesc subplot = SUBPLOT(entries);
    struct SubplotPlan plan = PlanSubplot(&subplot, sRules);

    EXPECT_EQ(plan.mainSlot[0], SUBPLOT_DROPPED);
    EXPECT_EQ(plan.problem, SUBPLOT_PLAN_SLOT_UNDERFILLED);
    EXPECT_EQ(plan.problemIndex, 2);
}

TEST("Subplot: An optional entry stays when required entries after it cover the later minimums")
{
    static const struct SubplotEntry entries[] = {OPT(sOneStage), REQ(sOneStage)};
    static const struct SubplotDesc subplot = SUBPLOT(entries);
    struct SubplotPlan plan = PlanSubplot(&subplot, sRules);

    EXPECT_EQ(plan.problem, SUBPLOT_PLAN_OK);
    EXPECT_EQ(plan.mainSlot[0], 0);
    EXPECT_EQ(plan.mainSlot[1], 2);
}

TEST("Subplot: A required entry with no slot left fails the plan and names the entry")
{
    static const struct SubplotEntry entries[] = {REQ(sOneStage), REQ(sOneStage), REQ(sOneStage), REQ(sOneStage), REQ(sOneStage)};
    static const struct SubplotDesc subplot = SUBPLOT(entries);
    struct SubplotPlan plan = PlanSubplot(&subplot, sRules);

    EXPECT_EQ(plan.problem, SUBPLOT_PLAN_NO_ROOM);
    EXPECT_EQ(plan.problemIndex, 4);
}

TEST("Subplot: A slot below its minimum at the end fails the plan and names the slot")
{
    static const struct SubplotSlotRule rules[COUNTY_MAIN_SLOTS] = {{1, 2}, {0, 0}, {1, 2}, {0, 0}};
    static const struct SubplotEntry entries[] = {REQ(sOneStage)};
    static const struct SubplotDesc subplot = SUBPLOT(entries);
    struct SubplotPlan plan = PlanSubplot(&subplot, rules);

    EXPECT_EQ(plan.problem, SUBPLOT_PLAN_SLOT_UNDERFILLED);
    EXPECT_EQ(plan.problemIndex, 2);
}

TEST("Subplot: The chain has an entry per placed chapter with its stage count, and resolves like any county chain")
{
    static const struct SubplotEntry entries[] = {REQ(sOneStage), OPT(sThreeStages), REQ(sThreeStages)};
    static const struct SubplotDesc subplot = SUBPLOT(entries);
    static const struct SubplotSlotRule rules[COUNTY_MAIN_SLOTS] = {{0, 1}, {0, 0}, {1, 2}, {0, 0}};
    struct SubplotPlan plan = PlanSubplot(&subplot, rules);
    struct CountyChainEntry chain[SUBPLOT_MAX_ENTRIES];
    struct ChainPosition position;
    u32 n = BuildSubplotChain(&subplot, &plan, 1, chain);

    EXPECT_EQ(n, 2);
    EXPECT_EQ(chain[0].mainSlot, 0);
    EXPECT_EQ(chain[0].chapterSlot, 1);
    EXPECT_EQ(chain[0].numStages, 1);
    EXPECT_EQ(chain[1].mainSlot, 2);
    EXPECT_EQ(chain[1].chapterSlot, 2);
    EXPECT_EQ(chain[1].numStages, 3);
    EXPECT_EQ(GetChainLength(chain, n), 4);
    position = ResolveChainPosition(chain, n, 2);
    EXPECT_EQ(position.entry, 1);
    EXPECT_EQ(position.stage, 1);
    EXPECT(IsChainMainSlotDone(chain, n, 1, 0));
}

TEST("Subplot: Mauville's three ported chapters plan into the slots with their real stage counts")
{
    struct SubplotEntry entries[3];
    struct SubplotDesc subplot = {ARRAY_COUNT(entries), entries};
    struct CountyChainEntry chain[SUBPLOT_MAX_ENTRIES];
    struct SubplotPlan plan;
    u32 i, n, stages = 0;

    for (i = 0; i < 3; i++)
    {
        entries[i].chapter = GetCountyChapter(COUNTY_ID(2, 1), i);
        entries[i].optional = i == 2;
        ASSUME(entries[i].chapter != NULL);
        stages += entries[i].chapter->numStages;
    }
    plan = PlanSubplot(&subplot, sRules);
    EXPECT_EQ(plan.problem, SUBPLOT_PLAN_OK);
    n = BuildSubplotChain(&subplot, &plan, 0, chain);
    EXPECT_EQ(n, GetSubplotPlacedCount(&subplot, &plan));
    EXPECT(GetChainLength(chain, n) <= stages);
    for (i = 0; i < n; i++)
        EXPECT(chain[i].chapterSlot < CHAPTERS_PER_COUNTY);
}

TEST("Subplot: A full subplot of seven optional entries and a required last one keeps the last in slot 2")
{
    struct SubplotEntry entries[SUBPLOT_MAX_ENTRIES];
    struct SubplotDesc subplot = {SUBPLOT_MAX_ENTRIES, entries};
    struct SubplotPlan plan;
    u32 i;

    for (i = 0; i < SUBPLOT_MAX_ENTRIES; i++)
    {
        entries[i].chapter = &sOneStage;
        entries[i].optional = i != SUBPLOT_MAX_ENTRIES - 1;
    }
    plan = PlanSubplot(&subplot, sRules);
    EXPECT_EQ(plan.problem, SUBPLOT_PLAN_OK);
    EXPECT_LE(plan.count[0], 2);
    EXPECT_LE(plan.count[2], 2);
    EXPECT_EQ(plan.mainSlot[SUBPLOT_MAX_ENTRIES - 1], 2);
}

TEST("Subplot: An empty subplot with no minimums plans to nothing")
{
    static const struct SubplotSlotRule rules[COUNTY_MAIN_SLOTS] = {{0, 2}, {0, 0}, {0, 2}, {0, 0}};
    struct SubplotDesc subplot = {0, NULL};
    struct SubplotPlan plan = PlanSubplot(&subplot, rules);
    struct CountyChainEntry chain[1];

    EXPECT_EQ(plan.problem, SUBPLOT_PLAN_OK);
    EXPECT_EQ(GetSubplotPlacedCount(&subplot, &plan), 0);
    EXPECT_EQ(BuildSubplotChain(&subplot, &plan, 0, chain), 0);
}
