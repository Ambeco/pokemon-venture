#ifndef GUARD_COUNTY_PLAN_H
#define GUARD_COUNTY_PLAN_H

/**
 * The county's chain planner (design_chapters.md, "County chain"): the subplots a county hosts, planned against its
 * real main-story slot rules into one county chain.
 *
 * GetCountySlotRules reads the planner's lists for the county's block: a slot with no get a chapter can supply (HM,
 * utility item, scripted get, rival beat; not a gym badge, Wally, arc, Devon or special item), a step-3 unlock, or a
 * callback hosted by another county takes no chapters ({0, 0}); every other slot takes up to CHAPTERS_PER_COUNTY and
 * needs one chapter when one of those gets is blocking (gyms are not chapters yet).
 *
 * PlanCountyChain plans each hosted subplot in declaration order with PlanSubplot, handing each the capacity the
 * earlier ones left, and merges the placed entries by main-story slot (declaration order within a slot). Each entry
 * keeps the host table's chapter slot, so no stored state moves. Only required entries go on the chain; optional
 * ones are off-chain: they run beside it on their own slot var and never hold a main-story slot (design_scenes.md,
 * "Interleaving subplots").
 */

#include "global.h"
#include "chapter.h"
#include "county_chain.h"
#include "subplot.h"

struct CountyChainPlan
{
    u8 problem;         // enum SubplotPlanProblem
    u8 problemSubplot;  // index among the county's subplots when problem is set
    u8 problemIndex;    // SubplotPlan.problemIndex of that subplot
    u8 numEntries;
    u8 numOffChain;
    struct CountyChainEntry chain[CHAPTERS_PER_COUNTY];
    u8 offChainSlot[CHAPTERS_PER_COUNTY]; // chapter slots of the dropped entries
};

void GetCountySlotRules(u32 county, struct SubplotSlotRule rules[COUNTY_MAIN_SLOTS]);

// FALSE with `out->problem` set when a subplot does not fit or a subplot's chapter is not hosted in `county`.
bool32 PlanCountyChain(u32 county, const struct SubplotHost *subplots, u32 numSubplots, const struct ChapterHost *hosts, u32 numHosts, struct CountyChainPlan *out);

// PlanCountyChain over Emerald's subplots and hosts.
bool32 PlanEmeraldCountyChain(u32 county, struct CountyChainPlan *out);

#endif // GUARD_COUNTY_PLAN_H
