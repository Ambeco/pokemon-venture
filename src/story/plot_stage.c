#include "global.h"
#include "constants/flags.h"
#include "constants/vars.h"
#include "event_data.h"
#include "plot_stage.h"
#include "region_city.h"

#include "data/region/emerald_plot_gets.h"

u32 GetBadgeCount(void)
{
    u32 i, count = 0;

    for (i = 0; i < NUM_BADGES; i++)
    {
        if (FlagGet(FLAG_BADGE01_GET + i))
            count++;
    }
    return count;
}

bool32 IsStoryPredicateTrue(const struct StoryPredicate *pred)
{
    switch (pred->op)
    {
    case STORY_PRED_FLAG:
        return FlagGet(pred->a);
    case STORY_PRED_ANY_FLAG:
        return FlagGet(pred->a) || FlagGet(pred->b);
    case STORY_PRED_VAR_GE:
        return VarGet(pred->a) >= pred->b;
    case STORY_PRED_ALWAYS:
        return TRUE;
    case STORY_PRED_BADGES_GE:
        return GetBadgeCount() >= pred->a;
    }
    AGB_ASSERT(FALSE); // unknown StoryPredicateOp: add a case above
    return FALSE;
}

u32 GetVanillaPlotGetEntryCount(void)
{
    return ARRAY_COUNT(sEmeraldPlotGets);
}

u32 GetPlotSlotGetDescs(const struct PlotSlot *slot, struct PlotGetDesc out[2])
{
    u32 i;

    if (slot->kind == PLOT_KIND_GYM)
    {
        u32 count = 0;

        if (!slot->hmOnly)
            out[count++] = sEmeraldGymGets[slot->ordinal - 1];
        if (slot->hm != PLOT_HM_NONE)
            out[count++] = sEmeraldHmGets[slot->hm];
        AGB_ASSERT(count > 0); // a gym slot carries a badge or an HM
        return count;
    }
    for (i = 0; i < ARRAY_COUNT(sEmeraldPlotGets); i++)
    {
        if (sEmeraldPlotGets[i].kind == slot->kind && sEmeraldPlotGets[i].ordinal == slot->ordinal)
        {
            out[0] = sEmeraldPlotGets[i].desc;
            return 1;
        }
    }
    AGB_ASSERT(FALSE); // add the item to sEmeraldPlotGets in data/region/emerald_plot_gets.h
    return 0;
}

static void AddGet(struct PlotListStage *stage, const struct PlotGetDesc *desc)
{
    AGB_ASSERT(stage->numGets < MAX_GETS_PER_LIST);
    if (IsStoryPredicateTrue(&desc->done))
        stage->doneMask |= 1u << stage->numGets;
    if (desc->getClass == PLOT_GET_BLOCKING)
        stage->blockingMask |= 1u << stage->numGets;
    stage->numGets++;
}

struct PlotListStage GetPlotListStage(u32 index)
{
    struct PlotSlot slots[MAX_GETS_PER_LIST];
    struct PlotListStage stage = {0};
    u32 count, i;

    if ((index & 3) == 3)
    {
        for (i = 0; i < ARRAY_COUNT(sEmeraldPlotUnlocks); i++)
        {
            if (sEmeraldPlotUnlocks[i].index == index)
                AddGet(&stage, &sEmeraldPlotUnlocks[i].desc);
        }
        return stage;
    }
    count = ResolveVanillaPlotList(index, slots, ARRAY_COUNT(slots));
    for (i = 0; i < count; i++)
    {
        struct PlotGetDesc descs[2];
        u32 numDescs = GetPlotSlotGetDescs(&slots[i], descs);
        u32 j;

        for (j = 0; j < numDescs; j++)
            AddGet(&stage, &descs[j]);
    }
    return stage;
}

bool32 GetFirstUndonePlotSlot(u32 index, struct PlotSlot *out)
{
    struct PlotSlot slots[MAX_GETS_PER_LIST];
    struct PlotListStage stage;
    u32 count, i, bit = 0;

    if ((index & 3) == 3)
        return FALSE;
    stage = GetPlotListStage(index);
    count = ResolveVanillaPlotList(index, slots, ARRAY_COUNT(slots));
    for (i = 0; i < count; i++)
    {
        struct PlotGetDesc descs[2];
        u32 j, numDescs = GetPlotSlotGetDescs(&slots[i], descs);

        for (j = 0; j < numDescs; j++, bit++)
        {
            if ((stage.blockingMask & ~stage.doneMask) & (1u << bit))
            {
                *out = slots[i];
                return TRUE;
            }
        }
    }
    return FALSE;
}

u32 GetMainStoryIndex(void)
{
    u32 index;

    for (index = 0; index < MAIN_STORY_INDEX_POSTGAME; index++)
    {
        struct PlotListStage stage = GetPlotListStage(index);

        if (!IsPlotListDone(&stage))
            return index;
    }
    return MAIN_STORY_INDEX_POSTGAME;
}
