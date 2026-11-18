// Subplots ported to the descriptor and where Emerald hosts them (design_subplots.md, "Subplot abstraction").

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
        .anchor = {BEAT_ANCHOR_NPC, ROUTE_ID_NONE, ROUTE_ID_NONE, TOWN_LOCATION_MUSEUM, 0},
        .numHinters = 1,
    },
    [LILYCOVE_MUSEUM_EXHIBIT_SHOWN] =
    {
        .anchor = {BEAT_ANCHOR_NPC, ROUTE_ID_NONE, ROUTE_ID_NONE, TOWN_LOCATION_MUSEUM, 0},
        .numHinters = 1,
    },
};

enum LilycoveCaseStage
{
    LILYCOVE_CASE_NOT_GIVEN,
    LILYCOVE_CASE_GIVEN,
};

// The Pokéblock Case is a reported-only plot get (FLAG_RECEIVED_POKEBLOCK_CASE, still set by the script and read by
// the adapter), so the subplot produces no key. The receptionist is an immobile NPC of the contest hall role; no hinters
// are placed yet (numHinters 0), since nothing binds NPCs for generated Lilycove maps.
static const struct SubplotStage sLilycoveCaseStages[] =
{
    [LILYCOVE_CASE_NOT_GIVEN] = {.anchor = {BEAT_ANCHOR_NPC, ROUTE_ID_NONE, ROUTE_ID_NONE, BEAT_LOCATION_NONE, BUILDING_CONTEST_HALL}},
    [LILYCOVE_CASE_GIVEN] = {.anchor = {BEAT_ANCHOR_NPC, ROUTE_ID_NONE, ROUTE_ID_NONE, BEAT_LOCATION_NONE, BUILDING_CONTEST_HALL}},
};

static const struct SubplotHint sLilycoveCaseHints[] =
{
    {0, 0, TEXT_LIST(sLilycoveCase0Far), TEXT_LIST(sLilycoveCase0Close)},
    {1, 1, TEXT_LIST(sLilycoveCase1Far), TEXT_LIST(sLilycoveCase1Close)},
};

static const struct SubplotDesc sLilycoveCaseSubplot =
{
    .backend = SUBPLOT_STORED,
    .numStages = ARRAY_COUNT(sLilycoveCaseStages),
    .stages = sLilycoveCaseStages,
    .numHints = ARRAY_COUNT(sLilycoveCaseHints),
    .hints = sLilycoveCaseHints,
    .requiredRoles = BUILDING_CONTEST_HALL,
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
        .anchor = {BEAT_ANCHOR_NPC, ROUTE_ID_NONE, ROUTE_ID_NONE, BEAT_LOCATION_NONE, BUILDING_HOUSE},
        .numHinters = 1,
    },
    [RUSTBORO_CUTTER_HM_GIVEN] =
    {
        .anchor = {BEAT_ANCHOR_NPC, ROUTE_ID_NONE, ROUTE_ID_NONE, BEAT_LOCATION_NONE, BUILDING_HOUSE},
        .numHinters = 1,
    },
};

// Hint ranges follow the stages; each lists the lines that were once keyed by beat and state, so any NPC
// of the subplot may say any of them.
enum MossdeepSuperRodStage
{
    MOSSDEEP_SUPER_ROD_NOT_GIVEN,
    MOSSDEEP_SUPER_ROD_GIVEN,
};

// The fisherman is an immobile NPC of the generic house role, and his talk runs the scene MossdeepSuperRod. The
// Super Rod is a reported-only plot get, read by the adapter through FLAG_RECEIVED_SUPER_ROD, which the scene still
// sets; the subplot produces no key. No hinters are placed yet (numHinters 0): nothing binds NPCs for Mossdeep, so the
// hint texts are reachable only through the county query.
static const struct SubplotStage sMossdeepSuperRodStages[] =
{
    [MOSSDEEP_SUPER_ROD_NOT_GIVEN] = {.anchor = {BEAT_ANCHOR_NPC, ROUTE_ID_NONE, ROUTE_ID_NONE, BEAT_LOCATION_NONE, BUILDING_HOUSE}},
    [MOSSDEEP_SUPER_ROD_GIVEN] = {.anchor = {BEAT_ANCHOR_NPC, ROUTE_ID_NONE, ROUTE_ID_NONE, BEAT_LOCATION_NONE, BUILDING_HOUSE}},
};

static const struct SubplotHint sMossdeepSuperRodHints[] =
{
    {0, 0, TEXT_LIST(sMossdeepStage0Far), TEXT_LIST(sMossdeepStage0Close)},
    {1, 1, TEXT_LIST(sMossdeepStage1Far), TEXT_LIST(sMossdeepStage1Close)},
};

static const struct SubplotHint sRustboroCutterHints[] =
{
    {0, 0, TEXT_LIST(sRustboroStage0Far), TEXT_LIST(sRustboroStage0Close)},
    {1, 1, TEXT_LIST(sRustboroStage1Far), TEXT_LIST(sRustboroStage1Close)},
};

static const struct SubplotHint sLilycoveMuseumHints[] =
{
    {0, 0, TEXT_LIST(sLilycoveStage0Far), TEXT_LIST(sLilycoveStage0Close)},
    {1, 1, TEXT_LIST(sLilycoveStage1Far), TEXT_LIST(sLilycoveStage1Close)},
};

static const struct SubplotDesc sRustboroCutterSubplot =
{
    .backend = SUBPLOT_STORED,
    .numStages = ARRAY_COUNT(sRustboroCutterStages),
    .stages = sRustboroCutterStages,
    .numHints = ARRAY_COUNT(sRustboroCutterHints),
    .hints = sRustboroCutterHints,
};

static const struct SubplotDesc sLilycoveMuseumSubplot =
{
    .backend = SUBPLOT_STORED,
    .numStages = ARRAY_COUNT(sLilycoveMuseumStages),
    .stages = sLilycoveMuseumStages,
    .numHints = ARRAY_COUNT(sLilycoveMuseumHints),
    .hints = sLilycoveMuseumHints,
};

static const struct SubplotDesc sMossdeepSuperRodSubplot =
{
    .backend = SUBPLOT_STORED,
    .numStages = ARRAY_COUNT(sMossdeepSuperRodStages),
    .stages = sMossdeepSuperRodStages,
    .numHints = ARRAY_COUNT(sMossdeepSuperRodHints),
    .hints = sMossdeepSuperRodHints,
    .producesMainGet = TRUE,
    .mainGet = {PLOT_KIND_UTILITY, PLOT_UTILITY_SUPER_ROD},
};

// The tutorial's first arc (design_story.md, "Opening"): the twin sends the player to Route 101, where Birch is
// cornered, and his lab hands over the starter. Stages are SAVE_BIRCH_* (constants/vars.h) in VAR_SAVE_BIRCH_BEAT; its
// scenes and scripts advance them. It produces no key (the plot adapter reads the Starter from FLAG_SYS_POKEMON_GET).
// Birch's Lab is not a special location of Littleroot's tile entry yet, so every stage is anchored to the Oldale road.
static const struct SubplotStage sSaveBirchStages[SAVE_BIRCH_NUM_STAGES] =
{
    [SAVE_BIRCH_TWIN_GUARDS] = {.anchor = {BEAT_ANCHOR_ROUTE_ROUTE, COUNTY_ID(1, 3), COUNTY_ID(1, 2), BEAT_LOCATION_NONE, 0}},
    [SAVE_BIRCH_TWIN_ASKS] = {.anchor = {BEAT_ANCHOR_ROUTE_ROUTE, COUNTY_ID(1, 3), COUNTY_ID(1, 2), BEAT_LOCATION_NONE, 0}},
    [SAVE_BIRCH_BIRCH_IN_GRASS] = {.anchor = {BEAT_ANCHOR_ROUTE_ROUTE, COUNTY_ID(1, 3), COUNTY_ID(1, 2), BEAT_LOCATION_NONE, 0}},
    [SAVE_BIRCH_AWAIT_BAG] = {.anchor = {BEAT_ANCHOR_ROUTE_ROUTE, COUNTY_ID(1, 3), COUNTY_ID(1, 2), BEAT_LOCATION_NONE, 0}},
    [SAVE_BIRCH_STARTER_TAKEN] = {.anchor = {BEAT_ANCHOR_ROUTE_ROUTE, COUNTY_ID(1, 3), COUNTY_ID(1, 2), BEAT_LOCATION_NONE, 0}},
    [SAVE_BIRCH_SEE_RIVAL] = {.anchor = {BEAT_ANCHOR_ROUTE_ROUTE, COUNTY_ID(1, 3), COUNTY_ID(1, 2), BEAT_LOCATION_NONE, 0}},
};

static const struct SubplotHint sSaveBirchHints[] =
{
    {SUBPLOT_STAGE_BEFORE, SAVE_BIRCH_TWIN_ASKS, TEXT_LIST(sLittlerootStage01Far), TEXT_LIST(sLittlerootStage01Close)},
    {SAVE_BIRCH_BIRCH_IN_GRASS, SAVE_BIRCH_AWAIT_BAG, TEXT_LIST(sLittlerootStage23Far), TEXT_LIST(sLittlerootStage23Close)},
    {SAVE_BIRCH_STARTER_TAKEN, SUBPLOT_STAGE_AFTER, TEXT_LIST(sLittlerootStage45Far), TEXT_LIST(sLittlerootStage45Close)},
};

static const struct SubplotDesc sSaveBirchSubplot =
{
    .backend = SUBPLOT_STORED,
    .numStages = ARRAY_COUNT(sSaveBirchStages),
    .stages = sSaveBirchStages,
    .numHints = ARRAY_COUNT(sSaveBirchHints),
    .hints = sSaveBirchHints,
};

// The tutorial's opening (design_story.md, "Opening"): the truck ride, the first look at the house and the wall
// clock. Stages are MOVING_IN_* (constants/vars.h) in VAR_MOVING_IN_BEAT, which kept VAR_LITTLEROOT_INTRO_STATE's
// values, so the vanilla scripts and scenes advance them. Every stage is anchored to the player's home (the mom's house).
// The stage 6 clock beat and the mom's TV report are still vanilla scripts. The neighbor's house and its Lati TV beat
// (VAR_LITTLEROOT_HOUSES_STATE_*, values 3-5) are main arc, so those vars stay legacy.
static const struct SubplotStage sMovingInStages[MOVING_IN_NUM_STAGES] =
{
    [0 ... MOVING_IN_NUM_STAGES - 1] = {.anchor = {BEAT_ANCHOR_NPC, ROUTE_ID_NONE, ROUTE_ID_NONE, BEAT_LOCATION_NONE, BUILDING_HOME_HEALING}},
};

static const struct SubplotHint sMovingInHints[] =
{
    {SUBPLOT_STAGE_BEFORE, MOVING_IN_TRUCK_FEMALE, TEXT_LIST(sLittlerootMove0Far), TEXT_LIST(sLittlerootMove0Close)},
    {MOVING_IN_ENTERED_HOUSE, MOVING_IN_CLOCK_SET, TEXT_LIST(sLittlerootMove1Far), TEXT_LIST(sLittlerootMove1Close)},
    {MOVING_IN_MEET_RIVAL, SUBPLOT_STAGE_AFTER, TEXT_LIST(sLittlerootMove2Far), TEXT_LIST(sLittlerootMove2Close)},
};

static const struct SubplotDesc sMovingInSubplot =
{
    .backend = SUBPLOT_STORED,
    .numStages = ARRAY_COUNT(sMovingInStages),
    .stages = sMovingInStages,
    .numHints = ARRAY_COUNT(sMovingInHints),
    .hints = sMovingInHints,
};

// The tutorial's second arc: the Route 103 rival, the rival's reappearance in Oldale, the Pokédex in Birch's lab and the
// Running Shoes. Stages are RIVAL_DEX_* in VAR_RIVAL_DEX_BEAT; vanilla scripts advance them (the Oldale rival is
// hidden by Littleroot's transition if the player skips him). Every stage is anchored to the Oldale road.
static const struct SubplotStage sRivalDexStages[RIVAL_DEX_NUM_STAGES] =
{
    [0 ... RIVAL_DEX_NUM_STAGES - 1] = {.anchor = {BEAT_ANCHOR_ROUTE_ROUTE, COUNTY_ID(1, 2), COUNTY_ID(1, 3), BEAT_LOCATION_NONE, 0}},
};

static const struct SubplotHint sRivalDexHints[] =
{
    {SUBPLOT_STAGE_BEFORE, RIVAL_DEX_CHALLENGE, TEXT_LIST(sOldaleDex0Far), TEXT_LIST(sOldaleDex0Close)},
    {RIVAL_DEX_RIVAL_IN_OLDALE, RIVAL_DEX_RIVAL_AT_LAB, TEXT_LIST(sOldaleDex1Far), TEXT_LIST(sOldaleDex1Close)},
    {RIVAL_DEX_POKEDEX_GIVEN, SUBPLOT_STAGE_AFTER, TEXT_LIST(sOldaleDex2Far), TEXT_LIST(sOldaleDex2Close)},
};

static const struct SubplotDesc sRivalDexSubplot =
{
    .backend = SUBPLOT_STORED,
    .numStages = ARRAY_COUNT(sRivalDexStages),
    .stages = sRivalDexStages,
    .numHints = ARRAY_COUNT(sRivalDexHints),
    .hints = sRivalDexHints,
};

static const struct SubplotHost sEmeraldSubplotHosts[] =
{
    {COUNTY_ID(1, 2), 0, &sRivalDexSubplot}, // Oldale, county-story-index 1
    {COUNTY_ID(1, 3), 0, &sMovingInSubplot}, // Littleroot, county-story-index 0
    {COUNTY_ID(1, 3), 1, &sSaveBirchSubplot}, // Littleroot, county-story-index 0
    {COUNTY_ID(0, 1), 0, &sRustboroCutterSubplot}, // Rustboro, county-story-index 3
    {COUNTY_ID(4, 0), 0, &sLilycoveMuseumSubplot}, // Lilycove, county-story-index 15
    {COUNTY_ID(4, 0), 1, &sLilycoveCaseSubplot}, // Lilycove, county-story-index 15
    {COUNTY_ID(5, 1), 0, &sMossdeepSuperRodSubplot}, // Mossdeep
};
