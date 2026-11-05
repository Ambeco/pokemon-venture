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

// Devon Goods shaped: the order of the predicates, not set-ness, defines the stage.
static const struct SubplotStage sDevonStages[] =
{
    {.hintText = sTextA, .stateText = sTextA},
    {.hintText = sTextB, .stateText = sTextB, .done = {STORY_PRED_FLAG, FLAG_DEVON_GOODS_STOLEN, 0}},
    {.hintText = sTextC, .stateText = sTextC, .done = {STORY_PRED_FLAG, FLAG_RECOVERED_DEVON_GOODS, 0}},
    {.hintText = sTextD, .stateText = sTextD, .done = {STORY_PRED_FLAG, FLAG_RETURNED_DEVON_GOODS, 0}},
};

static const struct SubplotDesc sDevonAdapter =
{
    .backend = SUBPLOT_ADAPTER,
    .numStages = ARRAY_COUNT(sDevonStages),
    .stages = sDevonStages,
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
    const u8 *hint0 = GetCountySlotText(LILYCOVE, 0, SUBPLOT_TEXT_HINT);
    const u8 *state0 = GetCountySlotText(LILYCOVE, 0, SUBPLOT_TEXT_STATE);

    SetCountySlotStage(LILYCOVE, 0, 1);
    EXPECT(hint0 != NULL && state0 != NULL);
    EXPECT(GetCountySlotText(LILYCOVE, 0, SUBPLOT_TEXT_HINT) != hint0);
    EXPECT(GetCountySlotText(LILYCOVE, 0, SUBPLOT_TEXT_STATE) != state0);
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

TEST("Subplot: The text query treats adapter and stored subplots alike")
{
    FlagSet(FLAG_RECOVERED_DEVON_GOODS);
    EXPECT(GetSubplotText(&sDevonAdapter, GetSubplotStage(0, 0, &sDevonAdapter), SUBPLOT_TEXT_HINT) == sTextC);
    EXPECT(GetSubplotText(&sDevonAdapter, 3, SUBPLOT_TEXT_STATE) == sTextD);
}

TEST("Subplot: A county is found from its city's MAPSEC")
{
    EXPECT_EQ(GetCountyFromCityMapSec(MAPSEC_LILYCOVE_CITY), LILYCOVE);
}
