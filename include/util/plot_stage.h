#ifndef GUARD_PLOT_STAGE_H
#define GUARD_PLOT_STAGE_H

/**
 * Main-plot stage descriptor, accessor and read-only adapter (design.md, "Subplot abstraction";
 * Emerald.md, "Step predicates for the read-only adapter").
 *
 * A get (one item the planner places, from ResolveVanillaPlotList) is described by a PlotGetDesc: a
 * done-predicate over Emerald's flags and vars, and a class. The stage accessor reports, for a
 * main-story-index list, which of its gets are done and which of them gate the list; callers cannot
 * tell where the answer comes from. Today only the adapter answers it, from Emerald's old flags and
 * vars; the stored backend (the host county's completion mask, Milestone 3) is not written yet.
 *
 * A gym with an HM is two gets (badge, HM receipt), and an HM displaced from its gym's slot is one
 * (HM receipt only). A step-3 unlock is a list with no gets, except an unlock whose key is not a get
 * (the Letter), which is one.
 *
 * PLOT_GET_BLOCKING gets hold a list: the list is done when all of them are. PLOT_GET_REPORTED gets are
 * reported in the done mask but ignored for completion. A get is BLOCKING only if vanilla forces it,
 * because a BLOCKING get that the player may skip freezes GetMainStoryIndex() for good; when in doubt
 * it is REPORTED. This differs from the planner, where the Good Rod, for example, is mandatory.
 *
 * GetMainStoryIndex() scans from 0 and returns the first list that is not done, not the highest
 * true predicate, because Emerald lets some gets happen early (Fly with RB4, Strength at Rusturf).
 * Predicates must be monotone (never cleared again by the game): FLAG_DEFEATED_MAGMA_SPACE_CENTER is
 * not, so the Space Center beat reads a var instead. Once every list is done it returns
 * MAIN_STORY_INDEX_POSTGAME.
 */

#include "global.h"
#include "region_plot.h"

enum StoryPredicateOp
{
    STORY_PRED_FLAG,     // flag a is set
    STORY_PRED_ANY_FLAG, // flag a or flag b is set
    STORY_PRED_VAR_GE,   // var a >= b
    STORY_PRED_ALWAYS,   // the get leaves no state to read
};

struct StoryPredicate
{
    u8 op;
    u16 a;
    u16 b;
};

enum PlotGetClass
{
    PLOT_GET_BLOCKING,
    PLOT_GET_REPORTED,
};

struct PlotGetDesc
{
    u8 getClass;
    struct StoryPredicate done;
};

#define MAX_GETS_PER_LIST 16

struct PlotListStage
{
    u32 numGets;
    u32 doneMask;     // bit i = get i is done; gets are in ResolveVanillaPlotList order, a gym's badge before its HM
    u32 blockingMask; // bit i = get i holds the list
};

#define MAIN_STORY_INDEX_POSTGAME 84 // first reserved county's list: open, nothing left to hold

bool32 IsStoryPredicateTrue(const struct StoryPredicate *pred);

// Fills out with the gets of a placed item (1, or 2 for a gym with its HM); asserts it has a descriptor.
u32 GetPlotSlotGetDescs(const struct PlotSlot *slot, struct PlotGetDesc out[2]);

// Number of non-gym vanilla items with a descriptor, for coverage checks.
u32 GetVanillaPlotGetEntryCount(void);

struct PlotListStage GetPlotListStage(u32 index);

static inline bool32 IsPlotListDone(const struct PlotListStage *stage)
{
    return (stage->doneMask & stage->blockingMask) == stage->blockingMask;
}

u32 GetMainStoryIndex(void);

#endif // GUARD_PLOT_STAGE_H
