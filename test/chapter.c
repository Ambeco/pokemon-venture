#include "global.h"
#include "test/test.h"
#include "constants/flags.h"
#include "constants/items.h"
#include "constants/region_map_sections.h"
#include "constants/vars.h"
#include "constants/characters.h"
#include "event_data.h"
#include "plot_stage.h"
#include "region_county.h"
#include "region_plot.h"
#include "region_unlock.h"
#include "scene.h"
#include "region_city.h"
#include "chapter.h"
#include "county_position.h"
#include "county_plan.h"
#include "subplot.h"
#include "string_util.h"

#define LILYCOVE COUNTY_ID(4, 0)
#define RUSTBORO COUNTY_ID(0, 1)
#define MOSSDEEP COUNTY_ID(5, 1)
#define DEWFORD COUNTY_ID(0, 3)
#define MAUVILLE COUNTY_ID(2, 1)
#define SLATEPORT COUNTY_ID(2, 3)
#define LITTLEROOT COUNTY_ID(1, 3)
#define OLDALE COUNTY_ID(1, 2)
#define PETALBURG COUNTY_ID(0, 2)
#define WEATHER COUNTY_ID(3, 0)
#define ROUTE110_COUNTY COUNTY_ID(2, 2)

static const u8 sTextA[] = _("a");
static const u8 sTextB[] = _("b");
static const u8 sTextC[] = _("c");
static const u8 sTextD[] = _("d");

static const u8 *const sListA[] = {sTextA};
static const u8 *const sListB[] = {sTextB, sTextC};
static const u8 *const sListC[] = {sTextC};
static const u8 *const sListD[] = {sTextD};
static const u8 *const sListCD[] = {sTextC, sTextD};

// Devon Goods shaped: the order of the predicates, not set-ness, defines the stage.
static const struct ChapterStage sDevonStages[] =
{
    {},
    {.done = {STORY_PRED_FLAG, FLAG_DEVON_GOODS_STOLEN, 0}},
    {.done = {STORY_PRED_FLAG, FLAG_RECOVERED_DEVON_GOODS, 0}},
    {.done = {STORY_PRED_FLAG, FLAG_RETURNED_DEVON_GOODS, 0}},
};

static const struct ChapterHint sDevonHints[] =
{
    {0, 0, TEXT_LIST(sListA)},
    {1, 1, TEXT_LIST(sListB)},
    {2, 2, TEXT_LIST(sListC)},
    {3, 3, TEXT_LIST(sListD)},
};

static const struct ChapterDesc sDevonAdapter =
{
    .backend = CHAPTER_ADAPTER,
    .numStages = ARRAY_COUNT(sDevonStages),
    .numHints = ARRAY_COUNT(sDevonHints),
    .stages = sDevonStages,
    .hints = sDevonHints,
};

// Ranges that end at the pseudo-stages; stage 2 and AFTER have empty lists, so they say generic text.
static const struct ChapterHint sFullHints[] =
{
    {CHAPTER_STAGE_BEFORE, 0, TEXT_LIST(sListB)},
    {1, 1, TEXT_LIST(sListA), TEXT_LIST(sListCD)},
    {2, CHAPTER_STAGE_AFTER},
};

static const struct ChapterStage sFullStages[] =
{
    {},
    {
        .numHinters = 2,
        .hinterRoles = 0x4,
    },
    {},
};

static const struct ChapterActor sActors[] =
{
    {0, 1, 0x2, 7},
    {1, 2, 0, 8},
    {2, 2, 0x1, 9},
};

static const struct ChapterDesc sFullDesc =
{
    .backend = CHAPTER_ADAPTER,
    .numStages = ARRAY_COUNT(sFullStages),
    .numActors = ARRAY_COUNT(sActors),
    .numHints = ARRAY_COUNT(sFullHints),
    .stages = sFullStages,
    .hints = sFullHints,
    .actors = sActors,
};

TEST("Chapter: Mossdeep, Dewford and Slateport host one chapter in slot 0, the Weather Institute and Route 110 counties in slots 0 and 1, Mauville, Lilycove and Rustboro in slots 0 to 2, Littleroot in slots 0 and 1, Oldale in slot 0")
{
    u32 county, slot;

    for (county = 0; county < NUM_COUNTIES; county++)
    {
        for (slot = 0; slot < CHAPTERS_PER_COUNTY; slot++)
        {
            if (((county == MOSSDEEP || county == DEWFORD || county == SLATEPORT) && slot == 0)
             || ((county == WEATHER || county == ROUTE110_COUNTY) && slot <= 1)
             || ((county == MAUVILLE || county == LILYCOVE || county == RUSTBORO) && slot <= 2)
             || (county == LITTLEROOT && slot <= 1) || (county == OLDALE && slot == 0))
                EXPECT(GetCountyChapter(county, slot) != NULL);
            else
                EXPECT(GetCountyChapter(county, slot) == NULL);
        }
    }
}

TEST("Chapter: Slot vars lie in the county's own slice after its chain var")
{
    u32 county, slot;

    for (county = 0; county < NUM_COUNTIES; county++)
    {
        u32 start = COUNTY_VARS_START + GetCountyVarSlice(county) * VARS_PER_COUNTY;

        if (GetCountyVarSlice(county) >= COUNTY_VAR_SLICES_FREE)
            continue;
        for (slot = 0; slot < CHAPTERS_PER_COUNTY; slot++)
        {
            EXPECT_EQ(GetCountySlotVar(county, slot), start + 1 + slot);
            EXPECT(GetCountySlotVar(county, slot) < start + VARS_PER_COUNTY);
        }
    }
    EXPECT(GetCountyVarSlice(LILYCOVE) < COUNTY_VAR_SLICES_FREE);
}

TEST("Chapter: County var slices are a permutation, and the tutorial counties' slices are free")
{
    bool8 used[NUM_COUNTIES] = {FALSE};
    u32 county;

    for (county = 0; county < NUM_COUNTIES; county++)
    {
        ASSUME(GetCountyVarSlice(county) < NUM_COUNTIES);
        EXPECT(!used[GetCountyVarSlice(county)]);
        used[GetCountyVarSlice(county)] = TRUE;
    }
    EXPECT_EQ(GetCountyVarSlice(LITTLEROOT), COUNTY_VAR_SLICE_LITTLEROOT);
    EXPECT_EQ(GetCountyVarSlice(OLDALE), COUNTY_VAR_SLICE_OLDALE);
    EXPECT(COUNTY_VAR_SLICE_LITTLEROOT < COUNTY_VAR_SLICES_FREE);
    EXPECT(COUNTY_VAR_SLICE_OLDALE < COUNTY_VAR_SLICES_FREE);
    EXPECT_EQ(GetCountySlotVar(LITTLEROOT, 1), VAR_SAVE_BIRCH_BEAT);
}

TEST("Chapter: Whole-var masks for the tutorial chapters, handed out in declaration order, land on the addresses scripts use")
{
    struct CountyBitMask mask;

    ASSUME(GetChapterMask(LITTLEROOT, 0, &mask));
    EXPECT_EQ(COUNTY_VARS_START + COUNTY_VAR_SLICE_LITTLEROOT * VARS_PER_COUNTY + mask.var, VAR_MOVING_IN_BEAT);
    EXPECT_EQ(mask.shift, 0);
    EXPECT_EQ(mask.width, COUNTY_MASK_BITS_PER_VAR);
    ASSUME(GetChapterMask(LITTLEROOT, 1, &mask));
    EXPECT_EQ(COUNTY_VARS_START + COUNTY_VAR_SLICE_LITTLEROOT * VARS_PER_COUNTY + mask.var, VAR_SAVE_BIRCH_BEAT);
    EXPECT_EQ(mask.shift, 0);
    ASSUME(GetChapterMask(OLDALE, 0, &mask));
    EXPECT_EQ(COUNTY_VARS_START + COUNTY_VAR_SLICE_OLDALE * VARS_PER_COUNTY + mask.var, VAR_RIVAL_DEX_BEAT);
    EXPECT_EQ(mask.shift, 0);
}

static bool32 IsWholeVarMask(u32 var)
{
    return var == VAR_MOVING_IN_BEAT || var == VAR_SAVE_BIRCH_BEAT || var == VAR_RIVAL_DEX_BEAT;
}

// A var inside a free county slice may be packed (var 0) or shared bits (vars 1-3), so a >= compare is meaningful only on a whole-var mask.
static void ExpectPredicateAvoidsPackedVars(const struct StoryPredicate *pred, const char *where)
{
    if (pred->op != STORY_PRED_VAR_GE)
        return;
    if (pred->a >= COUNTY_VARS_START && pred->a < COUNTY_VARS_START + COUNTY_VAR_SLICES_FREE * VARS_PER_COUNTY)
    {
        if (!IsWholeVarMask(pred->a))
            Test_ExitWithResult(TEST_RESULT_FAIL, __LINE__, ":L%s:%d: %s compares county slice var %d (offset from COUNTY_VARS_START) with >=; a packed or shared var cannot be compared whole, so read it through a chapter accessor or give it a whole-var mask", gTestRunnerState.test->filename, __LINE__, where, pred->a - COUNTY_VARS_START);
    }
}

TEST("Chapter: No >= predicate on a county slice var unless it is a whole-var mask")
{
    struct PlotGetDesc descs[MAX_GETS_PER_LIST];
    const struct ChapterHost *hosts;
    u32 index, i, s, numHosts, count;

    for (index = 0; index < MAIN_STORY_INDEX_POSTGAME; index++)
    {
        count = GetPlotListGetDescs(index, descs);
        for (i = 0; i < count; i++)
            ExpectPredicateAvoidsPackedVars(&descs[i].done, "a plot-list get");
    }
    hosts = GetChapterHosts(&numHosts);
    for (i = 0; i < numHosts; i++)
    {
        const struct ChapterDesc *desc = hosts[i].desc;

        if (desc->backend != CHAPTER_ADAPTER)
            continue;
        for (s = 1; s < desc->numStages; s++)
            ExpectPredicateAvoidsPackedVars(&desc->stages[s].done, "an adapter chapter stage");
    }
}

TEST("Chapter: A stored chapter starts at stage 0 and keeps the stage set through its slot")
{
    EXPECT_EQ(GetCountySlotStage(LILYCOVE, 0), 0);
    SetCountySlotStage(LILYCOVE, 0, 1);
    EXPECT_EQ(GetCountySlotStage(LILYCOVE, 0), 1);
    EXPECT_EQ(VarGet(GetCountySlotVar(LILYCOVE, 0)), 1);
}

TEST("Chapter: Setting one county's slot leaves other slices untouched")
{
    u32 var;

    SetCountySlotStage(LILYCOVE, 0, 1);
    for (var = COUNTY_VARS_START; var < COUNTY_VARS_START + COUNTY_VAR_SLICES_FREE * VARS_PER_COUNTY; var++)
    {
        if (var != GetCountySlotVar(LILYCOVE, 0))
            EXPECT_EQ(VarGet(var), 0);
    }
}

TEST("Chapter: The talk query follows the stage and the stage texts differ")
{
    const u8 *far0 = GetCountySlotHint(LILYCOVE, 0, CHAPTER_FAR, 0);
    const u8 *close0 = GetCountySlotHint(LILYCOVE, 0, CHAPTER_CLOSE, 0);

    SetCountySlotStage(LILYCOVE, 0, 1);
    EXPECT(far0 != NULL && close0 != NULL);
    EXPECT(GetCountySlotHint(LILYCOVE, 0, CHAPTER_FAR, 0) != far0);
    EXPECT(GetCountySlotHint(LILYCOVE, 0, CHAPTER_CLOSE, 0) != close0);
}

TEST("Chapter: The query returns the range's list for each distance and stage")
{
    EXPECT(GetChapterHint(&sFullDesc, 0, CHAPTER_FAR, 0) == sTextB);
    EXPECT(GetChapterHint(&sFullDesc, 1, CHAPTER_FAR, 0) == sTextA);
    EXPECT(GetChapterHint(&sFullDesc, 0, CHAPTER_CLOSE, 0) == NULL);
    EXPECT(GetChapterHint(&sFullDesc, 1, CHAPTER_CLOSE, 0) == sTextC);
    EXPECT(GetChapterHint(&sFullDesc, 1, CHAPTER_CLOSE, 1) == sTextD);
}

TEST("Chapter: A range may end at a pseudo-stage, which the query accepts")
{
    EXPECT(FindChapterHint(&sFullDesc, CHAPTER_STAGE_BEFORE) == &sFullHints[0]);
    EXPECT(FindChapterHint(&sFullDesc, 0) == &sFullHints[0]);
    EXPECT(FindChapterHint(&sFullDesc, 1) == &sFullHints[1]);
    EXPECT(FindChapterHint(&sFullDesc, 2) == &sFullHints[2]);
    EXPECT(FindChapterHint(&sFullDesc, CHAPTER_STAGE_AFTER) == &sFullHints[2]);
    EXPECT(GetChapterHint(&sFullDesc, CHAPTER_STAGE_BEFORE, CHAPTER_FAR, 0) == sTextB);
    EXPECT(GetChapterHint(&sFullDesc, CHAPTER_STAGE_AFTER, CHAPTER_FAR, 0) == NULL);
}

TEST("Chapter: A stage no range covers has no hint")
{
    static const struct ChapterHint sGap[] = {{1, 1, TEXT_LIST(sListA)}};
    static const struct ChapterDesc sGapDesc = {.backend = CHAPTER_ADAPTER, .numStages = 3, .numHints = 1, .hints = sGap};

    EXPECT(FindChapterHint(&sGapDesc, 0) == NULL);
    EXPECT(FindChapterHint(&sGapDesc, CHAPTER_STAGE_BEFORE) == NULL);
    EXPECT(GetChapterHint(&sGapDesc, 2, CHAPTER_FAR, 0) == NULL);
    EXPECT(GetChapterHint(&sGapDesc, 1, CHAPTER_FAR, 0) == sTextA);
}

TEST("Chapter: A list entry is picked by selector, stably")
{
    EXPECT(GetChapterHint(&sFullDesc, 0, CHAPTER_FAR, 0) == sTextB);
    EXPECT(GetChapterHint(&sFullDesc, 0, CHAPTER_FAR, 1) == sTextC);
    EXPECT(GetChapterHint(&sFullDesc, 0, CHAPTER_FAR, 2) == sTextB);
    EXPECT(GetChapterHint(&sFullDesc, 0, CHAPTER_FAR, 1) == GetChapterHint(&sFullDesc, 0, CHAPTER_FAR, 1));
}

TEST("Chapter: Empty lists give NULL so the NPC says generic text")
{
    EXPECT(GetChapterHint(&sFullDesc, 2, CHAPTER_FAR, 5) == NULL);
    EXPECT(GetChapterHint(&sFullDesc, 2, CHAPTER_CLOSE, 5) == NULL);
}

TEST("Chapter: The county query uses the current stage")
{
    const struct ChapterDesc *desc = GetCountyChapter(LILYCOVE, 0);

    EXPECT(GetCountySlotHint(LILYCOVE, 0, CHAPTER_CLOSE, 0) == GetChapterHint(desc, 0, CHAPTER_CLOSE, 0));
    SetCountySlotStage(LILYCOVE, 0, 1);
    EXPECT(GetCountySlotHint(LILYCOVE, 0, CHAPTER_CLOSE, 0) == GetChapterHint(desc, 1, CHAPTER_CLOSE, 0));
    EXPECT(GetChapterHint(desc, 0, CHAPTER_CLOSE, 0) != GetChapterHint(desc, 1, CHAPTER_CLOSE, 0));
}

TEST("Chapter: Each ported chapter's hint ranges cover every stage once and have both lists")
{
    u32 county, slot, stage, i;

    for (county = 0; county < NUM_COUNTIES; county++)
    {
        for (slot = 0; slot < CHAPTERS_PER_COUNTY; slot++)
        {
            const struct ChapterDesc *desc = GetCountyChapter(county, slot);

            if (desc == NULL)
                continue;
            for (stage = 0; stage < desc->numStages; stage++)
            {
                const struct ChapterHint *hint = FindChapterHint(desc, stage);
                u32 covering = 0;

                for (i = 0; i < desc->numHints; i++)
                    covering += DoesChapterHintCover(&desc->hints[i], stage);
                EXPECT_EQ(covering, 1);
                ASSUME(hint != NULL);
                EXPECT(hint->far.count > 0);
                EXPECT(hint->close.count > 0);
            }
        }
    }
}

TEST("Chapter: A chapter that produces a McGuffin gives a reward")
{
    u32 numHosts, i;
    const struct ChapterHost *hosts = GetChapterHosts(&numHosts);

    for (i = 0; i < numHosts; i++)
    {
        if (hosts[i].desc->producedKey.type != CHAPTER_KEY_NONE)
            EXPECT(hosts[i].desc->givesReward);
    }
}

TEST("Chapter: No chapter both requires and produces a key")
{
    u32 numHosts, i;
    const struct ChapterHost *hosts = GetChapterHosts(&numHosts);

    for (i = 0; i < numHosts; i++)
    {
        const struct ChapterDesc *desc = hosts[i].desc;
        bool32 requires = desc->requiredKeys[0].type != CHAPTER_KEY_NONE || desc->requiredKeys[1].type != CHAPTER_KEY_NONE;

        EXPECT(!(requires && desc->producedKey.type != CHAPTER_KEY_NONE));
    }
}

TEST("Chapter: Hinter counts come from the stage")
{
    EXPECT_EQ(sFullDesc.stages[1].numHinters, 2);
    EXPECT_EQ(GetCountyChapter(LILYCOVE, 0)->stages[0].numHinters, 1);
}

TEST("Chapter: Actors are listed for the stages they cover")
{
    const struct ChapterActor *out[2];

    EXPECT_EQ(GetChapterActors(&sFullDesc, 0, out, 2), 1);
    EXPECT_EQ(out[0]->gfxId, 7);
    EXPECT_EQ(GetChapterActors(&sFullDesc, 1, out, 2), 2);
    EXPECT_EQ(out[0]->gfxId, 7);
    EXPECT_EQ(out[1]->gfxId, 8);
    EXPECT_EQ(GetChapterActors(&sFullDesc, 2, out, 1), 2); // total, even past maxOut
    EXPECT_EQ(out[0]->gfxId, 8);
    EXPECT_EQ(GetChapterActors(GetCountyChapter(LILYCOVE, 0), 0, out, 2), 0);
}

TEST("Chapter: An adapter chapter's stage is the highest true predicate")
{
    EXPECT_EQ(GetChapterStage(0, 0, &sDevonAdapter), 0);
    FlagSet(FLAG_DEVON_GOODS_STOLEN);
    EXPECT_EQ(GetChapterStage(0, 0, &sDevonAdapter), 1);
    FlagSet(FLAG_RECOVERED_DEVON_GOODS);
    FlagClear(FLAG_DEVON_GOODS_STOLEN); // the game clears STOLEN on recovery
    EXPECT_EQ(GetChapterStage(0, 0, &sDevonAdapter), 2);
    FlagSet(FLAG_RETURNED_DEVON_GOODS);
    EXPECT_EQ(GetChapterStage(0, 0, &sDevonAdapter), 3);
}

TEST("Chapter: An adapter chapter skipped ahead reports the later stage")
{
    FlagSet(FLAG_RETURNED_DEVON_GOODS);
    EXPECT_EQ(GetChapterStage(0, 0, &sDevonAdapter), 3);
}

TEST("Chapter: The hint query treats adapter and stored chapters alike")
{
    FlagSet(FLAG_RECOVERED_DEVON_GOODS);
    EXPECT(GetChapterHint(&sDevonAdapter, GetChapterStage(0, 0, &sDevonAdapter), CHAPTER_FAR, 0) == sTextC);
    EXPECT(GetChapterHint(&sDevonAdapter, 3, CHAPTER_FAR, 0) == sTextD);
}

TEST("Chapter: A county is found from its city's MAPSEC")
{
    EXPECT_EQ(GetCountyFromCityMapSec(MAPSEC_LILYCOVE_CITY), LILYCOVE);
}

TEST("Chapter: Ported chapters anchor every stage to a real place")
{
    u32 county, slot, stage;

    for (county = 0; county < NUM_COUNTIES; county++)
    {
        for (slot = 0; slot < CHAPTERS_PER_COUNTY; slot++)
        {
            const struct ChapterDesc *desc = GetCountyChapter(county, slot);

            if (desc == NULL)
                continue;
            for (stage = 0; stage < desc->numStages; stage++)
            {
                const struct BeatAnchor *anchor = &desc->stages[stage].anchor;

                EXPECT(anchor->kind <= BEAT_ANCHOR_DOOR);
                EXPECT(anchor->locationKind == BEAT_LOCATION_NONE || anchor->locationKind < NUM_LOCATION_KINDS);
                EXPECT(anchor->routeId == ROUTE_ID_NONE || anchor->routeId < NUM_COUNTIES + MAX_SPECIAL_LOCATIONS_PER_COUNTY);
                // An immobile NPC or item may stand outdoors in the city (no role, location or route).
                EXPECT(anchor->role != 0 || anchor->locationKind != BEAT_LOCATION_NONE || anchor->routeId != ROUTE_ID_NONE
                    || anchor->kind == BEAT_ANCHOR_NPC || anchor->kind == BEAT_ANCHOR_ITEM);
            }
        }
    }
}

TEST("Chapter: The Lilycove Museum beat is the curator in the museum")
{
    const struct BeatAnchor *anchor = &GetCountyChapter(LILYCOVE, 0)->stages[0].anchor;

    EXPECT_EQ(anchor->kind, BEAT_ANCHOR_NPC);
    EXPECT_EQ(anchor->locationKind, TOWN_LOCATION_MUSEUM);
}

TEST("Chapter: A hinter's distance follows the anchor's city")
{
    const struct BeatAnchor *anchor = &GetCountyChapter(LILYCOVE, 0)->stages[0].anchor;
    u32 county;

    for (county = 0; county < NUM_COUNTIES; county++)
    {
        if (county == LILYCOVE)
            EXPECT_EQ(GetBeatDistance(LILYCOVE, anchor, county), CHAPTER_CLOSE);
        else
            EXPECT_EQ(GetBeatDistance(LILYCOVE, anchor, county), CHAPTER_FAR);
    }
}

TEST("Chapter: The anchor reference names the building, location, road or city")
{
    u8 buf[64];
    struct BeatAnchor door = {BEAT_ANCHOR_DOOR, ROUTE_ID_NONE, ROUTE_ID_NONE, BEAT_LOCATION_NONE, BUILDING_POKEMON_CENTER};
    struct BeatAnchor house = {BEAT_ANCHOR_DOOR, ROUTE_ID_NONE, ROUTE_ID_NONE, BEAT_LOCATION_NONE, BUILDING_HOUSE};
    struct BeatAnchor museum = GetCountyChapter(LILYCOVE, 0)->stages[0].anchor;
    struct BeatAnchor road = {BEAT_ANCHOR_ROUTE_ROUTE, LILYCOVE, COUNTY_ID(4, 1), BEAT_LOCATION_NONE, 0};
    struct BeatAnchor outdoors = {BEAT_ANCHOR_ITEM, ROUTE_ID_NONE, ROUTE_ID_NONE, BEAT_LOCATION_NONE, 0};

    WriteBeatAnchorReference(buf, LILYCOVE, &door);
    EXPECT_EQ(StringCompare(buf, COMPOUND_STRING("near the POKéMON CENTER")), 0);
    WriteBeatAnchorReference(buf, LILYCOVE, &house);
    EXPECT_EQ(StringCompare(buf, COMPOUND_STRING("near the house")), 0);
    EXPECT_EQ(WriteBeatAnchorReference(buf, LILYCOVE, &museum) - buf, StringLength(COMPOUND_STRING("near the MUSEUM")));
    EXPECT_EQ(StringCompare(buf, COMPOUND_STRING("near the MUSEUM")), 0);
    WriteBeatAnchorReference(buf, LILYCOVE, &road);
    EXPECT_EQ(StringCompareN(buf, COMPOUND_STRING("near the road to "), StringLength(COMPOUND_STRING("near the road to "))), 0);
    EXPECT_EQ(StringCompare(buf + StringLength(COMPOUND_STRING("near the road to ")), GetCountyCityName(LILYCOVE)), 0);
    WriteBeatAnchorReference(buf, LILYCOVE, &outdoors);
    EXPECT_EQ(buf[0], COMPOUND_STRING("in ")[0]);
}

TEST("Chapter: A hint template expands {HINT_ANCHOR} to the anchor reference and copies the rest")
{
    u8 out[96];
    const struct BeatAnchor *anchor = &GetCountyChapter(LILYCOVE, 0)->stages[0].anchor;
    static const u8 sTemplate[] = _("Go {HINT_ANCHOR}, {PLAYER}!");

    ExpandHintTemplate(out, sTemplate, LILYCOVE, anchor);
    EXPECT_EQ(StringCompare(out, COMPOUND_STRING("Go near the MUSEUM, {PLAYER}!")), 0);
}

TEST("Chapter: A hint template expands {HINT_CITY} to the host county's city name")
{
    u8 out[96];
    u8 expect[96];
    static const u8 sTemplate[] = _("A man in {HINT_CITY} waits.");

    ExpandHintTemplate(out, sTemplate, RUSTBORO, &GetCountyChapter(RUSTBORO, 0)->stages[0].anchor);
    StringCopy(StringCopy(StringCopy(expect, COMPOUND_STRING("A man in ")), GetCountyCityName(RUSTBORO)), COMPOUND_STRING(" waits."));
    EXPECT_EQ(StringCompare(out, expect), 0);
}

TEST("Chapter: The Lilycove stage-0 close hint about the unopened hall names the museum through the template")
{
    u8 out[96];
    const struct ChapterDesc *desc = GetCountyChapter(LILYCOVE, 0);
    const u8 *text = GetChapterHint(desc, 0, CHAPTER_CLOSE, 1);

    ASSUME(text != NULL);
    ExpandHintTemplate(out, text, LILYCOVE, &desc->stages[1].anchor);
    EXPECT(StringLength(out) > StringLength(text) - 4);
    EXPECT(StringCompareN(out, COMPOUND_STRING("There's talk of a new exhibit hall\nnear the MUSEUM."), 40) == 0);
}

// A gym-4-gated chapter with a parameterized hint, a produced host-pool key and a two-building cutscene.
static const struct ChapterStage sGatedStages[] =
{
    {.hintCount = CHAPTER_COUNT_BADGES_SHORT,
     .anchor = {BEAT_ANCHOR_NPC, ROUTE_ID_NONE, ROUTE_ID_NONE, BEAT_LOCATION_NONE, BUILDING_GYM}, .extraRoles = BUILDING_HOUSE},
    {.anchor = {BEAT_ANCHOR_DOOR, ROUTE_ID_NONE, ROUTE_ID_NONE, BEAT_LOCATION_NONE, BUILDING_MART}},
    {},
};

static const struct ChapterDesc sGatedDesc =
{
    .backend = CHAPTER_STORED,
    .numStages = ARRAY_COUNT(sGatedStages),
    .requiredRoles = BUILDING_POKEMON_CENTER,
    .minBadges = 4,
    .producedKeyStage = 2,
    .producedKey = {CHAPTER_KEY_HOST_POOL, 1, 0},
    .stages = sGatedStages,
};

// Stages with no anchor role, so any city with a Pokémon Center could host them.
static const struct ChapterStage sPlainStages[] =
{
    {},
    {},
};

static const struct ChapterDesc sConsumerDesc =
{
    .backend = CHAPTER_STORED,
    .numStages = ARRAY_COUNT(sPlainStages),
    .requiredKeys = {{CHAPTER_KEY_HOST_POOL, 1, 0}},
    .stages = sPlainStages,
};

static const struct ChapterDesc sCrossConsumerDesc =
{
    .backend = CHAPTER_STORED,
    .numStages = ARRAY_COUNT(sPlainStages),
    .requiredKeys = {{CHAPTER_KEY_ITEM, PLOT_KIND_GYM, 4}},
    .stages = sPlainStages,
};

static const struct ChapterDesc sItemProducerDesc =
{
    .backend = CHAPTER_STORED,
    .numStages = ARRAY_COUNT(sPlainStages),
    .producedKeyStage = 1,
    .producedKey = {CHAPTER_KEY_ITEM, PLOT_KIND_GYM, 4},
    .stages = sPlainStages,
};

static void SetBadges(u32 mask)
{
    u32 i;

    for (i = 0; i < NUM_BADGES; i++)
    {
        if (mask & (1u << i))
            FlagSet(FLAG_BADGE01_GET + i);
        else
            FlagClear(FLAG_BADGE01_GET + i);
    }
}

TEST("Chapter: Rustboro's Cutter chapter is stored, role-anchored and needs only a house")
{
    const struct ChapterDesc *desc = GetCountyChapter(RUSTBORO, 0);

    ASSUME(desc != NULL);
    EXPECT_EQ(desc->backend, CHAPTER_STORED);
    EXPECT_EQ(desc->numStages, 2);
    EXPECT_EQ(desc->stages[0].anchor.role, BUILDING_HOUSE);
    EXPECT_EQ(GetChapterNeededRoles(desc), BUILDING_HOUSE);
    EXPECT_EQ(desc->producedKey.type, CHAPTER_KEY_NONE);
}

TEST("Chapter: The Cutter chapter keeps its stage in Rustboro's slice and not Lilycove's")
{
    EXPECT_EQ(GetCountySlotStage(RUSTBORO, 0), 0);
    SetCountySlotStage(RUSTBORO, 0, 1);
    EXPECT_EQ(GetCountySlotStage(RUSTBORO, 0), 1);
    EXPECT_EQ(VarGet(GetCountySlotVar(RUSTBORO, 0)), 1);
    EXPECT_EQ(GetCountySlotStage(LILYCOVE, 0), 0);
}

TEST("Chapter: The Cutter hints follow the stage and expand the house reference")
{
    u8 out[96];
    const struct ChapterDesc *desc = GetCountyChapter(RUSTBORO, 0);
    const u8 *before = GetChapterHint(desc, 0, CHAPTER_CLOSE, 1);

    ASSUME(before != NULL);
    ExpandHintTemplate(out, before, RUSTBORO, &desc->stages[1].anchor);
    EXPECT_EQ(StringCompareN(out, COMPOUND_STRING("People say the man near the house"), 33), 0);
    EXPECT(GetCountySlotHint(RUSTBORO, 0, CHAPTER_FAR, 0) == desc->hints[0].far.texts[0]);
    SetCountySlotStage(RUSTBORO, 0, 1);
    EXPECT(GetCountySlotHint(RUSTBORO, 0, CHAPTER_FAR, 0) == desc->hints[1].far.texts[0]);
}

TEST("Chapter: Every Emerald host passes the placement check")
{
    u32 numHosts, i, j, numOthers;
    struct ChapterHost hosts[48], others[48];

    numHosts = GetPlannedChapterHosts(hosts, ARRAY_COUNT(hosts));
    EXPECT_EQ(numHosts, 40);
    for (i = 0; i < numHosts; i++)
    {
        for (j = 0, numOthers = 0; j < numHosts; j++)
        {
            if (j != i)
                others[numOthers++] = hosts[j];
        }
        EXPECT_EQ(CheckChapterPlacement(hosts[i].desc, hosts[i].county, hosts[i].mainSlot, others, numOthers), CHAPTER_PLACEMENT_OK);
    }
}

TEST("Chapter: Rustboro's fossil revival is stored in slot 1, anchored on the service role, and its choice is a flag")
{
    const struct ChapterDesc *desc = GetCountyChapter(RUSTBORO, 1);

    ASSUME(desc != NULL);
    EXPECT_EQ(desc->backend, CHAPTER_STORED);
    EXPECT_EQ(desc->numStages, 3);
    EXPECT_EQ(GetChapterNeededRoles(desc), BUILDING_SERVICE);
    EXPECT_EQ(GetCountySlotStage(RUSTBORO, 1), 0);
    SetCountySlotStage(RUSTBORO, 1, 2);
    EXPECT_EQ(GetCountySlotStage(RUSTBORO, 1), 2);
    EXPECT_EQ(GetCountySlotStage(RUSTBORO, 0), 0);
    EXPECT(GetCountySlotHint(RUSTBORO, 1, CHAPTER_FAR, 0) == desc->hints[2].far.texts[0]);
    FlagSet(FLAG_FOSSIL_IS_ANORITH);
    EXPECT(FlagGet(FLAG_FOSSIL_IS_ANORITH));
}

TEST("Chapter: Lilycove's Pokeblock Case chapter is stored in slot 1, anchored on the contest hall, and its hints follow the stage")
{
    const struct ChapterDesc *desc = GetCountyChapter(LILYCOVE, 1);

    ASSUME(desc != NULL);
    EXPECT_EQ(desc->backend, CHAPTER_STORED);
    EXPECT_EQ(desc->numStages, 2);
    EXPECT_EQ(GetChapterNeededRoles(desc), BUILDING_CONTEST_HALL);
    EXPECT_EQ(desc->producedKey.type, CHAPTER_KEY_NONE);
    EXPECT_EQ(GetCountySlotStage(LILYCOVE, 1), 0);
    EXPECT(GetCountySlotHint(LILYCOVE, 1, CHAPTER_FAR, 0) == desc->hints[0].far.texts[0]);
    SetCountySlotStage(LILYCOVE, 1, 1);
    EXPECT_EQ(GetCountySlotStage(LILYCOVE, 0), 0);
    EXPECT(GetCountySlotHint(LILYCOVE, 1, CHAPTER_FAR, 0) == desc->hints[1].far.texts[0]);
}

TEST("Chapter: Mossdeep's Super Rod chapter is stored, role-anchored, and its scene writes the slot var")
{
    const struct ChapterDesc *desc = GetCountyChapter(MOSSDEEP, 0);
    const struct Scene *scene = GetScene(SCENE_MOSSDEEP_SUPER_ROD);

    ASSUME(desc != NULL);
    EXPECT_EQ(desc->backend, CHAPTER_STORED);
    EXPECT_EQ(desc->numStages, 2);
    EXPECT_EQ(GetChapterNeededRoles(desc), BUILDING_HOUSE);
    EXPECT_EQ(desc->producedKey.type, CHAPTER_KEY_NONE);
    EXPECT_EQ(scene->stageVar, SCENE_STAGE(MOSSDEEP, 0));
}

TEST("Chapter: Dewford's Old Rod chapter is stored, anchored outdoors, needs no role, and its scene writes the slot var")
{
    const struct ChapterDesc *desc = GetCountyChapter(DEWFORD, 0);
    const struct Scene *scene = GetScene(SCENE_DEWFORD_OLD_ROD);

    ASSUME(desc != NULL);
    EXPECT_EQ(desc->backend, CHAPTER_STORED);
    EXPECT_EQ(desc->numStages, 2);
    EXPECT_EQ(GetChapterNeededRoles(desc), 0);
    EXPECT_EQ(desc->producedKey.type, CHAPTER_KEY_NONE);
    EXPECT_EQ(scene->stageVar, SCENE_STAGE(DEWFORD, 0));
}

TEST("Chapter: Route 118's Good Rod chapter is stored, anchored outdoors, needs no role, reports the Good Rod get, and its scene writes the slot var")
{
    const struct ChapterDesc *desc = GetCountyChapter(WEATHER, 0);
    const struct Scene *scene = GetScene(SCENE_ROUTE118_GOOD_ROD);

    ASSUME(desc != NULL);
    EXPECT_EQ(desc->backend, CHAPTER_STORED);
    EXPECT_EQ(desc->numStages, 2);
    EXPECT_EQ(GetChapterNeededRoles(desc), 0);
    EXPECT_EQ(desc->producedKey.type, CHAPTER_KEY_NONE);
    EXPECT(desc->producesMainGet);
    EXPECT_EQ(desc->mainGet.kind, PLOT_KIND_UTILITY);
    EXPECT_EQ(desc->mainGet.ordinal, PLOT_UTILITY_GOOD_ROD);
    EXPECT_EQ(scene->stageVar, SCENE_STAGE(WEATHER, 0));
}

TEST("Chapter: Route 110's rival and Itemfinder chapters are stored in slots 0 and 1, report their gets, and their scenes write their own slot vars")
{
    const struct ChapterDesc *rival = GetCountyChapter(ROUTE110_COUNTY, 0);
    const struct ChapterDesc *finder = GetCountyChapter(ROUTE110_COUNTY, 1);

    ASSUME(rival != NULL && finder != NULL);
    EXPECT_EQ(rival->backend, CHAPTER_STORED);
    EXPECT_EQ(rival->numStages, 2);
    EXPECT(rival->producesMainGet);
    EXPECT_EQ(rival->mainGet.kind, PLOT_KIND_RIVAL);
    EXPECT_EQ(rival->mainGet.ordinal, 3);
    EXPECT_EQ(finder->backend, CHAPTER_STORED);
    EXPECT_EQ(finder->numStages, 2);
    EXPECT(finder->producesMainGet);
    EXPECT_EQ(finder->mainGet.kind, PLOT_KIND_UTILITY);
    EXPECT_EQ(finder->mainGet.ordinal, PLOT_UTILITY_ITEMFINDER);
    EXPECT_EQ(GetChapterNeededRoles(rival), 0);
    EXPECT_EQ(GetChapterNeededRoles(finder), 0);
    EXPECT_EQ(GetScene(SCENE_ROUTE110_RIVAL_MAY)->stageVar, SCENE_STAGE(ROUTE110_COUNTY, 0));
    EXPECT_EQ(GetScene(SCENE_ROUTE110_RIVAL_BRENDAN)->stageVar, SCENE_STAGE(ROUTE110_COUNTY, 0));
    EXPECT_EQ(GetScene(SCENE_ROUTE110_ITEMFINDER_MAY)->stageVar, SCENE_STAGE(ROUTE110_COUNTY, 1));
    EXPECT_EQ(GetScene(SCENE_ROUTE110_ITEMFINDER_BRENDAN)->stageVar, SCENE_STAGE(ROUTE110_COUNTY, 1));
}

TEST("Chapter: Route 119's rival chapter is stored in the Weather Institute county's slot 1 with three stages, and all four of its scenes write that slot var")
{
    const struct ChapterDesc *desc = GetCountyChapter(WEATHER, 1);
    static const u16 sceneIds[] = {SCENE_ROUTE119_RIVAL_MAY, SCENE_ROUTE119_RIVAL_BRENDAN, SCENE_ROUTE119_FLY_MAY, SCENE_ROUTE119_FLY_BRENDAN};
    u32 i;

    ASSUME(desc != NULL);
    EXPECT_EQ(desc->backend, CHAPTER_STORED);
    EXPECT_EQ(desc->numStages, 3);
    EXPECT(desc->producesMainGet);
    EXPECT_EQ(desc->mainGet.kind, PLOT_KIND_RIVAL);
    EXPECT_EQ(desc->mainGet.ordinal, 4);
    EXPECT_EQ(GetCountySlotVar(WEATHER, 1) == GetCountySlotVar(WEATHER, 0), FALSE);
    for (i = 0; i < ARRAY_COUNT(sceneIds); i++)
        EXPECT_EQ(GetScene(sceneIds[i])->stageVar, SCENE_STAGE(WEATHER, 1));
}

TEST("Chapter: Lilycove's rival chapter is stored in slot 2 with two stages, reports the fifth rival get and all four of its scenes write that slot var")
{
    const struct ChapterDesc *desc = GetCountyChapter(LILYCOVE, 2);
    static const u16 sceneIds[] = {SCENE_LILYCOVE_RIVAL_MAY, SCENE_LILYCOVE_RIVAL_MAY_AGAIN, SCENE_LILYCOVE_RIVAL_BRENDAN, SCENE_LILYCOVE_RIVAL_BRENDAN_AGAIN};
    u32 i;

    ASSUME(desc != NULL);
    EXPECT_EQ(desc->backend, CHAPTER_STORED);
    EXPECT_EQ(desc->numStages, 2);
    EXPECT(desc->producesMainGet);
    EXPECT_EQ(desc->mainGet.kind, PLOT_KIND_RIVAL);
    EXPECT_EQ(desc->mainGet.ordinal, 5);
    EXPECT_EQ(GetChapterNeededRoles(desc), 0);
    for (i = 0; i < ARRAY_COUNT(sceneIds); i++)
        EXPECT_EQ(GetScene(sceneIds[i])->stageVar, SCENE_STAGE(LILYCOVE, 2));
}

TEST("Chapter: Rival 2's chapter is stored in Rustboro's slot 2 with two stages and reports the second rival get")
{
    const struct ChapterDesc *desc = GetCountyChapter(RUSTBORO, 2);

    ASSUME(desc != NULL);
    EXPECT_EQ(desc->backend, CHAPTER_STORED);
    EXPECT_EQ(desc->numStages, 2);
    EXPECT(desc->producesMainGet);
    EXPECT_EQ(desc->mainGet.kind, PLOT_KIND_RIVAL);
    EXPECT_EQ(desc->mainGet.ordinal, 2);
    EXPECT_EQ(GetChapterNeededRoles(desc), 0);
}

TEST("Chapter: Mauville's Rock Smash chapter is stored, house-anchored, produces no key, and keeps its stage in Mauville's slice")
{
    const struct ChapterDesc *desc = GetCountyChapter(MAUVILLE, 0);

    ASSUME(desc != NULL);
    EXPECT_EQ(desc->backend, CHAPTER_STORED);
    EXPECT_EQ(desc->numStages, 2);
    EXPECT_EQ(GetChapterNeededRoles(desc), BUILDING_HOUSE);
    EXPECT_EQ(desc->producedKey.type, CHAPTER_KEY_NONE);
    EXPECT_EQ(GetCountySlotStage(MAUVILLE, 0), 0);
    SetCountySlotStage(MAUVILLE, 0, 1);
    EXPECT_EQ(VarGet(GetCountySlotVar(MAUVILLE, 0)), 1);
    EXPECT_EQ(GetCountySlotStage(MOSSDEEP, 0), 0);
}

TEST("Chapter: Mauville's Coin Case chapter is slot 1, house-anchored, reports the Coin Case plot get and keeps its stage apart from the Rock Smash slot")
{
    const struct ChapterDesc *desc = GetCountyChapter(MAUVILLE, 1);

    ASSUME(desc != NULL);
    EXPECT_EQ(desc->backend, CHAPTER_STORED);
    EXPECT_EQ(desc->numStages, 2);
    EXPECT_EQ(GetChapterNeededRoles(desc), BUILDING_HOUSE);
    EXPECT(desc->producesMainGet);
    EXPECT_EQ(desc->mainGet.kind, PLOT_KIND_UTILITY);
    EXPECT_EQ(desc->mainGet.ordinal, PLOT_UTILITY_COIN_CASE);
    SetCountySlotStage(MAUVILLE, 1, 1);
    EXPECT_EQ(GetCountySlotStage(MAUVILLE, 1), 1);
    EXPECT_EQ(GetCountySlotStage(MAUVILLE, 0), 0);
}

TEST("Chapter: Harbor Mail is a shop-item key produced by Slateport's Mart chapter and required by the Coin Case trade")
{
    const struct ChapterDesc *mail = GetCountyChapter(SLATEPORT, 0);
    const struct ChapterDesc *coinCase = GetCountyChapter(MAUVILLE, 1);

    ASSUME(mail != NULL);
    EXPECT_EQ(mail->producedKey.type, CHAPTER_KEY_SHOP_ITEM);
    EXPECT_EQ(GetChapterNeededRoles(mail), BUILDING_MART);
    EXPECT_EQ(coinCase->requiredKeys[0].type, CHAPTER_KEY_SHOP_ITEM);
    EXPECT_EQ(ResolveChapterKey(&coinCase->requiredKeys[0], MAUVILLE), ResolveChapterKey(&mail->producedKey, SLATEPORT));
    EXPECT(ResolveChapterKey(&mail->producedKey, SLATEPORT) != GetPlotItemMcGuffinId(PLOT_KIND_UTILITY, PLOT_UTILITY_COIN_CASE));
}

TEST("Chapter: The Coin Case chapter is unavailable until Slateport's Mart chapter reaches its bought stage")
{
    u32 numHosts;
    const struct ChapterHost *hosts = GetChapterHosts(&numHosts);

    EXPECT(!IsChapterAvailable(GetCountyChapter(MAUVILLE, 1), MAUVILLE, hosts, numHosts));
    SetCountySlotStage(SLATEPORT, 0, 1);
    EXPECT(IsChapterAvailable(GetCountyChapter(MAUVILLE, 1), MAUVILLE, hosts, numHosts));
}

TEST("Chapter: Mauville's Rydel chapter is slot 2, house-anchored, reports the Bicycle plot get and keeps its stage apart from slots 0 and 1")
{
    const struct ChapterDesc *desc = GetCountyChapter(MAUVILLE, 2);

    ASSUME(desc != NULL);
    EXPECT_EQ(desc->backend, CHAPTER_STORED);
    EXPECT_EQ(desc->numStages, 2);
    EXPECT_EQ(GetChapterNeededRoles(desc), BUILDING_HOUSE);
    EXPECT(desc->producesMainGet);
    EXPECT_EQ(desc->mainGet.ordinal, PLOT_UTILITY_BICYCLE);
    SetCountySlotStage(MAUVILLE, 2, 1);
    EXPECT_EQ(GetCountySlotStage(MAUVILLE, 2), 1);
    EXPECT_EQ(GetCountySlotStage(MAUVILLE, 0), 0);
    EXPECT_EQ(GetCountySlotStage(MAUVILLE, 1), 0);
}

static const struct ChapterDesc *FindMauvilleStatelessAdapter(u32 skip)
{
    u32 numHosts, i;
    const struct ChapterHost *hosts = GetChapterHosts(&numHosts);

    for (i = 0; i < numHosts; i++)
    {
        if (hosts[i].county == MAUVILLE && hosts[i].slot == CHAPTER_SLOT_STATELESS && hosts[i].desc->backend == CHAPTER_ADAPTER
         && hosts[i].desc->numStages == 4 && skip-- == 0)
            return hosts[i].desc;
    }
    return NULL;
}

TEST("Chapter: New Mauville is a stateless adapter whose stage follows Wattson's key, the generator and his thank-you")
{
    const struct ChapterDesc *desc = FindMauvilleStatelessAdapter(0);

    ASSUME(desc != NULL);
    EXPECT_EQ(GetChapterStage(MAUVILLE, CHAPTER_SLOT_STATELESS, desc), 0);
    FlagSet(FLAG_GOT_BASEMENT_KEY_FROM_WATTSON);
    EXPECT_EQ(GetChapterStage(MAUVILLE, CHAPTER_SLOT_STATELESS, desc), 1);
    VarSet(VAR_NEW_MAUVILLE_STATE, 1); // the door opened, the generator still runs
    EXPECT_EQ(GetChapterStage(MAUVILLE, CHAPTER_SLOT_STATELESS, desc), 1);
    VarSet(VAR_NEW_MAUVILLE_STATE, 2);
    EXPECT_EQ(GetChapterStage(MAUVILLE, CHAPTER_SLOT_STATELESS, desc), 2);
    FlagSet(FLAG_GOT_TM_THUNDERBOLT_FROM_WATTSON);
    EXPECT_EQ(GetChapterStage(MAUVILLE, CHAPTER_SLOT_STATELESS, desc), 3);
}

TEST("Chapter: New Mauville takes no slot var and anchors its key stage on the facility")
{
    const struct ChapterDesc *desc = FindMauvilleStatelessAdapter(0);

    ASSUME(desc != NULL);
    EXPECT_EQ(desc->stages[1].anchor.locationKind, ROUTE_LOCATION_FACILITY);
    EXPECT(!desc->producesMainGet);
    EXPECT_EQ(GetCountySlotStage(MAUVILLE, 0), 0);
    EXPECT_EQ(GetCountySlotStage(MAUVILLE, 1), 0);
    EXPECT_EQ(GetCountySlotStage(MAUVILLE, 2), 0);
}

static u32 FindNewMauvilleIndex(void)
{
    u32 i;

    for (i = 0; GetCountyStatelessChapter(MAUVILLE, i) != NULL; i++)
    {
        if (GetCountyStatelessChapter(MAUVILLE, i) == FindMauvilleStatelessAdapter(0))
            return i;
    }
    return 0xFF;
}

TEST("Chapter: A stateless chapter's hints follow its stage and have both lists in every stage")
{
    u32 index = FindNewMauvilleIndex(), stage, i;
    const struct ChapterDesc *desc = FindMauvilleStatelessAdapter(0);
    const u8 *far, *close;

    ASSUME(index != 0xFF);
    EXPECT(GetCountyStatelessChapter(MAUVILLE, 99) == NULL);
    far = GetCountyStatelessHint(MAUVILLE, index, CHAPTER_FAR, 0);
    close = GetCountyStatelessHint(MAUVILLE, index, CHAPTER_CLOSE, 0);
    EXPECT(far != NULL && close != NULL && far != close);
    for (stage = 0; stage < desc->numStages; stage++)
    {
        u32 covering = 0;
        const struct ChapterHint *hint = FindChapterHint(desc, stage);

        for (i = 0; i < desc->numHints; i++)
            covering += DoesChapterHintCover(&desc->hints[i], stage);
        EXPECT_EQ(covering, 1);
        ASSUME(hint != NULL);
        EXPECT(hint->far.count > 0 && hint->close.count > 0);
    }
    FlagSet(FLAG_GOT_BASEMENT_KEY_FROM_WATTSON);
    EXPECT(GetCountyStatelessHint(MAUVILLE, index, CHAPTER_CLOSE, 0) != close);
}

static const struct ChapterDesc *FindStatelessAdapterByGet(u32 kind, u32 ordinal)
{
    u32 numHosts, i;
    const struct ChapterHost *hosts = GetChapterHosts(&numHosts);

    for (i = 0; i < numHosts; i++)
    {
        if (hosts[i].slot == CHAPTER_SLOT_STATELESS && hosts[i].desc->backend == CHAPTER_ADAPTER
         && DoesChapterProduce(hosts[i].desc, kind, ordinal))
            return hosts[i].desc;
    }
    return NULL;
}

static const struct { u8 kind; u8 ordinal; } sHintedPlotGets[] =
{
    {PLOT_KIND_SCRIPTED, PLOT_SCRIPTED_STARTER},
    {PLOT_KIND_UTILITY, PLOT_UTILITY_POKENAV},
    {PLOT_KIND_SCRIPTED, PLOT_SCRIPTED_SHIPYARD},
    {PLOT_KIND_SCRIPTED, PLOT_SCRIPTED_ELITE_FOUR},
    {PLOT_KIND_SCRIPTED, PLOT_SCRIPTED_CHAMPION},
    {PLOT_KIND_GYM, ELITE_FOUR_ORDINAL},
};

TEST("Chapter: The Starter, Pokenav, Shipyard, Elite Four, Champion and League lobby adapters hint before and after their get and anchor both stages")
{
    u32 i;

    for (i = 0; i < ARRAY_COUNT(sHintedPlotGets); i++)
    {
        const struct ChapterDesc *desc = FindStatelessAdapterByGet(sHintedPlotGets[i].kind, sHintedPlotGets[i].ordinal);
        const u8 *before, *after;
        u32 last, stage;

        ASSUME(desc != NULL);
        ASSUME(desc->numStages >= 2);
        last = desc->numStages - 1;
        before = GetChapterHint(desc, 0, CHAPTER_FAR, 0);
        after = GetChapterHint(desc, last, CHAPTER_FAR, 0);
        EXPECT(before != NULL && after != NULL && before != after);
        before = GetChapterHint(desc, 0, CHAPTER_CLOSE, 0);
        after = GetChapterHint(desc, last, CHAPTER_CLOSE, 0);
        EXPECT(before != NULL && after != NULL && before != after);
        for (stage = 0; stage <= last; stage++)
        {
            EXPECT(GetChapterHint(desc, stage, CHAPTER_FAR, 0) != NULL && GetChapterHint(desc, stage, CHAPTER_CLOSE, 0) != NULL);
            EXPECT(desc->stages[stage].anchor.role != 0 || desc->stages[stage].anchor.locationKind != BEAT_LOCATION_NONE
                || desc->stages[stage].anchor.kind == BEAT_ANCHOR_ROUTE_ROUTE);
        }
    }
}

TEST("Chapter: The Elite Four adapter advances one stage per flag and the Shipyard adapter skips the refusal when goods are delivered")
{
    const struct ChapterDesc *elite = FindStatelessAdapterByGet(PLOT_KIND_SCRIPTED, PLOT_SCRIPTED_ELITE_FOUR);
    const struct ChapterDesc *yard = FindStatelessAdapterByGet(PLOT_KIND_SCRIPTED, PLOT_SCRIPTED_SHIPYARD);

    ASSUME(elite != NULL && yard != NULL);
    EXPECT_EQ(GetChapterStage(LITTLEROOT, CHAPTER_SLOT_STATELESS, elite), 0);
    FlagSet(FLAG_ENTERED_ELITE_FOUR);
    EXPECT_EQ(GetChapterStage(LITTLEROOT, CHAPTER_SLOT_STATELESS, elite), 1);
    FlagSet(FLAG_DEFEATED_ELITE_4_SIDNEY);
    EXPECT_EQ(GetChapterStage(LITTLEROOT, CHAPTER_SLOT_STATELESS, elite), 2);
    FlagSet(FLAG_DEFEATED_ELITE_4_DRAKE);
    EXPECT_EQ(GetChapterStage(LITTLEROOT, CHAPTER_SLOT_STATELESS, elite), 5);

    EXPECT_EQ(GetChapterStage(LITTLEROOT, CHAPTER_SLOT_STATELESS, yard), 0);
    FlagSet(FLAG_DOCK_REJECTED_DEVON_GOODS);
    EXPECT_EQ(GetChapterStage(LITTLEROOT, CHAPTER_SLOT_STATELESS, yard), 1);
    FlagSet(FLAG_DELIVERED_DEVON_GOODS);
    EXPECT_EQ(GetChapterStage(LITTLEROOT, CHAPTER_SLOT_STATELESS, yard), 2);
    FlagClear(FLAG_DOCK_REJECTED_DEVON_GOODS);
    EXPECT_EQ(GetChapterStage(LITTLEROOT, CHAPTER_SLOT_STATELESS, yard), 2);
}

// The stage constants scripts use are the descriptor's stage indices.
static bool32 SaveBirchStageNamesMatch(void)
{
    return SAVE_BIRCH_TWIN_GUARDS == 0 && SAVE_BIRCH_SEE_RIVAL == SAVE_BIRCH_NUM_STAGES - 1
        && SAVE_BIRCH_BIRCH_IN_GRASS == SAVE_BIRCH_TWIN_ASKS + 1 && SAVE_BIRCH_AWAIT_BAG == SAVE_BIRCH_BIRCH_IN_GRASS + 1
        && SAVE_BIRCH_STARTER_TAKEN == SAVE_BIRCH_AWAIT_BAG + 1 && SAVE_BIRCH_SEE_RIVAL == SAVE_BIRCH_STARTER_TAKEN + 1;
}

TEST("Chapter: Save Birch is stored in Littleroot's slot 1 and its scenes run on the slot var")
{
    const struct ChapterDesc *desc = GetCountyChapter(LITTLEROOT, 1);

    ASSUME(desc != NULL);
    EXPECT_EQ(desc->backend, CHAPTER_STORED);
    EXPECT_EQ(desc->numStages, SAVE_BIRCH_NUM_STAGES);
    EXPECT_EQ(desc->producedKey.type, CHAPTER_KEY_NONE);
    EXPECT_EQ(GetScene(SCENE_ROUTE101_RESCUE)->stageVar, SCENE_STAGE(LITTLEROOT, 1));
    EXPECT_EQ(GetScene(SCENE_LITTLEROOT_NEED_POKEMON)->stageVar, SCENE_STAGE(LITTLEROOT, 1));
    EXPECT(SaveBirchStageNamesMatch());
}

TEST("Chapter: Moving in is stored in Littleroot's slot 0 and its scenes run on the slot var")
{
    const struct ChapterDesc *desc = GetCountyChapter(LITTLEROOT, 0);

    ASSUME(desc != NULL);
    EXPECT_EQ(desc->backend, CHAPTER_STORED);
    EXPECT_EQ(desc->numStages, MOVING_IN_NUM_STAGES);
    EXPECT_EQ(desc->producedKey.type, CHAPTER_KEY_NONE);
    EXPECT_EQ(GetCountySlotVar(LITTLEROOT, 0), VAR_MOVING_IN_BEAT);
    EXPECT_EQ(GetScene(SCENE_LITTLEROOT_ARRIVE_MALE)->stageVar, SCENE_STAGE(LITTLEROOT, 0));
    EXPECT_EQ(GetScene(SCENE_LITTLEROOT_ARRIVE_FEMALE)->stageVar, SCENE_STAGE(LITTLEROOT, 0));
    EXPECT_EQ(MOVING_IN_TRUCK_MALE, 1);
    EXPECT_EQ(MOVING_IN_TRUCK_FEMALE, 2);
    EXPECT_EQ(MOVING_IN_MEET_RIVAL, MOVING_IN_NUM_STAGES - 1);
}

TEST("Chapter: Rival and Pokedex is stored in Oldale's slot 0 and the Route 103 rival scenes run on the slot var")
{
    const struct ChapterDesc *desc = GetCountyChapter(OLDALE, 0);

    ASSUME(desc != NULL);
    EXPECT_EQ(desc->backend, CHAPTER_STORED);
    EXPECT_EQ(desc->numStages, RIVAL_DEX_NUM_STAGES);
    EXPECT_EQ(GetCountySlotVar(OLDALE, 0), VAR_RIVAL_DEX_BEAT);
    EXPECT_EQ(GetScene(SCENE_ROUTE103_RIVAL_MAY)->stageVar, SCENE_STAGE(OLDALE, 0));
    EXPECT_EQ(GetScene(SCENE_ROUTE103_RIVAL_BRENDAN)->stageVar, SCENE_STAGE(OLDALE, 0));
    EXPECT_EQ(RIVAL_DEX_SHOES_GIVEN, RIVAL_DEX_NUM_STAGES - 1);
    EXPECT_EQ(RIVAL_DEX_RIVAL_AT_LAB, RIVAL_DEX_RIVAL_IN_OLDALE + 1);
    EXPECT_EQ(RIVAL_DEX_POKEDEX_GIVEN, RIVAL_DEX_RIVAL_AT_LAB + 1);
}

// Each get chapter is the get's own predicate (emerald_plot_gets.h) as a two-stage adapter, so the two tables must agree.
TEST("Chapter: Every adapter chapter that supplies a main-plot get uses the get's own done predicate")
{
    u32 i, numHosts, checked = 0;
    const struct ChapterHost *hosts = GetChapterHosts(&numHosts);

    for (i = 0; i < numHosts; i++)
    {
        const struct ChapterDesc *desc = hosts[i].desc;
        struct PlotSlot slot;
        struct PlotGetDesc gets[2];

        if (desc->backend != CHAPTER_ADAPTER || !desc->producesMainGet || desc->numStages != 2)
            continue;
        slot = GetVanillaPlotSlot(desc->mainGet.kind, desc->mainGet.ordinal);
        GetPlotSlotGetDescs(&slot, gets);
        EXPECT_EQ(desc->stages[1].done.op, gets[0].done.op);
        EXPECT_EQ(desc->stages[1].done.a, gets[0].done.a);
        EXPECT_EQ(desc->stages[1].done.b, gets[0].done.b);
        checked++;
    }
    EXPECT_GT(checked, 10);
}

TEST("Chapter: Rival 1, the Pokedex, the Poke Balls and the Running Shoes are views of the Oldale chapter's stages")
{
    const struct ChapterDesc *desc = GetCountyChapter(OLDALE, 0);
    u32 stage;
    bool32 rival, dex, balls, shoes;

    ASSUME(desc != NULL);
    for (stage = 0; stage < RIVAL_DEX_NUM_STAGES; stage++)
    {
        struct PlotSlot slot;
        struct PlotGetDesc get[2];

        VarSet(VAR_RIVAL_DEX_BEAT, stage);
        slot = GetVanillaPlotSlot(PLOT_KIND_RIVAL, 1);
        GetPlotSlotGetDescs(&slot, get);
        rival = IsStoryPredicateTrue(&get[0].done);
        slot = GetVanillaPlotSlot(PLOT_KIND_SCRIPTED, PLOT_SCRIPTED_POKEDEX);
        GetPlotSlotGetDescs(&slot, get);
        dex = IsStoryPredicateTrue(&get[0].done);
        slot = GetVanillaPlotSlot(PLOT_KIND_SCRIPTED, PLOT_SCRIPTED_POKE_BALLS);
        GetPlotSlotGetDescs(&slot, get);
        balls = IsStoryPredicateTrue(&get[0].done);
        slot = GetVanillaPlotSlot(PLOT_KIND_SCRIPTED, PLOT_SCRIPTED_RUNNING_SHOES);
        GetPlotSlotGetDescs(&slot, get);
        shoes = IsStoryPredicateTrue(&get[0].done);
        EXPECT_EQ(rival, stage >= RIVAL_DEX_RIVAL_IN_OLDALE);
        EXPECT_EQ(dex, stage >= RIVAL_DEX_POKEDEX_GIVEN);
        EXPECT_EQ(balls, stage >= RIVAL_DEX_POKEDEX_GIVEN);
        EXPECT_EQ(shoes, stage >= RIVAL_DEX_SHOES_GIVEN);
    }
}

TEST("Chapter: The first chapter of the chain is available from the start")
{
    EXPECT(IsCountySlotAvailable(LITTLEROOT, 0));
}

TEST("Chapter: With no declared key, a chapter waits for the previous slot of its county to complete")
{
    EXPECT(!IsCountySlotAvailable(LITTLEROOT, 1));
    SetCountySlotStage(LITTLEROOT, 0, MOVING_IN_NUM_STAGES - 2);
    EXPECT(!IsCountySlotAvailable(LITTLEROOT, 1));
    SetCountySlotStage(LITTLEROOT, 0, MOVING_IN_NUM_STAGES - 1);
    EXPECT(IsCountySlotAvailable(LITTLEROOT, 1));
}

TEST("Chapter: Rydel's Cycles does not wait for Rock Smash or the Coin Case, only for Mauville to be reached")
{
    u32 numHosts;
    const struct ChapterHost *hosts = GetChapterHosts(&numHosts);

    EXPECT(!IsChainPredecessorComplete(hosts, numHosts, MAUVILLE, 2, 4 * 7 - 1));
    EXPECT(IsChainPredecessorComplete(hosts, numHosts, MAUVILLE, 2, 4 * 7));
    EXPECT(IsChainPredecessorComplete(hosts, numHosts, MAUVILLE, 1, 4 * 7));
    EXPECT_EQ(GetCountySlotStage(MAUVILLE, 0), 0);
    EXPECT_EQ(GetCountySlotStage(MAUVILLE, 1), 0);
}

TEST("Chapter: The Coin Case still waits for Harbor Mail through its key though it has no subplot predecessor")
{
    u32 numHosts;
    const struct ChapterHost *hosts = GetChapterHosts(&numHosts);

    EXPECT(IsChainPredecessorComplete(hosts, numHosts, MAUVILLE, 1, 4 * 7));
    EXPECT(!IsChapterAvailable(GetCountyChapter(MAUVILLE, 1), MAUVILLE, hosts, numHosts));
}

TEST("Chapter: Chapters of different subplots in one county are unordered, those of one subplot are ordered")
{
    u32 numSubplots;
    const struct SubplotHost *subplots = GetSubplotHosts(&numSubplots);

    EXPECT(GetSubplotPredecessor(subplots, numSubplots, LITTLEROOT, GetCountyChapter(LITTLEROOT, 1)) == GetCountyChapter(LITTLEROOT, 0));
    EXPECT(GetSubplotPredecessor(subplots, numSubplots, LITTLEROOT, GetCountyChapter(LITTLEROOT, 0)) == NULL);
    EXPECT(GetSubplotPredecessor(subplots, numSubplots, MAUVILLE, GetCountyChapter(MAUVILLE, 1)) == NULL);
    EXPECT(GetSubplotPredecessor(subplots, numSubplots, MAUVILLE, GetCountyChapter(MAUVILLE, 2)) == NULL);
    EXPECT(GetSubplotPredecessor(subplots, numSubplots, RUSTBORO, GetCountyChapter(RUSTBORO, 1)) == NULL);
    EXPECT(GetSubplotPredecessor(subplots, numSubplots, LILYCOVE, GetCountyChapter(LILYCOVE, 1)) == NULL);
}

TEST("Chapter: A county's first chapter waits for the main-story-index to reach the county")
{
    u32 numHosts;
    const struct ChapterHost *hosts = GetChapterHosts(&numHosts);

    EXPECT(!IsChainPredecessorComplete(hosts, numHosts, OLDALE, 0, 3));
    EXPECT(IsChainPredecessorComplete(hosts, numHosts, OLDALE, 0, 4));
    EXPECT(IsChainPredecessorComplete(hosts, numHosts, OLDALE, 0, 7));
    EXPECT(IsChainPredecessorComplete(hosts, numHosts, LITTLEROOT, 0, 0));
    EXPECT(!IsCountySlotAvailable(OLDALE, 0));
}

TEST("Chapter: Placement needs every role of the descriptor, its anchors and its cutscene buildings")
{
    struct ChapterStage stages[] = {{.anchor = {BEAT_ANCHOR_NPC, ROUTE_ID_NONE, ROUTE_ID_NONE, BEAT_LOCATION_NONE, BUILDING_GYM}, .extraRoles = BUILDING_HOUSE}};
    struct ChapterDesc desc = {.backend = CHAPTER_STORED, .numStages = 1, .requiredRoles = BUILDING_POKEMON_CENTER, .stages = stages};

    EXPECT_EQ(GetChapterNeededRoles(&desc), BUILDING_POKEMON_CENTER | BUILDING_GYM | BUILDING_HOUSE);
    EXPECT_EQ(CheckChapterPlacement(&desc, RUSTBORO, 0, NULL, 0), CHAPTER_PLACEMENT_OK);
    EXPECT_EQ(CheckChapterPlacement(&desc, LILYCOVE, 0, NULL, 0), CHAPTER_PLACEMENT_MISSING_ROLE);
    EXPECT_EQ(CheckChapterPlacement(&desc, COUNTY_ID(0, 0), 0, NULL, 0), CHAPTER_PLACEMENT_OK); // a cityless county builds on its routes
}

TEST("Chapter: A chapter giving a main-plot get must come after the gym its badges wait for")
{
    struct ChapterDesc desc = {.backend = CHAPTER_STORED, .numStages = 1, .stages = sGatedStages, .producesMainGet = TRUE,
                               .mainGet = {PLOT_KIND_GYM, 1}, .minBadges = 4};

    EXPECT_EQ(CheckChapterPlacement(&desc, RUSTBORO, 0, NULL, 0), CHAPTER_PLACEMENT_BEFORE_BADGES);
    desc.mainGet.ordinal = 8;
    EXPECT_EQ(CheckChapterPlacement(&desc, RUSTBORO, 0, NULL, 0), CHAPTER_PLACEMENT_OK);
    desc.minBadges = 0;
    desc.mainGet.ordinal = 1;
    EXPECT_EQ(CheckChapterPlacement(&desc, RUSTBORO, 0, NULL, 0), CHAPTER_PLACEMENT_OK);
}

TEST("Chapter: A host-pool key resolves against the host, so a moved chapter moves its key")
{
    u32 inRustboro = ResolveChapterKey(&sGatedDesc.producedKey, RUSTBORO);
    u32 inLilycove = ResolveChapterKey(&sGatedDesc.producedKey, LILYCOVE);

    EXPECT_EQ(inRustboro, GetCountyMcGuffinId(RUSTBORO, 1));
    EXPECT_EQ(inLilycove, GetCountyMcGuffinId(LILYCOVE, 1));
    EXPECT(inRustboro != inLilycove);
    EXPECT_EQ(ResolveChapterKey(&sCrossConsumerDesc.requiredKeys[0], RUSTBORO), GetPlotItemMcGuffinId(PLOT_KIND_GYM, 4));
    EXPECT_EQ(ResolveChapterKey(&sCrossConsumerDesc.requiredKeys[1], RUSTBORO), MCGUFFIN_ID_NONE);
}

TEST("Chapter: A key is produced once the producer reaches its production stage, not before")
{
    // Mauville slot 0 is hosted, so its mask exists (stages are stored through the real host table's mask).
    // The mask is as wide as the hosted chapter's stages need, so the producing stage is 1 to fit any hosted chapter.
    struct ChapterDesc desc = sGatedDesc;
    struct ChapterHost hosts[] = {{MAUVILLE, 0, &desc}};
    u32 key = GetCountyMcGuffinId(MAUVILLE, 1);

    desc.producedKeyStage = 1;
    EXPECT(!IsMcGuffinProducedBy(hosts, 1, key));
    SetChapterStage(MAUVILLE, 0, &desc, 1);
    EXPECT(IsMcGuffinProducedBy(hosts, 1, key));
    EXPECT(!IsMcGuffinProducedBy(hosts, 1, GetCountyMcGuffinId(LILYCOVE, 1)));
}

TEST("Chapter: A consumer waits for a producer in another host")
{
    struct ChapterHost hosts[] = {{MAUVILLE, 0, &sItemProducerDesc}, {RUSTBORO, 1, &sCrossConsumerDesc}};

    EXPECT(!IsChapterAvailable(&sCrossConsumerDesc, RUSTBORO, hosts, 2));
    SetChapterStage(MAUVILLE, 0, &sItemProducerDesc, 1);
    EXPECT(IsChapterAvailable(&sCrossConsumerDesc, RUSTBORO, hosts, 2));
}

TEST("Chapter: A key consumed must have a producer in an earlier county, or an earlier main-story slot")
{
    struct ChapterHost producer[] = {{RUSTBORO, 0, &sItemProducerDesc}};

    EXPECT_EQ(CheckChapterPlacement(&sCrossConsumerDesc, LILYCOVE, 0, producer, 1), CHAPTER_PLACEMENT_OK);
    EXPECT_EQ(CheckChapterPlacement(&sCrossConsumerDesc, RUSTBORO, 1, producer, 1), CHAPTER_PLACEMENT_OK);
    EXPECT_EQ(CheckChapterPlacement(&sCrossConsumerDesc, RUSTBORO, 0, producer, 1), CHAPTER_PLACEMENT_KEY_NOT_EARLIER);
    EXPECT_EQ(CheckChapterPlacement(&sCrossConsumerDesc, PETALBURG, 0, NULL, 0), CHAPTER_PLACEMENT_KEY_NOT_EARLIER);
    producer[0].county = LILYCOVE;
    EXPECT_EQ(CheckChapterPlacement(&sCrossConsumerDesc, RUSTBORO, 0, producer, 1), CHAPTER_PLACEMENT_KEY_NOT_EARLIER);
}

TEST("Chapter: A host-pool key consumed in the same host needs the producer's main-story slot to be earlier")
{
    struct ChapterHost producer[] = {{RUSTBORO, 0, &sGatedDesc, 0}};

    EXPECT_EQ(CheckChapterPlacement(&sConsumerDesc, RUSTBORO, 1, producer, 1), CHAPTER_PLACEMENT_OK);
    EXPECT_EQ(CheckChapterPlacement(&sConsumerDesc, RUSTBORO, 0, producer, 1), CHAPTER_PLACEMENT_KEY_NOT_EARLIER);
    EXPECT_EQ(CheckChapterPlacement(&sConsumerDesc, LILYCOVE, 0, producer, 1), CHAPTER_PLACEMENT_KEY_NOT_EARLIER);
}

TEST("Chapter: Storage order is not story order: a producer in a lower chapter slot but a later main-story slot is not earlier")
{
    struct ChapterHost producer[] = {{RUSTBORO, 0, &sGatedDesc, 2}};

    EXPECT_EQ(CheckChapterPlacement(&sConsumerDesc, RUSTBORO, 1, producer, 1), CHAPTER_PLACEMENT_KEY_NOT_EARLIER);
    EXPECT_EQ(CheckChapterPlacement(&sConsumerDesc, RUSTBORO, 3, producer, 1), CHAPTER_PLACEMENT_OK);
}

TEST("Chapter: An off-chain producer is earlier than any main-story slot of its county, but not than an off-chain consumer there")
{
    struct ChapterHost producer[] = {{RUSTBORO, 0, &sGatedDesc, CHAPTER_MAIN_SLOT_OFF_CHAIN}};

    EXPECT_EQ(CheckChapterPlacement(&sConsumerDesc, RUSTBORO, 0, producer, 1), CHAPTER_PLACEMENT_OK);
    EXPECT_EQ(CheckChapterPlacement(&sConsumerDesc, RUSTBORO, CHAPTER_MAIN_SLOT_OFF_CHAIN, producer, 1), CHAPTER_PLACEMENT_KEY_NOT_EARLIER);
}

static const struct ChapterDesc sLateWaiterDesc =
{
    .backend = CHAPTER_STORED,
    .numStages = ARRAY_COUNT(sPlainStages),
    .requiredKeys = {{CHAPTER_KEY_ITEM, PLOT_KIND_GYM, 4}},
    .waitsForLaterKey = TRUE,
    .stages = sPlainStages,
};

TEST("Chapter: An off-chain consumer that waitsForLaterKey may wait for a later producer, no other chapter may, and a tie is refused")
{
    struct ChapterHost callbackProducer[] = {{PETALBURG, CHAPTER_SLOT_STATELESS, &sItemProducerDesc, CHAPTER_MAIN_SLOT_CALLBACK(11)}};
    struct ChapterHost sameCounty[] = {{LILYCOVE, 0, &sItemProducerDesc, 2}};

    // MAUVILLE is county story index 7 and the callback is block 11, step 1: the producer is later than any Mauville slot.
    EXPECT_EQ(CheckChapterPlacement(&sLateWaiterDesc, MAUVILLE, CHAPTER_MAIN_SLOT_OFF_CHAIN, callbackProducer, 1), CHAPTER_PLACEMENT_OK);
    EXPECT_EQ(CheckChapterPlacement(&sLateWaiterDesc, MAUVILLE, 2, callbackProducer, 1), CHAPTER_PLACEMENT_KEY_NOT_EARLIER);
    EXPECT_EQ(CheckChapterPlacement(&sCrossConsumerDesc, MAUVILLE, CHAPTER_MAIN_SLOT_OFF_CHAIN, callbackProducer, 1), CHAPTER_PLACEMENT_KEY_NOT_EARLIER);
    EXPECT_EQ(CheckChapterPlacement(&sLateWaiterDesc, MAUVILLE, CHAPTER_MAIN_SLOT_OFF_CHAIN, NULL, 0), CHAPTER_PLACEMENT_KEY_NOT_EARLIER);
    // An off-chain waiter in the producer's own county and position still ties.
    sameCounty[0].county = MAUVILLE;
    sameCounty[0].mainSlot = CHAPTER_MAIN_SLOT_OFF_CHAIN;
    EXPECT_EQ(CheckChapterPlacement(&sLateWaiterDesc, MAUVILLE, CHAPTER_MAIN_SLOT_OFF_CHAIN, sameCounty, 1), CHAPTER_PLACEMENT_KEY_NOT_EARLIER);
}

TEST("Chapter: A callback host sorts at its callback block's step 1, not in the host county's own block")
{
    // Petalburg is story index 2; its callback is block 11. A consumer in Lavaridge's (block 11) slot 0 or 1 is not later than it.
    struct ChapterHost callbackProducer[] = {{PETALBURG, CHAPTER_SLOT_STATELESS, &sItemProducerDesc, CHAPTER_MAIN_SLOT_CALLBACK(11)}};
    u32 lavaridge = COUNTY_ID(2, 0);

    EXPECT_EQ((u32)GetCountyHeader(lavaridge)->storyIdx, 11);
    EXPECT_EQ(CheckChapterPlacement(&sCrossConsumerDesc, lavaridge, 1, callbackProducer, 1), CHAPTER_PLACEMENT_KEY_NOT_EARLIER);
    EXPECT_EQ(CheckChapterPlacement(&sCrossConsumerDesc, lavaridge, 2, callbackProducer, 1), CHAPTER_PLACEMENT_OK);
    EXPECT_EQ(CheckChapterPlacement(&sCrossConsumerDesc, COUNTY_ID(3, 1), 0, callbackProducer, 1), CHAPTER_PLACEMENT_OK);
}

TEST("Chapter: Petalburg's gym is a three-stage stateless adapter producing the gym 5 key at the Surf hand-over")
{
    u32 numHosts, i;
    const struct ChapterHost *hosts = GetChapterHosts(&numHosts);
    const struct ChapterDesc *desc = NULL;
    u32 key = GetPlotItemMcGuffinId(PLOT_KIND_GYM, 5);

    for (i = 0; i < numHosts; i++)
    {
        if (hosts[i].county == PETALBURG && hosts[i].desc->producesMainGet)
            desc = hosts[i].desc;
    }
    ASSUME(desc != NULL);
    EXPECT_EQ(desc->numStages, 3);
    EXPECT_EQ(ResolveChapterKey(&desc->producedKey, PETALBURG), key);
    EXPECT_EQ(GetChapterStage(PETALBURG, CHAPTER_SLOT_STATELESS, desc), 0);
    EXPECT(!IsMcGuffinProducedBy(hosts, numHosts, key));
    FlagSet(FLAG_BADGE05_GET);
    EXPECT_EQ(GetChapterStage(PETALBURG, CHAPTER_SLOT_STATELESS, desc), 1);
    EXPECT(!IsMcGuffinProducedBy(hosts, numHosts, key));
    FlagSet(FLAG_RECEIVED_HM_SURF);
    EXPECT_EQ(GetChapterStage(PETALBURG, CHAPTER_SLOT_STATELESS, desc), 2);
    EXPECT(IsMcGuffinProducedBy(hosts, numHosts, key));
    FlagClear(FLAG_BADGE05_GET);
    FlagClear(FLAG_RECEIVED_HM_SURF);
}

TEST("Chapter: New Mauville needs Norman's badge and Surf, not a badge count")
{
    const struct ChapterDesc *desc = FindMauvilleStatelessAdapter(0);
    u32 numHosts;
    const struct ChapterHost *hosts = GetChapterHosts(&numHosts);

    ASSUME(desc != NULL);
    EXPECT_EQ(desc->minBadges, 0);
    EXPECT(desc->waitsForLaterKey);
    EXPECT_EQ(desc->requiredKeys[0].type, CHAPTER_KEY_ITEM);
    EXPECT_EQ(ResolveChapterKey(&desc->requiredKeys[0], MAUVILLE), GetPlotItemMcGuffinId(PLOT_KIND_GYM, 5));
    SetBadges(0xFF); // every badge, Norman's included: the key still waits for the Surf hand-over
    EXPECT(!IsChapterAvailable(desc, MAUVILLE, hosts, numHosts));
    FlagSet(FLAG_RECEIVED_HM_SURF);
    EXPECT(IsChapterAvailable(desc, MAUVILLE, hosts, numHosts));
    FlagClear(FLAG_RECEIVED_HM_SURF);
    SetBadges(0);
}

TEST("Chapter: Availability counts badges held in any order")
{
    EXPECT(!IsChapterAvailable(&sGatedDesc, RUSTBORO, NULL, 0));
    SetBadges(0x12);
    EXPECT_EQ(GetBadgeCount(), 2);
    EXPECT(!IsChapterAvailable(&sGatedDesc, RUSTBORO, NULL, 0));
    SetBadges(0x4B);
    EXPECT_EQ(GetBadgeCount(), 4);
    EXPECT(IsChapterAvailable(&sGatedDesc, RUSTBORO, NULL, 0));
}

TEST("Chapter: A hint's count is the badges still missing")
{
    SetBadges(0x12);
    EXPECT_EQ(GetChapterHintCount(&sGatedDesc, 0, GetBadgeCount()), 2);
    EXPECT_EQ(GetChapterHintCount(&sGatedDesc, 0, 6), 0);
    EXPECT_EQ(GetChapterHintCount(&sGatedDesc, 1, 0), 0);
}

TEST("Chapter: Buffering a hint count puts the number in STR_VAR_1 and leaves it alone for uncounted stages")
{
    SetBadges(0x12);
    gStringVar1[0] = EOS;
    BufferChapterHintCount(&sGatedDesc, 1);
    EXPECT_EQ(gStringVar1[0], EOS);
    BufferChapterHintCount(&sGatedDesc, 0);
    EXPECT_EQ(StringCompare(gStringVar1, COMPOUND_STRING("2")), 0);
}

TEST("Chapter: Derived stages come from the badge count with no stored counter")
{
    static const struct ChapterStage sStages[] =
    {
        {},
        {.done = {STORY_PRED_BADGES_GE, 1, 0}},
        {.done = {STORY_PRED_BADGES_GE, 4, 0}},
    };
    static const struct ChapterDesc sDesc = {.backend = CHAPTER_ADAPTER, .numStages = 3, .stages = sStages};

    EXPECT_EQ(GetChapterStage(0, 0, &sDesc), 0);
    SetBadges(0x20);
    EXPECT_EQ(GetChapterStage(0, 0, &sDesc), 1);
    SetBadges(0xF0);
    EXPECT_EQ(GetChapterStage(0, 0, &sDesc), 2);
}

TEST("Chapter: Two stages of one chapter anchor different buildings")
{
    EXPECT_EQ(sGatedDesc.stages[0].anchor.role, BUILDING_GYM);
    EXPECT_EQ(sGatedDesc.stages[1].anchor.role, BUILDING_MART);
    EXPECT_EQ(sGatedDesc.stages[1].anchor.kind, BEAT_ANCHOR_DOOR);
}

TEST("Chapter: A hint template expands {HINT_PLACE} to the anchor's building or location name")
{
    u8 out[96];
    static const u8 sTemplate[] = _("the {HINT_PLACE} in {HINT_CITY}");
    static const struct BeatAnchor mart = {BEAT_ANCHOR_NPC, ROUTE_ID_NONE, ROUTE_ID_NONE, BEAT_LOCATION_NONE, BUILDING_MART};
    static const struct BeatAnchor cave = {BEAT_ANCHOR_NPC, ROUTE_ID_NONE, ROUTE_ID_NONE, ROUTE_LOCATION_CAVE, 0};
    static const struct BeatAnchor outdoors = {BEAT_ANCHOR_NPC, ROUTE_ID_NONE, ROUTE_ID_NONE, BEAT_LOCATION_NONE, 0};
    u8 expect[96];

    ExpandHintTemplate(out, sTemplate, RUSTBORO, &mart);
    StringCopy(StringCopy(expect, COMPOUND_STRING("the MART in ")), GetCountyCityName(RUSTBORO));
    EXPECT_EQ(StringCompare(out, expect), 0);
    ExpandHintTemplate(out, sTemplate, RUSTBORO, &cave);
    EXPECT_EQ(StringCompareN(out, COMPOUND_STRING("the cave in "), 12), 0);
    ExpandHintTemplate(out, sTemplate, RUSTBORO, &outdoors);
    EXPECT_EQ(StringCompareN(out, COMPOUND_STRING("the spot in "), 12), 0);
}

TEST("Chapter: A pseudo-stage hint reads its anchor and count from the nearest real stage")
{
    EXPECT_EQ(ResolveHintStage(&sGatedDesc, CHAPTER_STAGE_BEFORE), 0);
    EXPECT_EQ(ResolveHintStage(&sGatedDesc, CHAPTER_STAGE_AFTER), ARRAY_COUNT(sGatedStages) - 1);
    EXPECT_EQ(ResolveHintStage(&sGatedDesc, 1), 1);
    EXPECT_EQ(GetChapterHintCount(&sGatedDesc, CHAPTER_STAGE_BEFORE, 0), 4);
    EXPECT_EQ(GetChapterHintCount(&sGatedDesc, CHAPTER_STAGE_AFTER, 0), 0);
    SetBadges(0x12);
    BufferChapterHintCount(&sGatedDesc, CHAPTER_STAGE_BEFORE);
    EXPECT_EQ(StringCompare(gStringVar1, COMPOUND_STRING("2")), 0);
}

static const struct ChapterActor sPseudoActors[] =
{
    {CHAPTER_STAGE_BEFORE, 0, 0x1, 1},
    {1, CHAPTER_STAGE_AFTER, 0x2, 2},
    {CHAPTER_STAGE_BEFORE, CHAPTER_STAGE_AFTER, 0, 3},
};

static const struct ChapterDesc sPseudoActorDesc =
{
    .backend = CHAPTER_ADAPTER,
    .numStages = 3,
    .numActors = ARRAY_COUNT(sPseudoActors),
    .actors = sPseudoActors,
};

TEST("Chapter: Actors with ranges ending at a pseudo-stage are listed for it")
{
    const struct ChapterActor *out[3];

    EXPECT_EQ(GetChapterActors(&sPseudoActorDesc, CHAPTER_STAGE_BEFORE, out, 3), 2);
    EXPECT_EQ(out[0]->gfxId, 1);
    EXPECT_EQ(out[1]->gfxId, 3);
    EXPECT_EQ(GetChapterActors(&sPseudoActorDesc, CHAPTER_STAGE_AFTER, out, 3), 2);
    EXPECT_EQ(out[0]->gfxId, 2);
    EXPECT_EQ(GetChapterActors(&sPseudoActorDesc, 0, out, 3), 2);
    EXPECT_EQ(GetChapterActors(&sPseudoActorDesc, 1, out, 3), 2);
}

// Script specials are not in a header; the scripts reach them through the special table.
extern void SetCurrentChapterStage(void);
extern void GetCurrentChapterStage(void);

TEST("Chapter: A shop-item key keeps the high byte of the item id, and different items get different keys")
{
    static const struct ChapterKeyRef big = SHOP_ITEM_KEY(0x1234);
    static const struct ChapterKeyRef mail = SHOP_ITEM_KEY(ITEM_HARBOR_MAIL);
    static const struct ChapterKeyRef other = SHOP_ITEM_KEY(ITEM_HARBOR_MAIL + 1);

    EXPECT_EQ(big.a, 0x34);
    EXPECT_EQ(big.b, 0x12);
    EXPECT_EQ(ResolveChapterKey(&big, MAUVILLE), GetShopItemMcGuffinId(0x1234));
    EXPECT_EQ(ResolveChapterKey(&mail, MAUVILLE), ResolveChapterKey(&mail, SLATEPORT));
    EXPECT(ResolveChapterKey(&mail, MAUVILLE) != ResolveChapterKey(&other, MAUVILLE));
}

TEST("Chapter: A shop-item key lies in the cross-county id range")
{
    EXPECT_GE(GetShopItemMcGuffinId(ITEM_HARBOR_MAIL), NUM_COUNTY_MCGUFFIN_IDS);
}

TEST("Chapter: Harbor Mail counts as produced only once the Slateport Mart chapter reaches its bought stage")
{
    u32 numHosts;
    const struct ChapterHost *hosts = GetChapterHosts(&numHosts);
    u32 key = GetShopItemMcGuffinId(ITEM_HARBOR_MAIL);

    EXPECT(!IsMcGuffinProducedBy(hosts, numHosts, key));
    SetCountySlotStage(SLATEPORT, 0, 1);
    EXPECT(IsMcGuffinProducedBy(hosts, numHosts, key));
}

TEST("Chapter: The Coin Case chapter's placement needs the Mart chapter among the hosts as the key's producer")
{
    u32 numHosts, i, numOthers = 0;
    const struct ChapterHost *hosts = GetChapterHosts(&numHosts);
    struct ChapterHost others[48];

    for (i = 0; i < numHosts; i++)
    {
        if (!(hosts[i].county == MAUVILLE && hosts[i].slot == 1))
            others[numOthers++] = hosts[i];
    }
    EXPECT_EQ(CheckChapterPlacement(GetCountyChapter(MAUVILLE, 1), MAUVILLE, CHAPTER_MAIN_SLOT_OFF_CHAIN, others, numOthers), CHAPTER_PLACEMENT_OK);
    for (i = 0, numOthers = 0; i < numHosts; i++)
    {
        if (hosts[i].county != SLATEPORT && !(hosts[i].county == MAUVILLE && hosts[i].slot == 1))
            others[numOthers++] = hosts[i];
    }
    EXPECT_EQ(CheckChapterPlacement(GetCountyChapter(MAUVILLE, 1), MAUVILLE, CHAPTER_MAIN_SLOT_OFF_CHAIN, others, numOthers), CHAPTER_PLACEMENT_KEY_NOT_EARLIER);
}

TEST("Chapter: Slateport's Mart hints follow the stage, and its far and close lists differ")
{
    const u8 *before = GetCountySlotHint(SLATEPORT, 0, CHAPTER_FAR, 0);
    const u8 *beforeClose = GetCountySlotHint(SLATEPORT, 0, CHAPTER_CLOSE, 0);
    const u8 *after;

    EXPECT(before != NULL && beforeClose != NULL && before != beforeClose);
    SetCountySlotStage(SLATEPORT, 0, 1);
    after = GetCountySlotHint(SLATEPORT, 0, CHAPTER_FAR, 0);
    EXPECT(after != NULL && after != before);
}

TEST("Chapter: Rydel's hints follow the stage")
{
    const u8 *before = GetCountySlotHint(MAUVILLE, 2, CHAPTER_CLOSE, 0);
    const u8 *after;

    EXPECT(before != NULL);
    SetCountySlotStage(MAUVILLE, 2, 1);
    after = GetCountySlotHint(MAUVILLE, 2, CHAPTER_CLOSE, 0);
    EXPECT(after != NULL && after != before);
}

TEST("Chapter: The script specials read and write the stage of the current map's county")
{
    gMapHeader.regionMapSectionId = MAPSEC_SLATEPORT_CITY;
    gSpecialVar_0x8004 = 0;
    gSpecialVar_0x8005 = 1;
    SetCurrentChapterStage();
    EXPECT_EQ(GetCountySlotStage(SLATEPORT, 0), 1);
    EXPECT_EQ(GetCountySlotStage(MAUVILLE, 0), 0);
    GetCurrentChapterStage();
    EXPECT_EQ(gSpecialVar_Result, 1);

    gMapHeader.regionMapSectionId = MAPSEC_MAUVILLE_CITY;
    gSpecialVar_0x8004 = 2;
    gSpecialVar_0x8005 = 1;
    SetCurrentChapterStage();
    EXPECT_EQ(GetCountySlotStage(MAUVILLE, 2), 1);
    EXPECT_EQ(GetCountySlotStage(SLATEPORT, 0), 1);
}

static const struct ChapterDesc *FindMauvilleProxy(const struct ChapterHost *hosts, u32 numHosts)
{
    u32 i;

    for (i = 0; i < numHosts; i++)
    {
        if (hosts[i].county == MAUVILLE && hosts[i].slot == CHAPTER_SLOT_STATELESS)
            return hosts[i].desc;
    }
    return NULL;
}

TEST("Chapter: Rydel's proxy has no state: it reads stage 1 exactly when the Bicycle chapter has produced its key")
{
    u32 numHosts;
    const struct ChapterHost *hosts = GetChapterHosts(&numHosts);
    const struct ChapterDesc *proxy = FindMauvilleProxy(hosts, numHosts);

    ASSUME(proxy != NULL);
    EXPECT_EQ(proxy->backend, CHAPTER_PROXY);
    EXPECT_EQ(GetChapterStage(MAUVILLE, CHAPTER_SLOT_STATELESS, proxy), 0);
    EXPECT(!IsChapterAvailable(proxy, MAUVILLE, hosts, numHosts));
    SetCountySlotStage(MAUVILLE, 2, 1);
    EXPECT_EQ(GetChapterStage(MAUVILLE, CHAPTER_SLOT_STATELESS, proxy), 1);
    EXPECT(IsChapterAvailable(proxy, MAUVILLE, hosts, numHosts));
}

TEST("Chapter: Rydel's proxy supplies the Bicycle's slot for planning but the news still points at the Bicycle chapter")
{
    u32 numHosts;
    const struct ChapterHost *hosts = GetChapterHosts(&numHosts);
    const struct ChapterDesc *proxy = FindMauvilleProxy(hosts, numHosts);
    const struct ChapterDesc *bike = GetCountyChapter(MAUVILLE, 2);
    struct PlotSlot proxyGet, bikeGet;

    ASSUME(proxy != NULL && bike != NULL);
    EXPECT(GetChapterSuppliedSlot(proxy, &proxyGet));
    EXPECT(GetChapterSuppliedSlot(bike, &bikeGet));
    EXPECT_EQ(proxyGet.block, bikeGet.block);
    EXPECT_EQ(proxyGet.step, bikeGet.step);
    EXPECT(!DoesChapterProduce(proxy, PLOT_KIND_UTILITY, PLOT_UTILITY_BICYCLE));
    EXPECT(DoesChapterProduce(bike, PLOT_KIND_UTILITY, PLOT_UTILITY_BICYCLE));
}

TEST("Chapter: Every Emerald proxy keeps the four soft-lock conditions, and its key has a producer before it")
{
    u32 numHosts, numSubplots, i, j, numOthers, proxies = 0;
    struct ChapterHost hosts[48], others[48];
    const struct SubplotHost *subplots = GetSubplotHosts(&numSubplots);

    numHosts = GetPlannedChapterHosts(hosts, ARRAY_COUNT(hosts));
    for (i = 0; i < numHosts; i++)
    {
        if (hosts[i].desc->backend != CHAPTER_PROXY)
            continue;
        proxies++;
        EXPECT_EQ(CheckProxySoftLock(&hosts[i], hosts, numHosts, subplots, numSubplots), PROXY_OK);
        for (j = 0, numOthers = 0; j < numHosts; j++)
        {
            if (hosts[j].desc != hosts[i].desc && hosts[j].desc != GetCountyChapter(hosts[i].county, 2))
                others[numOthers++] = hosts[j];
        }
        EXPECT_EQ(CheckProxySoftLock(&hosts[i], others, numOthers, subplots, numSubplots), PROXY_NO_SUBPLOT);
    }
    EXPECT_GE(proxies, 1);
}

static const struct ChapterStage sBlankStages[2] = {};

static const struct ChapterDesc sSoftLockSub =
{
    .backend = CHAPTER_STORED,
    .numStages = 2,
    .stages = sBlankStages,
    .producedKeyStage = 1,
    .producedKey = {CHAPTER_KEY_HOST_POOL, 5},
};

static const struct ChapterDesc sSoftLockProxy =
{
    .backend = CHAPTER_PROXY,
    .numStages = 2,
    .stages = sBlankStages,
    .requiredKeys = {{CHAPTER_KEY_HOST_POOL, 5}},
};

static const struct ChapterDesc sSoftLockEarlier = {.backend = CHAPTER_STORED, .numStages = 2, .stages = sBlankStages, .producedKeyStage = 1, .producedKey = {CHAPTER_KEY_HOST_POOL, 6}};

// Mauville hosting `sub` (off-chain, in a subplot of its own) and a proxy in main-story slot 2; `extra`, if set, is a third host.
static u32 CheckSoftLockFixture(const struct ChapterDesc *sub, bool32 subOptional, const struct ChapterDesc *proxy, const struct ChapterHost *extra)
{
    struct SubplotEntry entries[1] = {{sub, subOptional}};
    struct SubplotDesc subplot = {1, entries};
    struct SubplotHost subplotHost = {MAUVILLE, &subplot};
    struct ChapterHost hosts[3] = {{MAUVILLE, 0, sub, CHAPTER_MAIN_SLOT_OFF_CHAIN}, {MAUVILLE, CHAPTER_SLOT_STATELESS, proxy, 2}};
    u32 numHosts = 2;

    if (extra != NULL)
        hosts[numHosts++] = *extra;
    return CheckProxySoftLock(&hosts[1], hosts, numHosts, &subplotHost, 1);
}

TEST("Chapter: The soft-lock check passes a proxy whose subplot is off-chain, consumes nothing and needs no more badges")
{
    EXPECT_EQ(CheckSoftLockFixture(&sSoftLockSub, TRUE, &sSoftLockProxy, NULL), PROXY_OK);
}

TEST("Chapter: Soft-lock condition 2: a proxy that opens with fewer badges than its subplot needs is refused")
{
    struct ChapterDesc sub = sSoftLockSub, proxy = sSoftLockProxy;

    sub.minBadges = 3;
    proxy.minBadges = 2;
    EXPECT_EQ(CheckSoftLockFixture(&sub, TRUE, &proxy, NULL), PROXY_BADGES_BELOW_SUBPLOT);
    proxy.minBadges = 3;
    EXPECT_EQ(CheckSoftLockFixture(&sub, TRUE, &proxy, NULL), PROXY_OK);
}

TEST("Chapter: Soft-lock condition 4: a subplot with a required entry can wait behind the chain, so its proxy is refused")
{
    EXPECT_EQ(CheckSoftLockFixture(&sSoftLockSub, FALSE, &sSoftLockProxy, NULL), PROXY_SUBPLOT_ON_CHAIN);
}

TEST("Chapter: A proxy whose key has no producer, or whose producer reaches the key before its last stage, is refused")
{
    struct ChapterDesc sub = sSoftLockSub;

    EXPECT_EQ(CheckProxySoftLock(&(struct ChapterHost){MAUVILLE, CHAPTER_SLOT_STATELESS, &sSoftLockProxy, 2}, NULL, 0, NULL, 0), PROXY_NO_SUBPLOT);
    sub.producedKeyStage = 0;
    EXPECT_EQ(CheckSoftLockFixture(&sub, TRUE, &sSoftLockProxy, NULL), PROXY_NO_SUBPLOT);
}

TEST("Chapter: Soft-lock condition 1: a subplot consuming a key produced in the proxy's own slot or later is refused")
{
    struct ChapterDesc sub = sSoftLockSub;
    struct ChapterHost earlier = {MAUVILLE, 1, &sSoftLockEarlier, 2};

    sub.requiredKeys[0] = (struct ChapterKeyRef){CHAPTER_KEY_HOST_POOL, 6};
    EXPECT_EQ(CheckSoftLockFixture(&sub, TRUE, &sSoftLockProxy, &earlier), PROXY_SUBPLOT_NEEDS_LATER_KEY);
    earlier.mainSlot = 3;
    EXPECT_EQ(CheckSoftLockFixture(&sub, TRUE, &sSoftLockProxy, &earlier), PROXY_SUBPLOT_NEEDS_LATER_KEY);
}

TEST("Chapter: Soft-lock condition 3: a subplot whose own key producer is not before the subplot is refused, though it is before the proxy")
{
    struct ChapterDesc sub = sSoftLockSub;
    struct ChapterHost earlier = {MAUVILLE, 1, &sSoftLockEarlier, 0};

    sub.requiredKeys[0] = (struct ChapterKeyRef){CHAPTER_KEY_HOST_POOL, 6};
    EXPECT_EQ(CheckSoftLockFixture(&sub, TRUE, &sSoftLockProxy, &earlier), PROXY_SUBPLOT_MISPLACED);
}

#define LAVARIDGE COUNTY_ID(2, 0)

static bool32 StartsWith(const u8 *text, const u8 *prefix)
{
    return StringCompareN(text, prefix, StringLength(prefix)) == 0;
}

TEST("Chapter: A county with a city is referred to by its city, from anywhere, for any reason")
{
    u8 out[96];

    WriteCountyReferenceAt(out, LILYCOVE, RUSTBORO, FALSE, 0);
    EXPECT_EQ(StringCompare(out, GetCountyCityName(RUSTBORO)), 0);
    WriteCountyReferenceAt(out, RUSTBORO, RUSTBORO, TRUE, 20);
    EXPECT_EQ(StringCompare(out, GetCountyCityName(RUSTBORO)), 0);
}

TEST("Chapter: A cityless county is 'these parts' to its own NPCs and 'the area <dir> of here' to an adjacent county")
{
    u8 out[96];

    WriteCountyReferenceAt(out, WEATHER, WEATHER, FALSE, 0);
    EXPECT_EQ(StringCompare(out, COMPOUND_STRING("these parts")), 0);
    WriteCountyReferenceAt(out, LAVARIDGE, WEATHER, FALSE, 0);
    EXPECT_EQ(StringCompare(out, COMPOUND_STRING("the area east of here")), 0);
    WriteCountyReferenceAt(out, LAVARIDGE, WEATHER, TRUE, 20);
    EXPECT_EQ(StringCompare(out, COMPOUND_STRING("the area east of here")), 0);
}

TEST("Chapter: A callback hint, or a remote county the story is past, names the cityless county's landmark")
{
    u8 out[96];

    ASSUME(!AreCountiesAdjacent(RUSTBORO, WEATHER));
    WriteCountyReferenceAt(out, RUSTBORO, WEATHER, TRUE, 0);
    EXPECT(StartsWith(out, COMPOUND_STRING("the area near ")));
    WriteCountyReferenceAt(out, RUSTBORO, WEATHER, FALSE, 20);
    EXPECT(StartsWith(out, COMPOUND_STRING("the area near ")));
}

TEST("Chapter: A remote cityless county not yet passed is placed by direction from an earlier neighboring city")
{
    u8 out[96], expect[96];

    ASSUME(!AreCountiesAdjacent(RUSTBORO, WEATHER));
    ASSUME(GetCountyHeader(LAVARIDGE)->storyIdx < GetCountyHeader(WEATHER)->storyIdx);
    WriteCountyReferenceAt(out, RUSTBORO, WEATHER, FALSE, 0);
    StringAppend(StringCopy(expect, COMPOUND_STRING("the area east of ")), GetCountyCityName(LAVARIDGE));
    EXPECT_EQ(StringCompare(out, expect), 0);
}

TEST("Chapter: Direction names are diagonal when the offset is more diagonal than straight, and straight on the boundary")
{
    EXPECT_EQ(StringCompare(GetCountyDirectionName(COUNTY_ID(2, 2), COUNTY_ID(2, 1)), COMPOUND_STRING("north")), 0);
    EXPECT_EQ(StringCompare(GetCountyDirectionName(COUNTY_ID(2, 2), COUNTY_ID(3, 2)), COMPOUND_STRING("east")), 0);
    EXPECT_EQ(StringCompare(GetCountyDirectionName(COUNTY_ID(2, 2), COUNTY_ID(3, 1)), COMPOUND_STRING("northeast")), 0);
    EXPECT_EQ(StringCompare(GetCountyDirectionName(COUNTY_ID(2, 2), COUNTY_ID(1, 3)), COMPOUND_STRING("southwest")), 0);
    EXPECT_EQ(StringCompare(GetCountyDirectionName(COUNTY_ID(0, 0), COUNTY_ID(3, 3)), COMPOUND_STRING("southeast")), 0);
    // 1 over 3 is flatter than 22.5 degrees' worth of diagonal, so it stays straight; 2 over 3 is diagonal.
    EXPECT_EQ(StringCompare(GetCountyDirectionName(COUNTY_ID(0, 0), COUNTY_ID(3, 1)), COMPOUND_STRING("east")), 0);
    EXPECT_EQ(StringCompare(GetCountyDirectionName(COUNTY_ID(0, 0), COUNTY_ID(3, 2)), COMPOUND_STRING("southeast")), 0);
    EXPECT_EQ(StringCompare(GetCountyDirectionName(COUNTY_ID(3, 3), COUNTY_ID(0, 0)), COMPOUND_STRING("northwest")), 0);
}
