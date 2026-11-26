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
// the adapter) that the chapter supplies, and it produces no key. The receptionist is an immobile NPC of the contest
// hall role; no hinters are placed yet (numHinters 0), since nothing binds NPCs for generated Lilycove maps.
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
    .producesMainGet = TRUE,
    .mainGet = {PLOT_KIND_UTILITY, PLOT_UTILITY_POKEBLOCK_CASE},
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
    .mainHm = PLOT_HM_CUT,
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

enum Route118GoodRodStage
{
    ROUTE118_GOOD_ROD_NOT_GIVEN,
    ROUTE118_GOOD_ROD_GIVEN,
};

// The fisherman is an immobile NPC outdoors (anchor role 0) in the Weather Institute county, which has no city: his
// talk runs the scene Route118GoodRod, and his script reads FLAG_RECEIVED_GOOD_ROD (not the stage) because a route has
// no city map section to find its county from. The Good Rod is a reported-only plot get, read by the adapter through
// that flag; the chapter produces no key. No hinters are placed yet (numHinters 0).
static const struct ChapterStage sRoute118GoodRodStages[] =
{
    [0 ... ROUTE118_GOOD_ROD_GIVEN] = {.anchor = {BEAT_ANCHOR_NPC, ROUTE_ID_NONE, ROUTE_ID_NONE, BEAT_LOCATION_NONE, 0}},
};

static const struct ChapterHint sRoute118GoodRodHints[] =
{
    {0, 0, TEXT_LIST(sRoute118Stage0Far), TEXT_LIST(sRoute118Stage0Close)},
    {1, 1, TEXT_LIST(sRoute118Stage1Far), TEXT_LIST(sRoute118Stage1Close)},
};

static const struct ChapterDesc sGoodRodChapter =
{
    .backend = CHAPTER_STORED,
    .numStages = ARRAY_COUNT(sRoute118GoodRodStages),
    .stages = sRoute118GoodRodStages,
    .numHints = ARRAY_COUNT(sRoute118GoodRodHints),
    .hints = sRoute118GoodRodHints,
    .producesMainGet = TRUE,
    .mainGet = {PLOT_KIND_UTILITY, PLOT_UTILITY_GOOD_ROD},
};

// A stored chapter for a rival beat or the Itemfinder: an NPC outdoors (anchor role 0), completing a main-plot get.
// The scene writes the slot var; on a cityless county's route the map script also sets the legacy var
// (VAR_ROUTE110_STATE or VAR_ROUTE119_STATE) that its coord events and the plot-get adapter predicates read, because
// a route script cannot find its county's stage. Lilycove's rival is the same in a city; the adapter reads its flag.
#define ROUTE_GET_CHAPTER(name, getKind, getOrdinal) \
static const struct ChapterDesc name = \
{ \
    .backend = CHAPTER_STORED, \
    .numStages = ARRAY_COUNT(name##Stages), \
    .stages = name##Stages, \
    .numHints = ARRAY_COUNT(name##Hints), \
    .hints = name##Hints, \
    .producesMainGet = TRUE, \
    .mainGet = {getKind, getOrdinal}, \
}

#define ROUTE_NPC_STAGE {.anchor = {BEAT_ANCHOR_NPC, ROUTE_ID_NONE, ROUTE_ID_NONE, BEAT_LOCATION_NONE, 0}}

static const struct ChapterStage sRival3ChapterStages[] = {[0 ... 1] = ROUTE_NPC_STAGE};
static const struct ChapterHint sRival3ChapterHints[] =
{
    {0, 0, TEXT_LIST(sRoute110Rival0Far), TEXT_LIST(sRoute110Rival0Close)},
    {1, 1, TEXT_LIST(sRoute110Rival1Far), TEXT_LIST(sRoute110Rival1Close)},
};
ROUTE_GET_CHAPTER(sRival3Chapter, PLOT_KIND_RIVAL, 3);

static const struct ChapterStage sItemfinderChapterStages[] = {[0 ... 1] = ROUTE_NPC_STAGE};
static const struct ChapterHint sItemfinderChapterHints[] =
{
    {0, 0, TEXT_LIST(sRoute110Finder0Far), TEXT_LIST(sRoute110Finder0Close)},
    {1, 1, TEXT_LIST(sRoute110Finder1Far), TEXT_LIST(sRoute110Finder1Close)},
};
ROUTE_GET_CHAPTER(sItemfinderChapter, PLOT_KIND_UTILITY, PLOT_UTILITY_ITEMFINDER);

static const struct ChapterStage sRival4ChapterStages[] = {[0 ... 2] = ROUTE_NPC_STAGE};
static const struct ChapterHint sRival4ChapterHints[] =
{
    {0, 0, TEXT_LIST(sRoute119Rival0Far), TEXT_LIST(sRoute119Rival0Close)},
    {1, 1, TEXT_LIST(sRoute119Rival1Far), TEXT_LIST(sRoute119Rival1Close)},
    {2, 2, TEXT_LIST(sRoute119Rival2Far), TEXT_LIST(sRoute119Rival2Close)},
};
ROUTE_GET_CHAPTER(sRival4Chapter, PLOT_KIND_RIVAL, 4);

// Lilycove's rival is the one city-county rival: he stands in the open city (anchor role 0), and the scene writes slot 2
// of the county slice. The script sets FLAG_MET_RIVAL_LILYCOVE after his fly-away, which the adapter predicate reads.
static const struct ChapterStage sRival5ChapterStages[] = {[0 ... 1] = ROUTE_NPC_STAGE};
static const struct ChapterHint sRival5ChapterHints[] =
{
    {0, 0, TEXT_LIST(sLilycoveRival0Far), TEXT_LIST(sLilycoveRival0Close)},
    {1, 1, TEXT_LIST(sLilycoveRival1Far), TEXT_LIST(sLilycoveRival1Close)},
};
ROUTE_GET_CHAPTER(sRival5Chapter, PLOT_KIND_RIVAL, 5);

// Rival 2 appears on Route 104 or in Rustboro (whichever the player reaches first); both sites' scenes write this slot,
// and the scripts keep setting FLAG_DEFEATED_RIVAL_ROUTE_104 / FLAG_DEFEATED_RIVAL_RUSTBORO for the plot get's predicate (emerald_plot_gets.h).
static const struct ChapterStage sRival2ChapterStages[] = {[0 ... 1] = ROUTE_NPC_STAGE};
static const struct ChapterHint sRival2ChapterHints[] =
{
    {0, 0, TEXT_LIST(sRival2Hint0Far), TEXT_LIST(sRival2Hint0Close)},
    {1, 1, TEXT_LIST(sRival2Hint1Far), TEXT_LIST(sRival2Hint1Close)},
};
ROUTE_GET_CHAPTER(sRival2Chapter, PLOT_KIND_RIVAL, 2);

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
    .mainHm = PLOT_HM_ROCK_SMASH,
};

enum MauvilleCoinCaseStage
{
    MAUVILLE_COIN_CASE_NOT_TRADED,
    MAUVILLE_COIN_CASE_TRADED,
};

// The woman is an immobile NPC of the generic house role; she trades the Coin Case for Harbor Mail, a shop-item key
// produced by Slateport's Mart chapter. The script still sets FLAG_RECEIVED_COIN_CASE (a reported-only plot get read
// by the adapter, and by the Game Corner's own checks). No hinters are placed yet (numHinters 0).
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
    .requiredKeys = {SHOP_ITEM_KEY(ITEM_HARBOR_MAIL)},
    .producesMainGet = TRUE,
    .mainGet = {PLOT_KIND_UTILITY, PLOT_UTILITY_COIN_CASE},
};

enum SlateportHarborMailStage
{
    SLATEPORT_HARBOR_MAIL_NOT_BOUGHT,
    SLATEPORT_HARBOR_MAIL_BOUGHT,
};

// A shop-item chapter: the clerk is an immobile NPC of the Mart role, and Harbor Mail is a Mart item, so no scene
// advances the stage. The clerk's script checks the bag after the shop closes and sets the stage once the player holds
// the mail. The stage is the key: the Coin Case trade requires it (SHOP_ITEM_KEY). Any other Mart could sell the item.
static const struct ChapterStage sSlateportHarborMailStages[] =
{
    [0 ... SLATEPORT_HARBOR_MAIL_BOUGHT] = {.anchor = {BEAT_ANCHOR_NPC, ROUTE_ID_NONE, ROUTE_ID_NONE, BEAT_LOCATION_NONE, BUILDING_MART}},
};

static const struct ChapterHint sSlateportHarborMailHints[] =
{
    {0, 0, TEXT_LIST(sSlateportMail0Far), TEXT_LIST(sSlateportMail0Close)},
    {1, 1, TEXT_LIST(sSlateportMail1Far), TEXT_LIST(sSlateportMail1Close)},
};

static const struct ChapterDesc sSlateportHarborMailChapter =
{
    .backend = CHAPTER_STORED,
    .numStages = ARRAY_COUNT(sSlateportHarborMailStages),
    .stages = sSlateportHarborMailStages,
    .numHints = ARRAY_COUNT(sSlateportHarborMailHints),
    .hints = sSlateportHarborMailHints,
    .producedKeyStage = SLATEPORT_HARBOR_MAIL_BOUGHT,
    .producedKey = SHOP_ITEM_KEY(ITEM_HARBOR_MAIL),
    .givesReward = TRUE,
};

enum MauvilleBikeStage
{
    MAUVILLE_BIKE_NOT_GIVEN,
    MAUVILLE_BIKE_GIVEN,
};

// Rydel is an immobile NPC of the generic house role. Declining ("not far") is a side branch that leaves the stage
// alone (FLAG_DECLINED_BIKE changes only his greeting), and later swaps of Mach for Acro Bike are a repeatable service
// that stays in the script, so two stages suffice. The Bicycle is a reported-only plot get read through
// FLAG_RECEIVED_BIKE, which the script still sets; the Running Shoes check is a plot-adapter get, not a key here.
// No hinters are placed yet (numHinters 0).
static const struct ChapterStage sMauvilleBikeStages[] =
{
    [0 ... MAUVILLE_BIKE_GIVEN] = {.anchor = {BEAT_ANCHOR_NPC, ROUTE_ID_NONE, ROUTE_ID_NONE, BEAT_LOCATION_NONE, BUILDING_HOUSE}},
};

static const struct ChapterHint sMauvilleBikeHints[] =
{
    {0, 0, TEXT_LIST(sMauvilleBike0Far), TEXT_LIST(sMauvilleBike0Close)},
    {1, 1, TEXT_LIST(sMauvilleBike1Far), TEXT_LIST(sMauvilleBike1Close)},
};

static const struct ChapterDesc sMauvilleBikeChapter =
{
    .backend = CHAPTER_STORED,
    .numStages = ARRAY_COUNT(sMauvilleBikeStages),
    .stages = sMauvilleBikeStages,
    .numHints = ARRAY_COUNT(sMauvilleBikeHints),
    .hints = sMauvilleBikeHints,
    .producesMainGet = TRUE,
    .mainGet = {PLOT_KIND_UTILITY, PLOT_UTILITY_BICYCLE},
    // Rydel's Cycles runs beside Mauville's chain as a parallel subplot; its proxy on the chain waits for this key.
    // Pool index MCGUFFIN_IDS_PER_COUNTY - 1 stays clear of the special locations' ids (at most 6, from index 0).
    .producedKeyStage = MAUVILLE_BIKE_GIVEN,
    .producedKey = {CHAPTER_KEY_HOST_POOL, MCGUFFIN_IDS_PER_COUNTY - 1},
    .givesReward = TRUE,
};

// The chain's stand-in for Rydel's Cycles (design_chapters.md, "Parallel subplots (proxy entries)"): stage 1 once the
// Bicycle is given. It has no state, hosts no hinters and supplies the Bicycle's plot get to the planner, which puts it in that get's slot.
static const struct ChapterStage sMauvilleBikeProxyStages[] =
{
    [0 ... 1] = {.anchor = {BEAT_ANCHOR_NPC, ROUTE_ID_NONE, ROUTE_ID_NONE, BEAT_LOCATION_NONE, BUILDING_HOUSE}},
};

static const struct ChapterHint sMauvilleBikeProxyHints[] =
{
    {0, 1, {NULL, 0}, {NULL, 0}},
};

static const struct ChapterDesc sMauvilleBikeProxyChapter =
{
    .backend = CHAPTER_PROXY,
    .numStages = ARRAY_COUNT(sMauvilleBikeProxyStages),
    .stages = sMauvilleBikeProxyStages,
    .numHints = ARRAY_COUNT(sMauvilleBikeProxyHints),
    .hints = sMauvilleBikeProxyHints,
    .requiredKeys = {{CHAPTER_KEY_HOST_POOL, MCGUFFIN_IDS_PER_COUNTY - 1}},
    .mainGet = {PLOT_KIND_UTILITY, PLOT_UTILITY_BICYCLE},
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

// A gym is a two-stage adapter chapter: stage 1 once its badge flag is set. It is stateless (hosted with
// CHAPTER_SLOT_STATELESS, so it takes no slot var), reports no hints (the main-beat hints already name the leader), and supplies the
// badge's plot get (`producesMainGet`), so the planner binds it to the gym's main-story slot; the news lookup scans stateful slots only, so it never points at a gym.
// It declares no key and no minBadges: gym order is the planner's, and no chapter consumes a badge key yet.
#define GYM_CHAPTER(name, ordinal, badgeFlag) \
static const struct ChapterStage name##Stages[] = \
{ \
    {.anchor = {BEAT_ANCHOR_NPC, ROUTE_ID_NONE, ROUTE_ID_NONE, BEAT_LOCATION_NONE, BUILDING_GYM}}, \
    {.anchor = {BEAT_ANCHOR_NPC, ROUTE_ID_NONE, ROUTE_ID_NONE, BEAT_LOCATION_NONE, BUILDING_GYM}, .done = {STORY_PRED_FLAG, badgeFlag, 0}}, \
}; \
static const struct ChapterHint name##Hints[] = {{0, 1, {NULL, 0}, {NULL, 0}}}; \
static const struct ChapterDesc name = \
{ \
    .backend = CHAPTER_ADAPTER, \
    .numStages = ARRAY_COUNT(name##Stages), \
    .stages = name##Stages, \
    .numHints = ARRAY_COUNT(name##Hints), \
    .hints = name##Hints, \
    .requiredRoles = BUILDING_GYM, \
    .producesMainGet = TRUE, \
    .mainGet = {PLOT_KIND_GYM, ordinal}, \
}

GYM_CHAPTER(sRustboroGymChapter, 1, FLAG_BADGE01_GET);
GYM_CHAPTER(sDewfordGymChapter, 2, FLAG_BADGE02_GET);
GYM_CHAPTER(sMauvilleGymChapter, 3, FLAG_BADGE03_GET);
GYM_CHAPTER(sLavaridgeGymChapter, 4, FLAG_BADGE04_GET);
GYM_CHAPTER(sFortreeGymChapter, 6, FLAG_BADGE06_GET);
GYM_CHAPTER(sMossdeepGymChapter, 7, FLAG_BADGE07_GET);
GYM_CHAPTER(sSootopolisGymChapter, 8, FLAG_BADGE08_GET); // block 19's callback, hosted by Sootopolis

enum PetalburgGymStage
{
    PETALBURG_GYM_NOT_BEATEN,
    PETALBURG_GYM_BADGE_GIVEN,
    PETALBURG_GYM_SURF_GIVEN,
};

// Norman's gym (gym 5) is Lavaridge's block-11 callback hosted by Petalburg, so it sits on Petalburg's chain at that
// callback, not in Petalburg's own block. Its last stage is the Surf hand-over, which produces the "gym 5" key (badge and
// HM, region_unlock.h) that New Mauville consumes. Emerald hands Surf over in Wally's house after a walk from the gym
// (design_chapters.md, "Stage anchors"); Petalburg has no house role yet, so the stage anchors on the gym.
static const struct ChapterStage sPetalburgGymStages[] =
{
    [PETALBURG_GYM_NOT_BEATEN] = {.anchor = {BEAT_ANCHOR_NPC, ROUTE_ID_NONE, ROUTE_ID_NONE, BEAT_LOCATION_NONE, BUILDING_GYM}},
    [PETALBURG_GYM_BADGE_GIVEN] = {.anchor = {BEAT_ANCHOR_NPC, ROUTE_ID_NONE, ROUTE_ID_NONE, BEAT_LOCATION_NONE, BUILDING_GYM}, .done = {STORY_PRED_FLAG, FLAG_BADGE05_GET, 0}},
    [PETALBURG_GYM_SURF_GIVEN] = {.anchor = {BEAT_ANCHOR_NPC, ROUTE_ID_NONE, ROUTE_ID_NONE, BEAT_LOCATION_NONE, BUILDING_GYM}, .done = {STORY_PRED_FLAG, FLAG_RECEIVED_HM_SURF, 0}},
};

static const struct ChapterHint sPetalburgGymHints[] = {{0, 2, {NULL, 0}, {NULL, 0}}};

static const struct ChapterDesc sPetalburgGymChapter =
{
    .backend = CHAPTER_ADAPTER,
    .numStages = ARRAY_COUNT(sPetalburgGymStages),
    .stages = sPetalburgGymStages,
    .numHints = ARRAY_COUNT(sPetalburgGymHints),
    .hints = sPetalburgGymHints,
    .requiredRoles = BUILDING_GYM,
    .producesMainGet = TRUE,
    .mainGet = {PLOT_KIND_GYM, 5},
    .producedKeyStage = PETALBURG_GYM_SURF_GIVEN,
    .producedKey = {CHAPTER_KEY_ITEM, PLOT_KIND_GYM, 5},
    .givesReward = TRUE,
};

// Petalburg's own block-2 get: the scene where Wally is shown with his Ralts (`VAR_PETALBURG_CITY_STATE` 3). It is a
// blocking scripted get, so Petalburg's chain needs a chapter for it; the legacy scripts are untouched.
static const struct ChapterStage sWallyRaltsStages[] =
{
    {.anchor = {BEAT_ANCHOR_NPC, ROUTE_ID_NONE, ROUTE_ID_NONE, BEAT_LOCATION_NONE, 0}},
    {.anchor = {BEAT_ANCHOR_NPC, ROUTE_ID_NONE, ROUTE_ID_NONE, BEAT_LOCATION_NONE, 0}, .done = {STORY_PRED_VAR_GE, VAR_PETALBURG_CITY_STATE, 3}},
};

static const struct ChapterHint sWallyRaltsHints[] = {{0, 1, {NULL, 0}, {NULL, 0}}};

static const struct ChapterDesc sWallyRaltsChapter =
{
    .backend = CHAPTER_ADAPTER,
    .numStages = ARRAY_COUNT(sWallyRaltsStages),
    .stages = sWallyRaltsStages,
    .numHints = ARRAY_COUNT(sWallyRaltsHints),
    .hints = sWallyRaltsHints,
    .producesMainGet = TRUE,
    .mainGet = {PLOT_KIND_SCRIPTED, PLOT_SCRIPTED_WALLY_RALTS},
};

enum NewMauvilleStage
{
    NEW_MAUVILLE_NOT_ASKED,
    NEW_MAUVILLE_KEY_GIVEN,
    NEW_MAUVILLE_GENERATOR_OFF,
    NEW_MAUVILLE_REPORTED,
};

// Wattson's favor (walkthrough #83-85): he gives the Basement Key, the player shuts the generator off in New Mauville,
// and he pays out TM Thunderbolt. An adapter chapter hosted stateless (Mauville's three stateful slots are full), so it
// reads the legacy flags and VAR_NEW_MAUVILLE_STATE, and its hints are reached by GetCountyStatelessHint. No hinters
// are placed yet (numHinters 0), and it supplies no main-plot get, so the news never mentions it. It needs the "gym 5"
// key (Norman's badge and Surf); it is off-chain, so it waits for that key at runtime although the key's producer, a
// callback, comes later in story order.
static const struct ChapterStage sNewMauvilleStages[] =
{
    [NEW_MAUVILLE_NOT_ASKED] = {.anchor = {BEAT_ANCHOR_NPC, ROUTE_ID_NONE, ROUTE_ID_NONE, BEAT_LOCATION_NONE, 0}},
    [NEW_MAUVILLE_KEY_GIVEN] =
    {
        .anchor = {BEAT_ANCHOR_NPC, ROUTE_ID_NONE, ROUTE_ID_NONE, ROUTE_LOCATION_FACILITY, 0},
        .done = {STORY_PRED_FLAG, FLAG_GOT_BASEMENT_KEY_FROM_WATTSON, 0},
    },
    [NEW_MAUVILLE_GENERATOR_OFF] =
    {
        .anchor = {BEAT_ANCHOR_NPC, ROUTE_ID_NONE, ROUTE_ID_NONE, BEAT_LOCATION_NONE, 0},
        .done = {STORY_PRED_VAR_GE, VAR_NEW_MAUVILLE_STATE, 2},
    },
    [NEW_MAUVILLE_REPORTED] =
    {
        .anchor = {BEAT_ANCHOR_NPC, ROUTE_ID_NONE, ROUTE_ID_NONE, BEAT_LOCATION_NONE, 0},
        .done = {STORY_PRED_FLAG, FLAG_GOT_TM_THUNDERBOLT_FROM_WATTSON, 0},
    },
};

static const struct ChapterHint sNewMauvilleHints[] =
{
    {0, 0, TEXT_LIST(sMauvilleGen0Far), TEXT_LIST(sMauvilleGen0Close)},
    {1, 1, TEXT_LIST(sMauvilleGen1Far), TEXT_LIST(sMauvilleGen1Close)},
    {2, 2, TEXT_LIST(sMauvilleGen2Far), TEXT_LIST(sMauvilleGen2Close)},
    {3, 3, TEXT_LIST(sMauvilleGen3Far), TEXT_LIST(sMauvilleGen3Close)},
};

static const struct ChapterDesc sNewMauvilleChapter =
{
    .backend = CHAPTER_ADAPTER,
    .numStages = ARRAY_COUNT(sNewMauvilleStages),
    .stages = sNewMauvilleStages,
    .numHints = ARRAY_COUNT(sNewMauvilleHints),
    .hints = sNewMauvilleHints,
    .requiredKeys = {{CHAPTER_KEY_ITEM, PLOT_KIND_GYM, 5}},
    .waitsForLaterKey = TRUE,
};

// A two-stage stateless adapter chapter for a main-plot get whose scene stays vanilla: stage 1 once the get's own
// predicate (as in emerald_plot_gets.h) holds. It reports no hints and anchors nowhere in particular.
#define PLOT_GET_CHAPTER(name, getKind, getOrdinal, roles, ...) \
static const struct ChapterStage name##Stages[] = \
{ \
    {.anchor = {BEAT_ANCHOR_NPC, ROUTE_ID_NONE, ROUTE_ID_NONE, BEAT_LOCATION_NONE, 0}}, \
    {.anchor = {BEAT_ANCHOR_NPC, ROUTE_ID_NONE, ROUTE_ID_NONE, BEAT_LOCATION_NONE, 0}, .done = {__VA_ARGS__}}, \
}; \
static const struct ChapterHint name##Hints[] = {{0, 1, {NULL, 0}, {NULL, 0}}}; \
static const struct ChapterDesc name = \
{ \
    .backend = CHAPTER_ADAPTER, \
    .numStages = ARRAY_COUNT(name##Stages), \
    .stages = name##Stages, \
    .numHints = ARRAY_COUNT(name##Hints), \
    .hints = name##Hints, \
    .requiredRoles = roles, \
    .producesMainGet = TRUE, \
    .mainGet = {getKind, getOrdinal}, \
}

// PLOT_GET_CHAPTER with a pair of hints (before and after the get; texts `s<texts>0Far` ... `s<texts>1Close`) and one
// anchor for both stages. No hinters are placed yet, so the hints are reached only through GetCountyStatelessHint.
#define PLOT_GET_HINTED_CHAPTER(name, texts, getKind, getOrdinal, roles, anchorInit, ...) \
static const struct ChapterStage name##Stages[] = \
{ \
    {.anchor = anchorInit}, \
    {.anchor = anchorInit, .done = {__VA_ARGS__}}, \
}; \
static const struct ChapterHint name##Hints[] = \
{ \
    {0, 0, TEXT_LIST(s##texts##0Far), TEXT_LIST(s##texts##0Close)}, \
    {1, 1, TEXT_LIST(s##texts##1Far), TEXT_LIST(s##texts##1Close)}, \
}; \
static const struct ChapterDesc name = \
{ \
    .backend = CHAPTER_ADAPTER, \
    .numStages = ARRAY_COUNT(name##Stages), \
    .stages = name##Stages, \
    .numHints = ARRAY_COUNT(name##Hints), \
    .hints = name##Hints, \
    .requiredRoles = roles, \
    .producesMainGet = TRUE, \
    .mainGet = {getKind, getOrdinal}, \
}

#define OLDALE_ROAD_ANCHOR {BEAT_ANCHOR_ROUTE_ROUTE, COUNTY_ID(1, 3), COUNTY_ID(1, 2), BEAT_LOCATION_NONE, 0}
#define DEVON_CORP_ANCHOR {BEAT_ANCHOR_NPC, ROUTE_ID_NONE, ROUTE_ID_NONE, TOWN_LOCATION_INSTITUTION, 0}
#define HARBOR_ANCHOR {BEAT_ANCHOR_NPC, ROUTE_ID_NONE, ROUTE_ID_NONE, TOWN_LOCATION_HARBOR, 0}
#define LEAGUE_ANCHOR {BEAT_ANCHOR_NPC, ROUTE_ID_NONE, ROUTE_ID_NONE, BEAT_LOCATION_NONE, BUILDING_LEAGUE}

PLOT_GET_HINTED_CHAPTER(sEverGrandeLobbyChapter, Lobby, PLOT_KIND_GYM, ELITE_FOUR_ORDINAL, BUILDING_LEAGUE, LEAGUE_ANCHOR, STORY_PRED_FLAG, FLAG_ENTERED_ELITE_FOUR, 0);
// The four gets sRivalDexChapter's var covers are views of its stages (it is their only writer): the predicates read VAR_RIVAL_DEX_BEAT, not the flags the scripts also set.
PLOT_GET_CHAPTER(sRival1Chapter, PLOT_KIND_RIVAL, 1, 0, STORY_PRED_VAR_GE, VAR_RIVAL_DEX_BEAT, RIVAL_DEX_RIVAL_IN_OLDALE);
PLOT_GET_HINTED_CHAPTER(sPokenavChapter, Pokenav, PLOT_KIND_UTILITY, PLOT_UTILITY_POKENAV, 0, DEVON_CORP_ANCHOR, STORY_PRED_FLAG, FLAG_RECEIVED_POKENAV, 0);
// The Starter keeps its engine flag, which the debug menu sets without advancing Save Birch's stage (it would leave Birch hidden).
PLOT_GET_HINTED_CHAPTER(sStarterChapter, Starter, PLOT_KIND_SCRIPTED, PLOT_SCRIPTED_STARTER, 0, OLDALE_ROAD_ANCHOR, STORY_PRED_FLAG, FLAG_SYS_POKEMON_GET, 0);
PLOT_GET_CHAPTER(sPokedexChapter, PLOT_KIND_SCRIPTED, PLOT_SCRIPTED_POKEDEX, 0, STORY_PRED_VAR_GE, VAR_RIVAL_DEX_BEAT, RIVAL_DEX_POKEDEX_GIVEN);
PLOT_GET_CHAPTER(sPokeBallsChapter, PLOT_KIND_SCRIPTED, PLOT_SCRIPTED_POKE_BALLS, 0, STORY_PRED_VAR_GE, VAR_RIVAL_DEX_BEAT, RIVAL_DEX_POKEDEX_GIVEN);
PLOT_GET_CHAPTER(sRunningShoesChapter, PLOT_KIND_SCRIPTED, PLOT_SCRIPTED_RUNNING_SHOES, 0, STORY_PRED_VAR_GE, VAR_RIVAL_DEX_BEAT, RIVAL_DEX_SHOES_GIVEN);
// Stages follow vanilla flags only (no slot var): the dock's refusal, then the delivery, which can also happen without a
// refusal. The Shipyard plot get is "refused or delivered" (emerald_plot_gets.h), i.e. stage 1 or later.
static const struct ChapterStage sShipyardStages[] =
{
    {.anchor = HARBOR_ANCHOR},
    {.anchor = HARBOR_ANCHOR, .done = {STORY_PRED_FLAG, FLAG_DOCK_REJECTED_DEVON_GOODS, 0}},
    {.anchor = HARBOR_ANCHOR, .done = {STORY_PRED_FLAG, FLAG_DELIVERED_DEVON_GOODS, 0}},
};

static const struct ChapterHint sShipyardHints[] =
{
    {0, 0, TEXT_LIST(sShipyard0Far), TEXT_LIST(sShipyard0Close)},
    {1, 1, TEXT_LIST(sShipyardRejectedFar), TEXT_LIST(sShipyardRejectedClose)},
    {2, 2, TEXT_LIST(sShipyard1Far), TEXT_LIST(sShipyard1Close)},
};

static const struct ChapterDesc sShipyardChapter =
{
    .backend = CHAPTER_ADAPTER,
    .numStages = ARRAY_COUNT(sShipyardStages),
    .stages = sShipyardStages,
    .numHints = ARRAY_COUNT(sShipyardHints),
    .hints = sShipyardHints,
    .producesMainGet = TRUE,
    .mainGet = {PLOT_KIND_SCRIPTED, PLOT_SCRIPTED_SHIPYARD},
};

// The Elite Four run, one stage per member beaten (vanilla's four defeat flags). The Elite Four plot get is stage 1 or later.
static const struct ChapterStage sEliteFourStages[] =
{
    {.anchor = LEAGUE_ANCHOR},
    {.anchor = LEAGUE_ANCHOR, .done = {STORY_PRED_FLAG, FLAG_ENTERED_ELITE_FOUR, 0}},
    {.anchor = LEAGUE_ANCHOR, .done = {STORY_PRED_FLAG, FLAG_DEFEATED_ELITE_4_SIDNEY, 0}},
    {.anchor = LEAGUE_ANCHOR, .done = {STORY_PRED_FLAG, FLAG_DEFEATED_ELITE_4_PHOEBE, 0}},
    {.anchor = LEAGUE_ANCHOR, .done = {STORY_PRED_FLAG, FLAG_DEFEATED_ELITE_4_GLACIA, 0}},
    {.anchor = LEAGUE_ANCHOR, .done = {STORY_PRED_FLAG, FLAG_DEFEATED_ELITE_4_DRAKE, 0}},
};

static const struct ChapterHint sEliteFourHints[] =
{
    {0, 0, TEXT_LIST(sEliteFour0Far), TEXT_LIST(sEliteFour0Close)},
    {1, 1, TEXT_LIST(sEliteFour1Far), TEXT_LIST(sEliteFour1Close)},
    {2, 4, TEXT_LIST(sEliteFourMidFar), TEXT_LIST(sEliteFourMidClose)},
    {5, 5, TEXT_LIST(sEliteFourLastFar), TEXT_LIST(sEliteFourLastClose)},
};

static const struct ChapterDesc sEliteFourChapter =
{
    .backend = CHAPTER_ADAPTER,
    .numStages = ARRAY_COUNT(sEliteFourStages),
    .stages = sEliteFourStages,
    .numHints = ARRAY_COUNT(sEliteFourHints),
    .hints = sEliteFourHints,
    .producesMainGet = TRUE,
    .mainGet = {PLOT_KIND_SCRIPTED, PLOT_SCRIPTED_ELITE_FOUR},
};
PLOT_GET_HINTED_CHAPTER(sChampionChapter, Champion, PLOT_KIND_SCRIPTED, PLOT_SCRIPTED_CHAMPION, 0, LEAGUE_ANCHOR, STORY_PRED_FLAG, FLAG_SYS_GAME_CLEAR, 0);

static const struct ChapterHost sEmeraldChapterHosts[] =
{
    {COUNTY_ID(1, 2), 0, &sRivalDexChapter}, // Oldale, county-story-index 1
    {COUNTY_ID(1, 3), 0, &sMovingInChapter}, // Littleroot, county-story-index 0
    {COUNTY_ID(1, 3), 1, &sSaveBirchChapter}, // Littleroot, county-story-index 0
    {COUNTY_ID(0, 1), 0, &sRustboroCutterChapter}, // Rustboro, county-story-index 3
    {COUNTY_ID(0, 1), 1, &sRustboroFossilChapter}, // Rustboro, county-story-index 3
    {COUNTY_ID(0, 1), 2, &sRival2Chapter}, // Rival 2, met on Route 104 or in Rustboro
    {COUNTY_ID(4, 0), 0, &sLilycoveMuseumChapter}, // Lilycove, county-story-index 15
    {COUNTY_ID(4, 0), 1, &sLilycoveCaseChapter}, // Lilycove, county-story-index 15
    {COUNTY_ID(4, 0), 2, &sRival5Chapter}, // Lilycove's rival
    {COUNTY_ID(5, 1), 0, &sMossdeepSuperRodChapter}, // Mossdeep
    {COUNTY_ID(0, 3), 0, &sDewfordOldRodChapter}, // Dewford, county-story-index 4
    {COUNTY_ID(3, 0), 0, &sGoodRodChapter}, // Weather Institute county: Route 118's fisherman
    {COUNTY_ID(3, 0), 1, &sRival4Chapter}, // Route 119
    {COUNTY_ID(2, 2), 0, &sRival3Chapter}, // Route 110
    {COUNTY_ID(2, 2), 1, &sItemfinderChapter},
    {COUNTY_ID(2, 1), 0, &sMauvilleRockSmashChapter}, // Mauville, county-story-index 7
    {COUNTY_ID(2, 1), 1, &sMauvilleCoinCaseChapter},
    {COUNTY_ID(2, 1), 2, &sMauvilleBikeChapter},
    {COUNTY_ID(2, 1), CHAPTER_SLOT_STATELESS, &sMauvilleBikeProxyChapter},
    {COUNTY_ID(2, 3), 0, &sSlateportHarborMailChapter}, // Slateport
    {COUNTY_ID(0, 1), CHAPTER_SLOT_STATELESS, &sRustboroGymChapter},
    {COUNTY_ID(0, 3), CHAPTER_SLOT_STATELESS, &sDewfordGymChapter},
    {COUNTY_ID(2, 1), CHAPTER_SLOT_STATELESS, &sMauvilleGymChapter},
    {COUNTY_ID(2, 0), CHAPTER_SLOT_STATELESS, &sLavaridgeGymChapter},
    {COUNTY_ID(2, 1), CHAPTER_SLOT_STATELESS, &sNewMauvilleChapter},
    {COUNTY_ID(3, 1), CHAPTER_SLOT_STATELESS, &sFortreeGymChapter},
    {COUNTY_ID(5, 1), CHAPTER_SLOT_STATELESS, &sMossdeepGymChapter},
    {COUNTY_ID(4, 2), CHAPTER_SLOT_STATELESS, &sSootopolisGymChapter},
    {COUNTY_ID(0, 2), CHAPTER_SLOT_STATELESS, &sWallyRaltsChapter},
    {COUNTY_ID(0, 2), CHAPTER_SLOT_STATELESS, &sPetalburgGymChapter}, // Petalburg, hosting block 11's callback
    {COUNTY_ID(5, 2), CHAPTER_SLOT_STATELESS, &sEverGrandeLobbyChapter}, // League, hosting the lobby (gym 9)
    {COUNTY_ID(5, 2), CHAPTER_SLOT_STATELESS, &sEliteFourChapter},
    {COUNTY_ID(5, 2), CHAPTER_SLOT_STATELESS, &sChampionChapter},
    {COUNTY_ID(1, 2), CHAPTER_SLOT_STATELESS, &sRival1Chapter},
    {COUNTY_ID(0, 1), CHAPTER_SLOT_STATELESS, &sPokenavChapter},
    {COUNTY_ID(1, 3), CHAPTER_SLOT_STATELESS, &sStarterChapter},
    {COUNTY_ID(1, 3), CHAPTER_SLOT_STATELESS, &sPokedexChapter}, // Littleroot, hosting block 1's callback
    {COUNTY_ID(1, 3), CHAPTER_SLOT_STATELESS, &sPokeBallsChapter},
    {COUNTY_ID(1, 3), CHAPTER_SLOT_STATELESS, &sRunningShoesChapter},
    {COUNTY_ID(2, 3), CHAPTER_SLOT_STATELESS, &sShipyardChapter},
};
