#include "global.h"
#include "constants/vars.h"
#include "assertf.h"
#include "county_chain.h"
#include "event_data.h"
#include "subplot.h"

static void AssertChainValid(const struct CountyChainEntry *chain, u32 numEntries)
{
    u32 i;

    for (i = 0; i < numEntries; i++)
    {
        assertf(chain[i].numStages >= 1, "chain entry %d (subplot slot %d) has no stages; did you mean to leave the subplot out of the chain?", i, chain[i].subplotSlot)
        {
            return;
        }
        assertf(chain[i].mainSlot < COUNTY_MAIN_SLOTS, "chain entry %d has main-story slot %d >= %d", i, chain[i].mainSlot, COUNTY_MAIN_SLOTS)
        {
            return;
        }
        assertf(i == 0 || chain[i].mainSlot >= chain[i - 1].mainSlot, "chain entry %d has main-story slot %d after slot %d; entries must be in slot order", i, chain[i].mainSlot, chain[i - 1].mainSlot)
        {
            return;
        }
    }
}

u32 GetChainLength(const struct CountyChainEntry *chain, u32 numEntries)
{
    u32 i, length = 0;

    AssertChainValid(chain, numEntries);
    for (i = 0; i < numEntries; i++)
        length += chain[i].numStages;
    return length;
}

struct ChainPosition ResolveChainPosition(const struct CountyChainEntry *chain, u32 numEntries, u32 position)
{
    struct ChainPosition result;
    u32 i, remaining = position;

    AssertChainValid(chain, numEntries);
    for (i = 0; i < numEntries; i++)
    {
        if (remaining < chain[i].numStages)
        {
            result.state = position == 0 ? CHAIN_NOT_STARTED : CHAIN_IN_PROGRESS;
            result.entry = i;
            result.stage = remaining;
            result.mainSlot = chain[i].mainSlot;
            return result;
        }
        remaining -= chain[i].numStages;
    }
    result.state = CHAIN_COMPLETE;
    result.entry = numEntries;
    result.stage = 0;
    result.mainSlot = COUNTY_MAIN_SLOTS;
    return result;
}

u32 GetChainEntryStart(const struct CountyChainEntry *chain, u32 numEntries, u32 entry)
{
    u32 i, start = 0;

    assertf(entry <= numEntries, "chain entry %d >= %d entries", entry, numEntries)
    {
        return 0;
    }
    for (i = 0; i < entry; i++)
        start += chain[i].numStages;
    return start;
}

// The index one past the last entry of the main-story slot, or -1 if the slot has no entries.
static s32 GetEndEntryOfMainSlot(const struct CountyChainEntry *chain, u32 numEntries, u32 mainSlot)
{
    u32 i;
    s32 end = -1;

    assertf(mainSlot < COUNTY_MAIN_SLOTS, "main-story slot %d >= %d", mainSlot, COUNTY_MAIN_SLOTS)
    {
        return -1;
    }
    for (i = 0; i < numEntries; i++)
    {
        if (chain[i].mainSlot == mainSlot)
            end = i + 1;
    }
    return end;
}

bool32 IsChainMainSlotDone(const struct CountyChainEntry *chain, u32 numEntries, u32 position, u32 mainSlot)
{
    s32 end = GetEndEntryOfMainSlot(chain, numEntries, mainSlot);

    if (end < 0)
        return FALSE;
    return position >= GetChainEntryStart(chain, numEntries, end);
}

bool32 DidChainJustFinishMainSlot(const struct CountyChainEntry *chain, u32 numEntries, u32 position, u32 mainSlot)
{
    s32 end = GetEndEntryOfMainSlot(chain, numEntries, mainSlot);

    if (end < 0)
        return FALSE;
    return position == GetChainEntryStart(chain, numEntries, end);
}

static u16 GetChainVar(u32 countyId)
{
    assertf(countyId < COUNTY_VAR_SLICES_FREE, "county %d slice overlaps legacy vars (only counties 0-%d are free); retire legacy vars first", countyId, COUNTY_VAR_SLICES_FREE - 1)
    {
        return COUNTY_VARS_START;
    }
    return COUNTY_VARS_START + countyId * VARS_PER_COUNTY;
}

u32 GetCountyChainPosition(u32 countyId)
{
    return VarGet(GetChainVar(countyId));
}

void SetCountyChainPosition(u32 countyId, u32 position)
{
    VarSet(GetChainVar(countyId), position);
}

u32 AdvanceCountyChainPosition(u32 countyId, const struct CountyChainEntry *chain, u32 numEntries)
{
    u32 position = GetCountyChainPosition(countyId);

    assertf(position < GetChainLength(chain, numEntries), "county %d chain is already complete at position %d; did you mean to check ResolveChainPosition(...).state first?", countyId, position)
    {
        return position;
    }
    SetCountyChainPosition(countyId, position + 1);
    return position + 1;
}
