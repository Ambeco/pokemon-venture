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

// Where the talking NPC is relative to the beat: far = another city or county, close = at the place.
enum SubplotDistance
{
    SUBPLOT_FAR,
    SUBPLOT_CLOSE,
};

// The subplot's progress relative to a stage (its beat): the beat has not happened, is happening, or is over.
enum SubplotBeatState
{
    SUBPLOT_BEFORE,
    SUBPLOT_CURRENT,
    SUBPLOT_AFTER,
};

struct SubplotTextList
{
    const u8 *const *texts;
    u8 count;
};

// The six hint entries of a stage (design.md, "City planning"). NULL or an empty list means "say generic text".
// The required entries are farCurrent and closeCurrent, plus closeAfter unless the stage is the last.
struct SubplotStageHints
{
    struct SubplotTextList farBefore;
    const u8 *farCurrent;
    struct SubplotTextList farAfter;
    const u8 *closeBefore;
    const u8 *closeCurrent;
    const u8 *closeAfter;
};

struct SubplotStage
{
    struct SubplotStageHints hints;
    u16 triggerRole;            // BuildingRole bit of the building where the stage's beat happens; 0 = outdoors
    u16 hinterRoles;            // BuildingRole bits of the buildings where this stage's hinters may stand; 0 = anywhere in the county
    u8 numHinters;              // hinter NPCs the county must place for this stage, 0 = none
    struct StoryPredicate done; // adapter backend only: stage reached; stage 0 is the default and never tested
};

// An NPC the subplot controls entirely while its stage is in [firstStage, lastStage].
struct SubplotActor
{
    u8 firstStage;
    u8 lastStage;
    u16 role; // BuildingRole bit of the building hosting the actor; 0 = outdoors
    u16 gfxId;
};

struct SubplotDesc
{
    u8 backend;
    u8 numStages;
    u8 numActors;
    u16 requiredRoles; // BuildingRole bits the host city must have
    u8 minBadges;      // badges the host's progress must allow before the subplot starts
    u8 producedKey;    // McGuffin id this subplot's completion gives, 0 = none
    const struct SubplotStage *stages;
    const struct SubplotActor *actors;
};

// NULL when the county hosts nothing in the slot.
const struct SubplotDesc *GetCountySubplot(u32 countyId, u32 slot);

u16 GetCountySlotVar(u32 countyId, u32 slot);

// Adapter subplots are read-only.
u32 GetSubplotStage(u32 countyId, u32 slot, const struct SubplotDesc *desc);
void SetSubplotStage(u32 countyId, u32 slot, const struct SubplotDesc *desc, u32 stage);

u32 GetCountySlotStage(u32 countyId, u32 slot);
void SetCountySlotStage(u32 countyId, u32 slot, u32 stage);

u32 GetSubplotBeatState(u32 subplotStage, u32 beat);

// The hint about stage `beat` for a subplot at `subplotStage`, from the entry for the distance and the beat
// state. A list entry yields texts[selector % count], so one NPC (selector = its identity) always says the
// same thing while the state holds and different NPCs differ. NULL means the NPC says generic text.
const u8 *GetSubplotHint(const struct SubplotDesc *desc, u32 subplotStage, u32 beat, u32 distance, u32 selector);

// The NPC-talk query: the hint about the beat the (county, slot) subplot is currently in. NULL = generic text.
const u8 *GetCountySlotHint(u32 countyId, u32 slot, u32 distance, u32 selector);

// As above for any beat, e.g. an NPC recalling a stage the subplot has passed.
const u8 *GetCountySlotHintAbout(u32 countyId, u32 slot, u32 beat, u32 distance, u32 selector);

// Actors this subplot wants present at `stage`: fills `out` (up to maxOut) and returns the total count.
u32 GetSubplotActors(const struct SubplotDesc *desc, u32 stage, const struct SubplotActor **out, u32 maxOut);

// County whose city has this MAPSEC; asserts there is none.
u32 GetCountyFromCityMapSec(u32 mapSec);

#endif // GUARD_SUBPLOT_H
