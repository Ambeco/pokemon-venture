#include "global.h"
#include "chapter.h"
#include "subplot.h"

static bool32 IsBound(const struct SubplotDesc *subplot, const u8 *boundSlots, u32 index)
{
    return boundSlots != NULL && !subplot->entries[index].optional && boundSlots[index] != SUBPLOT_UNBOUND;
}

// Required entries after `index` that are free to take any slot.
static u32 CountUnboundRequiredAfter(const struct SubplotDesc *subplot, const u8 *boundSlots, u32 index)
{
    u32 i, count = 0;

    for (i = index + 1; i < subplot->numEntries; i++)
    {
        if (!subplot->entries[i].optional && !IsBound(subplot, boundSlots, i))
            count++;
    }
    return count;
}

// Later required entries bound to `slot`: their room there is not for earlier entries to take.
static u32 CountBoundAfter(const struct SubplotDesc *subplot, const u8 *boundSlots, u32 index, u32 slot)
{
    u32 i, count = 0;

    for (i = index + 1; i < subplot->numEntries; i++)
    {
        if (IsBound(subplot, boundSlots, i) && boundSlots[i] == slot)
            count++;
    }
    return count;
}

// Entries before a bound entry may not go past its slot, or the subplot would run out of order.
static u32 GetNextBoundSlot(const struct SubplotDesc *subplot, const u8 *boundSlots, u32 index)
{
    u32 i, limit = SUBPLOT_UNBOUND;

    for (i = index + 1; i < subplot->numEntries; i++)
    {
        if (IsBound(subplot, boundSlots, i) && boundSlots[i] < limit)
            limit = boundSlots[i];
    }
    return limit;
}

// Minimums of the slots after `slot` that later bound entries do not already fill.
static u32 GetLaterMinimums(const struct SubplotDesc *subplot, const u8 *boundSlots, u32 index, const struct SubplotSlotRule rules[COUNTY_MAIN_SLOTS], u32 slot)
{
    u32 s, bound, total = 0;

    for (s = slot + 1; s < COUNTY_MAIN_SLOTS; s++)
    {
        bound = CountBoundAfter(subplot, boundSlots, index, s);
        total += rules[s].min > bound ? rules[s].min - bound : 0;
    }
    return total;
}

struct SubplotPlan PlanSubplot(const struct SubplotDesc *subplot, const struct SubplotSlotRule rules[COUNTY_MAIN_SLOTS])
{
    return PlanSubplotBound(subplot, rules, NULL);
}

struct SubplotPlan PlanSubplotBound(const struct SubplotDesc *subplot, const struct SubplotSlotRule rules[COUNTY_MAIN_SLOTS], const u8 *boundSlots)
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
        u32 requiredAfter = CountUnboundRequiredAfter(subplot, boundSlots, i);
        u32 limit = GetNextBoundSlot(subplot, boundSlots, i);
        bool32 placed = FALSE;

        if (IsBound(subplot, boundSlots, i))
        {
            u32 bound = boundSlots[i];

            if (bound >= COUNTY_MAIN_SLOTS || bound < slot)
            {
                plan.problem = SUBPLOT_PLAN_BOUND_OUT_OF_ORDER;
                plan.problemIndex = i;
                return plan;
            }
            if (plan.count[bound] >= rules[bound].max)
            {
                plan.problem = SUBPLOT_PLAN_NO_ROOM;
                plan.problemIndex = i;
                return plan;
            }
            plan.mainSlot[i] = bound;
            plan.count[bound]++;
            slot = bound;
            continue;
        }
        while (slot < COUNTY_MAIN_SLOTS && rules[slot].max == 0)
            slot++;
        while (slot < COUNTY_MAIN_SLOTS && slot <= limit)
        {
            bool32 full = plan.count[slot] + CountBoundAfter(subplot, boundSlots, i, slot) >= rules[slot].max;
            // A slot below its minimum takes the entry regardless; above it, an entry must leave the later minimums to required entries.
            bool32 starves = plan.count[slot] >= rules[slot].min && requiredAfter < GetLaterMinimums(subplot, boundSlots, i, rules, slot);

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
