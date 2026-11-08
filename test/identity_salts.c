#include "global.h"
#include "test/test.h"
#include "constants/identity_salts.h"

// Each salt with the number of consecutive values it owns (a salt used as `SALT + n` owns n + 1).
static const u8 sSalts[][2] =
{
    {REGION_SALT_ROUTE_BEND, 2},
    {REGION_SALT_ROUTE_CROSSING, 2},
    {REGION_SALT_ROUTE_INTRA, 1},
    {REGION_SALT_PITSTOP, 1},
    {REGION_SALT_PITSTOP_DEAL, 1},
    {REGION_SALT_MCGUFFIN_STRIDE, 1},
    {REGION_SALT_MCGUFFIN_CROSS, 1},
    {NPC_SALT_NEWS_GENERIC, 1},
};

TEST("IdentitySalts: No two salts share a value")
{
    u32 a, b;

    for (a = 0; a < ARRAY_COUNT(sSalts); a++)
    {
        for (b = a + 1; b < ARRAY_COUNT(sSalts); b++)
        {
            bool32 overlap = sSalts[a][0] < sSalts[b][0] + sSalts[b][1] && sSalts[b][0] < sSalts[a][0] + sSalts[a][1];

            EXPECT(!overlap);
        }
    }
}
