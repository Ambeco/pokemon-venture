#include "global.h"
#include "region_city.h"
#include "region_county.h"
#include "region_plot.h"

#include "data/region/emerald_plot.h"

#define MASK_BLOCKS_1_TO_19 0x000FFFFE
#define MASK_WATER_HM       0x000F8018 // blocks 3, 4, 15-19: county x or x+1 is Ocean in Emerald

u32 GetPlotKindCount(u32 kind)
{
    switch (kind)
    {
    case PLOT_KIND_GYM:     return NUM_GYM_ORDINALS;
    case PLOT_KIND_HM:      return NUM_PLOT_HM;
    case PLOT_KIND_RIVAL:   return NUM_PLOT_RIVAL;
    case PLOT_KIND_WALLY:   return NUM_PLOT_WALLY;
    case PLOT_KIND_ARC:     return NUM_PLOT_ARC;
    case PLOT_KIND_DEVON:   return NUM_PLOT_DEVON;
    case PLOT_KIND_SPECIAL: return NUM_PLOT_SPECIAL;
    case PLOT_KIND_UTILITY: return NUM_PLOT_UTILITY;
    }
    AGB_ASSERT(FALSE);
    return 0;
}

struct PlotSlot GetVanillaPlotSlot(u32 kind, u32 ordinal)
{
    u32 i;
    struct PlotSlot slot;

    if (kind == PLOT_KIND_GYM)
    {
        struct GymSlot gym = GetGymSlot(ordinal);

        slot.kind = PLOT_KIND_GYM;
        slot.ordinal = ordinal;
        slot.block = gym.block;
        slot.step = gym.step;
        slot.host = GetCountyHeader(GetGymCounty(ordinal))->storyIdx;
        return slot;
    }
    for (i = 0; i < ARRAY_COUNT(sEmeraldPlotSlots); i++)
    {
        if (sEmeraldPlotSlots[i].kind == kind && sEmeraldPlotSlots[i].ordinal == ordinal)
            return sEmeraldPlotSlots[i];
    }
    AGB_ASSERT(FALSE); // no such item: check GetPlotKindCount(kind) for the valid ordinals
    return sEmeraldPlotSlots[0];
}

u32 GetPlotCallbackHost(u32 block)
{
    u32 i;

    for (i = 0; i < ARRAY_COUNT(sEmeraldPlotCallbacks); i++)
    {
        if (sEmeraldPlotCallbacks[i].block == block)
            return sEmeraldPlotCallbacks[i].host;
    }
    return PLOT_NO_CALLBACK;
}

static u32 CeilDiv(u32 a, u32 b)
{
    return (a + b - 1) / b;
}

// Last block of gym k's window (k = 0 gives 0, so window k starts at cut(k - 1) + 1).
static u32 GymWindowEnd(u32 k, u32 n, u32 elite)
{
    if (k == 0)
        return 0;
    if (k <= 4)
        return CeilDiv(k * (n - 1), 4);
    if (k == 5)
        return n;
    if (k <= 8)
        return n + CeilDiv((k - 5) * (elite - 1 - n), 3);
    return elite;
}

static struct PlotRange MakeRange(u32 lo, u32 hi)
{
    struct PlotRange range = {lo, hi};

    return range;
}

static struct PlotRange GymRange(u32 ordinal, u32 n, u32 elite)
{
    return MakeRange(GymWindowEnd(ordinal - 1, n, elite) + 1, GymWindowEnd(ordinal, n, elite));
}

static struct PlotRange ClippedAround(u32 nominal, u32 elite)
{
    u32 lo = nominal > 4 ? nominal - 2 : 2;
    u32 hi = nominal + 2 < elite - 1 ? nominal + 2 : elite - 1;

    return MakeRange(lo, hi);
}

struct PlotRange GetPlotRange(u32 kind, u32 ordinal, u32 n, u32 elite)
{
    AGB_ASSERT(n >= 5 && n + 4 <= elite);
    switch (kind)
    {
    case PLOT_KIND_GYM:
    case PLOT_KIND_HM:
        return GymRange(ordinal, n, elite);
    case PLOT_KIND_RIVAL:
        switch (ordinal)
        {
        case 1: return MakeRange(1, 1);
        case 2: return MakeRange(2, GymWindowEnd(1, n, elite));
        case 3: return MakeRange(GymWindowEnd(1, n, elite) + 1, GymWindowEnd(3, n, elite));
        case 4: return MakeRange(GymWindowEnd(3, n, elite) + 1, GymWindowEnd(5, n, elite));
        case 5: return MakeRange(GymWindowEnd(5, n, elite) + 1, GymWindowEnd(7, n, elite));
        }
        break;
    case PLOT_KIND_WALLY:
        if (ordinal == 1)
            return GymRange(3, n, elite);
        return MakeRange(elite, elite);
    case PLOT_KIND_ARC:
        AGB_ASSERT(ordinal < NUM_PLOT_ARC);
        return ClippedAround(sEmeraldArcNominalBlock[ordinal], elite);
    case PLOT_KIND_DEVON:
        AGB_ASSERT(ordinal < NUM_PLOT_DEVON);
        return ClippedAround(sEmeraldDevonBlock[ordinal], elite);
    case PLOT_KIND_SPECIAL:
        AGB_ASSERT(ordinal < NUM_PLOT_SPECIAL);
        return MakeRange(sEmeraldSpecialBlock[ordinal], sEmeraldSpecialBlock[ordinal]);
    case PLOT_KIND_UTILITY:
        switch (ordinal)
        {
        case PLOT_UTILITY_BICYCLE:
        case PLOT_UTILITY_ITEMFINDER:
            return MakeRange(1, elite / 2);
        case PLOT_UTILITY_OLD_ROD:
        case PLOT_UTILITY_POKENAV:
            return MakeRange(1, CeilDiv(elite, 3));
        case PLOT_UTILITY_GOOD_ROD:
            return MakeRange(CeilDiv(elite, 3) + 1, 2 * elite / 3);
        case PLOT_UTILITY_SUPER_ROD:
            return MakeRange(2 * elite / 3 + 1, elite - 1);
        default:
            return MakeRange(1, elite - 1);
        }
    }
    AGB_ASSERT(FALSE);
    return MakeRange(0, 0);
}

static u32 BlocksMask(u32 lo, u32 hi)
{
    return ((2u << hi) - 1) & ~((1u << lo) - 1);
}

u32 GetPlotHostMask(u32 kind, u32 ordinal, u32 elite)
{
    u32 lastGeneric = elite - 1;

    AGB_ASSERT(elite == 20); // MASK_* constants are written for E = 20
    switch (kind)
    {
    case PLOT_KIND_GYM:
        if (ordinal == ELITE_FOUR_ORDINAL)
            return 1u << elite;
        return BlocksMask(1, lastGeneric);
    case PLOT_KIND_HM:
        if (ordinal == 5 || ordinal == 7 || ordinal == 8) // Surf, Dive, Waterfall
            return MASK_WATER_HM;
        return BlocksMask(1, lastGeneric);
    case PLOT_KIND_RIVAL:
    case PLOT_KIND_UTILITY:
        return BlocksMask(1, lastGeneric);
    case PLOT_KIND_WALLY:
        return BlocksMask(1, elite);
    case PLOT_KIND_ARC:
        return 1u << sEmeraldArcNominalBlock[ordinal];
    case PLOT_KIND_DEVON:
        return 1u << sEmeraldDevonBlock[ordinal];
    case PLOT_KIND_SPECIAL:
        return 1u << sEmeraldSpecialBlock[ordinal];
    }
    AGB_ASSERT(FALSE);
    return 0;
}

u32 ResolveVanillaPlotList(u32 index, struct PlotSlot *out, u32 maxItems)
{
    u32 block = index >> 2;
    u32 step = index & 3;
    u32 count = 0;
    u32 kind, ordinal;

    if (step == 3)
        return 0;
    for (kind = 0; kind < NUM_PLOT_KINDS; kind++)
    {
        u32 first = (kind == PLOT_KIND_GYM || kind == PLOT_KIND_HM || kind == PLOT_KIND_RIVAL
                  || kind == PLOT_KIND_WALLY) ? 1 : 0;
        u32 end = first + GetPlotKindCount(kind);

        for (ordinal = first; ordinal < end; ordinal++)
        {
            struct PlotSlot slot = GetVanillaPlotSlot(kind, ordinal);

            if (slot.block != block || slot.step != step)
                continue;
            AGB_ASSERT(count < maxItems);
            if (count < maxItems)
                out[count] = slot;
            count++;
        }
    }
    return count < maxItems ? count : maxItems;
}
