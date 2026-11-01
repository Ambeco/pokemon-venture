#ifndef GUARD_REGION_PLOT_H
#define GUARD_REGION_PLOT_H

/**
 * Main plot planner inputs (design.md, "Main plot planner"; Emerald.md, "Planner inputs").
 *
 * Every item the planner places is a (kind, ordinal) pair. Two layers describe where it goes:
 * the vanilla slot table (a ROM table of {block, step, host} read while the planner's
 * randomization bit is off; gyms stay in GetGymSlot) and the generic ranges and masks the
 * randomized path draws from, which also bound the vanilla slots. The only vanilla slots allowed
 * outside them are the numbered exceptions in Emerald.md: gym 6 outside its range (1), Surf
 * outside the water mask (2), Strength outside its range (3).
 *
 * A block is a county-story-index x; the main-story-index is 4x + step. Step 1 is the callback
 * step, whose host is GetPlotCallbackHost(block); other steps are hosted by the block's county.
 * Ranges are inclusive blocks and depend on the Surf-lock block n and the Elite Four index E.
 * Masks are bit sets over county-story-indices; a callback host must pass the mask too, except an
 * arc beat's, whose host is authored and whose mask covers its block only.
 *
 * Ranges of a kind are disjoint and ordered except arc beats, which are ordered instead: each
 * beat's block is at least the previous beat's, and two beats never share a {block, step}. Devon
 * Goods beats are a separate chain, so they may share a slot with an arc beat (the Museum scene).
 * Wally beats are not rival beats for masks: Wally 2 is in the Elite Four block, so their mask
 * includes E. Special items and the arc kinds are fixed to their authored block.
 *
 * Not placed by the planner (hardcoded in the vanilla scripts): the tutorial lists of blocks 0
 * and 1, Wally having Ralts, the Shipyard visit, the Elite Four and the Champion.
 */

#include "global.h"

#define PLOT_NO_CALLBACK 0xFF

// Callback chance on the randomized path: hash(seed, x) % 256 < this.
#define PLOT_CALLBACK_CHANCE 51

enum PlotKind
{
    PLOT_KIND_GYM,     // ordinal 1-9, 9 = League lobby
    PLOT_KIND_HM,      // ordinal = its gym's ordinal (Strength is 4)
    PLOT_KIND_RIVAL,   // ordinal 1-5
    PLOT_KIND_WALLY,   // ordinal 1-2
    PLOT_KIND_ARC,     // ordinal 0-14, in arc order
    PLOT_KIND_DEVON,   // enum PlotDevon
    PLOT_KIND_SPECIAL, // enum PlotSpecial
    PLOT_KIND_UTILITY, // enum PlotUtility
    NUM_PLOT_KINDS,
};

enum PlotDevon
{
    PLOT_DEVON_RECOVERED,
    PLOT_DEVON_LETTER_DELIVERED,
    PLOT_DEVON_GOODS_DELIVERED,
    NUM_PLOT_DEVON,
};

enum PlotSpecial
{
    PLOT_SPECIAL_WAILMER_PAIL,
    PLOT_SPECIAL_GO_GOGGLES,
    PLOT_SPECIAL_DEVON_SCOPE,
    NUM_PLOT_SPECIAL,
};

enum PlotUtility
{
    PLOT_UTILITY_POKENAV,
    PLOT_UTILITY_OLD_ROD,
    PLOT_UTILITY_GOOD_ROD,
    PLOT_UTILITY_ITEMFINDER,
    PLOT_UTILITY_BICYCLE,
    PLOT_UTILITY_COIN_CASE,
    PLOT_UTILITY_POKEBLOCK_CASE,
    PLOT_UTILITY_SUPER_ROD,
    NUM_PLOT_UTILITY,
};

#define NUM_PLOT_RIVAL 5
#define NUM_PLOT_WALLY 2
#define NUM_PLOT_ARC   15
#define NUM_PLOT_HM    8

struct PlotSlot
{
    u8 kind;
    u8 ordinal;
    u8 block;
    u8 step; // 0-2
    u8 host; // county-story-index
};

struct PlotRange
{
    u8 lo;
    u8 hi;
};

// Number of items of the kind: the valid ordinals are 1..n for gyms, HMs, rivals and Wally, else 0..n-1.
u32 GetPlotKindCount(u32 kind);

// Vanilla {block, step, host}. Gyms come from GetGymSlot; asserts the item exists.
struct PlotSlot GetVanillaPlotSlot(u32 kind, u32 ordinal);

// Host county-story-index of the block's callback, or PLOT_NO_CALLBACK. Vanilla stage only.
u32 GetPlotCallbackHost(u32 block);

// Generic range for the randomized path, given the Surf-lock block n and the Elite Four index E.
struct PlotRange GetPlotRange(u32 kind, u32 ordinal, u32 n, u32 elite);

// Generic host mask (bit i = county-story-index i). The water HM mask is Emerald's terrain result.
u32 GetPlotHostMask(u32 kind, u32 ordinal, u32 elite);

// Fills out with the vanilla items given at main-story-index (none for step 3); returns the count.
u32 ResolveVanillaPlotList(u32 index, struct PlotSlot *out, u32 maxItems);

#endif // GUARD_REGION_PLOT_H
