#include "global.h"
#include "test/test.h"
#include "constants/vars.h"
#include "county_chain.h"
#include "event_data.h"
#include "subplot.h"

// design.md's worked example: slot 0 is Nugget Bridge (1), Copycat Girl (1) and the Rocket hideout (3);
// slot 1 is a callback hosted by another county, so it has no entries; slot 2 is two stages, slot 3 one.
static const struct CountyChainEntry sChain[] =
{
    {0, 0, 1},
    {0, 1, 1},
    {0, 2, 3},
    {2, 0, 2},
    {3, 1, 1},
};

#define N ARRAY_COUNT(sChain)

TEST("CountyChain: The length is the sum of the entries' stages")
{
    EXPECT_EQ(GetChainLength(sChain, N), 8);
    EXPECT_EQ(GetChainLength(sChain, 0), 0);
}

TEST("CountyChain: Positions 0-4 of slot 0 resolve to the worked example")
{
    struct ChainPosition p = ResolveChainPosition(sChain, N, 0);

    EXPECT_EQ(p.state, CHAIN_NOT_STARTED);
    EXPECT_EQ(p.entry, 0);
    EXPECT_EQ(p.stage, 0);
    p = ResolveChainPosition(sChain, N, 1);
    EXPECT_EQ(p.state, CHAIN_IN_PROGRESS);
    EXPECT_EQ(p.entry, 1);
    EXPECT_EQ(p.stage, 0);
    p = ResolveChainPosition(sChain, N, 2);
    EXPECT_EQ(p.entry, 2);
    EXPECT_EQ(p.stage, 0);
    p = ResolveChainPosition(sChain, N, 3);
    EXPECT_EQ(p.entry, 2);
    EXPECT_EQ(p.stage, 1);
    p = ResolveChainPosition(sChain, N, 4);
    EXPECT_EQ(p.entry, 2);
    EXPECT_EQ(p.stage, 2);
    EXPECT_EQ(p.mainSlot, 0);
}

TEST("CountyChain: The position after slot 0 continues in slot 2, skipping the callback slot")
{
    struct ChainPosition p = ResolveChainPosition(sChain, N, 5);

    EXPECT_EQ(p.entry, 3);
    EXPECT_EQ(p.stage, 0);
    EXPECT_EQ(p.mainSlot, 2);
    p = ResolveChainPosition(sChain, N, 7);
    EXPECT_EQ(p.entry, 4);
    EXPECT_EQ(p.mainSlot, 3);
}

TEST("CountyChain: A position at or past the end is complete")
{
    struct ChainPosition p = ResolveChainPosition(sChain, N, 8);

    EXPECT_EQ(p.state, CHAIN_COMPLETE);
    EXPECT_EQ(p.entry, N);
    EXPECT_EQ(p.mainSlot, COUNTY_MAIN_SLOTS);
    EXPECT_EQ(ResolveChainPosition(sChain, N, 200).state, CHAIN_COMPLETE);
    EXPECT_EQ(ResolveChainPosition(sChain, 0, 0).state, CHAIN_COMPLETE);
}

TEST("CountyChain: Entry starts are cumulative stage counts")
{
    EXPECT_EQ(GetChainEntryStart(sChain, N, 0), 0);
    EXPECT_EQ(GetChainEntryStart(sChain, N, 2), 2);
    EXPECT_EQ(GetChainEntryStart(sChain, N, 3), 5);
    EXPECT_EQ(GetChainEntryStart(sChain, N, N), 8);
}

TEST("CountyChain: A main-story slot is done once its last entry's last stage is passed")
{
    EXPECT(!IsChainMainSlotDone(sChain, N, 0, 0));
    EXPECT(!IsChainMainSlotDone(sChain, N, 4, 0));
    EXPECT(IsChainMainSlotDone(sChain, N, 5, 0));
    EXPECT(IsChainMainSlotDone(sChain, N, 8, 0));
    EXPECT(!IsChainMainSlotDone(sChain, N, 6, 2));
    EXPECT(IsChainMainSlotDone(sChain, N, 7, 2));
    EXPECT(!IsChainMainSlotDone(sChain, N, 7, 3));
    EXPECT(IsChainMainSlotDone(sChain, N, 8, 3));
}

TEST("CountyChain: A slot with no entries is never done")
{
    EXPECT(!IsChainMainSlotDone(sChain, N, 0, 1));
    EXPECT(!IsChainMainSlotDone(sChain, N, 8, 1));
    EXPECT(!DidChainJustFinishMainSlot(sChain, N, 5, 1));
}

TEST("CountyChain: Finishing a main-story slot is true only at its exact boundary position")
{
    EXPECT(!DidChainJustFinishMainSlot(sChain, N, 4, 0));
    EXPECT(DidChainJustFinishMainSlot(sChain, N, 5, 0));
    EXPECT(!DidChainJustFinishMainSlot(sChain, N, 6, 0));
    EXPECT(DidChainJustFinishMainSlot(sChain, N, 7, 2));
    EXPECT(DidChainJustFinishMainSlot(sChain, N, 8, 3));
    EXPECT(!DidChainJustFinishMainSlot(sChain, N, 9, 3));
}

TEST("CountyChain: The stored position lives in var 0 of the county's slice")
{
    EXPECT_EQ(GetCountyChainPosition(0), 0);
    SetCountyChainPosition(0, 3);
    EXPECT_EQ(GetCountyChainPosition(0), 3);
    EXPECT_EQ(VarGet(COUNTY_VARS_START), 3);
    EXPECT_EQ(GetCountyChainPosition(1), 0);
    EXPECT_EQ(VarGet(GetCountySlotVar(0, 0)), 0);
}

TEST("CountyChain: Advancing moves the stored position by one up to the end")
{
    u32 i;

    for (i = 1; i <= GetChainLength(sChain, N); i++)
        EXPECT_EQ(AdvanceCountyChainPosition(2, sChain, N), i);
    EXPECT_EQ(GetCountyChainPosition(2), 8);
    EXPECT_EQ(ResolveChainPosition(sChain, N, GetCountyChainPosition(2)).state, CHAIN_COMPLETE);
}

