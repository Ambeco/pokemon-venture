// Subplots ported to the descriptor and where Emerald hosts them (design.md, "Subplot abstraction").

enum LilycoveMuseumStage
{
    LILYCOVE_MUSEUM_EXHIBIT_NOT_SHOWN,
    LILYCOVE_MUSEUM_EXHIBIT_SHOWN,
};

// The museum is an in-city special location, not a BuildingRole, so the curator (an immobile NPC) is anchored
// by its location kind. Hinters may stand anywhere in the county, one per stage.
static const struct SubplotStage sLilycoveMuseumStages[] =
{
    [LILYCOVE_MUSEUM_EXHIBIT_NOT_SHOWN] =
    {
        .hints =
        {
            .farCurrent = gText_Lilycove_Exhibit0FarCurrent,
            .farAfter = {sLilycoveExhibit0FarAfter, ARRAY_COUNT(sLilycoveExhibit0FarAfter)},
            .closeCurrent = gText_Lilycove_Exhibit0CloseCurrent,
            .closeAfter = gText_Lilycove_Exhibit0CloseAfter,
        },
        .anchor = {BEAT_ANCHOR_NPC, ROUTE_ID_NONE, ROUTE_ID_NONE, TOWN_LOCATION_MUSEUM, 0},
        .numHinters = 1,
    },
    [LILYCOVE_MUSEUM_EXHIBIT_SHOWN] =
    {
        .hints =
        {
            .farCurrent = gText_Lilycove_Exhibit1FarCurrent,
            .closeBefore = gText_Lilycove_Exhibit1CloseBefore,
            .closeCurrent = gText_Lilycove_Exhibit1CloseCurrent,
        },
        .anchor = {BEAT_ANCHOR_NPC, ROUTE_ID_NONE, ROUTE_ID_NONE, TOWN_LOCATION_MUSEUM, 0},
        .numHinters = 1,
    },
};

enum RustboroCutterStage
{
    RUSTBORO_CUTTER_HM_NOT_GIVEN,
    RUSTBORO_CUTTER_HM_GIVEN,
};

// The Cutter's House holds an immobile NPC of the generic house role. HM Cut is a gate (the key that opens
// Petalburg Woods and Rusturf is the HM and badge, a planner item), so the subplot produces no key; the
// script still sets FLAG_RECEIVED_HM_CUT, which the main-plot adapter reads.
static const struct SubplotStage sRustboroCutterStages[] =
{
    [RUSTBORO_CUTTER_HM_NOT_GIVEN] =
    {
        .hints =
        {
            .farCurrent = gText_Rustboro_Cut0FarCurrent,
            .farAfter = {sRustboroCut0FarAfter, ARRAY_COUNT(sRustboroCut0FarAfter)},
            .closeCurrent = gText_Rustboro_Cut0CloseCurrent,
            .closeAfter = gText_Rustboro_Cut0CloseAfter,
        },
        .anchor = {BEAT_ANCHOR_NPC, ROUTE_ID_NONE, ROUTE_ID_NONE, BEAT_LOCATION_NONE, BUILDING_HOUSE},
        .numHinters = 1,
    },
    [RUSTBORO_CUTTER_HM_GIVEN] =
    {
        .hints =
        {
            .farCurrent = gText_Rustboro_Cut1FarCurrent,
            .closeBefore = gText_Rustboro_Cut1CloseBefore,
            .closeCurrent = gText_Rustboro_Cut1CloseCurrent,
        },
        .anchor = {BEAT_ANCHOR_NPC, ROUTE_ID_NONE, ROUTE_ID_NONE, BEAT_LOCATION_NONE, BUILDING_HOUSE},
        .numHinters = 1,
    },
};

static const struct SubplotDesc sRustboroCutterSubplot =
{
    .backend = SUBPLOT_STORED,
    .numStages = ARRAY_COUNT(sRustboroCutterStages),
    .stages = sRustboroCutterStages,
};

static const struct SubplotDesc sLilycoveMuseumSubplot =
{
    .backend = SUBPLOT_STORED,
    .numStages = ARRAY_COUNT(sLilycoveMuseumStages),
    .stages = sLilycoveMuseumStages,
};

static const struct SubplotHost sEmeraldSubplotHosts[] =
{
    {COUNTY_ID(0, 1), 0, &sRustboroCutterSubplot}, // Rustboro, county-story-index 3
    {COUNTY_ID(4, 0), 0, &sLilycoveMuseumSubplot}, // Lilycove, county-story-index 15
};
