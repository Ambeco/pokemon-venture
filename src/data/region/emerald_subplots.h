// Emerald's subplots over the ported chapters (design_chapters.md, "Plot kinds"). Chapters of one subplot run in order;
// chapters of different subplots never wait for each other, so a chapter is alone in its subplot unless its story
// needs the one before it. Required entries supply a main-story get; optional ones run beside the chain when no slot has room.

#define SUBPLOT_ENTRY_REQUIRED(chapter) {&chapter, FALSE}
#define SUBPLOT_ENTRY_OPTIONAL(chapter) {&chapter, TRUE}

// The tutorial in Littleroot: the opening, then Birch's rescue.
static const struct SubplotEntry sTutorialLittlerootEntries[] =
{
    SUBPLOT_ENTRY_REQUIRED(sMovingInChapter),
    SUBPLOT_ENTRY_REQUIRED(sSaveBirchChapter),
};
static const struct SubplotDesc sTutorialLittlerootSubplot = {ARRAY_COUNT(sTutorialLittlerootEntries), sTutorialLittlerootEntries};

static const struct SubplotEntry sTutorialOldaleEntries[] = {SUBPLOT_ENTRY_REQUIRED(sRivalDexChapter)};
static const struct SubplotDesc sTutorialOldaleSubplot = {ARRAY_COUNT(sTutorialOldaleEntries), sTutorialOldaleEntries};

static const struct SubplotEntry sRustboroCutterEntries[] = {SUBPLOT_ENTRY_REQUIRED(sRustboroCutterChapter)};
static const struct SubplotDesc sRustboroCutterSubplot = {ARRAY_COUNT(sRustboroCutterEntries), sRustboroCutterEntries};

static const struct SubplotEntry sRustboroFossilEntries[] = {SUBPLOT_ENTRY_OPTIONAL(sRustboroFossilChapter)};
static const struct SubplotDesc sRustboroFossilSubplot = {ARRAY_COUNT(sRustboroFossilEntries), sRustboroFossilEntries};

static const struct SubplotEntry sLilycoveMuseumEntries[] = {SUBPLOT_ENTRY_OPTIONAL(sLilycoveMuseumChapter)};
static const struct SubplotDesc sLilycoveMuseumSubplot = {ARRAY_COUNT(sLilycoveMuseumEntries), sLilycoveMuseumEntries};

static const struct SubplotEntry sLilycoveCaseEntries[] = {SUBPLOT_ENTRY_OPTIONAL(sLilycoveCaseChapter)};
static const struct SubplotDesc sLilycoveCaseSubplot = {ARRAY_COUNT(sLilycoveCaseEntries), sLilycoveCaseEntries};

static const struct SubplotEntry sMossdeepSuperRodEntries[] = {SUBPLOT_ENTRY_REQUIRED(sMossdeepSuperRodChapter)};
static const struct SubplotDesc sMossdeepSuperRodSubplot = {ARRAY_COUNT(sMossdeepSuperRodEntries), sMossdeepSuperRodEntries};

static const struct SubplotEntry sDewfordOldRodEntries[] = {SUBPLOT_ENTRY_REQUIRED(sDewfordOldRodChapter)};
static const struct SubplotDesc sDewfordOldRodSubplot = {ARRAY_COUNT(sDewfordOldRodEntries), sDewfordOldRodEntries};

// Mauville's three chapters are separate subplots: Rock Smash, the Coin Case (which waits on Slateport's Harbor Mail
// through a key, not through order) and Rydel's Cycles are independent errands.
static const struct SubplotEntry sMauvilleRockSmashEntries[] = {SUBPLOT_ENTRY_REQUIRED(sMauvilleRockSmashChapter)};
static const struct SubplotDesc sMauvilleRockSmashSubplot = {ARRAY_COUNT(sMauvilleRockSmashEntries), sMauvilleRockSmashEntries};

static const struct SubplotEntry sMauvilleCoinCaseEntries[] = {SUBPLOT_ENTRY_OPTIONAL(sMauvilleCoinCaseChapter)};
static const struct SubplotDesc sMauvilleCoinCaseSubplot = {ARRAY_COUNT(sMauvilleCoinCaseEntries), sMauvilleCoinCaseEntries};

// Rydel's Cycles is a parallel subplot: it runs beside the chain, and the chain holds its proxy, which waits for the
// Bicycle (design_chapters.md, "Parallel subplots (proxy entries)"). Both stand in the Bicycle's main-story slot.
static const struct SubplotEntry sMauvilleBikeEntries[] = {SUBPLOT_ENTRY_OPTIONAL(sMauvilleBikeChapter)};
static const struct SubplotDesc sMauvilleBikeSubplot = {ARRAY_COUNT(sMauvilleBikeEntries), sMauvilleBikeEntries};

static const struct SubplotEntry sMauvilleBikeProxyEntries[] = {SUBPLOT_ENTRY_REQUIRED(sMauvilleBikeProxyChapter)};
static const struct SubplotDesc sMauvilleBikeProxySubplot = {ARRAY_COUNT(sMauvilleBikeProxyEntries), sMauvilleBikeProxyEntries};

static const struct SubplotEntry sSlateportHarborMailEntries[] = {SUBPLOT_ENTRY_OPTIONAL(sSlateportHarborMailChapter)};
static const struct SubplotDesc sSlateportHarborMailSubplot = {ARRAY_COUNT(sSlateportHarborMailEntries), sSlateportHarborMailEntries};

// Wattson's New Mauville favor supplies no main-story get, so it runs off the chain beside the gym.
static const struct SubplotEntry sNewMauvilleEntries[] = {SUBPLOT_ENTRY_OPTIONAL(sNewMauvilleChapter)};
static const struct SubplotDesc sNewMauvilleSubplot = {ARRAY_COUNT(sNewMauvilleEntries), sNewMauvilleEntries};

static const struct SubplotEntry sWallyRaltsEntries[] = {SUBPLOT_ENTRY_REQUIRED(sWallyRaltsChapter)};
static const struct SubplotDesc sWallyRaltsSubplot = {ARRAY_COUNT(sWallyRaltsEntries), sWallyRaltsEntries};

// Each gym is its own required subplot (its badge is a main-story get, so it is bound to the gym's slot).
#define GYM_SUBPLOT(name) \
static const struct SubplotEntry name##Entries[] = {SUBPLOT_ENTRY_REQUIRED(name##Chapter)}; \
static const struct SubplotDesc name##Subplot = {ARRAY_COUNT(name##Entries), name##Entries}

GYM_SUBPLOT(sRustboroGym);
GYM_SUBPLOT(sDewfordGym);
GYM_SUBPLOT(sMauvilleGym);
GYM_SUBPLOT(sLavaridgeGym);
GYM_SUBPLOT(sFortreeGym);
GYM_SUBPLOT(sMossdeepGym);
GYM_SUBPLOT(sSootopolisGym);
GYM_SUBPLOT(sPetalburgGym);

static const struct SubplotHost sEmeraldSubplotHosts[] =
{
    {COUNTY_ID(1, 3), &sTutorialLittlerootSubplot},
    {COUNTY_ID(1, 2), &sTutorialOldaleSubplot},
    {COUNTY_ID(0, 1), &sRustboroCutterSubplot},
    {COUNTY_ID(0, 1), &sRustboroFossilSubplot},
    {COUNTY_ID(4, 0), &sLilycoveMuseumSubplot},
    {COUNTY_ID(4, 0), &sLilycoveCaseSubplot},
    {COUNTY_ID(5, 1), &sMossdeepSuperRodSubplot},
    {COUNTY_ID(0, 3), &sDewfordOldRodSubplot},
    {COUNTY_ID(2, 1), &sMauvilleRockSmashSubplot},
    {COUNTY_ID(2, 1), &sMauvilleCoinCaseSubplot},
    {COUNTY_ID(2, 1), &sMauvilleBikeSubplot},
    {COUNTY_ID(2, 1), &sMauvilleBikeProxySubplot},
    {COUNTY_ID(2, 3), &sSlateportHarborMailSubplot},
    {COUNTY_ID(0, 1), &sRustboroGymSubplot},
    {COUNTY_ID(0, 3), &sDewfordGymSubplot},
    {COUNTY_ID(2, 1), &sMauvilleGymSubplot},
    {COUNTY_ID(2, 0), &sLavaridgeGymSubplot},
    {COUNTY_ID(2, 1), &sNewMauvilleSubplot},
    {COUNTY_ID(3, 1), &sFortreeGymSubplot},
    {COUNTY_ID(5, 1), &sMossdeepGymSubplot},
    {COUNTY_ID(4, 2), &sSootopolisGymSubplot},
    {COUNTY_ID(0, 2), &sWallyRaltsSubplot},
    {COUNTY_ID(0, 2), &sPetalburgGymSubplot},
};
