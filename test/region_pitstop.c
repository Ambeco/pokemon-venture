#include "global.h"
#include "test/test.h"
#include "test/seed_sweep.h"
#include "region_county.h"
#include "region_pitstop.h"
#include "region_tile.h"

static u32 Gcd(u32 a, u32 b)
{
    return b == 0 ? a : Gcd(b, a % b);
}

TEST("RegionPitstop: The pool is Emerald's pitstops and the strides are coprime to the sizes")
{
    u32 pool = GetPitstopPoolCount(), i, seed;

    EXPECT_GT(pool, 0);
    for (i = 0; i < pool; i++)
        EXPECT_EQ(GetPitstopPoolEntry(i).locationClass, LOCATION_CLASS_PITSTOP);
    FOR_EACH_SWEEP_SEED(i, seed)
    {
        EXPECT_EQ(Gcd(GetPitstopDealStride(), pool), 1);
        EXPECT_LT(GetPitstopDealOffset(), pool);
    }
}

TEST("RegionPitstop: County ranks are a permutation of the counties")
{
    u32 i, seed;

    FOR_EACH_SWEEP_SEED(i, seed)
    {
        u32 seen = 0, id;

        for (id = 0; id < NUM_COUNTIES; id++)
        {
            u32 rank = GetCountyPitstopRank(id);

            EXPECT_LT(rank, NUM_COUNTIES);
            EXPECT(!(seen & (1u << rank)));
            seen |= 1u << rank;
        }
    }
}

TEST("RegionPitstop: Windows are disjoint and cover the pool, and no pitstop is dealt twice")
{
    u32 pool = GetPitstopPoolCount(), i, seed;

    FOR_EACH_SWEEP_SEED(i, seed)
    {
        u32 id, total = 0, dealt = 0;
        u8 out[MAX_DEALT_PITSTOPS];

        for (id = 0; id < NUM_COUNTIES; id++)
        {
            u32 first, width = GetCountyPitstopWindow(id, &first), count, j;

            EXPECT_LE(width, MAX_DEALT_PITSTOPS);
            total += width;
            count = DealCountyPitstops(id, out, MAX_DEALT_PITSTOPS);
            EXPECT_LE(count, width);
            for (j = 0; j < count; j++)
            {
                EXPECT_LT(out[j], pool);
                EXPECT(!(dealt & (1u << out[j])));
                dealt |= 1u << out[j];
            }
        }
        EXPECT_EQ(total, pool);
    }
}

TEST("RegionPitstop: A county never gets more pitstops than it has free route tiles")
{
    u32 id, i, seed, dealtTotal = 0;
    u8 out[MAX_DEALT_PITSTOPS];

    (void)seed;
    // Each seed replans every county's routes, so a few seeds are enough.
    FOR_EACH_SWEEP_SEED(i, seed)
    {
        if (i >= 6)
            break;
        for (id = 0; id < NUM_COUNTIES; id++)
        {
            u32 count = DealCountyPitstops(id, out, MAX_DEALT_PITSTOPS);

            EXPECT_LE(count, GetCountyFreeRouteTileCount(id));
            dealtTotal += count;
        }
    }
    EXPECT_GT(dealtTotal, 0);
}

TEST("RegionPitstop: The deal depends on the seed")
{
    u32 i, seed, first = 0, different = 0;
    u8 out[MAX_DEALT_PITSTOPS];

    (void)seed;
    FOR_EACH_SWEEP_SEED(i, seed)
    {
        u32 id, signature = 0;

        if (i >= 6)
            break;

        for (id = 0; id < NUM_COUNTIES; id++)
        {
            u32 count = DealCountyPitstops(id, out, MAX_DEALT_PITSTOPS), j;

            for (j = 0; j < count; j++)
                signature = signature * 31 + out[j] + id * 7;
        }
        if (i == 0)
            first = signature;
        else if (signature != first)
            different++;
    }
    EXPECT_GT(different, 0);
}
