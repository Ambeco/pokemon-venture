#ifndef GUARD_SUBPLOT_H
#define GUARD_SUBPLOT_H

/**
 * A subplot (design.md, "Vocabulary"): an ordered sequence of related chapters telling one story, each entry
 * required or optional. A county plans its subplots into its main-story slots with PlanSubplot and turns the
 * plan into county-chain entries (county_chain.h) with BuildSubplotChain.
 *
 * Each main-story slot has a rule: at most `max` chapters (0 for a callback hosted elsewhere) and at least `min`
 * (a slot that supplies gets needs one). Entries are placed in order into the first slot that has room. A required
 * entry that does not fit spills into the next slot with capacity; an optional one is dropped. Optional entries
 * are also dropped when placing them would leave the later slots' minimums to chance, since only required entries
 * after one are guaranteed to be placed. The plan fails, naming the problem, when a required entry has no slot
 * left or a slot ends below its minimum.
 *
 * The functions are pure. A chapter's chain length is its stage count, so a subplot's chain is as long as the
 * stages of its placed chapters.
 */

#include "global.h"
#include "county_chain.h"

#define SUBPLOT_MAX_ENTRIES 8
#define SUBPLOT_DROPPED 0xFF

struct ChapterDesc;

struct SubplotEntry
{
    const struct ChapterDesc *chapter;
    bool8 optional;
};

struct SubplotDesc
{
    u8 numEntries; // at most SUBPLOT_MAX_ENTRIES
    const struct SubplotEntry *entries;
};

struct SubplotSlotRule
{
    u8 min;
    u8 max;
};

enum SubplotPlanProblem
{
    SUBPLOT_PLAN_OK,
    SUBPLOT_PLAN_NO_ROOM,          // a required entry found no slot with capacity left: `problemIndex` is the entry
    SUBPLOT_PLAN_SLOT_UNDERFILLED, // a slot ended below its minimum: `problemIndex` is the main-story slot
};

struct SubplotPlan
{
    u8 problem;                         // enum SubplotPlanProblem
    u8 problemIndex;
    u8 mainSlot[SUBPLOT_MAX_ENTRIES];   // per entry: its main-story slot, or SUBPLOT_DROPPED
    u8 count[COUNTY_MAIN_SLOTS];        // chapters placed per main-story slot
};

// Places the subplot's entries into the main-story slots under `rules`. On a problem every entry is still
// reported as far as it was placed, so a test can show where the plan broke.
struct SubplotPlan PlanSubplot(const struct SubplotDesc *subplot, const struct SubplotSlotRule rules[COUNTY_MAIN_SLOTS]);

// Entries the plan placed, in order; the chain length in chapters.
u32 GetSubplotPlacedCount(const struct SubplotDesc *subplot, const struct SubplotPlan *plan);

// Fills `out` (room for SUBPLOT_MAX_ENTRIES) with a chain entry per placed chapter: its main-story slot, the county
// chapter slot `firstChapterSlot` + its placed index, and its stage count. Returns the number of entries; asserts if
// the plan failed or the chapter slots would run past CHAPTERS_PER_COUNTY.
u32 BuildSubplotChain(const struct SubplotDesc *subplot, const struct SubplotPlan *plan, u32 firstChapterSlot, struct CountyChainEntry *out);

#endif // GUARD_SUBPLOT_H
