#include "global.h"
#include "test/test.h"
#include "region_city.h"
#include "region_county.h"
#include "region_plot.h"

#define SURF_BLOCK 12
#define ELITE      20

// Vanilla ordinals start at 1 for these kinds, 0 for the rest.
static u32 FirstOrdinal(u32 kind)
{
    return (kind == PLOT_KIND_GYM || kind == PLOT_KIND_RIVAL || kind == PLOT_KIND_WALLY) ? 1 : 0;
}

static bool32 SlotInRange(struct PlotSlot slot)
{
    struct PlotRange range = GetPlotRange(slot.kind, slot.ordinal, SURF_BLOCK, ELITE);

    return slot.block >= range.lo && slot.block <= range.hi;
}

static bool32 SlotInMask(struct PlotSlot slot)
{
    u32 mask = GetPlotHostMask(slot.kind, slot.ordinal, ELITE);

    // Arc and scripted hosts are authored (callback hosts included), so only their block is masked.
    return (mask & (1u << slot.block))
        && (slot.kind == PLOT_KIND_ARC || slot.kind == PLOT_KIND_SCRIPTED || (mask & (1u << slot.host)));
}

TEST("RegionPlot: Generic gym and rival windows match the Emerald.md table for n = 12, E = 20")
{
    static const u8 expected[][2] = {{1, 3}, {4, 6}, {7, 9}, {10, 11}, {12, 12}, {13, 15}, {16, 17}, {18, 19}, {20, 20}};
    u32 i;
    struct PlotRange r;

    for (i = 0; i < ARRAY_COUNT(expected); i++)
    {
        r = GetPlotRange(PLOT_KIND_GYM, i + 1, SURF_BLOCK, ELITE);
        EXPECT_EQ(r.lo, expected[i][0]);
        EXPECT_EQ(r.hi, expected[i][1]);
    }
    r = GetPlotRange(PLOT_KIND_RIVAL, 1, SURF_BLOCK, ELITE);
    EXPECT_EQ(r.lo, 1); EXPECT_EQ(r.hi, 1);
    r = GetPlotRange(PLOT_KIND_RIVAL, 2, SURF_BLOCK, ELITE);
    EXPECT_EQ(r.lo, 2); EXPECT_EQ(r.hi, 3);
    r = GetPlotRange(PLOT_KIND_RIVAL, 3, SURF_BLOCK, ELITE);
    EXPECT_EQ(r.lo, 4); EXPECT_EQ(r.hi, 9);
    r = GetPlotRange(PLOT_KIND_RIVAL, 4, SURF_BLOCK, ELITE);
    EXPECT_EQ(r.lo, 10); EXPECT_EQ(r.hi, 12);
    r = GetPlotRange(PLOT_KIND_RIVAL, 5, SURF_BLOCK, ELITE);
    EXPECT_EQ(r.lo, 13); EXPECT_EQ(r.hi, 17);
    r = GetPlotRange(PLOT_KIND_WALLY, 1, SURF_BLOCK, ELITE);
    EXPECT_EQ(r.lo, 7); EXPECT_EQ(r.hi, 9);
    r = GetPlotRange(PLOT_KIND_WALLY, 2, SURF_BLOCK, ELITE);
    EXPECT_EQ(r.lo, 20); EXPECT_EQ(r.hi, 20);
}

TEST("RegionPlot: Generic utility windows match the Emerald.md table for E = 20")
{
    static const u8 expected[NUM_PLOT_UTILITY][2] =
    {
        [PLOT_UTILITY_POKENAV] = {1, 7}, [PLOT_UTILITY_OLD_ROD] = {1, 7},
        [PLOT_UTILITY_GOOD_ROD] = {8, 13}, [PLOT_UTILITY_ITEMFINDER] = {1, 10},
        [PLOT_UTILITY_BICYCLE] = {1, 10}, [PLOT_UTILITY_COIN_CASE] = {1, 19},
        [PLOT_UTILITY_POKEBLOCK_CASE] = {1, 19}, [PLOT_UTILITY_SUPER_ROD] = {14, 19},
    };
    u32 i;

    for (i = 0; i < NUM_PLOT_UTILITY; i++)
    {
        struct PlotRange r = GetPlotRange(PLOT_KIND_UTILITY, i, SURF_BLOCK, ELITE);

        EXPECT_EQ(r.lo, expected[i][0]);
        EXPECT_EQ(r.hi, expected[i][1]);
    }
}

TEST("RegionPlot: Generic masks and the callback chance match Emerald.md")
{
    EXPECT_EQ(GetPlotHostMask(PLOT_KIND_GYM, 1, ELITE), 0x000FFFFE);
    EXPECT_EQ(GetPlotHostMask(PLOT_KIND_RIVAL, 1, ELITE), 0x000FFFFE);
    EXPECT_EQ(GetPlotHmMask(PLOT_HM_SURF, ELITE), 0x000F8018);
    EXPECT_EQ(GetPlotHmMask(PLOT_HM_CUT, ELITE), 0x000FFFFE);
    EXPECT_EQ(GetPlotHostMask(PLOT_KIND_ARC, 7, ELITE), 1u << 14);
    EXPECT_EQ(PLOT_CALLBACK_CHANCE, 51);
}

TEST("RegionPlot: The water HM mask is the blocks where county x or x+1 is Ocean in Emerald")
{
    u32 x, mask = 0;

    for (x = 0; x < NUM_COUNTIES - 1; x++)
    {
        if (GetCountyHeader(GetCountyByStoryIdx(x))->terrain == COUNTY_TERRAIN_OCEAN
         || GetCountyHeader(GetCountyByStoryIdx(x + 1))->terrain == COUNTY_TERRAIN_OCEAN)
            mask |= 1u << x;
    }
    // Only blocks 1-19 can host an HM, so ignore the reserved counties' bits.
    EXPECT_EQ(mask & 0x000FFFFE, GetPlotHmMask(PLOT_HM_SURF, ELITE));
}

TEST("RegionPlot: Every vanilla slot lies in its kind's range and mask except exceptions 1, 2 and 3")
{
    u32 kind, ordinal, hm, rangeMisses = 0, maskMisses = 0;

    for (kind = 0; kind < NUM_PLOT_KINDS; kind++)
    {
        for (ordinal = FirstOrdinal(kind); ordinal < FirstOrdinal(kind) + GetPlotKindCount(kind); ordinal++)
        {
            struct PlotSlot slot = GetVanillaPlotSlot(kind, ordinal);
            bool32 g6 = kind == PLOT_KIND_GYM && ordinal == 6;

            if (!SlotInRange(slot))
            {
                rangeMisses++;
                EXPECT(g6);
            }
            else
                EXPECT(!g6);
            EXPECT(SlotInMask(slot));
        }
    }
    // HM slots: the range is the gym's, the mask is the HM's own.
    for (hm = PLOT_HM_CUT; hm < NUM_PLOT_HM_IDS; hm++)
    {
        struct PlotSlot slot = GetVanillaHmSlot(hm);
        struct PlotRange range = GetPlotRange(PLOT_KIND_GYM, GetPlotHmGym(hm), SURF_BLOCK, ELITE);
        u32 mask = GetPlotHmMask(hm, ELITE);

        // An HM at its gym's slot was range-checked with the gym.
        if (slot.hmOnly && (slot.block < range.lo || slot.block > range.hi))
        {
            rangeMisses++;
            EXPECT_EQ(hm, PLOT_HM_STRENGTH);
        }
        if (!(mask & (1u << slot.block)))
        {
            maskMisses++;
            EXPECT_EQ(hm, PLOT_HM_SURF);
        }
    }
    EXPECT_EQ(rangeMisses, 2); // gym 6 (with Fly) and Strength
    EXPECT_EQ(maskMisses, 1);
}

TEST("RegionPlot: Each gym's HM is at the gym's slot except Strength, which has its own entry")
{
    u32 hm;

    for (hm = PLOT_HM_CUT; hm < NUM_PLOT_HM_IDS; hm++)
    {
        struct PlotSlot slot = GetVanillaHmSlot(hm);
        struct PlotSlot gym = GetVanillaPlotSlot(PLOT_KIND_GYM, GetPlotHmGym(hm));

        EXPECT_EQ(slot.hm, hm);
        if (hm == PLOT_HM_STRENGTH)
        {
            EXPECT(slot.hmOnly);
            EXPECT_EQ(gym.hm, PLOT_HM_NONE);
            continue;
        }
        EXPECT(!slot.hmOnly);
        EXPECT_EQ(gym.hm, hm);
        EXPECT_EQ(slot.block, gym.block);
        EXPECT_EQ(slot.step, gym.step);
        EXPECT_EQ(slot.host, gym.host);
    }
    EXPECT_EQ(GetVanillaPlotSlot(PLOT_KIND_GYM, ELITE_FOUR_ORDINAL).hm, PLOT_HM_NONE);
}

TEST("RegionPlot: Step-1 slots are hosted by their block's callback host, others by the block's county")
{
    u32 kind, ordinal, callbacks = 0, block;

    for (kind = 0; kind < NUM_PLOT_KINDS; kind++)
    {
        for (ordinal = FirstOrdinal(kind); ordinal < FirstOrdinal(kind) + GetPlotKindCount(kind); ordinal++)
        {
            struct PlotSlot slot = GetVanillaPlotSlot(kind, ordinal);

            EXPECT_LE(slot.step, 2);
            if (slot.step == 1)
            {
                EXPECT_EQ(slot.host, GetPlotCallbackHost(slot.block));
                EXPECT_LT(slot.host, slot.block);
            }
            else
                EXPECT_EQ(slot.host, slot.block);
        }
    }
    for (block = 0; block <= ELITE; block++)
    {
        if (GetPlotCallbackHost(block) != PLOT_NO_CALLBACK)
            callbacks++;
    }
    EXPECT_EQ(callbacks, 6);
    EXPECT_EQ(GetPlotCallbackHost(1), 0);
    EXPECT_EQ(GetPlotCallbackHost(12), 2);
    EXPECT_EQ(GetPlotCallbackHost(14), 11);
    EXPECT_EQ(GetPlotCallbackHost(15), 5);
    EXPECT_EQ(GetPlotCallbackHost(19), 18);
    EXPECT_EQ(GetPlotCallbackHost(20), 18);
}

TEST("RegionPlot: Arc beats are ordered, never share a slot, and stay inside their clipped windows")
{
    u32 i, j;

    for (i = 0; i < NUM_PLOT_ARC; i++)
    {
        struct PlotSlot a = GetVanillaPlotSlot(PLOT_KIND_ARC, i);
        struct PlotRange r = GetPlotRange(PLOT_KIND_ARC, i, SURF_BLOCK, ELITE);

        EXPECT_GE(r.lo, 2);
        EXPECT_LE(r.hi, ELITE - 1);
        for (j = 0; j < i; j++)
        {
            struct PlotSlot b = GetVanillaPlotSlot(PLOT_KIND_ARC, j);

            EXPECT_LE(b.block, a.block);
            EXPECT(b.block != a.block || b.step != a.step);
        }
    }
}

TEST("RegionPlot: Gyms, rivals and utility ranges are ordered within their kind")
{
    u32 ordinal;
    struct PlotRange prev = {0, 0};

    for (ordinal = 1; ordinal <= NUM_GYM_ORDINALS; ordinal++)
    {
        struct PlotRange r = GetPlotRange(PLOT_KIND_GYM, ordinal, SURF_BLOCK, ELITE);

        EXPECT_GT(r.lo, prev.hi);
        EXPECT_LE(r.lo, r.hi);
        prev = r;
    }
}

TEST("RegionPlot: Item quotas match Emerald.md")
{
    EXPECT_EQ(GetPlotKindCount(PLOT_KIND_GYM), 9);
    EXPECT_EQ(GetPlotKindCount(PLOT_KIND_RIVAL), 5);
    EXPECT_EQ(GetPlotKindCount(PLOT_KIND_WALLY), 2);
    EXPECT_EQ(GetPlotKindCount(PLOT_KIND_ARC), 15);
    EXPECT_EQ(GetPlotKindCount(PLOT_KIND_DEVON), 3);
    EXPECT_EQ(GetPlotKindCount(PLOT_KIND_SPECIAL), 3);
    EXPECT_EQ(GetPlotKindCount(PLOT_KIND_UTILITY), 8);
    EXPECT_EQ(GetPlotKindCount(PLOT_KIND_SCRIPTED), 8);
    EXPECT_EQ(NUM_PLOT_HM_IDS - 1, 8);
}

// Transcribed from the Emerald.md main-story-index table, not from the slot table.
struct Expected
{
    u8 index;
    u8 kind;
    u8 ordinal;
    u8 hm;
    u8 hmOnly;
};

#define ARC(i) PLOT_KIND_ARC, i
#define GYM(i, hm) PLOT_KIND_GYM, i, hm
#define SCRIPT(s) PLOT_KIND_SCRIPTED, PLOT_SCRIPTED_##s
#define UTIL(u) PLOT_KIND_UTILITY, PLOT_UTILITY_##u

static const struct Expected sExpected[] =
{
    {0, SCRIPT(STARTER)},
    {4, PLOT_KIND_RIVAL, 1},
    {5, SCRIPT(POKEDEX)}, {5, SCRIPT(POKE_BALLS)}, {5, SCRIPT(RUNNING_SHOES)},
    {8, ARC(0)}, {8, SCRIPT(WALLY_RALTS)},
    {10, PLOT_KIND_SPECIAL, PLOT_SPECIAL_WAILMER_PAIL},
    {12, GYM(1, PLOT_HM_CUT)}, {12, ARC(1)},
    {14, PLOT_KIND_DEVON, PLOT_DEVON_RECOVERED}, {14, UTIL(POKENAV)}, {14, PLOT_KIND_RIVAL, 2},
    {16, GYM(2, PLOT_HM_FLASH)}, {16, UTIL(OLD_ROD)},
    {18, PLOT_KIND_DEVON, PLOT_DEVON_LETTER_DELIVERED},
    {20, SCRIPT(SHIPYARD)},
    {22, PLOT_KIND_DEVON, PLOT_DEVON_GOODS_DELIVERED}, {22, ARC(2)},
    {24, PLOT_KIND_RIVAL, 3}, {24, UTIL(ITEMFINDER)},
    {28, PLOT_KIND_WALLY, 1},
    {30, GYM(3, PLOT_HM_ROCK_SMASH)}, {30, UTIL(COIN_CASE)}, {30, UTIL(BICYCLE)},
    {32, GYM(4, PLOT_HM_STRENGTH), TRUE},
    {40, ARC(3)},
    {44, ARC(4)}, {44, GYM(4, PLOT_HM_NONE)},
    {46, PLOT_KIND_SPECIAL, PLOT_SPECIAL_GO_GOGGLES},
    {48, PLOT_KIND_RIVAL, 4}, {48, PLOT_KIND_SPECIAL, PLOT_SPECIAL_DEVON_SCOPE},
    {49, GYM(5, PLOT_HM_SURF)},
    {50, GYM(6, PLOT_HM_FLY)},
    {52, ARC(5)},
    {54, UTIL(GOOD_ROD)},
    {56, ARC(6)},
    {57, ARC(7)},
    {60, PLOT_KIND_RIVAL, 5}, {60, UTIL(POKEBLOCK_CASE)},
    {61, ARC(8)},
    {62, ARC(9)},
    {68, GYM(7, PLOT_HM_DIVE)}, {68, ARC(10)}, {68, UTIL(SUPER_ROD)},
    {70, ARC(11)},
    {72, ARC(12)},
    {76, ARC(13)},
    {77, ARC(14)}, {77, GYM(8, PLOT_HM_WATERFALL)},
    {81, PLOT_KIND_WALLY, 2},
    {82, GYM(9, PLOT_HM_NONE)}, {82, SCRIPT(ELITE_FOUR)}, {82, SCRIPT(CHAMPION)},
};

static bool32 ListHas(const struct PlotSlot *list, u32 count, const struct Expected *e)
{
    u32 i;

    for (i = 0; i < count; i++)
    {
        if (list[i].kind == e->kind && list[i].ordinal == e->ordinal && list[i].hm == e->hm
         && list[i].hmOnly == e->hmOnly)
            return TRUE;
    }
    return FALSE;
}

TEST("RegionPlot: The vanilla resolver reproduces the Emerald main-story-index table")
{
    static EWRAM_DATA struct PlotSlot list[16];
    u32 index, i, total = 0, expectedTotal = 0;

    for (index = 0; index < 96; index++)
    {
        u32 count = ResolveVanillaPlotList(index, list, ARRAY_COUNT(list));
        u32 expectedHere = 0;

        for (i = 0; i < ARRAY_COUNT(sExpected); i++)
        {
            if (sExpected[i].index != index)
                continue;
            expectedHere++;
            EXPECT(ListHas(list, count, &sExpected[i]));
        }
        EXPECT_EQ(count, expectedHere);
        for (i = 0; i < count; i++)
        {
            EXPECT_EQ(list[i].block, index >> 2);
            EXPECT_EQ(list[i].step, index & 3);
        }
        total += count;
    }
    expectedTotal = ARRAY_COUNT(sExpected);
    EXPECT_EQ(total, expectedTotal);
    EXPECT_EQ(ResolveVanillaPlotList(3, list, ARRAY_COUNT(list)), 0);
}
