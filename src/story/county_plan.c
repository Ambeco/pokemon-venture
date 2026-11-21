#include "global.h"
#include "assertf.h"
#include "county_plan.h"
#include "plot_stage.h"
#include "region_county.h"
#include "region_plot.h"

#define COUNTY_PLAN_NO_SLOT 0xFF

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
    struct SubplotSlotRule rules[COUNTY_MAIN_SLOTS];
    u32 sp, e, s, subplotIndex = 0;

    memset(out, 0, sizeof(*out));
    GetCountySlotRules(county, rules);
    for (sp = 0; sp < numSubplots; sp++)
    {
        const struct SubplotDesc *subplot = subplots[sp].subplot;
        struct SubplotEntry required[SUBPLOT_MAX_ENTRIES];
        u8 requiredSlot[SUBPLOT_MAX_ENTRIES];
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
            if (subplot->entries[e].optional)
            {
                out->offChainSlot[out->numOffChain++] = slot;
                continue;
            }
            required[requiredOnly.numEntries] = subplot->entries[e];
            requiredSlot[requiredOnly.numEntries++] = slot;
        }
        plan = PlanSubplot(&requiredOnly, rules);
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
