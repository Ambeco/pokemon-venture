#include "global.h"
#include "test/test.h"
#include "county_position.h"

// Unsorted chapters 0 (Aqua), 1 (Magma), 2 (finale); the required order is Magma, Aqua, finale.
static const u8 sOrder[] = {1, 0, 2};
#define N ARRAY_COUNT(sOrder)

TEST("CountyPosition: Fields pack into var 0's bits and unpack again")
{
    u16 p = PackCountyPosition(0xA, 0x5, 0xC3);

    EXPECT_EQ(p, 0xA5C3);
    EXPECT_EQ(GetCountyPosChapter(p), 0xA);
    EXPECT_EQ(GetCountyPosStage(p), 0x5);
    EXPECT_EQ(GetCountyPosState(p), 0xC3);
}

TEST("CountyPosition: Setting the stage or the state keeps the other fields")
{
    u16 p = PackCountyPosition(3, 4, 5);

    EXPECT_EQ(SetCountyPosStage(p, 9), PackCountyPosition(3, 9, 5));
    EXPECT_EQ(SetCountyPosState(p, 200), PackCountyPosition(3, 4, 200));
}

TEST("CountyPosition: The largest chapter, stage and state fit")
{
    u16 p = PackCountyPosition(15, 15, 255);

    EXPECT_EQ(p, 0xFFFF);
    EXPECT_EQ(GetCountyPosChapter(p), 15);
    EXPECT_EQ(GetCountyPosStage(p), 15);
    EXPECT_EQ(GetCountyPosState(p), 255);
}

TEST("CountyPosition: Starting writes the unsorted index of the order's first chapter, not 0")
{
    u16 p = 0xFFFF;

    EXPECT(StartCountyPosition(&p, sOrder, N, 0));
    EXPECT_EQ(p, PackCountyPosition(1, 0, 0));
}

TEST("CountyPosition: Starting skips chapters that are already done")
{
    u16 p = 0;

    EXPECT(StartCountyPosition(&p, sOrder, N, 1 << 1));
    EXPECT_EQ(GetCountyPosChapter(p), 0);
    EXPECT(StartCountyPosition(&p, sOrder, N, 1 << 1 | 1 << 0));
    EXPECT_EQ(GetCountyPosChapter(p), 2);
}

TEST("CountyPosition: Starting a slot whose chapters are all done, or empty, fails and leaves the position")
{
    u16 p = 0x1234;

    EXPECT(!StartCountyPosition(&p, sOrder, N, 0x7));
    EXPECT(!StartCountyPosition(&p, sOrder, 0, 0));
    EXPECT_EQ(p, 0x1234);
}

TEST("CountyPosition: Advancing follows the order and clears stage and state")
{
    u16 p = PackCountyPosition(1, 3, 77);

    EXPECT(AdvanceCountyPosition(&p, sOrder, N, 0));
    EXPECT_EQ(p, PackCountyPosition(0, 0, 0));
    EXPECT(AdvanceCountyPosition(&p, sOrder, N, 0));
    EXPECT_EQ(p, PackCountyPosition(2, 0, 0));
}

TEST("CountyPosition: Advancing past the last chapter fails and leaves the position")
{
    u16 p = PackCountyPosition(2, 1, 9);

    EXPECT(!AdvanceCountyPosition(&p, sOrder, N, 0));
    EXPECT_EQ(p, PackCountyPosition(2, 1, 9));
}

TEST("CountyPosition: Aqua done first, so finishing Magma advances twice")
{
    u16 p = PackCountyPosition(1, 2, 0);

    EXPECT(AdvanceCountyPosition(&p, sOrder, N, 1 << 0));
    EXPECT_EQ(GetCountyPosChapter(p), 2);
}

TEST("CountyPosition: Finishing the last undone chapter with only done ones after it ends the slot")
{
    u16 p = PackCountyPosition(0, 1, 0);

    EXPECT(!AdvanceCountyPosition(&p, sOrder, N, 1 << 2));
}

TEST("CountyPosition: Bit masks are handed out in order from var 1")
{
    u32 cursor = 0;
    struct CountyBitMask a, b;

    EXPECT(AllocCountyBits(&cursor, 3, &a));
    EXPECT(AllocCountyBits(&cursor, 2, &b));
    EXPECT_EQ(a.var, 1);
    EXPECT_EQ(a.shift, 0);
    EXPECT_EQ(b.var, 1);
    EXPECT_EQ(b.shift, 3);
    EXPECT_EQ(cursor, 5);
}

TEST("CountyPosition: A mask that does not fit the rest of its var starts the next var")
{
    u32 cursor = 0;
    struct CountyBitMask a, b, c;

    EXPECT(AllocCountyBits(&cursor, 14, &a));
    EXPECT(AllocCountyBits(&cursor, 3, &b));
    EXPECT_EQ(b.var, 2);
    EXPECT_EQ(b.shift, 0);
    EXPECT(AllocCountyBits(&cursor, 2, &c));
    EXPECT_EQ(c.var, 2);
    EXPECT_EQ(c.shift, 3);
}

TEST("CountyPosition: Vars 1-3 hold 48 bits and a fourth var is refused")
{
    u32 cursor = 0;
    struct CountyBitMask m;

    EXPECT(AllocCountyBits(&cursor, 16, &m));
    EXPECT(AllocCountyBits(&cursor, 16, &m));
    EXPECT(AllocCountyBits(&cursor, 16, &m));
    EXPECT_EQ(m.var, 3);
    EXPECT(!AllocCountyBits(&cursor, 1, &m));
    EXPECT_EQ(cursor, 48);
}

TEST("CountyPosition: Masked bits read and write without disturbing their neighbours")
{
    struct CountyBitMask m = {1, 4, 3};
    u16 v = 0xFFFF;

    v = SetCountyMaskedBits(v, &m, 0);
    EXPECT_EQ(v, 0xFF8F);
    EXPECT_EQ(GetCountyMaskedBits(v, &m), 0);
    v = SetCountyMaskedBits(v, &m, 5);
    EXPECT_EQ(v, 0xFFDF);
    EXPECT_EQ(GetCountyMaskedBits(v, &m), 5);
}
