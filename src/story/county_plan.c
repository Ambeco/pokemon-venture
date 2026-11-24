#include "global.h"
#include "assertf.h"
#include "county_plan.h"
#include "plot_stage.h"
#include "region_county.h"
#include "region_plot.h"

#define COUNTY_PLAN_NO_SLOT 0xFF
#define COUNTY_PLAN_BOUND_ELSEWHERE 0xFE
#define COUNTY_PLAN_BOUND_CALLBACK 0xFD

// Kinds a county's chapters supply today: gym badges, HMs, utility items, the tutorial's scripted gets and rival beats.
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

// The main-story slot of the get the chapter supplies, SUBPLOT_UNBOUND when it supplies none, COUNTY_PLAN_BOUND_CALLBACK when
// the get is the callback list another block hands to this county, or COUNTY_PLAN_BOUND_ELSEWHERE when it is in another
// county's block or a slot this county's rules give no room.
static u32 GetChapterBoundSlot(u32 county, const struct ChapterDesc *desc, const struct SubplotSlotRule rules[COUNTY_MAIN_SLOTS])
{
    struct PlotSlot get;
    u32 storyIdx = GetCountyHeader(county)->storyIdx;

    if (!GetChapterSuppliedSlot(desc, &get))
        return SUBPLOT_UNBOUND;
    if (get.block != storyIdx && get.step == 1 && GetPlotCallbackHost(get.block) == storyIdx)
        return COUNTY_PLAN_BOUND_CALLBACK;
    if (get.block != storyIdx || get.step >= COUNTY_MAIN_SLOTS || rules[get.step].max == 0)
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
static void InsertChainEntry(struct CountyChainPlan *out, const struct CountyChainEntry *entry, const struct ChapterDesc *desc)
{
    u32 pos = out->numEntries;

    while (pos > 0 && out->chain[pos - 1].mainSlot > entry->mainSlot)
    {
        out->chain[pos] = out->chain[pos - 1];
        out->chainDesc[pos] = out->chainDesc[pos - 1];
        pos--;
    }
    out->chain[pos] = *entry;
    out->chainDesc[pos] = desc;
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
            // A proxy has no stage var, so only stateful chapters count against the county's slot vars.
            assertf(numSeen < CHAPTERS_PER_COUNTY || slot == CHAPTER_SLOT_STATELESS, "county %d subplots list more than %d stateful entries; did you list one chapter in two subplots?", county, CHAPTERS_PER_COUNTY)
            {
                out->problem = SUBPLOT_PLAN_NO_ROOM;
                out->problemSubplot = subplotIndex;
                out->problemIndex = e;
                return FALSE;
            }
            assertf(slot == CHAPTER_SLOT_STATELESS || slot < CHAPTERS_PER_COUNTY, "subplot %d entry %d is hosted in slot %d, but a county has slots 0-%d or CHAPTER_SLOT_STATELESS", subplotIndex, e, slot, CHAPTERS_PER_COUNTY - 1)
            {
                out->problem = SUBPLOT_PLAN_NO_ROOM;
                out->problemSubplot = subplotIndex;
                out->problemIndex = e;
                return FALSE;
            }
            if (slot != CHAPTER_SLOT_STATELESS)
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
            if (boundSlot[requiredOnly.numEntries] == COUNTY_PLAN_BOUND_CALLBACK)
            {
                // A callback list has no slot rules: the chapter goes on the chain at its callback block, after the county's own slots.
                struct PlotSlot get;
                struct CountyChainEntry entry;

                GetChapterSuppliedSlot(subplot->entries[e].chapter, &get);
                assertf(subplot->entries[e].chapter->numStages >= 2, "subplot %d entry %d has %d stage(s); a chain entry needs at least 2", subplotIndex, e, subplot->entries[e].chapter->numStages)
                {
                    out->problem = SUBPLOT_PLAN_NO_ROOM;
                    out->problemSubplot = subplotIndex;
                    out->problemIndex = e;
                    return FALSE;
                }
                entry.mainSlot = CHAPTER_MAIN_SLOT_CALLBACK(get.block);
                entry.chapterSlot = slot;
                entry.numAdvances = subplot->entries[e].chapter->numStages - 1;
                InsertChainEntry(out, &entry, subplot->entries[e].chapter);
                continue;
            }
            required[requiredOnly.numEntries] = subplot->entries[e];
            requiredSlot[requiredOnly.numEntries++] = slot;
        }
        // Capacity (max) is per subplot, so every subplot sees the full base max. The minimum is shared: what earlier subplots
        // met is not owed again, and later subplots' bound entries keep their share of it free from this subplot's unbound ones.
        CountReservedSlots(county, subplots, numSubplots, sp + 1, baseRules, reserved);
        for (s = 0; s < COUNTY_MAIN_SLOTS; s++)
        {
            planRules[s].max = baseRules[s].max;
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
            assertf(out->numEntries < COUNTY_PLAN_MAX_ENTRIES, "county %d chain has more than %d entries", county, COUNTY_PLAN_MAX_ENTRIES)
            {
                out->problem = SUBPLOT_PLAN_NO_ROOM;
                out->problemSubplot = subplotIndex;
                return FALSE;
            }
            placed[e].chapterSlot = requiredSlot[e];
            InsertChainEntry(out, &placed[e], required[e].chapter);
        }
        for (s = 0; s < COUNTY_MAIN_SLOTS; s++)
            rules[s].min = rules[s].min > plan.count[s] ? rules[s].min - plan.count[s] : 0;
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

u32 GetPlannedChapterHosts(struct ChapterHost *out, u32 maxHosts)
{
    u32 numHosts, county, i, j;
    const struct ChapterHost *hosts = GetChapterHosts(&numHosts);

    assertf(numHosts <= maxHosts, "%d chapter hosts do not fit the %d-entry buffer", numHosts, maxHosts)
    {
        return 0;
    }
    for (i = 0; i < numHosts; i++)
    {
        out[i] = hosts[i];
        out[i].mainSlot = CHAPTER_MAIN_SLOT_OFF_CHAIN;
    }
    for (county = 0; county < NUM_COUNTIES; county++)
    {
        struct CountyChainPlan plan;
        bool32 hosted = FALSE;

        for (i = 0; i < numHosts; i++)
            hosted |= hosts[i].county == county;
        if (!hosted)
            continue;
        assertf(PlanEmeraldCountyChain(county, &plan), "county %d chain does not plan (problem %d); fix the plan before asking for planned hosts", county, plan.problem)
        {
            return 0;
        }
        for (j = 0; j < plan.numEntries; j++)
        {
            for (i = 0; i < numHosts; i++)
            {
                if (hosts[i].county == county && hosts[i].desc == plan.chainDesc[j])
                    out[i].mainSlot = plan.chain[j].mainSlot;
            }
        }
    }
    return numHosts;
}

bool32 IsCountyChainEntryDone(u32 county, u32 chapterSlot)
{
    const struct ChapterDesc *desc = GetCountyChapter(county, chapterSlot);

    assertf(desc != NULL, "county %d hosts no chapter in slot %d; did you mean a chapterSlot from PlanEmeraldCountyChain?", county, chapterSlot)
    {
        return FALSE;
    }
    return GetChapterStage(county, chapterSlot, desc) >= (u32)desc->numStages - 1;
}

bool32 IsCountyMainSlotDone(u32 county, u32 mainSlot)
{
    struct CountyChainPlan plan;
    u32 i, entries = 0;

    assertf(PlanEmeraldCountyChain(county, &plan), "county %d chain does not plan (problem %d); fix the plan before asking about its slots", county, plan.problem)
    {
        return FALSE;
    }
    for (i = 0; i < plan.numEntries; i++)
    {
        if (plan.chain[i].mainSlot != mainSlot)
            continue;
        entries++;
        if (GetChapterStage(county, plan.chain[i].chapterSlot, plan.chainDesc[i]) < (u32)plan.chainDesc[i]->numStages - 1)
            return FALSE;
    }
    return entries > 0;
}
