#include "global.h"
#include "test/test.h"
#include "chapter.h"
#include "county_chain.h"
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
