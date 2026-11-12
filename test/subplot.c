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
#include "region_city.h"
#include "subplot.h"
#include "string_util.h"

#define LILYCOVE COUNTY_ID(4, 0)
#define RUSTBORO COUNTY_ID(0, 1)
#define PETALBURG COUNTY_ID(0, 2)

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
static const struct SubplotStage sDevonStages[] =
{
    {},
    {.done = {STORY_PRED_FLAG, FLAG_DEVON_GOODS_STOLEN, 0}},
    {.done = {STORY_PRED_FLAG, FLAG_RECOVERED_DEVON_GOODS, 0}},
    {.done = {STORY_PRED_FLAG, FLAG_RETURNED_DEVON_GOODS, 0}},
};

static const struct SubplotHint sDevonHints[] =
{
    {0, 0, TEXT_LIST(sListA)},
    {1, 1, TEXT_LIST(sListB)},
    {2, 2, TEXT_LIST(sListC)},
    {3, 3, TEXT_LIST(sListD)},
};

static const struct SubplotDesc sDevonAdapter =
{
    .backend = SUBPLOT_ADAPTER,
    .numStages = ARRAY_COUNT(sDevonStages),
    .numHints = ARRAY_COUNT(sDevonHints),
    .stages = sDevonStages,
    .hints = sDevonHints,
};

// Ranges that end at the pseudo-stages; stage 2 and AFTER have empty lists, so they say generic text.
static const struct SubplotHint sFullHints[] =
{
    {SUBPLOT_STAGE_BEFORE, 0, TEXT_LIST(sListB)},
    {1, 1, TEXT_LIST(sListA), TEXT_LIST(sListCD)},
    {2, SUBPLOT_STAGE_AFTER},
};

static const struct SubplotStage sFullStages[] =
{
    {},
    {
        .numHinters = 2,
        .hinterRoles = 0x4,
    },
    {},
};

static const struct SubplotActor sActors[] =
{
    {0, 1, 0x2, 7},
    {1, 2, 0, 8},
    {2, 2, 0x1, 9},
};

static const struct SubplotDesc sFullDesc =
{
    .backend = SUBPLOT_ADAPTER,
    .numStages = ARRAY_COUNT(sFullStages),
    .numActors = ARRAY_COUNT(sActors),
    .numHints = ARRAY_COUNT(sFullHints),
    .stages = sFullStages,
    .hints = sFullHints,
    .actors = sActors,
};

TEST("Subplot: Rustboro and Lilycove host one subplot each, in slot 0")
{
    u32 county, slot;

    for (county = 0; county < NUM_COUNTIES; county++)
    {
        for (slot = 0; slot < SUBPLOTS_PER_COUNTY; slot++)
        {
            if ((county == LILYCOVE || county == RUSTBORO) && slot == 0)
                EXPECT(GetCountySubplot(county, slot) != NULL);
            else
                EXPECT(GetCountySubplot(county, slot) == NULL);
        }
    }
}

TEST("Subplot: Slot vars lie in the county's own slice after its chain var")
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
    const u8 *far0 = GetCountySlotHint(LILYCOVE, 0, SUBPLOT_FAR, 0);
    const u8 *close0 = GetCountySlotHint(LILYCOVE, 0, SUBPLOT_CLOSE, 0);

    SetCountySlotStage(LILYCOVE, 0, 1);
    EXPECT(far0 != NULL && close0 != NULL);
    EXPECT(GetCountySlotHint(LILYCOVE, 0, SUBPLOT_FAR, 0) != far0);
    EXPECT(GetCountySlotHint(LILYCOVE, 0, SUBPLOT_CLOSE, 0) != close0);
}

TEST("Subplot: The query returns the range's list for each distance and stage")
{
    EXPECT(GetSubplotHint(&sFullDesc, 0, SUBPLOT_FAR, 0) == sTextB);
    EXPECT(GetSubplotHint(&sFullDesc, 1, SUBPLOT_FAR, 0) == sTextA);
    EXPECT(GetSubplotHint(&sFullDesc, 0, SUBPLOT_CLOSE, 0) == NULL);
    EXPECT(GetSubplotHint(&sFullDesc, 1, SUBPLOT_CLOSE, 0) == sTextC);
    EXPECT(GetSubplotHint(&sFullDesc, 1, SUBPLOT_CLOSE, 1) == sTextD);
}

TEST("Subplot: A range may end at a pseudo-stage, which the query accepts")
{
    EXPECT(FindSubplotHint(&sFullDesc, SUBPLOT_STAGE_BEFORE) == &sFullHints[0]);
    EXPECT(FindSubplotHint(&sFullDesc, 0) == &sFullHints[0]);
    EXPECT(FindSubplotHint(&sFullDesc, 1) == &sFullHints[1]);
    EXPECT(FindSubplotHint(&sFullDesc, 2) == &sFullHints[2]);
    EXPECT(FindSubplotHint(&sFullDesc, SUBPLOT_STAGE_AFTER) == &sFullHints[2]);
    EXPECT(GetSubplotHint(&sFullDesc, SUBPLOT_STAGE_BEFORE, SUBPLOT_FAR, 0) == sTextB);
    EXPECT(GetSubplotHint(&sFullDesc, SUBPLOT_STAGE_AFTER, SUBPLOT_FAR, 0) == NULL);
}

TEST("Subplot: A stage no range covers has no hint")
{
    static const struct SubplotHint sGap[] = {{1, 1, TEXT_LIST(sListA)}};
    static const struct SubplotDesc sGapDesc = {.backend = SUBPLOT_ADAPTER, .numStages = 3, .numHints = 1, .hints = sGap};

    EXPECT(FindSubplotHint(&sGapDesc, 0) == NULL);
    EXPECT(FindSubplotHint(&sGapDesc, SUBPLOT_STAGE_BEFORE) == NULL);
    EXPECT(GetSubplotHint(&sGapDesc, 2, SUBPLOT_FAR, 0) == NULL);
    EXPECT(GetSubplotHint(&sGapDesc, 1, SUBPLOT_FAR, 0) == sTextA);
}

TEST("Subplot: A list entry is picked by selector, stably")
{
    EXPECT(GetSubplotHint(&sFullDesc, 0, SUBPLOT_FAR, 0) == sTextB);
    EXPECT(GetSubplotHint(&sFullDesc, 0, SUBPLOT_FAR, 1) == sTextC);
    EXPECT(GetSubplotHint(&sFullDesc, 0, SUBPLOT_FAR, 2) == sTextB);
    EXPECT(GetSubplotHint(&sFullDesc, 0, SUBPLOT_FAR, 1) == GetSubplotHint(&sFullDesc, 0, SUBPLOT_FAR, 1));
}

TEST("Subplot: Empty lists give NULL so the NPC says generic text")
{
    EXPECT(GetSubplotHint(&sFullDesc, 2, SUBPLOT_FAR, 5) == NULL);
    EXPECT(GetSubplotHint(&sFullDesc, 2, SUBPLOT_CLOSE, 5) == NULL);
}

TEST("Subplot: The county query uses the current stage")
{
    const struct SubplotDesc *desc = GetCountySubplot(LILYCOVE, 0);

    EXPECT(GetCountySlotHint(LILYCOVE, 0, SUBPLOT_CLOSE, 0) == GetSubplotHint(desc, 0, SUBPLOT_CLOSE, 0));
    SetCountySlotStage(LILYCOVE, 0, 1);
    EXPECT(GetCountySlotHint(LILYCOVE, 0, SUBPLOT_CLOSE, 0) == GetSubplotHint(desc, 1, SUBPLOT_CLOSE, 0));
    EXPECT(GetSubplotHint(desc, 0, SUBPLOT_CLOSE, 0) != GetSubplotHint(desc, 1, SUBPLOT_CLOSE, 0));
}

TEST("Subplot: Each ported subplot's hint ranges cover every stage once and have both lists")
{
    u32 county, slot, stage, i;

    for (county = 0; county < NUM_COUNTIES; county++)
    {
        for (slot = 0; slot < SUBPLOTS_PER_COUNTY; slot++)
        {
            const struct SubplotDesc *desc = GetCountySubplot(county, slot);

            if (desc == NULL)
                continue;
            for (stage = 0; stage < desc->numStages; stage++)
            {
                const struct SubplotHint *hint = FindSubplotHint(desc, stage);
                u32 covering = 0;

                for (i = 0; i < desc->numHints; i++)
                    covering += DoesSubplotHintCover(&desc->hints[i], stage);
                EXPECT_EQ(covering, 1);
                ASSUME(hint != NULL);
                EXPECT(hint->far.count > 0);
                EXPECT(hint->close.count > 0);
            }
        }
    }
}

TEST("Subplot: A subplot that produces a McGuffin gives a reward")
{
    u32 numHosts, i;
    const struct SubplotHost *hosts = GetSubplotHosts(&numHosts);

    for (i = 0; i < numHosts; i++)
    {
        if (hosts[i].desc->producedKey.type != SUBPLOT_KEY_NONE)
            EXPECT(hosts[i].desc->givesReward);
    }
}

TEST("Subplot: Hinter counts come from the stage")
{
    EXPECT_EQ(sFullDesc.stages[1].numHinters, 2);
    EXPECT_EQ(GetCountySubplot(LILYCOVE, 0)->stages[0].numHinters, 1);
}

TEST("Subplot: Actors are listed for the stages they cover")
{
    const struct SubplotActor *out[2];

    EXPECT_EQ(GetSubplotActors(&sFullDesc, 0, out, 2), 1);
    EXPECT_EQ(out[0]->gfxId, 7);
    EXPECT_EQ(GetSubplotActors(&sFullDesc, 1, out, 2), 2);
    EXPECT_EQ(out[0]->gfxId, 7);
    EXPECT_EQ(out[1]->gfxId, 8);
    EXPECT_EQ(GetSubplotActors(&sFullDesc, 2, out, 1), 2); // total, even past maxOut
    EXPECT_EQ(out[0]->gfxId, 8);
    EXPECT_EQ(GetSubplotActors(GetCountySubplot(LILYCOVE, 0), 0, out, 2), 0);
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

TEST("Subplot: The hint query treats adapter and stored subplots alike")
{
    FlagSet(FLAG_RECOVERED_DEVON_GOODS);
    EXPECT(GetSubplotHint(&sDevonAdapter, GetSubplotStage(0, 0, &sDevonAdapter), SUBPLOT_FAR, 0) == sTextC);
    EXPECT(GetSubplotHint(&sDevonAdapter, 3, SUBPLOT_FAR, 0) == sTextD);
}

TEST("Subplot: A county is found from its city's MAPSEC")
{
    EXPECT_EQ(GetCountyFromCityMapSec(MAPSEC_LILYCOVE_CITY), LILYCOVE);
}

TEST("Subplot: Ported subplots anchor every stage to a real place")
{
    u32 county, slot, stage;

    for (county = 0; county < NUM_COUNTIES; county++)
    {
        for (slot = 0; slot < SUBPLOTS_PER_COUNTY; slot++)
        {
            const struct SubplotDesc *desc = GetCountySubplot(county, slot);

            if (desc == NULL)
                continue;
            for (stage = 0; stage < desc->numStages; stage++)
            {
                const struct BeatAnchor *anchor = &desc->stages[stage].anchor;

                EXPECT(anchor->kind <= BEAT_ANCHOR_DOOR);
                EXPECT(anchor->locationKind == BEAT_LOCATION_NONE || anchor->locationKind < NUM_LOCATION_KINDS);
                EXPECT(anchor->routeId == ROUTE_ID_NONE || anchor->routeId < NUM_COUNTIES + MAX_SPECIAL_LOCATIONS_PER_COUNTY);
                EXPECT(anchor->role != 0 || anchor->locationKind != BEAT_LOCATION_NONE || anchor->routeId != ROUTE_ID_NONE);
            }
        }
    }
}

TEST("Subplot: The Lilycove Museum beat is the curator in the museum")
{
    const struct BeatAnchor *anchor = &GetCountySubplot(LILYCOVE, 0)->stages[0].anchor;

    EXPECT_EQ(anchor->kind, BEAT_ANCHOR_NPC);
    EXPECT_EQ(anchor->locationKind, TOWN_LOCATION_MUSEUM);
}

TEST("Subplot: A hinter's distance follows the anchor's city")
{
    const struct BeatAnchor *anchor = &GetCountySubplot(LILYCOVE, 0)->stages[0].anchor;
    u32 county;

    for (county = 0; county < NUM_COUNTIES; county++)
    {
        if (county == LILYCOVE)
            EXPECT_EQ(GetBeatDistance(LILYCOVE, anchor, county), SUBPLOT_CLOSE);
        else
            EXPECT_EQ(GetBeatDistance(LILYCOVE, anchor, county), SUBPLOT_FAR);
    }
}

TEST("Subplot: The anchor reference names the building, location, road or city")
{
    u8 buf[64];
    struct BeatAnchor door = {BEAT_ANCHOR_DOOR, ROUTE_ID_NONE, ROUTE_ID_NONE, BEAT_LOCATION_NONE, BUILDING_POKEMON_CENTER};
    struct BeatAnchor house = {BEAT_ANCHOR_DOOR, ROUTE_ID_NONE, ROUTE_ID_NONE, BEAT_LOCATION_NONE, BUILDING_HOUSE};
    struct BeatAnchor museum = GetCountySubplot(LILYCOVE, 0)->stages[0].anchor;
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

TEST("Subplot: A hint template expands {HINT_ANCHOR} to the anchor reference and copies the rest")
{
    u8 out[96];
    const struct BeatAnchor *anchor = &GetCountySubplot(LILYCOVE, 0)->stages[0].anchor;
    static const u8 sTemplate[] = _("Go {HINT_ANCHOR}, {PLAYER}!");

    ExpandHintTemplate(out, sTemplate, LILYCOVE, anchor);
    EXPECT_EQ(StringCompare(out, COMPOUND_STRING("Go near the MUSEUM, {PLAYER}!")), 0);
}

TEST("Subplot: A hint template expands {HINT_CITY} to the host county's city name")
{
    u8 out[96];
    u8 expect[96];
    static const u8 sTemplate[] = _("A man in {HINT_CITY} waits.");

    ExpandHintTemplate(out, sTemplate, RUSTBORO, &GetCountySubplot(RUSTBORO, 0)->stages[0].anchor);
    StringCopy(StringCopy(StringCopy(expect, COMPOUND_STRING("A man in ")), GetCountyCityName(RUSTBORO)), COMPOUND_STRING(" waits."));
    EXPECT_EQ(StringCompare(out, expect), 0);
}

TEST("Subplot: The Lilycove stage-0 close hint about the unopened hall names the museum through the template")
{
    u8 out[96];
    const struct SubplotDesc *desc = GetCountySubplot(LILYCOVE, 0);
    const u8 *text = GetSubplotHint(desc, 0, SUBPLOT_CLOSE, 1);

    ASSUME(text != NULL);
    ExpandHintTemplate(out, text, LILYCOVE, &desc->stages[1].anchor);
    EXPECT(StringLength(out) > StringLength(text) - 4);
    EXPECT(StringCompareN(out, COMPOUND_STRING("There's talk of a new exhibit hall\nnear the MUSEUM."), 40) == 0);
}

// A gym-4-gated subplot with a parameterized hint, a produced host-pool key and a two-building cutscene.
static const struct SubplotStage sGatedStages[] =
{
    {.hintCount = SUBPLOT_COUNT_BADGES_SHORT,
     .anchor = {BEAT_ANCHOR_NPC, ROUTE_ID_NONE, ROUTE_ID_NONE, BEAT_LOCATION_NONE, BUILDING_GYM}, .extraRoles = BUILDING_HOUSE},
    {.anchor = {BEAT_ANCHOR_DOOR, ROUTE_ID_NONE, ROUTE_ID_NONE, BEAT_LOCATION_NONE, BUILDING_MART}},
    {},
};

static const struct SubplotDesc sGatedDesc =
{
    .backend = SUBPLOT_STORED,
    .numStages = ARRAY_COUNT(sGatedStages),
    .requiredRoles = BUILDING_POKEMON_CENTER,
    .minBadges = 4,
    .producedKeyStage = 2,
    .producedKey = {SUBPLOT_KEY_HOST_POOL, 1, 0},
    .stages = sGatedStages,
};

// Stages with no anchor role, so any city with a Pokémon Center could host them.
static const struct SubplotStage sPlainStages[] =
{
    {},
    {},
};

static const struct SubplotDesc sConsumerDesc =
{
    .backend = SUBPLOT_STORED,
    .numStages = ARRAY_COUNT(sPlainStages),
    .requiredKeys = {{SUBPLOT_KEY_HOST_POOL, 1, 0}},
    .stages = sPlainStages,
};

static const struct SubplotDesc sCrossConsumerDesc =
{
    .backend = SUBPLOT_STORED,
    .numStages = ARRAY_COUNT(sPlainStages),
    .requiredKeys = {{SUBPLOT_KEY_ITEM, PLOT_KIND_GYM, 4}},
    .stages = sPlainStages,
};

static const struct SubplotDesc sItemProducerDesc =
{
    .backend = SUBPLOT_STORED,
    .numStages = ARRAY_COUNT(sPlainStages),
    .producedKeyStage = 1,
    .producedKey = {SUBPLOT_KEY_ITEM, PLOT_KIND_GYM, 4},
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

TEST("Subplot: Rustboro's Cutter subplot is stored, role-anchored and needs only a house")
{
    const struct SubplotDesc *desc = GetCountySubplot(RUSTBORO, 0);

    ASSUME(desc != NULL);
    EXPECT_EQ(desc->backend, SUBPLOT_STORED);
    EXPECT_EQ(desc->numStages, 2);
    EXPECT_EQ(desc->stages[0].anchor.role, BUILDING_HOUSE);
    EXPECT_EQ(GetSubplotNeededRoles(desc), BUILDING_HOUSE);
    EXPECT_EQ(desc->producedKey.type, SUBPLOT_KEY_NONE);
}

TEST("Subplot: The Cutter subplot keeps its stage in Rustboro's slice and not Lilycove's")
{
    EXPECT_EQ(GetCountySlotStage(RUSTBORO, 0), 0);
    SetCountySlotStage(RUSTBORO, 0, 1);
    EXPECT_EQ(GetCountySlotStage(RUSTBORO, 0), 1);
    EXPECT_EQ(VarGet(GetCountySlotVar(RUSTBORO, 0)), 1);
    EXPECT_EQ(GetCountySlotStage(LILYCOVE, 0), 0);
}

TEST("Subplot: The Cutter hints follow the stage and expand the house reference")
{
    u8 out[96];
    const struct SubplotDesc *desc = GetCountySubplot(RUSTBORO, 0);
    const u8 *before = GetSubplotHint(desc, 0, SUBPLOT_CLOSE, 1);

    ASSUME(before != NULL);
    ExpandHintTemplate(out, before, RUSTBORO, &desc->stages[1].anchor);
    EXPECT_EQ(StringCompareN(out, COMPOUND_STRING("People say the man near the house"), 33), 0);
    EXPECT(GetCountySlotHint(RUSTBORO, 0, SUBPLOT_FAR, 0) == desc->hints[0].far.texts[0]);
    SetCountySlotStage(RUSTBORO, 0, 1);
    EXPECT(GetCountySlotHint(RUSTBORO, 0, SUBPLOT_FAR, 0) == desc->hints[1].far.texts[0]);
}

TEST("Subplot: Every Emerald host passes the placement check")
{
    u32 numHosts, i;
    const struct SubplotHost *hosts = GetSubplotHosts(&numHosts);

    EXPECT_EQ(numHosts, 2);
    for (i = 0; i < numHosts; i++)
    {
        struct SubplotHost others[1] = {hosts[1 - i]};

        EXPECT_EQ(CheckSubplotPlacement(hosts[i].desc, hosts[i].county, hosts[i].slot, others, 1), SUBPLOT_PLACEMENT_OK);
    }
}

TEST("Subplot: Placement needs every role of the descriptor, its anchors and its cutscene buildings")
{
    struct SubplotStage stages[] = {{.anchor = {BEAT_ANCHOR_NPC, ROUTE_ID_NONE, ROUTE_ID_NONE, BEAT_LOCATION_NONE, BUILDING_GYM}, .extraRoles = BUILDING_HOUSE}};
    struct SubplotDesc desc = {.backend = SUBPLOT_STORED, .numStages = 1, .requiredRoles = BUILDING_POKEMON_CENTER, .stages = stages};

    EXPECT_EQ(GetSubplotNeededRoles(&desc), BUILDING_POKEMON_CENTER | BUILDING_GYM | BUILDING_HOUSE);
    EXPECT_EQ(CheckSubplotPlacement(&desc, RUSTBORO, 0, NULL, 0), SUBPLOT_PLACEMENT_OK);
    EXPECT_EQ(CheckSubplotPlacement(&desc, LILYCOVE, 0, NULL, 0), SUBPLOT_PLACEMENT_MISSING_ROLE);
    EXPECT_EQ(CheckSubplotPlacement(&desc, COUNTY_ID(0, 0), 0, NULL, 0), SUBPLOT_PLACEMENT_NO_CITY);
}

TEST("Subplot: A subplot giving a main-plot get must come after the gym its badges wait for")
{
    struct SubplotDesc desc = {.backend = SUBPLOT_STORED, .numStages = 1, .stages = sGatedStages, .producesMainGet = TRUE,
                               .mainGet = {PLOT_KIND_GYM, 1}, .minBadges = 4};

    EXPECT_EQ(CheckSubplotPlacement(&desc, RUSTBORO, 0, NULL, 0), SUBPLOT_PLACEMENT_BEFORE_BADGES);
    desc.mainGet.ordinal = 8;
    EXPECT_EQ(CheckSubplotPlacement(&desc, RUSTBORO, 0, NULL, 0), SUBPLOT_PLACEMENT_OK);
    desc.minBadges = 0;
    desc.mainGet.ordinal = 1;
    EXPECT_EQ(CheckSubplotPlacement(&desc, RUSTBORO, 0, NULL, 0), SUBPLOT_PLACEMENT_OK);
}

TEST("Subplot: A host-pool key resolves against the host, so a moved subplot moves its key")
{
    u32 inRustboro = ResolveSubplotKey(&sGatedDesc.producedKey, RUSTBORO);
    u32 inLilycove = ResolveSubplotKey(&sGatedDesc.producedKey, LILYCOVE);

    EXPECT_EQ(inRustboro, GetCountyMcGuffinId(RUSTBORO, 1));
    EXPECT_EQ(inLilycove, GetCountyMcGuffinId(LILYCOVE, 1));
    EXPECT(inRustboro != inLilycove);
    EXPECT_EQ(ResolveSubplotKey(&sCrossConsumerDesc.requiredKeys[0], RUSTBORO), GetPlotItemMcGuffinId(PLOT_KIND_GYM, 4));
    EXPECT_EQ(ResolveSubplotKey(&sCrossConsumerDesc.requiredKeys[1], RUSTBORO), MCGUFFIN_ID_NONE);
}

TEST("Subplot: A key is produced once the producer reaches its production stage, not before")
{
    struct SubplotHost hosts[] = {{RUSTBORO, 0, &sGatedDesc}};
    u32 key = GetCountyMcGuffinId(RUSTBORO, 1);

    EXPECT(!IsMcGuffinProducedBy(hosts, 1, key));
    SetSubplotStage(RUSTBORO, 0, &sGatedDesc, 1);
    EXPECT(!IsMcGuffinProducedBy(hosts, 1, key));
    SetSubplotStage(RUSTBORO, 0, &sGatedDesc, 2);
    EXPECT(IsMcGuffinProducedBy(hosts, 1, key));
    EXPECT(!IsMcGuffinProducedBy(hosts, 1, GetCountyMcGuffinId(LILYCOVE, 1)));
}

TEST("Subplot: A consumer waits for a producer in another host")
{
    struct SubplotHost hosts[] = {{PETALBURG, 1, &sItemProducerDesc}, {RUSTBORO, 1, &sCrossConsumerDesc}};

    EXPECT(!IsSubplotAvailable(&sCrossConsumerDesc, RUSTBORO, hosts, 2));
    SetSubplotStage(PETALBURG, 1, &sItemProducerDesc, 1);
    EXPECT(IsSubplotAvailable(&sCrossConsumerDesc, RUSTBORO, hosts, 2));
}

TEST("Subplot: A key consumed must have a producer in an earlier county, or an earlier slot")
{
    struct SubplotHost producer[] = {{RUSTBORO, 0, &sItemProducerDesc}};

    EXPECT_EQ(CheckSubplotPlacement(&sCrossConsumerDesc, LILYCOVE, 0, producer, 1), SUBPLOT_PLACEMENT_OK);
    EXPECT_EQ(CheckSubplotPlacement(&sCrossConsumerDesc, RUSTBORO, 1, producer, 1), SUBPLOT_PLACEMENT_OK);
    EXPECT_EQ(CheckSubplotPlacement(&sCrossConsumerDesc, RUSTBORO, 0, producer, 1), SUBPLOT_PLACEMENT_KEY_NOT_EARLIER);
    EXPECT_EQ(CheckSubplotPlacement(&sCrossConsumerDesc, PETALBURG, 0, NULL, 0), SUBPLOT_PLACEMENT_KEY_NOT_EARLIER);
    producer[0].county = LILYCOVE;
    EXPECT_EQ(CheckSubplotPlacement(&sCrossConsumerDesc, RUSTBORO, 0, producer, 1), SUBPLOT_PLACEMENT_KEY_NOT_EARLIER);
}

TEST("Subplot: A host-pool key consumed in the same host needs the producer's slot to be earlier")
{
    struct SubplotHost producer[] = {{RUSTBORO, 0, &sGatedDesc}};

    EXPECT_EQ(CheckSubplotPlacement(&sConsumerDesc, RUSTBORO, 1, producer, 1), SUBPLOT_PLACEMENT_OK);
    EXPECT_EQ(CheckSubplotPlacement(&sConsumerDesc, RUSTBORO, 0, producer, 1), SUBPLOT_PLACEMENT_KEY_NOT_EARLIER);
    EXPECT_EQ(CheckSubplotPlacement(&sConsumerDesc, LILYCOVE, 0, producer, 1), SUBPLOT_PLACEMENT_KEY_NOT_EARLIER);
}

TEST("Subplot: Availability counts badges held in any order")
{
    EXPECT(!IsSubplotAvailable(&sGatedDesc, RUSTBORO, NULL, 0));
    SetBadges(0x12);
    EXPECT_EQ(GetBadgeCount(), 2);
    EXPECT(!IsSubplotAvailable(&sGatedDesc, RUSTBORO, NULL, 0));
    SetBadges(0x4B);
    EXPECT_EQ(GetBadgeCount(), 4);
    EXPECT(IsSubplotAvailable(&sGatedDesc, RUSTBORO, NULL, 0));
}

TEST("Subplot: A hint's count is the badges still missing")
{
    SetBadges(0x12);
    EXPECT_EQ(GetSubplotHintCount(&sGatedDesc, 0, GetBadgeCount()), 2);
    EXPECT_EQ(GetSubplotHintCount(&sGatedDesc, 0, 6), 0);
    EXPECT_EQ(GetSubplotHintCount(&sGatedDesc, 1, 0), 0);
}

TEST("Subplot: Buffering a hint count puts the number in STR_VAR_1 and leaves it alone for uncounted stages")
{
    SetBadges(0x12);
    gStringVar1[0] = EOS;
    BufferSubplotHintCount(&sGatedDesc, 1);
    EXPECT_EQ(gStringVar1[0], EOS);
    BufferSubplotHintCount(&sGatedDesc, 0);
    EXPECT_EQ(StringCompare(gStringVar1, COMPOUND_STRING("2")), 0);
}

TEST("Subplot: Derived stages come from the badge count with no stored counter")
{
    static const struct SubplotStage sStages[] =
    {
        {},
        {.done = {STORY_PRED_BADGES_GE, 1, 0}},
        {.done = {STORY_PRED_BADGES_GE, 4, 0}},
    };
    static const struct SubplotDesc sDesc = {.backend = SUBPLOT_ADAPTER, .numStages = 3, .stages = sStages};

    EXPECT_EQ(GetSubplotStage(0, 0, &sDesc), 0);
    SetBadges(0x20);
    EXPECT_EQ(GetSubplotStage(0, 0, &sDesc), 1);
    SetBadges(0xF0);
    EXPECT_EQ(GetSubplotStage(0, 0, &sDesc), 2);
}

TEST("Subplot: Two stages of one subplot anchor different buildings")
{
    EXPECT_EQ(sGatedDesc.stages[0].anchor.role, BUILDING_GYM);
    EXPECT_EQ(sGatedDesc.stages[1].anchor.role, BUILDING_MART);
    EXPECT_EQ(sGatedDesc.stages[1].anchor.kind, BEAT_ANCHOR_DOOR);
}

TEST("Subplot: A hint template expands {HINT_PLACE} to the anchor's building or location name")
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

TEST("Subplot: A pseudo-stage hint reads its anchor and count from the nearest real stage")
{
    EXPECT_EQ(ResolveHintStage(&sGatedDesc, SUBPLOT_STAGE_BEFORE), 0);
    EXPECT_EQ(ResolveHintStage(&sGatedDesc, SUBPLOT_STAGE_AFTER), ARRAY_COUNT(sGatedStages) - 1);
    EXPECT_EQ(ResolveHintStage(&sGatedDesc, 1), 1);
    EXPECT_EQ(GetSubplotHintCount(&sGatedDesc, SUBPLOT_STAGE_BEFORE, 0), 4);
    EXPECT_EQ(GetSubplotHintCount(&sGatedDesc, SUBPLOT_STAGE_AFTER, 0), 0);
    SetBadges(0x12);
    BufferSubplotHintCount(&sGatedDesc, SUBPLOT_STAGE_BEFORE);
    EXPECT_EQ(StringCompare(gStringVar1, COMPOUND_STRING("2")), 0);
}

static const struct SubplotActor sPseudoActors[] =
{
    {SUBPLOT_STAGE_BEFORE, 0, 0x1, 1},
    {1, SUBPLOT_STAGE_AFTER, 0x2, 2},
    {SUBPLOT_STAGE_BEFORE, SUBPLOT_STAGE_AFTER, 0, 3},
};

static const struct SubplotDesc sPseudoActorDesc =
{
    .backend = SUBPLOT_ADAPTER,
    .numStages = 3,
    .numActors = ARRAY_COUNT(sPseudoActors),
    .actors = sPseudoActors,
};

TEST("Subplot: Actors with ranges ending at a pseudo-stage are listed for it")
{
    const struct SubplotActor *out[3];

    EXPECT_EQ(GetSubplotActors(&sPseudoActorDesc, SUBPLOT_STAGE_BEFORE, out, 3), 2);
    EXPECT_EQ(out[0]->gfxId, 1);
    EXPECT_EQ(out[1]->gfxId, 3);
    EXPECT_EQ(GetSubplotActors(&sPseudoActorDesc, SUBPLOT_STAGE_AFTER, out, 3), 2);
    EXPECT_EQ(out[0]->gfxId, 2);
    EXPECT_EQ(GetSubplotActors(&sPseudoActorDesc, 0, out, 3), 2);
    EXPECT_EQ(GetSubplotActors(&sPseudoActorDesc, 1, out, 3), 2);
}
