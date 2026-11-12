#ifndef GUARD_COUNTY_CHAIN_H
#define GUARD_COUNTY_CHAIN_H

/**
 * The county chain (design.md, "County chain"): a county's progress through the subplots that fulfil its
 * main-story slots, stored as one position in var 0 of its slice.
 *
 * A chain is an ordered list of entries, each a subplot (by its subplot slot, vars 1-3 of the slice) that
 * supplies one main-story slot, with the number of stages it contributes. The position counts through the
 * concatenated stages of all entries, so it only ever advances by one, and ResolveChainPosition turns it
 * back into (entry, stage). ResolveChainPosition and the other Chain* functions are pure, so an adapter
 * can feed them a position computed from Emerald's old vars; only Get/Set/AdvanceCountyChainPosition touch
 * the stored var.
 *
 * Two kinds of "slot" exist: a subplot slot addresses a county's subplot vars (see subplot.h), a main-story
 * slot (0 to COUNTY_MAIN_SLOTS - 1, the `s` of main-story-index `4x + s`) is a step of the main plot a county
 * hosts. A chain covers only the main-story slots its county fulfils itself: slot 1 is usually a callback
 * hosted by another county and has no entries, which the functions here accept.
 *
 * Only counties whose slice is free of legacy vars (COUNTY_VAR_SLICES_FREE) can hold a stored position.
 */

#include "global.h"

#define COUNTY_MAIN_SLOTS 4

struct CountyChainEntry
{
    u8 mainSlot;    // main-story slot this entry helps fulfil; entries are ordered by non-decreasing mainSlot
    u8 subplotSlot; // the county's subplot slot that runs the entry
    u8 numStages;   // stages this entry adds to the position (>= 1): the subplot's stages without before/after pseudo-stages
};

enum ChainState
{
    CHAIN_NOT_STARTED, // position 0 of a chain: its first stage is current, nothing is done
    CHAIN_IN_PROGRESS,
    CHAIN_COMPLETE,    // position past the last stage: every entry is done and entry/stage are not meaningful
};

struct ChainPosition
{
    u8 state;
    u8 entry;       // index into the chain; numEntries when complete
    u8 stage;       // stage within that entry
    u8 mainSlot;    // the entry's mainSlot; COUNTY_MAIN_SLOTS when complete
};

// Sum of the entries' stages: the position of a finished chain.
u32 GetChainLength(const struct CountyChainEntry *chain, u32 numEntries);

// Position `position` mapped to the entry and the stage it falls in. A position past the end is
// CHAIN_COMPLETE, not an error, because the stored position stays there once the chain is finished.
struct ChainPosition ResolveChainPosition(const struct CountyChainEntry *chain, u32 numEntries, u32 position);

// The position at which the entry's stage 0 is current.
u32 GetChainEntryStart(const struct CountyChainEntry *chain, u32 numEntries, u32 entry);

// TRUE iff every entry of the main-story slot is done at `position`. A slot with no entries is never done
// here (another county's callback fulfils it), so callers must not ask about slots the chain does not cover.
bool32 IsChainMainSlotDone(const struct CountyChainEntry *chain, u32 numEntries, u32 position, u32 mainSlot);

// TRUE iff `position` is exactly where the main-story slot's last stage has just been completed, i.e. the
// position of the next entry's stage 0 or the end of the chain. This is when the global main-story-index
// would advance to the next step.
bool32 DidChainJustFinishMainSlot(const struct CountyChainEntry *chain, u32 numEntries, u32 position, u32 mainSlot);

// The stored position, var 0 of the county's slice.
u32 GetCountyChainPosition(u32 countyId);
void SetCountyChainPosition(u32 countyId, u32 position);

// Moves the stored position forward by one stage and returns the new position. Asserts at the end of the chain.
u32 AdvanceCountyChainPosition(u32 countyId, const struct CountyChainEntry *chain, u32 numEntries);

#endif // GUARD_COUNTY_CHAIN_H
