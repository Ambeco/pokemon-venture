// Chapters ported to the descriptor and where Emerald hosts them (design_chapters.md, "Chapter abstraction").

enum LilycoveMuseumStage
{
    LILYCOVE_MUSEUM_EXHIBIT_NOT_SHOWN,
    LILYCOVE_MUSEUM_EXHIBIT_SHOWN,
};

// The museum is an in-city special location, not a BuildingRole, so the curator (an immobile NPC) is anchored
// by its location kind. Hinters may stand anywhere in the county, one per stage.
static const struct ChapterStage sLilycoveMuseumStages[] =
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

enum RustboroFossilStage
{
    RUSTBORO_FOSSIL_IDLE,
    RUSTBORO_FOSSIL_REVIVING,
    RUSTBORO_FOSSIL_READY,
};

// The Devon researcher is an immobile NPC of the service role (he offers a service, like a name rater). Receiving the
// revived Pokémon returns the stage to idle, so the other fossil can be revived next; FLAG_FOSSIL_IS_ANORITH records
// which one is in the machine, and FLAG_RECEIVED_REVIVED_FOSSIL_MON (read by Route 114's Fossil Maniac) is set on receipt.
// No hinters are placed yet (numHinters 0).
static const struct ChapterStage sRustboroFossilStages[] =
{
    [0 ... RUSTBORO_FOSSIL_READY] = {.anchor = {BEAT_ANCHOR_NPC, ROUTE_ID_NONE, ROUTE_ID_NONE, BEAT_LOCATION_NONE, BUILDING_SERVICE}},
};

static const struct ChapterHint sRustboroFossilHints[] =
{
    {RUSTBORO_FOSSIL_IDLE, RUSTBORO_FOSSIL_IDLE, TEXT_LIST(sRustboroFossil0Far), TEXT_LIST(sRustboroFossil0Close)},
    {RUSTBORO_FOSSIL_REVIVING, RUSTBORO_FOSSIL_REVIVING, TEXT_LIST(sRustboroFossil1Far), TEXT_LIST(sRustboroFossil1Close)},
    {RUSTBORO_FOSSIL_READY, RUSTBORO_FOSSIL_READY, TEXT_LIST(sRustboroFossil2Far), TEXT_LIST(sRustboroFossil2Close)},
};

static const struct ChapterDesc sRustboroFossilChapter =
{
    .backend = CHAPTER_STORED,
    .numStages = ARRAY_COUNT(sRustboroFossilStages),
    .stages = sRustboroFossilStages,
    .numHints = ARRAY_COUNT(sRustboroFossilHints),
    .hints = sRustboroFossilHints,
    .requiredRoles = BUILDING_SERVICE,
};

enum LilycoveCaseStage
{
    LILYCOVE_CASE_NOT_GIVEN,
    LILYCOVE_CASE_GIVEN,
};

// The Pokéblock Case is a reported-only plot get (FLAG_RECEIVED_POKEBLOCK_CASE, still set by the script and read by
// the adapter), so the chapter produces no key. The receptionist is an immobile NPC of the contest hall role; no hinters
// are placed yet (numHinters 0), since nothing binds NPCs for generated Lilycove maps.
static const struct ChapterStage sLilycoveCaseStages[] =
{
    [LILYCOVE_CASE_NOT_GIVEN] = {.anchor = {BEAT_ANCHOR_NPC, ROUTE_ID_NONE, ROUTE_ID_NONE, BEAT_LOCATION_NONE, BUILDING_CONTEST_HALL}},
    [LILYCOVE_CASE_GIVEN] = {.anchor = {BEAT_ANCHOR_NPC, ROUTE_ID_NONE, ROUTE_ID_NONE, BEAT_LOCATION_NONE, BUILDING_CONTEST_HALL}},
};

static const struct ChapterHint sLilycoveCaseHints[] =
{
    {0, 0, TEXT_LIST(sLilycoveCase0Far), TEXT_LIST(sLilycoveCase0Close)},
    {1, 1, TEXT_LIST(sLilycoveCase1Far), TEXT_LIST(sLilycoveCase1Close)},
};

static const struct ChapterDesc sLilycoveCaseChapter =
{
    .backend = CHAPTER_STORED,
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
// Petalburg Woods and Rusturf is the HM and badge, a planner item), so the chapter produces no key; the
// script still sets FLAG_RECEIVED_HM_CUT, which the main-plot adapter reads.
static const struct ChapterStage sRustboroCutterStages[] =
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
// of the chapter may say any of them.
enum MossdeepSuperRodStage
{
    MOSSDEEP_SUPER_ROD_NOT_GIVEN,
    MOSSDEEP_SUPER_ROD_GIVEN,
};

// The fisherman is an immobile NPC of the generic house role, and his talk runs the scene MossdeepSuperRod. The
// Super Rod is a reported-only plot get, read by the adapter through FLAG_RECEIVED_SUPER_ROD, which the scene still
// sets; the chapter produces no key. No hinters are placed yet (numHinters 0): nothing binds NPCs for Mossdeep, so the
// hint texts are reachable only through the county query.
static const struct ChapterStage sMossdeepSuperRodStages[] =
{
    [MOSSDEEP_SUPER_ROD_NOT_GIVEN] = {.anchor = {BEAT_ANCHOR_NPC, ROUTE_ID_NONE, ROUTE_ID_NONE, BEAT_LOCATION_NONE, BUILDING_HOUSE}},
    [MOSSDEEP_SUPER_ROD_GIVEN] = {.anchor = {BEAT_ANCHOR_NPC, ROUTE_ID_NONE, ROUTE_ID_NONE, BEAT_LOCATION_NONE, BUILDING_HOUSE}},
};

static const struct ChapterHint sMossdeepSuperRodHints[] =
{
    {0, 0, TEXT_LIST(sMossdeepStage0Far), TEXT_LIST(sMossdeepStage0Close)},
    {1, 1, TEXT_LIST(sMossdeepStage1Far), TEXT_LIST(sMossdeepStage1Close)},
};

static const struct ChapterHint sRustboroCutterHints[] =
{
    {0, 0, TEXT_LIST(sRustboroStage0Far), TEXT_LIST(sRustboroStage0Close)},
    {1, 1, TEXT_LIST(sRustboroStage1Far), TEXT_LIST(sRustboroStage1Close)},
};

static const struct ChapterHint sLilycoveMuseumHints[] =
{
    {0, 0, TEXT_LIST(sLilycoveStage0Far), TEXT_LIST(sLilycoveStage0Close)},
    {1, 1, TEXT_LIST(sLilycoveStage1Far), TEXT_LIST(sLilycoveStage1Close)},
};

static const struct ChapterDesc sRustboroCutterChapter =
{
    .backend = CHAPTER_STORED,
    .numStages = ARRAY_COUNT(sRustboroCutterStages),
    .stages = sRustboroCutterStages,
    .numHints = ARRAY_COUNT(sRustboroCutterHints),
    .hints = sRustboroCutterHints,
};

static const struct ChapterDesc sLilycoveMuseumChapter =
{
    .backend = CHAPTER_STORED,
    .numStages = ARRAY_COUNT(sLilycoveMuseumStages),
    .stages = sLilycoveMuseumStages,
    .numHints = ARRAY_COUNT(sLilycoveMuseumHints),
    .hints = sLilycoveMuseumHints,
};

static const struct ChapterDesc sMossdeepSuperRodChapter =
{
    .backend = CHAPTER_STORED,
    .numStages = ARRAY_COUNT(sMossdeepSuperRodStages),
    .stages = sMossdeepSuperRodStages,
    .numHints = ARRAY_COUNT(sMossdeepSuperRodHints),
    .hints = sMossdeepSuperRodHints,
    .producesMainGet = TRUE,
    .mainGet = {PLOT_KIND_UTILITY, PLOT_UTILITY_SUPER_ROD},
};

enum DewfordOldRodStage
{
    DEWFORD_OLD_ROD_NOT_GIVEN,
    DEWFORD_OLD_ROD_GIVEN,
};

// The fisherman is an immobile NPC outdoors in Dewford (anchor role 0), and his talk runs the scene DewfordOldRod. The
// Old Rod is a reported-only plot get, read by the adapter through FLAG_RECEIVED_OLD_ROD, which the scene still sets;
// the chapter produces no key. No hinters are placed yet (numHinters 0): nothing binds NPCs for Dewford.
static const struct ChapterStage sDewfordOldRodStages[] =
{
    [0 ... DEWFORD_OLD_ROD_GIVEN] = {.anchor = {BEAT_ANCHOR_NPC, ROUTE_ID_NONE, ROUTE_ID_NONE, BEAT_LOCATION_NONE, 0}},
};

static const struct ChapterHint sDewfordOldRodHints[] =
{
    {0, 0, TEXT_LIST(sDewfordStage0Far), TEXT_LIST(sDewfordStage0Close)},
    {1, 1, TEXT_LIST(sDewfordStage1Far), TEXT_LIST(sDewfordStage1Close)},
};

static const struct ChapterDesc sDewfordOldRodChapter =
{
    .backend = CHAPTER_STORED,
    .numStages = ARRAY_COUNT(sDewfordOldRodStages),
    .stages = sDewfordOldRodStages,
    .numHints = ARRAY_COUNT(sDewfordOldRodHints),
    .hints = sDewfordOldRodHints,
    .producesMainGet = TRUE,
    .mainGet = {PLOT_KIND_UTILITY, PLOT_UTILITY_OLD_ROD},
};

enum MauvilleRockSmashStage
{
    MAUVILLE_ROCK_SMASH_NOT_GIVEN,
    MAUVILLE_ROCK_SMASH_GIVEN,
};

// The Rock Smash dude is an immobile NPC of the generic house role. HM Rock Smash is a gate (it opens Rusturf, Granite
// Cave and other locks, a planner item), so the chapter produces no key; the script still sets FLAG_RECEIVED_HM_ROCK_SMASH,
// which the main-plot adapter reads, and the flag that hides his Route 111 tip-giver. No hinters are placed yet (numHinters 0).
static const struct ChapterStage sMauvilleRockSmashStages[] =
{
    [0 ... MAUVILLE_ROCK_SMASH_GIVEN] = {.anchor = {BEAT_ANCHOR_NPC, ROUTE_ID_NONE, ROUTE_ID_NONE, BEAT_LOCATION_NONE, BUILDING_HOUSE}},
};

static const struct ChapterHint sMauvilleRockSmashHints[] =
{
    {0, 0, TEXT_LIST(sMauvilleStage0Far), TEXT_LIST(sMauvilleStage0Close)},
    {1, 1, TEXT_LIST(sMauvilleStage1Far), TEXT_LIST(sMauvilleStage1Close)},
};

static const struct ChapterDesc sMauvilleRockSmashChapter =
{
    .backend = CHAPTER_STORED,
    .numStages = ARRAY_COUNT(sMauvilleRockSmashStages),
    .stages = sMauvilleRockSmashStages,
    .numHints = ARRAY_COUNT(sMauvilleRockSmashHints),
    .hints = sMauvilleRockSmashHints,
};

enum MauvilleCoinCaseStage
{
    MAUVILLE_COIN_CASE_NOT_TRADED,
    MAUVILLE_COIN_CASE_TRADED,
};

// The woman is an immobile NPC of the generic house role; she trades the Coin Case for Harbor Mail, an item sold in
// Slateport's Mart. The chapter declares no required key: nothing models a shop item as a key yet. The script still
// sets FLAG_RECEIVED_COIN_CASE (a reported-only plot get read by the adapter, and by the Game Corner's own checks).
// No hinters are placed yet (numHinters 0).
static const struct ChapterStage sMauvilleCoinCaseStages[] =
{
    [0 ... MAUVILLE_COIN_CASE_TRADED] = {.anchor = {BEAT_ANCHOR_NPC, ROUTE_ID_NONE, ROUTE_ID_NONE, BEAT_LOCATION_NONE, BUILDING_HOUSE}},
};

static const struct ChapterHint sMauvilleCoinCaseHints[] =
{
    {0, 0, TEXT_LIST(sMauvilleCase0Far), TEXT_LIST(sMauvilleCase0Close)},
    {1, 1, TEXT_LIST(sMauvilleCase1Far), TEXT_LIST(sMauvilleCase1Close)},
};

static const struct ChapterDesc sMauvilleCoinCaseChapter =
{
    .backend = CHAPTER_STORED,
    .numStages = ARRAY_COUNT(sMauvilleCoinCaseStages),
    .stages = sMauvilleCoinCaseStages,
    .numHints = ARRAY_COUNT(sMauvilleCoinCaseHints),
    .hints = sMauvilleCoinCaseHints,
    .producesMainGet = TRUE,
    .mainGet = {PLOT_KIND_UTILITY, PLOT_UTILITY_COIN_CASE},
};

// The tutorial's first arc (design_story.md, "Opening"): the twin sends the player to Route 101, where Birch is
// cornered, and his lab hands over the starter. Stages are SAVE_BIRCH_* (constants/vars.h) in VAR_SAVE_BIRCH_BEAT; its
// scenes and scripts advance them. It produces no key (the plot adapter reads the Starter from FLAG_SYS_POKEMON_GET).
// Birch's Lab is not a special location of Littleroot's tile entry yet, so every stage is anchored to the Oldale road.
static const struct ChapterStage sSaveBirchStages[SAVE_BIRCH_NUM_STAGES] =
{
    [SAVE_BIRCH_TWIN_GUARDS] = {.anchor = {BEAT_ANCHOR_ROUTE_ROUTE, COUNTY_ID(1, 3), COUNTY_ID(1, 2), BEAT_LOCATION_NONE, 0}},
    [SAVE_BIRCH_TWIN_ASKS] = {.anchor = {BEAT_ANCHOR_ROUTE_ROUTE, COUNTY_ID(1, 3), COUNTY_ID(1, 2), BEAT_LOCATION_NONE, 0}},
    [SAVE_BIRCH_BIRCH_IN_GRASS] = {.anchor = {BEAT_ANCHOR_ROUTE_ROUTE, COUNTY_ID(1, 3), COUNTY_ID(1, 2), BEAT_LOCATION_NONE, 0}},
    [SAVE_BIRCH_AWAIT_BAG] = {.anchor = {BEAT_ANCHOR_ROUTE_ROUTE, COUNTY_ID(1, 3), COUNTY_ID(1, 2), BEAT_LOCATION_NONE, 0}},
    [SAVE_BIRCH_STARTER_TAKEN] = {.anchor = {BEAT_ANCHOR_ROUTE_ROUTE, COUNTY_ID(1, 3), COUNTY_ID(1, 2), BEAT_LOCATION_NONE, 0}},
    [SAVE_BIRCH_SEE_RIVAL] = {.anchor = {BEAT_ANCHOR_ROUTE_ROUTE, COUNTY_ID(1, 3), COUNTY_ID(1, 2), BEAT_LOCATION_NONE, 0}},
};

static const struct ChapterHint sSaveBirchHints[] =
{
    {CHAPTER_STAGE_BEFORE, SAVE_BIRCH_TWIN_ASKS, TEXT_LIST(sLittlerootStage01Far), TEXT_LIST(sLittlerootStage01Close)},
    {SAVE_BIRCH_BIRCH_IN_GRASS, SAVE_BIRCH_AWAIT_BAG, TEXT_LIST(sLittlerootStage23Far), TEXT_LIST(sLittlerootStage23Close)},
    {SAVE_BIRCH_STARTER_TAKEN, CHAPTER_STAGE_AFTER, TEXT_LIST(sLittlerootStage45Far), TEXT_LIST(sLittlerootStage45Close)},
};

static const struct ChapterDesc sSaveBirchChapter =
{
    .backend = CHAPTER_STORED,
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
static const struct ChapterStage sMovingInStages[MOVING_IN_NUM_STAGES] =
{
    [0 ... MOVING_IN_NUM_STAGES - 1] = {.anchor = {BEAT_ANCHOR_NPC, ROUTE_ID_NONE, ROUTE_ID_NONE, BEAT_LOCATION_NONE, BUILDING_HOME_HEALING}},
};

static const struct ChapterHint sMovingInHints[] =
{
    {CHAPTER_STAGE_BEFORE, MOVING_IN_TRUCK_FEMALE, TEXT_LIST(sLittlerootMove0Far), TEXT_LIST(sLittlerootMove0Close)},
    {MOVING_IN_ENTERED_HOUSE, MOVING_IN_CLOCK_SET, TEXT_LIST(sLittlerootMove1Far), TEXT_LIST(sLittlerootMove1Close)},
    {MOVING_IN_MEET_RIVAL, CHAPTER_STAGE_AFTER, TEXT_LIST(sLittlerootMove2Far), TEXT_LIST(sLittlerootMove2Close)},
};

static const struct ChapterDesc sMovingInChapter =
{
    .backend = CHAPTER_STORED,
    .numStages = ARRAY_COUNT(sMovingInStages),
    .stages = sMovingInStages,
    .numHints = ARRAY_COUNT(sMovingInHints),
    .hints = sMovingInHints,
};

// The tutorial's second arc: the Route 103 rival, the rival's reappearance in Oldale, the Pokédex in Birch's lab and the
// Running Shoes. Stages are RIVAL_DEX_* in VAR_RIVAL_DEX_BEAT; vanilla scripts advance them (the Oldale rival is
// hidden by Littleroot's transition if the player skips him). Every stage is anchored to the Oldale road.
static const struct ChapterStage sRivalDexStages[RIVAL_DEX_NUM_STAGES] =
{
    [0 ... RIVAL_DEX_NUM_STAGES - 1] = {.anchor = {BEAT_ANCHOR_ROUTE_ROUTE, COUNTY_ID(1, 2), COUNTY_ID(1, 3), BEAT_LOCATION_NONE, 0}},
};

static const struct ChapterHint sRivalDexHints[] =
{
    {CHAPTER_STAGE_BEFORE, RIVAL_DEX_CHALLENGE, TEXT_LIST(sOldaleDex0Far), TEXT_LIST(sOldaleDex0Close)},
    {RIVAL_DEX_RIVAL_IN_OLDALE, RIVAL_DEX_RIVAL_AT_LAB, TEXT_LIST(sOldaleDex1Far), TEXT_LIST(sOldaleDex1Close)},
    {RIVAL_DEX_POKEDEX_GIVEN, CHAPTER_STAGE_AFTER, TEXT_LIST(sOldaleDex2Far), TEXT_LIST(sOldaleDex2Close)},
};

static const struct ChapterDesc sRivalDexChapter =
{
    .backend = CHAPTER_STORED,
    .numStages = ARRAY_COUNT(sRivalDexStages),
    .stages = sRivalDexStages,
    .numHints = ARRAY_COUNT(sRivalDexHints),
    .hints = sRivalDexHints,
};

static const struct ChapterHost sEmeraldChapterHosts[] =
{
    {COUNTY_ID(1, 2), 0, &sRivalDexChapter}, // Oldale, county-story-index 1
    {COUNTY_ID(1, 3), 0, &sMovingInChapter}, // Littleroot, county-story-index 0
    {COUNTY_ID(1, 3), 1, &sSaveBirchChapter}, // Littleroot, county-story-index 0
    {COUNTY_ID(0, 1), 0, &sRustboroCutterChapter}, // Rustboro, county-story-index 3
    {COUNTY_ID(0, 1), 1, &sRustboroFossilChapter}, // Rustboro, county-story-index 3
    {COUNTY_ID(4, 0), 0, &sLilycoveMuseumChapter}, // Lilycove, county-story-index 15
    {COUNTY_ID(4, 0), 1, &sLilycoveCaseChapter}, // Lilycove, county-story-index 15
    {COUNTY_ID(5, 1), 0, &sMossdeepSuperRodChapter}, // Mossdeep
    {COUNTY_ID(0, 3), 0, &sDewfordOldRodChapter}, // Dewford, county-story-index 4
    {COUNTY_ID(2, 1), 0, &sMauvilleRockSmashChapter}, // Mauville, county-story-index 7
    {COUNTY_ID(2, 1), 1, &sMauvilleCoinCaseChapter},
};
