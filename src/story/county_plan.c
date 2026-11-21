#include "global.h"
#include "assertf.h"
#include "county_plan.h"
#include "plot_stage.h"
#include "region_county.h"
#include "region_plot.h"

#define COUNTY_PLAN_NO_SLOT 0xFF
#define COUNTY_PLAN_BOUND_ELSEWHERE 0xFE

// Kinds a county's chapters supply today: HMs, utility items, the tutorial's scripted gets and rival beats. Gym badges,
// Wally, arc beats, Devon Goods and special items are hosted by other mechanisms and open no chapter capacity.
static bool32 IsChapterSuppliedKind(u32 kind)
{
    return kind == PLOT_KIND_UTILITY || kind == PLOT_KIND_SCRIPTED || kind == PLOT_KIND_RIVAL || kind == PLOT_KIND_GYM;
}

// Rule of the main-story list `index`: it takes chapters iff it holds a supplied get, and needs one iff that get blocks.
static void GetListRule(u32 index, struct SubplotSlotRule *rule)
{
    struct PlotSlot items[MAX_GETS_PER_LIST];
    u32 count = ResolveVanillaPlotList(index, items, MAX_GETS_PER_LIST);
    u32 i, j;

    rule->min = 0;
    rule->max = 0;
    for (i = 0; i < count; i++)
    {
        struct PlotGetDesc descs[2];
        u32 numDescs;

        if (!IsChapterSuppliedKind(items[i].kind))
            continue;
        numDescs = GetPlotSlotGetDescs(&items[i], descs);
        for (j = 0; j < numDescs; j++)
        {
            bool32 isBadge = j == 0 && items[i].kind == PLOT_KIND_GYM && !items[i].hmOnly;

            if (isBadge)
                continue;
            rule->max = CHAPTERS_PER_COUNTY;
            if (descs[j].getClass == PLOT_GET_BLOCKING)
                rule->min = 1;
        }
    }
}

void GetCountySlotRules(u32 county, struct SubplotSlotRule rules[COUNTY_MAIN_SLOTS])
{
    u32 block = GetCountyHeader(county)->storyIdx;
    u32 s;

    for (s = 0; s < COUNTY_MAIN_SLOTS; s++)
    {
        GetListRule(block * COUNTY_MAIN_SLOTS + s, &rules[s]);
        if (s == 1 && GetPlotCallbackHost(block) != block)
            rules[s].max = rules[s].min = 0;
    }
}

static u32 FindChapterSlot(const struct ChapterHost *hosts, u32 numHosts, u32 county, const struct ChapterDesc *desc)
{
    u32 i;

    for (i = 0; i < numHosts; i++)
    {
        if (hosts[i].county == county && hosts[i].desc == desc)
            return hosts[i].slot;
    }
    return COUNTY_PLAN_NO_SLOT;
}

// The main-story slot of the get the chapter supplies, SUBPLOT_UNBOUND when it supplies none, or COUNTY_PLAN_BOUND_ELSEWHERE
// when that get is in another county's block or a slot this county's rules give no room.
static u32 GetChapterBoundSlot(u32 county, const struct ChapterDesc *desc, const struct SubplotSlotRule rules[COUNTY_MAIN_SLOTS])
{
    struct PlotSlot get;

    if (!GetChapterSuppliedSlot(desc, &get))
        return SUBPLOT_UNBOUND;
    if (get.block != GetCountyHeader(county)->storyIdx || get.step >= COUNTY_MAIN_SLOTS || rules[get.step].max == 0)
        return COUNTY_PLAN_BOUND_ELSEWHERE;
    return get.step;
}

// Counts the required, bound entries of `county`'s subplots from `firstSubplot` on, so earlier subplots leave their slots free.
static void CountReservedSlots(u32 county, const struct SubplotHost *subplots, u32 numSubplots, u32 firstSubplot, const struct SubplotSlotRule rules[COUNTY_MAIN_SLOTS], u8 reserved[COUNTY_MAIN_SLOTS])
{
    u32 sp, e;

    memset(reserved, 0, COUNTY_MAIN_SLOTS);
    for (sp = firstSubplot; sp < numSubplots; sp++)
    {
        if (subplots[sp].county != county)
            continue;
        for (e = 0; e < subplots[sp].subplot->numEntries; e++)
        {
            const struct SubplotEntry *entry = &subplots[sp].subplot->entries[e];
            u32 bound = entry->optional ? SUBPLOT_UNBOUND : GetChapterBoundSlot(county, entry->chapter, rules);

            if (bound < COUNTY_MAIN_SLOTS)
                reserved[bound]++;
        }
    }
}

// Stable insert by main-story slot, so entries stay in declaration order within a slot.
static void InsertChainEntry(struct CountyChainPlan *out, const struct CountyChainEntry *entry)
{
    u32 pos = out->numEntries;

    while (pos > 0 && out->chain[pos - 1].mainSlot > entry->mainSlot)
    {
        out->chain[pos] = out->chain[pos - 1];
        pos--;
    }
    out->chain[pos] = *entry;
    out->numEntries++;
}

bool32 PlanCountyChain(u32 county, const struct SubplotHost *subplots, u32 numSubplots, const struct ChapterHost *hosts, u32 numHosts, struct CountyChainPlan *out)
{
    struct SubplotSlotRule rules[COUNTY_MAIN_SLOTS], baseRules[COUNTY_MAIN_SLOTS];
    u32 sp, e, s, subplotIndex = 0, numSeen = 0;

    memset(out, 0, sizeof(*out));
    GetCountySlotRules(county, rules);
    memcpy(baseRules, rules, sizeof(rules));
    for (sp = 0; sp < numSubplots; sp++)
    {
        const struct SubplotDesc *subplot = subplots[sp].subplot;
        struct SubplotEntry required[SUBPLOT_MAX_ENTRIES];
        u8 requiredSlot[SUBPLOT_MAX_ENTRIES];
        u8 boundSlot[SUBPLOT_MAX_ENTRIES];
        u8 reserved[COUNTY_MAIN_SLOTS];
        struct SubplotSlotRule planRules[COUNTY_MAIN_SLOTS];
        struct CountyChainEntry placed[SUBPLOT_MAX_ENTRIES];
        struct SubplotDesc requiredOnly = {0, required};
        struct SubplotPlan plan;
        u32 numPlaced;

        if (subplots[sp].county != county)
            continue;
        for (e = 0; e < subplot->numEntries; e++)
        {
            u32 slot = FindChapterSlot(hosts, numHosts, county, subplot->entries[e].chapter);

            assertf(slot != COUNTY_PLAN_NO_SLOT, "subplot %d entry %d is not hosted by county %d; did you mean to add it to the chapter host table?", subplotIndex, e, county)
            {
                out->problem = SUBPLOT_PLAN_NO_ROOM;
                out->problemSubplot = subplotIndex;
                out->problemIndex = e;
                return FALSE;
            }
            assertf(numSeen < CHAPTERS_PER_COUNTY, "county %d subplots list more than %d entries; did you list one chapter in two subplots?", county, CHAPTERS_PER_COUNTY)
            {
                out->problem = SUBPLOT_PLAN_NO_ROOM;
                out->problemSubplot = subplotIndex;
                out->problemIndex = e;
                return FALSE;
            }
            numSeen++;
            if (subplot->entries[e].optional)
            {
                out->offChainSlot[out->numOffChain++] = slot;
                continue;
            }
            boundSlot[requiredOnly.numEntries] = GetChapterBoundSlot(county, subplot->entries[e].chapter, baseRules);
            assertf(boundSlot[requiredOnly.numEntries] != COUNTY_PLAN_BOUND_ELSEWHERE, "subplot %d entry %d supplies a get that is not in county %d's block or has no room in its slot rules; did you mean to host it in the county whose block holds the get, or make it optional?", subplotIndex, e, county)
            {
                out->problem = SUBPLOT_PLAN_BOUND_ELSEWHERE;
                out->problemSubplot = subplotIndex;
                out->problemIndex = e;
                return FALSE;
            }
            required[requiredOnly.numEntries] = subplot->entries[e];
            requiredSlot[requiredOnly.numEntries++] = slot;
        }
        // Later subplots' bound entries keep their slots free from this subplot's unbound ones.
        CountReservedSlots(county, subplots, numSubplots, sp + 1, baseRules, reserved);
        for (s = 0; s < COUNTY_MAIN_SLOTS; s++)
        {
            planRules[s].max = rules[s].max > reserved[s] ? rules[s].max - reserved[s] : 0;
            planRules[s].min = rules[s].min > reserved[s] ? rules[s].min - reserved[s] : 0;
        }
        plan = PlanSubplotBound(&requiredOnly, planRules, boundSlot);
        if (plan.problem != SUBPLOT_PLAN_OK)
        {
            out->problem = plan.problem;
            out->problemSubplot = subplotIndex;
            out->problemIndex = plan.problemIndex;
            return FALSE;
        }
        numPlaced = BuildSubplotChain(&requiredOnly, &plan, 0, placed);
        for (e = 0; e < numPlaced; e++)
        {
            placed[e].chapterSlot = requiredSlot[e];
            InsertChainEntry(out, &placed[e]);
        }
        for (s = 0; s < COUNTY_MAIN_SLOTS; s++)
        {
            rules[s].max -= plan.count[s];
            rules[s].min = rules[s].min > plan.count[s] ? rules[s].min - plan.count[s] : 0;
        }
        subplotIndex++;
    }
    return TRUE;
}

bool32 PlanEmeraldCountyChain(u32 county, struct CountyChainPlan *out)
{
    u32 numHosts, numSubplots;
    const struct ChapterHost *hosts = GetChapterHosts(&numHosts);
    const struct SubplotHost *subplots = GetSubplotHosts(&numSubplots);

    return PlanCountyChain(county, subplots, numSubplots, hosts, numHosts, out);
}
