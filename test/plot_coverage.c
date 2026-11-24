#include "global.h"
#include "test/test.h"
#include "chapter.h"
#include "region_county.h"
#include "region_plot.h"

// Every get of the vanilla main plot must be supplied by a hosted chapter, or be an arc beat, special
// item, Devon Goods step or Wally battle (hosted by other mechanisms, county_plan.c `IsChapterSuppliedKind`).

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

TEST("PlotCoverage: Every get is supplied by a chapter or hosted by another mechanism")
{
    u32 index, i, count, missing = 0;
    struct PlotSlot items[MAX_GETS_PER_LIST];

    for (index = 0; index < NUM_COUNTIES * 4; index++)
    {
        count = ResolveVanillaPlotList(index, items, MAX_GETS_PER_LIST);
        for (i = 0; i < count; i++)
        {
            if (IsHostedByOtherMechanism(items[i].kind) || IsGetSupplied(&items[i]))
                continue;
            missing++;
            Test_MgbaPrintf("no chapter supplies the get {kind %d, ordinal %d} (list %d); did you mean to host a chapter that supplies it (emerald_chapters.h, PLOT_GET_CHAPTER)?", items[i].kind, items[i].ordinal, index);
        }
    }
    EXPECT_EQ(missing, 0);
}
