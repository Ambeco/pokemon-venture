// Subplots ported to the descriptor and where Emerald hosts them (design.md, "Subplot abstraction").

enum LilycoveMuseumStage
{
    LILYCOVE_MUSEUM_EXHIBIT_NOT_SHOWN,
    LILYCOVE_MUSEUM_EXHIBIT_SHOWN,
};

// The museum is an in-city special location, not a BuildingRole yet, so no role is required or triggered.
static const struct SubplotStage sLilycoveMuseumStages[] =
{
    [LILYCOVE_MUSEUM_EXHIBIT_NOT_SHOWN] =
    {
        .hintText = gText_LilycoveMuseumHint_NotShown,
        .stateText = gText_LilycoveMuseumState_NotShown,
    },
    [LILYCOVE_MUSEUM_EXHIBIT_SHOWN] =
    {
        .hintText = gText_LilycoveMuseumHint_Shown,
        .stateText = gText_LilycoveMuseumState_Shown,
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
