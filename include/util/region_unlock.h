#ifndef GUARD_REGION_UNLOCK_H
#define GUARD_REGION_UNLOCK_H

/**
 * Binds the main plot planner's step-3 unlock keys to route lock tiles, and gives each county (and
 * so each Enclave) its unlock point (design.md, "Main plot planner"; Emerald.md, "Main-story-index
 * table").
 *
 * Block x's step 3 (main-story-index 4x + 3) unlocks county x + 1 by a key: the items of block x's
 * lists that the player must hold. Every route into a county opens at once, so the key of a lock
 * depends only on the county it gates (GetRouteLock's gatedCounty), never on the route pair.
 * An HM key is named by its gym, since an HM takes its gym's slot (Surf is gym 5); a PLOT_KIND_GYM
 * key therefore means "badge and HM". An unlock may also be open (no key) or a blocker: a generic
 * NPC from the global blocker pool, not an item.
 *
 * The Elite Four county is unlocked by block 19, and every reserved county by block E (the Hall of
 * Fame), so Enclaves in reserved counties unlock at 4E + 3, below MAIN_STORY_INDEX_POSTGAME.
 *
 * Some vanilla unlocks have no get to name: the player is only expected to have passed somewhere.
 * Those are PLOT_UNLOCK_IMPLICIT, with an enum PlotImplicit saying which vanilla event stands in
 * for the key. An implicit key is not a planner item, so it is exempt from "a key is an item of its
 * own block" (Fortree's stand-in, the Route 119 rival, is a block 12 get).
 *
 * Intra-county locks are the county's private choice (design.md, "Main plot planner", rules) and
 * come back as PLOT_UNLOCK_COUNTY_PRIVATE with the id of a McGuffin from the county's reserved pool.
 *
 * McGuffin ids are one u8 space: [0, NUM_COUNTY_MCGUFFIN_IDS) is split into NUM_COUNTIES pools of
 * MCGUFFIN_IDS_PER_COUNTY by a coprime stride, so a county's ids are scattered and no two counties
 * share one; cross-county ids hash into the CROSS pool above them, so they never meet a county's.
 */

#include "global.h"
#include "region_plot.h"
#include "region_tile.h"

#define PLOT_MAX_UNLOCK_KEYS 2

enum PlotUnlockType
{
    PLOT_UNLOCK_OPEN,           // the step is passed with the block; nothing gates the routes
    PLOT_UNLOCK_KEYS,           // every key item must be held
    PLOT_UNLOCK_BLOCKER,        // a blocker McGuffin from the global pool
    PLOT_UNLOCK_COUNTY_PRIVATE, // intra-county lock: the county decides; mcguffinId is its pool id
    PLOT_UNLOCK_IMPLICIT,       // a vanilla event stands in for the key (enum PlotImplicit)
};

enum PlotImplicit
{
    PLOT_IMPLICIT_NONE,
    PLOT_IMPLICIT_FIERY_PATH_ENTERED, // Fallarbor: the way in is through Fiery Path
    PLOT_IMPLICIT_ROUTE119_RIVAL,     // Fortree: the rival on Route 119 north (Fortree access in the walkthrough)
};

#define MCGUFFIN_IDS_PER_COUNTY    8 // at least MAX_SPECIAL_LOCATIONS_PER_COUNTY
#define NUM_COUNTY_MCGUFFIN_IDS    (NUM_COUNTIES * MCGUFFIN_IDS_PER_COUNTY)
#define NUM_CROSS_MCGUFFIN_IDS     64
#define MCGUFFIN_ID_NONE           0xFF

struct PlotKeyRef
{
    u8 kind; // enum PlotKind
    u8 ordinal;
};

struct PlotUnlock
{
    u8 type; // enum PlotUnlockType
    u8 numKeys;
    u8 implicit;   // enum PlotImplicit, PLOT_UNLOCK_IMPLICIT only
    u8 mcguffinId; // PLOT_UNLOCK_COUNTY_PRIVATE only, else MCGUFFIN_ID_NONE
    struct PlotKeyRef keys[PLOT_MAX_UNLOCK_KEYS];
};

// Unlock of block x (county x + 1), vanilla stage. Valid for x in 0..GetEliteFourStoryIdx().
struct PlotUnlock GetPlotBlockUnlock(u32 block);

// Block whose step 3 unlocks the county: storyIdx - 1, clamped to the Elite Four's index for the
// reserved counties. Asserts the county is not the starting county (storyIdx 0).
u32 GetCountyUnlockBlock(u32 countyId);

// Main-story-index of the county's unlock step (4 * block + 3). The county is reachable once the
// main-story-index is above it. FALSE for the starting county, which is open from the start.
bool32 GetCountyUnlockIndex(u32 countyId, u32 *index);

// TRUE iff the county is open at the main-story-index. Enclaves unlock with their county.
bool32 IsCountyUnlockedAt(u32 countyId, u32 mainStoryIndex);

// Same as GetRouteLock, plus the key bound to it (the unlock of the gated county).
bool32 GetRouteLockKey(u32 countyA, u32 countyB, struct RegionTilePos *pos, u32 *gatedCounty, struct PlotUnlock *key);

// Same as GetCountyIntraRouteLock; the key is always PLOT_UNLOCK_COUNTY_PRIVATE.
bool32 GetCountyIntraRouteLockKey(u32 county, u32 specialIndex, struct RegionTilePos *pos, bool32 *gatesCity, struct PlotUnlock *key);

// The index-th McGuffin id of the county's reserved pool, index < MCGUFFIN_IDS_PER_COUNTY.
u32 GetCountyMcGuffinId(u32 countyId, u32 index);

// Id of a planner item used across counties, hashed from the item; always in the cross pool.
u32 GetPlotItemMcGuffinId(u32 kind, u32 ordinal);

#endif // GUARD_REGION_UNLOCK_H
