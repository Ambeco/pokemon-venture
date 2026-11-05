#ifndef GUARD_SUBPLOT_H
#define GUARD_SUBPLOT_H

/**
 * Subplot descriptor, stage accessor and NPC-talk query (design.md, "Subplot abstraction").
 *
 * A subplot is a linear list of stages. A county hosts up to SUBPLOTS_PER_COUNTY subplots, each in a
 * numbered slot; callers address a subplot by (county, slot) and never by identity, so moving a subplot to
 * another host moves no state. A stored subplot keeps its stage in the host county's slice var for that
 * slot (a county's var 0 is reserved for the completion mask). An adapter subplot has no state of its own:
 * its stage is the highest stage whose done-predicate is true, over Emerald's old flags and vars, and it
 * cannot be set. The query functions cannot tell the two apart.
 *
 * Slots are valid only for counties whose slice is free of legacy vars (COUNTY_VAR_SLICES_FREE). Specials
 * for scripts act on the county of the current map's MAPSEC, which must be a county's city for now.
 */

#include "global.h"
#include "plot_stage.h"

#define SUBPLOTS_PER_COUNTY (VARS_PER_COUNTY - 1)
#define COUNTY_VAR_SLICES_FREE ((LEGACY_VARS_START - COUNTY_VARS_START) / VARS_PER_COUNTY)

enum SubplotBackend
{
    SUBPLOT_STORED,
    SUBPLOT_ADAPTER,
};

enum SubplotTextKind
{
    SUBPLOT_TEXT_HINT,  // "I heard something is happening at ..."
    SUBPLOT_TEXT_STATE, // what the people involved say in this stage
};

struct SubplotStage
{
    const u8 *hintText;
    const u8 *stateText;
    u16 triggerRole;            // BuildingRole bit of the building where the stage's beat happens; 0 = outdoors
    struct StoryPredicate done; // adapter backend only: stage reached; stage 0 is the default and never tested
};

struct SubplotDesc
{
    u8 backend;
    u8 numStages;
    u16 requiredRoles; // BuildingRole bits the host city must have
    u8 minBadges;      // badges the host's progress must allow before the subplot starts
    u8 producedKey;    // McGuffin id this subplot's completion gives, 0 = none
    const struct SubplotStage *stages;
};

// NULL when the county hosts nothing in the slot.
const struct SubplotDesc *GetCountySubplot(u32 countyId, u32 slot);

u16 GetCountySlotVar(u32 countyId, u32 slot);

// Adapter subplots are read-only.
u32 GetSubplotStage(u32 countyId, u32 slot, const struct SubplotDesc *desc);
void SetSubplotStage(u32 countyId, u32 slot, const struct SubplotDesc *desc, u32 stage);

u32 GetCountySlotStage(u32 countyId, u32 slot);
void SetCountySlotStage(u32 countyId, u32 slot, u32 stage);

// NULL when the stage has no text of that kind.
const u8 *GetSubplotText(const struct SubplotDesc *desc, u32 stage, u32 kind);

// The NPC-talk query: the text to say about the (county, slot) subplot in its current stage.
const u8 *GetCountySlotText(u32 countyId, u32 slot, u32 kind);

// County whose city has this MAPSEC; asserts there is none.
u32 GetCountyFromCityMapSec(u32 mapSec);

#endif // GUARD_SUBPLOT_H
