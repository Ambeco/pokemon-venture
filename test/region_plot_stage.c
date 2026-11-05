#include "global.h"
#include "test/test.h"
#include "constants/flags.h"
#include "constants/vars.h"
#include "event_data.h"
#include "plot_stage.h"
#include "region_city.h"
#include "region_plot.h"

#define NUM_MAIN_INDICES 96

struct Step
{
    u16 flag;
    u16 var;
    u16 value;
    u8 expected; // main-story-index after the step
};

#define SETF(flag, expected) {flag, 0, 0, expected}
#define SETV(var, value, expected) {0, var, value, expected}

static void Apply(const struct Step *step)
{
    if (step->flag)
        FlagSet(step->flag);
    else
        VarSet(step->var, step->value);
}

// Vanilla Emerald order. Gets that the game lets the player skip (Good Rod, Wailmer Pail, Cut, ...)
// are never set, so reaching the end also proves a skipped get does not freeze the index.
static const struct Step sWalkthrough[] =
{
    SETF(FLAG_SYS_POKEMON_GET, 4),
    SETF(FLAG_DEFEATED_RIVAL_ROUTE103, 5),
    SETF(FLAG_RECEIVED_POKEDEX_FROM_BIRCH, 8),
    SETV(VAR_PETALBURG_CITY_STATE, 3, 8),
    SETV(VAR_PETALBURG_WOODS_STATE, 1, 12),
    SETF(FLAG_BADGE01_GET, 12),
    SETV(VAR_RUSTBORO_CITY_STATE, 2, 14),
    SETF(FLAG_RECOVERED_DEVON_GOODS, 15),
    SETF(FLAG_RETURNED_DEVON_GOODS, 16),
    SETF(FLAG_BADGE02_GET, 18),
    SETF(FLAG_DELIVERED_STEVEN_LETTER, 24),
    SETV(VAR_ROUTE110_STATE, 1, 30),
    SETF(FLAG_BADGE03_GET, 44),
    SETF(FLAG_BADGE04_GET, 48),
    SETV(VAR_ROUTE119_STATE, 1, 49),
    SETF(FLAG_BADGE05_GET, 49),
    SETF(FLAG_RECEIVED_HM_SURF, 50),
    SETF(FLAG_BADGE06_GET, 56),
    SETF(FLAG_RECEIVED_RED_OR_BLUE_ORB, 57),
    SETF(FLAG_GROUDON_AWAKENED_MAGMA_HIDEOUT, 61),
    SETV(VAR_SLATEPORT_HARBOR_STATE, 2, 62),
    SETF(FLAG_TEAM_AQUA_ESCAPED_IN_SUBMARINE, 68),
    SETF(FLAG_BADGE07_GET, 68),
    SETF(FLAG_RECEIVED_HM_DIVE, 68),
    SETV(VAR_MOSSDEEP_SPACE_CENTER_STATE, 3, 70),
    SETF(FLAG_KYOGRE_ESCAPED_SEAFLOOR_CAVERN, 72),
    SETF(FLAG_WALLACE_GOES_TO_SKY_PILLAR, 76),
    SETV(VAR_SKY_PILLAR_STATE, 1, 77),
    SETF(FLAG_SOOTOPOLIS_ARCHIE_MAXIE_LEAVE, 77),
    SETF(FLAG_BADGE08_GET, 77),
    SETF(FLAG_RECEIVED_HM_WATERFALL, 81),
    SETF(FLAG_DEFEATED_WALLY_VICTORY_ROAD, 82),
    SETF(FLAG_ENTERED_ELITE_FOUR, 82),
    SETF(FLAG_SYS_GAME_CLEAR, MAIN_STORY_INDEX_POSTGAME),
};

// Every get that Emerald leaves skippable, and the Poké Balls' stand-in. Setting all of them must
// not move the index.
static const u16 sReportedFlags[] =
{
    FLAG_RECEIVED_RUNNING_SHOES, FLAG_RECEIVED_WAILMER_PAIL, FLAG_RECEIVED_HM_CUT, FLAG_RECEIVED_POKENAV,
    FLAG_DEFEATED_RIVAL_RUSTBORO, FLAG_RECEIVED_HM_FLASH, FLAG_RECEIVED_OLD_ROD, FLAG_DOCK_REJECTED_DEVON_GOODS,
    FLAG_DELIVERED_DEVON_GOODS, FLAG_DEFEATED_WALLY_MAUVILLE, FLAG_RECEIVED_HM_ROCK_SMASH, FLAG_RECEIVED_COIN_CASE,
    FLAG_RECEIVED_BIKE, FLAG_RECEIVED_HM_STRENGTH, FLAG_MET_ARCHIE_METEOR_FALLS,
    FLAG_DEFEATED_EVIL_TEAM_MT_CHIMNEY, FLAG_RECEIVED_GO_GOGGLES, FLAG_RECEIVED_DEVON_SCOPE, FLAG_RECEIVED_HM_FLY,
    FLAG_RECEIVED_CASTFORM, FLAG_RECEIVED_GOOD_ROD, FLAG_MET_RIVAL_LILYCOVE, FLAG_RECEIVED_POKEBLOCK_CASE,
    FLAG_RECEIVED_SUPER_ROD,
};

TEST("RegionPlotStage: A fresh game is at index 0")
{
    EXPECT_EQ(GetMainStoryIndex(), 0);
}

TEST("RegionPlotStage: A vanilla walkthrough advances the index to the expected list after each step and never goes back")
{
    u32 i, previous = 0;

    for (i = 0; i < ARRAY_COUNT(sWalkthrough); i++)
    {
        u32 index;

        Apply(&sWalkthrough[i]);
        index = GetMainStoryIndex();
        EXPECT_EQ(index, sWalkthrough[i].expected);
        EXPECT(index >= previous);
        previous = index;
    }
}

TEST("RegionPlotStage: Reported-only gets never hold or move the index")
{
    u32 i;

    for (i = 0; i < ARRAY_COUNT(sReportedFlags); i++)
        FlagSet(sReportedFlags[i]);
    EXPECT_EQ(GetMainStoryIndex(), 0);
}

TEST("RegionPlotStage: Skipping every reported-only get still reaches the post-game")
{
    u32 i;

    for (i = 0; i < ARRAY_COUNT(sWalkthrough); i++)
        Apply(&sWalkthrough[i]);
    for (i = 0; i < ARRAY_COUNT(sReportedFlags); i++)
        EXPECT(!FlagGet(sReportedFlags[i]));
    EXPECT_EQ(GetMainStoryIndex(), MAIN_STORY_INDEX_POSTGAME);
}

TEST("RegionPlotStage: A true later predicate does not skip an earlier undone list")
{
    FlagSet(FLAG_BADGE06_GET);
    FlagSet(FLAG_RECEIVED_HM_FLY);
    FlagSet(FLAG_RECEIVED_HM_STRENGTH);
    FlagSet(FLAG_SYS_GAME_CLEAR);
    EXPECT_EQ(GetMainStoryIndex(), 0);
}

TEST("RegionPlotStage: Fly and Badge 6 granted with RB4 do not pass the Surf list")
{
    u32 i;

    for (i = 0; i < ARRAY_COUNT(sWalkthrough) && sWalkthrough[i].expected < 48; i++)
        Apply(&sWalkthrough[i]);
    Apply(&sWalkthrough[i]);
    EXPECT_EQ(GetMainStoryIndex(), 48);
    FlagSet(FLAG_RECEIVED_HM_FLY);
    EXPECT_EQ(GetMainStoryIndex(), 48);
    VarSet(VAR_ROUTE119_STATE, 1);
    EXPECT_EQ(GetMainStoryIndex(), 49);
    FlagSet(FLAG_BADGE06_GET);
    EXPECT_EQ(GetMainStoryIndex(), 49);
    FlagSet(FLAG_BADGE05_GET);
    FlagSet(FLAG_RECEIVED_HM_SURF);
    EXPECT_EQ(GetMainStoryIndex(), 56);
}

TEST("RegionPlotStage: Strength before Badge 3 leaves the index at the Rock Smash gym")
{
    u32 i;

    for (i = 0; i < ARRAY_COUNT(sWalkthrough) && sWalkthrough[i].expected <= 30; i++)
        Apply(&sWalkthrough[i]);
    EXPECT_EQ(GetMainStoryIndex(), 30);
    FlagSet(FLAG_RECEIVED_HM_STRENGTH);
    EXPECT_EQ(GetMainStoryIndex(), 30);
}

TEST("RegionPlotStage: RB2 is reported at either venue and never blocks")
{
    struct PlotListStage stage = GetPlotListStage(14);

    EXPECT_EQ(stage.numGets, 3); // RB2, Devon Goods recovered, PokeNav
    EXPECT_EQ(stage.blockingMask, 0x2);
    EXPECT_EQ(stage.doneMask & 1, 0);
    FlagSet(FLAG_DEFEATED_RIVAL_ROUTE_104);
    EXPECT_EQ(GetPlotListStage(14).doneMask & 1, 1);
    FlagClear(FLAG_DEFEATED_RIVAL_ROUTE_104);
    EXPECT_EQ(GetPlotListStage(14).doneMask & 1, 0);
    FlagSet(FLAG_DEFEATED_RIVAL_RUSTBORO);
    EXPECT_EQ(GetPlotListStage(14).doneMask & 1, 1);
}

TEST("RegionPlotStage: A gym list holds on its badge and HM, a displaced HM stands alone")
{
    struct PlotListStage stage = GetPlotListStage(49);

    EXPECT_EQ(stage.numGets, 2); // G5 badge, Surf
    EXPECT_EQ(stage.blockingMask, 0x3);
    FlagSet(FLAG_RECEIVED_HM_SURF);
    stage = GetPlotListStage(49);
    EXPECT_EQ(stage.doneMask, 0x2);
    EXPECT(!IsPlotListDone(&stage)); // badge still missing

    stage = GetPlotListStage(32); // Strength at Rusturf: HM only, not holding
    EXPECT_EQ(stage.numGets, 1);
    EXPECT_EQ(stage.blockingMask, 0);
    EXPECT(IsPlotListDone(&stage));
    EXPECT_EQ(stage.doneMask, 0);
    FlagSet(FLAG_RECEIVED_HM_STRENGTH);
    EXPECT_EQ(GetPlotListStage(32).doneMask, 1);

    stage = GetPlotListStage(44); // G4 without its displaced HM: the badge alone
    EXPECT_EQ(stage.numGets, 2); // Mt. Chimney beat, G4 badge
}

TEST("RegionPlotStage: Only the Letter and the two implicit unlocks have a get; other step-3 lists are empty")
{
    u32 index;

    for (index = 3; index < NUM_MAIN_INDICES; index += 4)
        EXPECT_EQ(GetPlotListStage(index).numGets, (index == 15 || index == 35 || index == 47) ? 1 : 0);
    EXPECT_EQ(GetPlotListStage(15).blockingMask, 1);
    EXPECT_EQ(GetPlotListStage(15).doneMask, 0);
    FlagSet(FLAG_RETURNED_DEVON_GOODS);
    EXPECT_EQ(GetPlotListStage(15).doneMask, 1);
}

TEST("RegionPlotStage: Fallarbor's unlock reads Fiery Path and Fortree's the Route 119 rival, neither blocking")
{
    EXPECT_EQ(GetPlotListStage(35).blockingMask, 0);
    EXPECT_EQ(GetPlotListStage(35).doneMask, 0);
    FlagSet(FLAG_LANDMARK_FIERY_PATH);
    EXPECT_EQ(GetPlotListStage(35).doneMask, 1);
    EXPECT_EQ(GetPlotListStage(47).blockingMask, 0);
    EXPECT_EQ(GetPlotListStage(47).doneMask, 0);
    VarSet(VAR_ROUTE119_STATE, 1);
    EXPECT_EQ(GetPlotListStage(47).doneMask, 1);
}

TEST("RegionPlotStage: Every vanilla slot has a descriptor")
{
    struct PlotSlot slots[MAX_GETS_PER_LIST];
    u32 index, i, total = 0;

    for (index = 0; index < NUM_MAIN_INDICES; index++)
    {
        u32 count = ResolveVanillaPlotList(index, slots, ARRAY_COUNT(slots));
        struct PlotListStage stage = GetPlotListStage(index);
        u32 expectedGets = (index == 15 || index == 35 || index == 47) ? 1 : 0;

        for (i = 0; i < count; i++)
        {
            struct PlotGetDesc descs[2];
            u32 n = GetPlotSlotGetDescs(&slots[i], descs);
            bool32 gymWithHm = slots[i].kind == PLOT_KIND_GYM && !slots[i].hmOnly && slots[i].hm != PLOT_HM_NONE;

            EXPECT_EQ(n, gymWithHm ? 2 : 1);
            expectedGets += n;
            if (slots[i].kind != PLOT_KIND_GYM)
                total++;
        }
        EXPECT_EQ(stage.numGets, expectedGets);
        EXPECT((stage.blockingMask & ~((1u << stage.numGets) - 1)) == 0);
    }
    // No descriptor is orphaned or duplicated: one per non-gym item.
    EXPECT_EQ(total, GetVanillaPlotGetEntryCount());
}

TEST("RegionPlotStage: Predicate operators")
{
    struct StoryPredicate flag = {STORY_PRED_FLAG, FLAG_BADGE01_GET, 0};
    struct StoryPredicate either = {STORY_PRED_ANY_FLAG, FLAG_BADGE01_GET, FLAG_BADGE02_GET};
    struct StoryPredicate var = {STORY_PRED_VAR_GE, VAR_ROUTE110_STATE, 2};
    struct StoryPredicate always = {STORY_PRED_ALWAYS, 0, 0};

    EXPECT(!IsStoryPredicateTrue(&flag));
    EXPECT(!IsStoryPredicateTrue(&either));
    EXPECT(!IsStoryPredicateTrue(&var));
    EXPECT(IsStoryPredicateTrue(&always));
    FlagSet(FLAG_BADGE02_GET);
    EXPECT(!IsStoryPredicateTrue(&flag));
    EXPECT(IsStoryPredicateTrue(&either));
    VarSet(VAR_ROUTE110_STATE, 1);
    EXPECT(!IsStoryPredicateTrue(&var));
    VarSet(VAR_ROUTE110_STATE, 2);
    EXPECT(IsStoryPredicateTrue(&var));
    VarSet(VAR_ROUTE110_STATE, 3);
    EXPECT(IsStoryPredicateTrue(&var));
}
