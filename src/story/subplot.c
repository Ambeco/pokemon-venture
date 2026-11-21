#include "global.h"
#include "chapter.h"
#include "subplot.h"

static u32 CountRequiredAfter(const struct SubplotDesc *subplot, u32 index)
{
    u32 i, count = 0;

    for (i = index + 1; i < subplot->numEntries; i++)
    {
        if (!subplot->entries[i].optional)
            count++;
    }
    return count;
}

static u32 GetLaterMinimums(const struct SubplotSlotRule rules[COUNTY_MAIN_SLOTS], u32 slot)
{
    u32 s, total = 0;

    for (s = slot + 1; s < COUNTY_MAIN_SLOTS; s++)
        total += rules[s].min;
    return total;
}

struct SubplotPlan PlanSubplot(const struct SubplotDesc *subplot, const struct SubplotSlotRule rules[COUNTY_MAIN_SLOTS])
{
    struct SubplotPlan plan = {.problem = SUBPLOT_PLAN_OK};
    u32 i, s, slot = 0;

    memset(plan.mainSlot, SUBPLOT_DROPPED, sizeof(plan.mainSlot));
    assertf(subplot->numEntries <= SUBPLOT_MAX_ENTRIES, "subplot has %d entries, over the SUBPLOT_MAX_ENTRIES of %d; did you mean to split it into two subplots?", subplot->numEntries, SUBPLOT_MAX_ENTRIES)
    {
        plan.problem = SUBPLOT_PLAN_NO_ROOM;
        plan.problemIndex = SUBPLOT_MAX_ENTRIES;
        return plan;
    }
    for (i = 0; i < subplot->numEntries; i++)
    {
        bool32 optional = subplot->entries[i].optional;
        u32 requiredAfter = CountRequiredAfter(subplot, i);
        bool32 placed = FALSE;

        while (slot < COUNTY_MAIN_SLOTS && rules[slot].max == 0)
            slot++;
        while (slot < COUNTY_MAIN_SLOTS)
        {
            bool32 full = plan.count[slot] >= rules[slot].max;
            // A slot below its minimum takes the entry regardless; above it, an entry must leave the later minimums to required entries.
            bool32 starves = plan.count[slot] >= rules[slot].min && requiredAfter < GetLaterMinimums(rules, slot);

            if (optional && (full || starves))
                break;
            if (!full && !starves)
            {
                plan.mainSlot[i] = slot;
                plan.count[slot]++;
                placed = TRUE;
                break;
            }
            slot++;
            while (slot < COUNTY_MAIN_SLOTS && rules[slot].max == 0)
                slot++;
        }
        if (!placed && !optional)
        {
            plan.problem = SUBPLOT_PLAN_NO_ROOM;
            plan.problemIndex = i;
            return plan;
        }
    }
    for (s = 0; s < COUNTY_MAIN_SLOTS; s++)
    {
        if (plan.count[s] < rules[s].min)
        {
            plan.problem = SUBPLOT_PLAN_SLOT_UNDERFILLED;
            plan.problemIndex = s;
            return plan;
        }
    }
    return plan;
}

const struct ChapterDesc *GetSubplotPredecessor(const struct SubplotHost *hosts, u32 numHosts, u32 county, const struct ChapterDesc *desc)
{
    u32 h, i;

    for (h = 0; h < numHosts; h++)
    {
        if (hosts[h].county != county)
            continue;
        for (i = 0; i < hosts[h].subplot->numEntries; i++)
        {
            if (hosts[h].subplot->entries[i].chapter == desc)
                return i == 0 ? NULL : hosts[h].subplot->entries[i - 1].chapter;
        }
    }
    return NULL;
}

u32 GetSubplotPlacedCount(const struct SubplotDesc *subplot, const struct SubplotPlan *plan)
{
    u32 i, count = 0;

    for (i = 0; i < subplot->numEntries; i++)
    {
        if (plan->mainSlot[i] != SUBPLOT_DROPPED)
            count++;
    }
    return count;
}

u32 BuildSubplotChain(const struct SubplotDesc *subplot, const struct SubplotPlan *plan, u32 firstChapterSlot, struct CountyChainEntry *out)
{
    u32 i, n = 0;

    assertf(plan->problem == SUBPLOT_PLAN_OK, "cannot build a chain from a failed subplot plan (problem %d at %d); did you mean to check plan.problem first?", plan->problem, plan->problemIndex)
    {
        return 0;
    }
    assertf(firstChapterSlot + GetSubplotPlacedCount(subplot, plan) <= CHAPTERS_PER_COUNTY, "subplot needs chapter slots %d to %d but a county has %d", firstChapterSlot, firstChapterSlot + GetSubplotPlacedCount(subplot, plan) - 1, CHAPTERS_PER_COUNTY)
    {
        return 0;
    }
    for (i = 0; i < subplot->numEntries; i++)
    {
        if (plan->mainSlot[i] == SUBPLOT_DROPPED)
            continue;
        out[n].mainSlot = plan->mainSlot[i];
        out[n].chapterSlot = firstChapterSlot + n;
        out[n].numStages = subplot->entries[i].chapter->numStages;
        n++;
    }
    return n;
}
