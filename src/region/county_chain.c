#include "global.h"
#include "assertf.h"
#include "county_chain.h"
#include "region_county.h"

static void AssertChainValid(const struct CountyChainEntry *chain, u32 numEntries)
{
    u32 i;

    for (i = 0; i < numEntries; i++)
    {
        assertf(chain[i].numAdvances >= 1, "chain entry %d (chapter slot %d) has no stage advances; a chapter needs at least 2 stages, since its last stage means done (did you mean to leave it out of the chain?)", i, chain[i].chapterSlot)
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
        length += chain[i].numAdvances;
    return length;
}

struct ChainPosition ResolveChainPosition(const struct CountyChainEntry *chain, u32 numEntries, u32 position)
{
    struct ChainPosition result;
    u32 i, remaining = position;

    AssertChainValid(chain, numEntries);
    for (i = 0; i < numEntries; i++)
    {
        if (remaining < chain[i].numAdvances)
        {
            result.state = position == 0 ? CHAIN_NOT_STARTED : CHAIN_IN_PROGRESS;
            result.entry = i;
            result.stage = remaining;
            result.mainSlot = chain[i].mainSlot;
            return result;
        }
        remaining -= chain[i].numAdvances;
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
        start += chain[i].numAdvances;
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
