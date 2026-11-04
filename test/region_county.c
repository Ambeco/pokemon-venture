#include "global.h"
#include "test/test.h"
#include "region_county.h"

// Invariants are written over GetRegionHeader() so they become seed-sweep tests once the generator exists.

TEST("RegionCounty: CountyHeader is 3 bytes")
{
    EXPECT_EQ(sizeof(struct CountyHeader), 3);
}

TEST("RegionCounty: County story indices are unique and cover 0..NUM_COUNTIES-1")
{
    u32 seen[NUM_COUNTIES] = {0};
    u32 i;

    for (i = 0; i < NUM_COUNTIES; i++)
    {
        u32 idx = GetCountyHeader(i)->storyIdx;
        ASSUME(idx < NUM_COUNTIES);
        EXPECT_EQ(seen[idx], 0);
        seen[idx]++;
        EXPECT_EQ(GetCountyByStoryIdx(idx), i);
    }
}

TEST("RegionCounty: Every county above story index 0 has an adjacent lower index (growth order)")
{
    u32 a, b;

    for (a = 0; a < NUM_COUNTIES; a++)
    {
        bool32 found = GetCountyHeader(a)->storyIdx == 0;
        for (b = 0; b < NUM_COUNTIES && !found; b++)
            found = AreCountiesAdjacent(a, b) && GetCountyHeader(b)->storyIdx < GetCountyHeader(a)->storyIdx;
        EXPECT_EQ(found, TRUE);
    }
}

TEST("RegionCounty: Reserved counties are the last R indices and the Elite Four precedes them")
{
    u32 reserved = 0, elite = 0, i;

    EXPECT(GetRegionHeader()->reservedCount == 3 || GetRegionHeader()->reservedCount == 4);
    for (i = 0; i < NUM_COUNTIES; i++)
    {
        reserved += IsCountyReserved(i);
        elite += IsCountyEliteFour(i);
    }
    EXPECT_EQ(reserved, GetRegionHeader()->reservedCount);
    EXPECT_EQ(elite, 1);
}

TEST("RegionCounty: Each key's county precedes its lock's county, and ids are paired once")
{
    u32 id, i;

    for (id = 1; id < 8; id++)
    {
        u32 keys = 0, locks = 0, keyIdx = 0, lockIdx = 0;
        for (i = 0; i < NUM_COUNTIES; i++)
        {
            if (GetCountyHeader(i)->keyId == id)
            {
                keys++;
                keyIdx = GetCountyHeader(i)->storyIdx;
            }
            if (GetCountyLockId(i) == id)
            {
                locks++;
                lockIdx = GetCountyHeader(i)->storyIdx;
            }
        }
        EXPECT_EQ(keys, locks);
        EXPECT(keys <= 1);
        if (keys == 1)
            EXPECT(keyIdx < lockIdx);
    }
}

TEST("RegionCounty: Every non-adjacent consecutive walk step has a key and lock")
{
    u32 idx;

    // Walk indices end at the Elite Four; its unlock and the reserved counties follow other rules.
    for (idx = 1; idx < GetEliteFourStoryIdx(); idx++)
    {
        u32 prev = GetCountyByStoryIdx(idx - 1);
        u32 cur = GetCountyByStoryIdx(idx);
        if (!AreCountiesAdjacent(prev, cur))
        {
            EXPECT(GetCountyHeader(prev)->keyId != 0);
            EXPECT_EQ((u32)GetCountyHeader(prev)->keyId, GetCountyLockId(cur));
        }
    }
}

TEST("RegionCounty: The capital has population 3 and is on land")
{
    const struct CountyHeader *capital = GetCountyHeader(GetRegionHeader()->capital);

    EXPECT_EQ((u32)capital->population, 3);
    EXPECT(capital->terrain != COUNTY_TERRAIN_OCEAN);
}

TEST("RegionCounty: Counties 0 and 1 are adjacent and both have settlements")
{
    u32 zero = GetCountyByStoryIdx(0), one = GetCountyByStoryIdx(1);

    EXPECT_EQ(AreCountiesAdjacent(zero, one), TRUE);
    EXPECT(GetCountyHeader(zero)->population > 0);
    EXPECT(GetCountyHeader(one)->population > 0);
}

TEST("RegionCounty: Settlement quota: story 0, 1 and the Elite Four plus 8 to 13 more")
{
    u32 settlements = 0, i;

    for (i = 0; i < NUM_COUNTIES; i++)
        settlements += GetCountyHeader(i)->population > 0;
    EXPECT(GetCountyHeader(GetCountyByStoryIdx(GetEliteFourStoryIdx()))->population > 0);
    EXPECT(settlements >= 11 && settlements <= 16);
}

TEST("RegionCounty: At least 10 counties are not ocean")
{
    u32 land = 0, i;

    for (i = 0; i < NUM_COUNTIES; i++)
        land += GetCountyHeader(i)->terrain != COUNTY_TERRAIN_OCEAN;
    EXPECT(land >= 10);
}

TEST("RegionCounty: Spare header bits are zero")
{
    u32 i;

    for (i = 0; i < NUM_COUNTIES; i++)
        EXPECT_EQ((u32)GetCountyHeader(i)->reserved, 0);
}

TEST("RegionCounty: Enclave-only counties are reserved, cityless, and not the capital")
{
    u32 i;

    for (i = 0; i < NUM_COUNTIES; i++)
    {
        if (!IsCountyEnclaveOnly(i))
            continue;
        EXPECT(IsCountyReserved(i));
        EXPECT_EQ((u32)GetCountyHeader(i)->population, 0);
        EXPECT(i != GetRegionHeader()->capital);
    }
}

TEST("RegionCounty: Biomes expand through the subset into the 25-biome grid")
{
    const struct RegionHeader *region = GetRegionHeader();
    u32 i;

    EXPECT(region->biomeHumidityBase + BIOME_SUBSET_HUMIDITY_LEVELS <= 5);
    EXPECT(region->biomeTemperatureBase + BIOME_SUBSET_TEMPERATURE_LEVELS <= 5);
    for (i = 0; i < NUM_COUNTIES; i++)
    {
        u32 biome = GetCountyBiome(i);
        EXPECT(GetCountyHeader(i)->biome < NUM_BIOMES_IN_SUBSET);
        EXPECT(BIOME_HUMIDITY(biome) >= region->biomeHumidityBase);
        EXPECT(BIOME_HUMIDITY(biome) < region->biomeHumidityBase + BIOME_SUBSET_HUMIDITY_LEVELS);
    }
    // Littleroot's county: Temperate fresh-water swamp forest.
    EXPECT_EQ(GetCountyBiome(COUNTY_ID(1, 3)), BIOME_TEMPERATE_FRESHWATER_SWAMP_FOREST);
    EXPECT_EQ(GetCountyBiome(COUNTY_ID(0, 0)), BIOME_SAVANNA);
}

TEST("RegionCounty: Ports: at most three, none in the starting county")
{
    u32 ports = 0, i;

    for (i = 0; i < NUM_COUNTIES; i++)
        ports += GetCountyHeader(i)->port;
    EXPECT(ports <= 3);
    EXPECT_EQ((u32)GetCountyHeader(GetCountyByStoryIdx(0))->port, 0);
}

TEST("RegionCounty: AreCountiesAdjacent is orthogonal only and does not wrap rows")
{
    EXPECT_EQ(AreCountiesAdjacent(COUNTY_ID(1, 1), COUNTY_ID(2, 1)), TRUE);
    EXPECT_EQ(AreCountiesAdjacent(COUNTY_ID(1, 1), COUNTY_ID(1, 2)), TRUE);
    EXPECT_EQ(AreCountiesAdjacent(COUNTY_ID(1, 1), COUNTY_ID(2, 2)), FALSE);
    EXPECT_EQ(AreCountiesAdjacent(COUNTY_ID(5, 0), COUNTY_ID(0, 1)), FALSE);
    EXPECT_EQ(AreCountiesAdjacent(COUNTY_ID(1, 1), COUNTY_ID(1, 1)), FALSE);
}

TEST("RegionCounty: County region bounds tile the 28x15 region map exactly")
{
    u32 covered[15][28] = {{0}};
    u32 id, x, y;

    for (id = 0; id < NUM_COUNTIES; id++)
    {
        u32 l, t, r, b;
        GetCountyRegionBounds(id, &l, &t, &r, &b);
        for (y = t; y <= b; y++)
        {
            for (x = l; x <= r; x++)
            {
                covered[y][x]++;
                EXPECT_EQ(GetCountyFromRegionCell(x, y), id);
            }
        }
    }
    for (y = 0; y < 15; y++)
        for (x = 0; x < 28; x++)
            EXPECT_EQ(covered[y][x], 1);
}

TEST("RegionCounty: County region bounds: edge counties are one tile smaller")
{
    u32 l, t, r, b;

    GetCountyRegionBounds(COUNTY_ID(0, 0), &l, &t, &r, &b);
    EXPECT_EQ(l, 0); EXPECT_EQ(t, 0); EXPECT_EQ(r, 3); EXPECT_EQ(b, 2);
    GetCountyRegionBounds(COUNTY_ID(2, 1), &l, &t, &r, &b);
    EXPECT_EQ(l, 9); EXPECT_EQ(t, 3); EXPECT_EQ(r, 13); EXPECT_EQ(b, 6);
    GetCountyRegionBounds(COUNTY_ID(5, 3), &l, &t, &r, &b);
    EXPECT_EQ(l, 24); EXPECT_EQ(t, 11); EXPECT_EQ(r, 27); EXPECT_EQ(b, 14);
}
