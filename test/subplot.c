#include "global.h"
#include "test/test.h"
#include "constants/flags.h"
#include "constants/region_map_sections.h"
#include "constants/vars.h"
#include "event_data.h"
#include "region_city.h"
#include "subplot.h"

#define LILYCOVE COUNTY_ID(4, 0)

static const u8 sTextA[] = _("a");
static const u8 sTextB[] = _("b");
static const u8 sTextC[] = _("c");
static const u8 sTextD[] = _("d");

static const u8 *const sListB[] = {sTextB, sTextC};

// Devon Goods shaped: the order of the predicates, not set-ness, defines the stage.
static const struct SubplotStage sDevonStages[] =
{
    {.hints = {.farCurrent = sTextA}},
    {.hints = {.farCurrent = sTextB}, .done = {STORY_PRED_FLAG, FLAG_DEVON_GOODS_STOLEN, 0}},
    {.hints = {.farCurrent = sTextC}, .done = {STORY_PRED_FLAG, FLAG_RECOVERED_DEVON_GOODS, 0}},
    {.hints = {.farCurrent = sTextD}, .done = {STORY_PRED_FLAG, FLAG_RETURNED_DEVON_GOODS, 0}},
};

static const struct SubplotDesc sDevonAdapter =
{
    .backend = SUBPLOT_ADAPTER,
    .numStages = ARRAY_COUNT(sDevonStages),
    .stages = sDevonStages,
};

// Every entry filled, with distinct texts, in the middle stage.
static const struct SubplotStage sFullStages[] =
{
    {},
    {
        .hints =
        {
            .farBefore = {sListB, ARRAY_COUNT(sListB)},
            .farCurrent = sTextA,
            .farAfter = {sListB, ARRAY_COUNT(sListB)},
            .closeBefore = sTextB,
            .closeCurrent = sTextC,
            .closeAfter = sTextD,
        },
        .numHinters = 2,
        .hinterRoles = 0x4,
    },
    {},
};

static const struct SubplotActor sActors[] =
{
    {0, 1, 0x2, 7},
    {1, 2, 0, 8},
    {2, 2, 0x1, 9},
};

static const struct SubplotDesc sFullDesc =
{
    .backend = SUBPLOT_ADAPTER,
    .numStages = ARRAY_COUNT(sFullStages),
    .numActors = ARRAY_COUNT(sActors),
    .stages = sFullStages,
    .actors = sActors,
};

TEST("Subplot: Lilycove hosts the museum subplot in slot 0 only")
{
    u32 county, slot;

    for (county = 0; county < NUM_COUNTIES; county++)
    {
        for (slot = 0; slot < SUBPLOTS_PER_COUNTY; slot++)
        {
            if (county == LILYCOVE && slot == 0)
                EXPECT(GetCountySubplot(county, slot) != NULL);
            else
                EXPECT(GetCountySubplot(county, slot) == NULL);
        }
    }
}

TEST("Subplot: Slot vars lie in the county's own slice after its mask var")
{
    u32 county, slot;

    for (county = 0; county < COUNTY_VAR_SLICES_FREE; county++)
    {
        u32 start = COUNTY_VARS_START + county * VARS_PER_COUNTY;

        for (slot = 0; slot < SUBPLOTS_PER_COUNTY; slot++)
        {
            EXPECT_EQ(GetCountySlotVar(county, slot), start + 1 + slot);
            EXPECT(GetCountySlotVar(county, slot) < start + VARS_PER_COUNTY);
        }
    }
    EXPECT(COUNTY_VAR_SLICES_FREE > LILYCOVE);
}

TEST("Subplot: A stored subplot starts at stage 0 and keeps the stage set through its slot")
{
    EXPECT_EQ(GetCountySlotStage(LILYCOVE, 0), 0);
    SetCountySlotStage(LILYCOVE, 0, 1);
    EXPECT_EQ(GetCountySlotStage(LILYCOVE, 0), 1);
    EXPECT_EQ(VarGet(GetCountySlotVar(LILYCOVE, 0)), 1);
}

TEST("Subplot: Setting one county's slot leaves other slices untouched")
{
    u32 var;

    SetCountySlotStage(LILYCOVE, 0, 1);
    for (var = COUNTY_VARS_START; var < COUNTY_VARS_START + COUNTY_VAR_SLICES_FREE * VARS_PER_COUNTY; var++)
    {
        if (var != GetCountySlotVar(LILYCOVE, 0))
            EXPECT_EQ(VarGet(var), 0);
    }
}

TEST("Subplot: The talk query follows the stage and the stage texts differ")
{
    const u8 *far0 = GetCountySlotHint(LILYCOVE, 0, SUBPLOT_FAR, 0);
    const u8 *close0 = GetCountySlotHint(LILYCOVE, 0, SUBPLOT_CLOSE, 0);

    SetCountySlotStage(LILYCOVE, 0, 1);
    EXPECT(far0 != NULL && close0 != NULL);
    EXPECT(GetCountySlotHint(LILYCOVE, 0, SUBPLOT_FAR, 0) != far0);
    EXPECT(GetCountySlotHint(LILYCOVE, 0, SUBPLOT_CLOSE, 0) != close0);
}

TEST("Subplot: The query returns the entry for each distance and beat state")
{
    // Stage 1 of sFullDesc: before = subplot at stage 0, current = 1, after = 2.
    EXPECT(GetSubplotHint(&sFullDesc, 0, 1, SUBPLOT_FAR, 0) == sTextB);
    EXPECT(GetSubplotHint(&sFullDesc, 1, 1, SUBPLOT_FAR, 0) == sTextA);
    EXPECT(GetSubplotHint(&sFullDesc, 2, 1, SUBPLOT_FAR, 0) == sTextB);
    EXPECT(GetSubplotHint(&sFullDesc, 0, 1, SUBPLOT_CLOSE, 0) == sTextB);
    EXPECT(GetSubplotHint(&sFullDesc, 1, 1, SUBPLOT_CLOSE, 0) == sTextC);
    EXPECT(GetSubplotHint(&sFullDesc, 2, 1, SUBPLOT_CLOSE, 0) == sTextD);
}

TEST("Subplot: Beat state compares the subplot stage with the beat")
{
    EXPECT_EQ(GetSubplotBeatState(0, 2), SUBPLOT_BEFORE);
    EXPECT_EQ(GetSubplotBeatState(2, 2), SUBPLOT_CURRENT);
    EXPECT_EQ(GetSubplotBeatState(3, 2), SUBPLOT_AFTER);
}

TEST("Subplot: A list entry is picked by selector, stably")
{
    EXPECT(GetSubplotHint(&sFullDesc, 0, 1, SUBPLOT_FAR, 0) == sTextB);
    EXPECT(GetSubplotHint(&sFullDesc, 0, 1, SUBPLOT_FAR, 1) == sTextC);
    EXPECT(GetSubplotHint(&sFullDesc, 0, 1, SUBPLOT_FAR, 2) == sTextB);
    EXPECT(GetSubplotHint(&sFullDesc, 0, 1, SUBPLOT_FAR, 1) == GetSubplotHint(&sFullDesc, 0, 1, SUBPLOT_FAR, 1));
}

TEST("Subplot: Missing entries and empty lists give NULL so the NPC says generic text")
{
    // Stage 0 and 2 of sFullDesc have no hints at all.
    EXPECT(GetSubplotHint(&sFullDesc, 0, 0, SUBPLOT_FAR, 0) == NULL);
    EXPECT(GetSubplotHint(&sFullDesc, 2, 0, SUBPLOT_FAR, 5) == NULL); // empty far-after list
    EXPECT(GetSubplotHint(&sFullDesc, 0, 2, SUBPLOT_FAR, 5) == NULL); // empty far-before list
    EXPECT(GetSubplotHint(&sFullDesc, 0, 2, SUBPLOT_CLOSE, 0) == NULL); // optional close-before
    EXPECT(GetSubplotHint(&sFullDesc, 1, 1, SUBPLOT_CLOSE, 0) != NULL);
}

TEST("Subplot: The county query uses the current beat, or any beat on request")
{
    SetCountySlotStage(LILYCOVE, 0, 1);
    EXPECT(GetCountySlotHint(LILYCOVE, 0, SUBPLOT_CLOSE, 0) == GetCountySlotHintAbout(LILYCOVE, 0, 1, SUBPLOT_CLOSE, 0));
    EXPECT(GetCountySlotHintAbout(LILYCOVE, 0, 0, SUBPLOT_CLOSE, 0) != NULL); // "the exhibit hall is open at last"
    EXPECT(GetCountySlotHintAbout(LILYCOVE, 0, 0, SUBPLOT_CLOSE, 0) != GetCountySlotHint(LILYCOVE, 0, SUBPLOT_CLOSE, 0));
}

TEST("Subplot: Each ported subplot fills its required hint entries")
{
    u32 county, slot, stage;

    for (county = 0; county < NUM_COUNTIES; county++)
    {
        for (slot = 0; slot < SUBPLOTS_PER_COUNTY; slot++)
        {
            const struct SubplotDesc *desc = GetCountySubplot(county, slot);

            if (desc == NULL)
                continue;
            for (stage = 0; stage < desc->numStages; stage++)
            {
                EXPECT(desc->stages[stage].hints.farCurrent != NULL);
                EXPECT(desc->stages[stage].hints.closeCurrent != NULL);
                if (stage + 1 < desc->numStages)
                    EXPECT(desc->stages[stage].hints.closeAfter != NULL);
            }
        }
    }
}

TEST("Subplot: Hinter counts come from the stage")
{
    EXPECT_EQ(sFullDesc.stages[1].numHinters, 2);
    EXPECT_EQ(GetCountySubplot(LILYCOVE, 0)->stages[0].numHinters, 1);
}

TEST("Subplot: Actors are listed for the stages they cover")
{
    const struct SubplotActor *out[2];

    EXPECT_EQ(GetSubplotActors(&sFullDesc, 0, out, 2), 1);
    EXPECT_EQ(out[0]->gfxId, 7);
    EXPECT_EQ(GetSubplotActors(&sFullDesc, 1, out, 2), 2);
    EXPECT_EQ(out[0]->gfxId, 7);
    EXPECT_EQ(out[1]->gfxId, 8);
    EXPECT_EQ(GetSubplotActors(&sFullDesc, 2, out, 1), 2); // total, even past maxOut
    EXPECT_EQ(out[0]->gfxId, 8);
    EXPECT_EQ(GetSubplotActors(GetCountySubplot(LILYCOVE, 0), 0, out, 2), 0);
}

TEST("Subplot: An adapter subplot's stage is the highest true predicate")
{
    EXPECT_EQ(GetSubplotStage(0, 0, &sDevonAdapter), 0);
    FlagSet(FLAG_DEVON_GOODS_STOLEN);
    EXPECT_EQ(GetSubplotStage(0, 0, &sDevonAdapter), 1);
    FlagSet(FLAG_RECOVERED_DEVON_GOODS);
    FlagClear(FLAG_DEVON_GOODS_STOLEN); // the game clears STOLEN on recovery
    EXPECT_EQ(GetSubplotStage(0, 0, &sDevonAdapter), 2);
    FlagSet(FLAG_RETURNED_DEVON_GOODS);
    EXPECT_EQ(GetSubplotStage(0, 0, &sDevonAdapter), 3);
}

TEST("Subplot: An adapter subplot skipped ahead reports the later stage")
{
    FlagSet(FLAG_RETURNED_DEVON_GOODS);
    EXPECT_EQ(GetSubplotStage(0, 0, &sDevonAdapter), 3);
}

TEST("Subplot: The hint query treats adapter and stored subplots alike")
{
    FlagSet(FLAG_RECOVERED_DEVON_GOODS);
    EXPECT(GetSubplotHint(&sDevonAdapter, GetSubplotStage(0, 0, &sDevonAdapter), 2, SUBPLOT_FAR, 0) == sTextC);
    EXPECT(GetSubplotHint(&sDevonAdapter, 3, 3, SUBPLOT_FAR, 0) == sTextD);
}

TEST("Subplot: A county is found from its city's MAPSEC")
{
    EXPECT_EQ(GetCountyFromCityMapSec(MAPSEC_LILYCOVE_CITY), LILYCOVE);
}
