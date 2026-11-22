#ifndef GUARD_COUNTY_PLAN_H
#define GUARD_COUNTY_PLAN_H

/**
 * The county's chain planner (design_chapters.md, "County chain"): the subplots a county hosts, planned against its
 * real main-story slot rules into one county chain.
 *
 * GetCountySlotRules reads the planner's lists for the county's block: a slot with no get a chapter can supply (gym badge, HM,
 * utility item, scripted get, rival beat; not Wally, arc, Devon or special item), a step-3 unlock, or a
 * callback hosted by another county takes no chapters ({0, 0}); every other slot takes up to CHAPTERS_PER_COUNTY and
 * needs one chapter when one of those gets is blocking.
 *
 * PlanCountyChain plans each hosted subplot in declaration order with PlanSubplotBound. A slot's max is per subplot (every
 * subplot sees the full max; storage, not slots, bounds how many chapters a county hosts), while its min is shared: it is
 * what the earlier subplots left unmet minus what later subplots' bound entries need. An entry whose chapter supplies a get
 * (GetChapterSuppliedSlot) is bound to the slot of that get; a get outside the county's block or in a slot with no
 * room asserts and fails the plan (SUBPLOT_PLAN_BOUND_ELSEWHERE). It merges the placed entries by main-story slot
 * (declaration order within a slot). Each entry keeps the host table's chapter slot, so no stored state moves. Only
 * required entries go on the chain; optional ones are off-chain: they run beside it on their own slot var and never
 * hold a main-story slot (design_scenes.md, "Interleaving subplots").
 */

#include "global.h"
#include "chapter.h"
#include "county_chain.h"
#include "subplot.h"

// Entries a county's chain can hold: its stateful chapters plus a proxy for each parallel subplot.
#define COUNTY_PLAN_MAX_ENTRIES (CHAPTERS_PER_COUNTY * 2)

struct CountyChainPlan
{
    u8 problem;         // enum SubplotPlanProblem
    u8 problemSubplot;  // index among the county's subplots when problem is set
    u8 problemIndex;    // SubplotPlan.problemIndex of that subplot
    u8 numEntries;
    u8 numOffChain;
    struct CountyChainEntry chain[COUNTY_PLAN_MAX_ENTRIES]; // a proxy's chapterSlot is CHAPTER_SLOT_STATELESS: use chainDesc
    const struct ChapterDesc *chainDesc[COUNTY_PLAN_MAX_ENTRIES];
    u8 offChainSlot[CHAPTERS_PER_COUNTY]; // chapter slots of the dropped entries
};

void GetCountySlotRules(u32 county, struct SubplotSlotRule rules[COUNTY_MAIN_SLOTS]);

// FALSE with `out->problem` set when a subplot does not fit or a subplot's chapter is not hosted in `county`.
bool32 PlanCountyChain(u32 county, const struct SubplotHost *subplots, u32 numSubplots, const struct ChapterHost *hosts, u32 numHosts, struct CountyChainPlan *out);

// PlanCountyChain over Emerald's subplots and hosts.
bool32 PlanEmeraldCountyChain(u32 county, struct CountyChainPlan *out);

// Emerald's chapter hosts with each one's planned main-story slot (CHAPTER_MAIN_SLOT_OFF_CHAIN when off the chain), the
// input of the story-order placement checks in chapter.h. Plans every hosting county; returns the host count, 0 on failure.
u32 GetPlannedChapterHosts(struct ChapterHost *out, u32 maxHosts);

// The stored chain is the chapters' own stage vars (var 0 of the slice is reserved): a chain entry is done when its chapter has reached its
// last stage, and a main-story slot is done when all its on-chain entries are. Entries of one slot may be played in any
// order (independent subplots), which a single linear position cannot represent. A slot with no entries is never done.
bool32 IsCountyChainEntryDone(u32 county, u32 chapterSlot);
bool32 IsCountyMainSlotDone(u32 county, u32 mainSlot);

#endif // GUARD_COUNTY_PLAN_H
