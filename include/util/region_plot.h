#ifndef GUARD_REGION_PLOT_H
#define GUARD_REGION_PLOT_H

/**
 * Main plot planner inputs (design.md, "Main plot planner"; Emerald.md, "Planner inputs").
 *
 * Every item the planner places is a (kind, ordinal) pair. Two layers describe where it goes:
 * the vanilla slot table (a ROM table of {block, step, host} read while the planner's
 * randomization bit is off; the gym part stays in GetGymSlot) and the generic ranges and masks the
 * randomized path draws from, which also bound the vanilla slots. The only vanilla slots allowed
 * outside them are the numbered exceptions in Emerald.md: gym 6 outside its range (1), Surf
 * outside the water mask (2), Strength outside its range (3).
 *
 * A gym item carries its HM (enum PlotHm, none for the League). The HM takes the gym's slot unless
 * the vanilla table displaces it (Strength), in which case the HM also has an hmOnly entry of its
 * own. HM ranges are the gym's; HM masks are per HM (the water HMs have a terrain mask).
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
 * PLOT_KIND_SCRIPTED items are the vanilla-script gets (tutorial, Wally's Ralts, the Shipyard
 * visit, Elite Four, Champion). They are fixed to their authored block and host.
 */

#include "global.h"

#define PLOT_NO_CALLBACK 0xFF

// Callback chance on the randomized path: hash(seed, x) % 256 < this.
#define PLOT_CALLBACK_CHANCE 51

enum PlotKind
{
    PLOT_KIND_GYM,     // ordinal 1-9, 9 = League lobby; carries its HM
    PLOT_KIND_RIVAL,   // ordinal 1-5
    PLOT_KIND_WALLY,   // ordinal 1-2
    PLOT_KIND_ARC,     // ordinal 0-14, in arc order
    PLOT_KIND_DEVON,   // enum PlotDevon
    PLOT_KIND_SPECIAL, // enum PlotSpecial
    PLOT_KIND_UTILITY, // enum PlotUtility
    PLOT_KIND_SCRIPTED, // enum PlotScripted
    NUM_PLOT_KINDS,
};

// Ordinal order is the vanilla gym order.
enum PlotHm
{
    PLOT_HM_NONE,
    PLOT_HM_CUT,
    PLOT_HM_FLASH,
    PLOT_HM_ROCK_SMASH,
    PLOT_HM_STRENGTH,
    PLOT_HM_SURF,
    PLOT_HM_FLY,
    PLOT_HM_DIVE,
    PLOT_HM_WATERFALL,
    NUM_PLOT_HM_IDS,
};

enum PlotScripted
{
    PLOT_SCRIPTED_STARTER,
    PLOT_SCRIPTED_POKEDEX,
    PLOT_SCRIPTED_POKE_BALLS,
    PLOT_SCRIPTED_RUNNING_SHOES,
    PLOT_SCRIPTED_WALLY_RALTS,
    PLOT_SCRIPTED_SHIPYARD,
    PLOT_SCRIPTED_ELITE_FOUR,
    PLOT_SCRIPTED_CHAMPION,
    NUM_PLOT_SCRIPTED,
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

struct PlotSlot
{
    u8 kind;
    u8 ordinal;
    u8 block;
    u8 step; // 0-2
    u8 host; // county-story-index
    u8 hm;     // enum PlotHm given here; gyms only
    u8 hmOnly; // TRUE for the separate entry of an HM displaced from its gym's slot
};

struct PlotRange
{
    u8 lo;
    u8 hi;
};

// Number of items of the kind: the valid ordinals are 1..n for gyms, rivals and Wally, else 0..n-1.
u32 GetPlotKindCount(u32 kind);

// Vanilla {block, step, host}. Gyms come from GetGymSlot; asserts the item exists. A gym's hm is
// PLOT_HM_NONE when its HM is displaced (see GetVanillaHmSlot).
struct PlotSlot GetVanillaPlotSlot(u32 kind, u32 ordinal);

// The HM's own vanilla slot, a gym entry with hm set; hmOnly iff it is not at its gym's slot.
struct PlotSlot GetVanillaHmSlot(u32 hm);

// Ordinal of the gym that hands out the HM.
u32 GetPlotHmGym(u32 hm);

// Host county-story-index of the block's callback, or PLOT_NO_CALLBACK. Vanilla stage only.
u32 GetPlotCallbackHost(u32 block);

// Generic range for the randomized path, given the Surf-lock block n and the Elite Four index E.
struct PlotRange GetPlotRange(u32 kind, u32 ordinal, u32 n, u32 elite);

// Generic host mask (bit i = county-story-index i). The water HM mask is Emerald's terrain result.
u32 GetPlotHostMask(u32 kind, u32 ordinal, u32 elite);

// Mask for the HM's slot: the water HMs depend on terrain, the others are the gym mask.
u32 GetPlotHmMask(u32 hm, u32 elite);

// Fills out with the vanilla items given at main-story-index (none for step 3); returns the count.
u32 ResolveVanillaPlotList(u32 index, struct PlotSlot *out, u32 maxItems);

#endif // GUARD_REGION_PLOT_H
