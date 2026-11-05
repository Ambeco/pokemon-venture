#include "global.h"
#include "identity_seed.h"
#include "region_county.h"
#include "region_pitstop.h"
#include "region_tile.h"

#define SALT_STRIDE      0
#define SALT_OFFSET      1
#define SALT_RANK_STRIDE 2
#define SALT_RANK_OFFSET 3

static u32 Gcd(u32 a, u32 b)
{
    while (b != 0)
    {
        u32 t = a % b;

        a = b;
        b = t;
    }
    return a;
}

static u32 HashOf(u32 which)
{
    return IdentityHash(GetPlayerIdentitySeed(), REGION_SALT_PITSTOP_DEAL, which);
}

// Smallest value from start (wrapping within 1..modulus-1) that is coprime to modulus.
static u32 CoprimeFrom(u32 start, u32 modulus)
{
    u32 i;

    if (modulus <= 2)
        return 1;
    for (i = 0; i < modulus; i++)
    {
        u32 candidate = 1 + (start + i) % (modulus - 1);

        if (Gcd(candidate, modulus) == 1)
            return candidate;
    }
    return 1;
}

u32 GetPitstopDealStride(void)
{
    return CoprimeFrom(HashOf(SALT_STRIDE), GetPitstopPoolCount());
}

u32 GetPitstopDealOffset(void)
{
    u32 pool = GetPitstopPoolCount();

    return pool == 0 ? 0 : HashOf(SALT_OFFSET) % pool;
}

u32 GetCountyPitstopRank(u32 countyId)
{
    u32 stride = CoprimeFrom(HashOf(SALT_RANK_STRIDE), NUM_COUNTIES);

    return (GetCountyHeader(countyId)->storyIdx * stride + HashOf(SALT_RANK_OFFSET) % NUM_COUNTIES) % NUM_COUNTIES;
}

u32 GetCountyPitstopWindow(u32 countyId, u32 *firstSlot)
{
    u32 pool = GetPitstopPoolCount();
    u32 rank = GetCountyPitstopRank(countyId);
    u32 first = pool * rank / NUM_COUNTIES;

    *firstSlot = first;
    return pool * (rank + 1) / NUM_COUNTIES - first;
}

u32 DealCountyPitstops(u32 countyId, u8 *out, u32 maxCount)
{
    u32 pool = GetPitstopPoolCount();
    u32 first, count = GetCountyPitstopWindow(countyId, &first);
    u32 stride = GetPitstopDealStride();
    u32 offset = GetPitstopDealOffset();
    u32 i;

    if (count != 0 && count > GetCountyFreeRouteTileCount(countyId)) // planning routes is slow
        count = GetCountyFreeRouteTileCount(countyId);
    AGB_ASSERT(count <= maxCount); // MAX_DEALT_PITSTOPS is enough for the hardcoded pool
    if (count > maxCount)
        count = maxCount;
    for (i = 0; i < count; i++)
        out[i] = ((first + i) * stride + offset) % pool;
    return count;
}
