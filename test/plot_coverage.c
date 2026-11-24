#include "global.h"
#include "test/test.h"
#include "chapter.h"
#include "region_county.h"
#include "region_plot.h"

// Coverage worklist: every get of the vanilla main plot must be supplied by a hosted chapter, or be an arc beat, special
// item, Devon Goods step or Wally battle (hosted by other mechanisms, county_plan.c `IsChapterSuppliedKind`). The gets
// not yet converted are pinned in sUnsuppliedGets; converting one makes the test name the line to delete.

struct UnsuppliedGet
{
    u8 kind;
    u8 ordinal;
};

static const struct UnsuppliedGet sUnsuppliedGets[] =
{
    {PLOT_KIND_GYM, 9}, // Ever Grande lobby
    {PLOT_KIND_RIVAL, 1}, {PLOT_KIND_RIVAL, 2}, {PLOT_KIND_RIVAL, 3}, {PLOT_KIND_RIVAL, 4}, {PLOT_KIND_RIVAL, 5},
    {PLOT_KIND_UTILITY, PLOT_UTILITY_POKENAV}, {PLOT_KIND_UTILITY, PLOT_UTILITY_GOOD_ROD},
    {PLOT_KIND_UTILITY, PLOT_UTILITY_ITEMFINDER}, {PLOT_KIND_UTILITY, PLOT_UTILITY_POKEBLOCK_CASE},
    {PLOT_KIND_SCRIPTED, PLOT_SCRIPTED_STARTER}, {PLOT_KIND_SCRIPTED, PLOT_SCRIPTED_POKEDEX},
    {PLOT_KIND_SCRIPTED, PLOT_SCRIPTED_POKE_BALLS}, {PLOT_KIND_SCRIPTED, PLOT_SCRIPTED_RUNNING_SHOES},
    {PLOT_KIND_SCRIPTED, PLOT_SCRIPTED_SHIPYARD}, {PLOT_KIND_SCRIPTED, PLOT_SCRIPTED_ELITE_FOUR},
    {PLOT_KIND_SCRIPTED, PLOT_SCRIPTED_CHAMPION},
};

static bool32 IsHostedByOtherMechanism(u32 kind)
{
    return kind == PLOT_KIND_ARC || kind == PLOT_KIND_SPECIAL || kind == PLOT_KIND_DEVON || kind == PLOT_KIND_WALLY;
}

static bool32 IsGetSupplied(const struct PlotSlot *item)
{
    u32 numHosts, i;
    const struct ChapterHost *hosts = GetChapterHosts(&numHosts);

    for (i = 0; i < numHosts; i++)
    {
        struct PlotSlot supplied;

        if (hosts[i].desc->mainHm != PLOT_HM_NONE && item->hmOnly && hosts[i].desc->mainHm == item->hm)
            return TRUE;
        if (GetChapterSuppliedSlot(hosts[i].desc, &supplied) && supplied.kind == item->kind && supplied.ordinal == item->ordinal)
            return TRUE;
    }
    return FALSE;
}

static bool32 IsPinnedUnsupplied(const struct PlotSlot *item)
{
    u32 i;

    for (i = 0; i < ARRAY_COUNT(sUnsuppliedGets); i++)
    {
        if (sUnsuppliedGets[i].kind == item->kind && sUnsuppliedGets[i].ordinal == item->ordinal)
            return TRUE;
    }
    return FALSE;
}

static bool32 IsPinStillUnsupplied(const struct UnsuppliedGet *pin)
{
    u32 index, i, count;
    struct PlotSlot items[MAX_GETS_PER_LIST];

    for (index = 0; index < NUM_COUNTIES * 4; index++)
    {
        count = ResolveVanillaPlotList(index, items, MAX_GETS_PER_LIST);
        for (i = 0; i < count; i++)
        {
            if (items[i].kind == pin->kind && items[i].ordinal == pin->ordinal && !IsGetSupplied(&items[i]))
                return TRUE;
        }
    }
    return FALSE;
}

TEST("PlotCoverage: Every get is supplied by a chapter, hosted by another mechanism, or pinned in the worklist")
{
    u32 index, i, count, missing = 0, stale = 0;
    struct PlotSlot items[MAX_GETS_PER_LIST];

    for (index = 0; index < NUM_COUNTIES * 4; index++)
    {
        count = ResolveVanillaPlotList(index, items, MAX_GETS_PER_LIST);
        for (i = 0; i < count; i++)
        {
            if (IsHostedByOtherMechanism(items[i].kind) || IsGetSupplied(&items[i]) || IsPinnedUnsupplied(&items[i]))
                continue;
            missing++;
            Test_MgbaPrintf("no chapter supplies the get {kind %d, ordinal %d} (list %d); did you mean to port it, or add {%d, %d} to sUnsuppliedGets?", items[i].kind, items[i].ordinal, index, items[i].kind, items[i].ordinal);
        }
    }
    for (i = 0; i < ARRAY_COUNT(sUnsuppliedGets); i++)
    {
        if (!IsPinStillUnsupplied(&sUnsuppliedGets[i]))
        {
            stale++;
            Test_MgbaPrintf("{%d, %d} is now supplied (or gone); did you mean to delete it from sUnsuppliedGets?", sUnsuppliedGets[i].kind, sUnsuppliedGets[i].ordinal);
        }
    }
    EXPECT_EQ(missing, 0);
    EXPECT_EQ(stale, 0);
}
