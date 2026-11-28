#ifndef GUARD_SUBPLOT_H
#define GUARD_SUBPLOT_H

/**
 * A subplot (design.md, "Vocabulary"): an ordered sequence of related chapters telling one story, each entry
 * required or optional. A county plans its subplots into its main-story slots with PlanSubplot and turns the
 * plan into county-chain entries (county_chain.h) with BuildSubplotChain.
 *
 * Each main-story slot has a rule: at most `max` chapters (0 for a callback hosted elsewhere) and at least `min`
 * (a slot that supplies gets needs one). An entry bound to a slot (PlanSubplotBound) goes there; the others are placed
 * in order into the first slot that has room. A required
 * entry that does not fit spills into the next slot with capacity; an optional one is dropped. Optional entries
 * are also dropped when placing them would leave the later slots' minimums to chance, since only required entries
 * after one are guaranteed to be placed. The plan fails, naming the problem, when a required entry has no slot
 * left or a slot ends below its minimum.
 *
 * Only placed entries are on the county chain; a dropped (optional) entry still runs, beside the chain, on its own
 * chapter-slot var. Order between chapters comes only from the subplot: each entry waits for the one before it, and
 * chapters of different subplots never wait for each other (design_scenes.md, "Interleaving subplots").
 *
 * The functions are pure. A chapter's chain length is its stage advances (numStages - 1, since its last stage means
 * done), so a subplot's chain is as long as the advances of its placed chapters.
 */

#include "global.h"
#include "county_chain.h"

#define SUBPLOT_MAX_ENTRIES 8
#define SUBPLOT_DROPPED 0xFF
#define SUBPLOT_UNBOUND 0xFF

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

// A subplot as hosted by a county; a chapter belongs to at most one subplot of its county.
struct SubplotHost
{
    u8 county;
    const struct SubplotDesc *subplot;
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
    SUBPLOT_PLAN_BOUND_OUT_OF_ORDER, // a bound entry's slot is before an earlier entry's (or past the last): `problemIndex` is the entry
    SUBPLOT_PLAN_BOUND_ELSEWHERE,    // set by PlanCountyChain: the entry's get is not in this county's block or slot rules: `problemIndex` is the entry
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

// PlanSubplot with `boundSlots[i]` (SUBPLOT_UNBOUND for none; NULL for all unbound) forcing required entry i into
// that slot, the one holding the get the chapter supplies. Unbound entries keep to first-fit but leave room for
// later bound ones in their slot and never pass the next bound slot. Bindings of optional entries are ignored.
struct SubplotPlan PlanSubplotBound(const struct SubplotDesc *subplot, const struct SubplotSlotRule rules[COUNTY_MAIN_SLOTS], const u8 *boundSlots);

// The chapter before `desc` in its subplot of `county`, which must be done before `desc` starts; NULL when `desc` is
// first in its subplot or in none. Chapters of different subplots are unordered relative to each other.
const struct ChapterDesc *GetSubplotPredecessor(const struct SubplotHost *hosts, u32 numHosts, u32 county, const struct ChapterDesc *desc);

// Entries the plan placed, in order; the chain length in chapters.
u32 GetSubplotPlacedCount(const struct SubplotDesc *subplot, const struct SubplotPlan *plan);

// Fills `out` (room for SUBPLOT_MAX_ENTRIES) with a chain entry per placed chapter: its main-story slot, the county
// chapter slot `firstChapterSlot` + its placed index, and its stage count. Returns the number of entries; asserts if
// the plan failed or the chapter slots would run past GetCountyChapterSlotCount().
u32 BuildSubplotChain(const struct SubplotDesc *subplot, const struct SubplotPlan *plan, u32 firstChapterSlot, struct CountyChainEntry *out);

#endif // GUARD_SUBPLOT_H
