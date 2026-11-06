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

static const struct SubplotDesc sLilycoveMuseumSubplot =
{
    .backend = SUBPLOT_STORED,
    .numStages = ARRAY_COUNT(sLilycoveMuseumStages),
    .stages = sLilycoveMuseumStages,
};

struct SubplotHost
{
    u8 county;
    u8 slot;
    const struct SubplotDesc *desc;
};

static const struct SubplotHost sEmeraldSubplotHosts[] =
{
    {COUNTY_ID(4, 0), 0, &sLilycoveMuseumSubplot}, // Lilycove, county-story-index 15
};
