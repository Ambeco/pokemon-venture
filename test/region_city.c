#include "global.h"
#include "constants/characters.h"
#include "string_util.h"
#include "test/test.h"
#include "region_city.h"
#include "region_tile.h"

// Invariants are written over the accessors so they become seed-sweep tests once the generator
// exists. Tests named "Emerald" check hardcoded data only and must not be ported to the sweep.

static bool32 IsCoastal(u32 countyId)
{
    u32 other;

    if (GetCountyHeader(countyId)->terrain == COUNTY_TERRAIN_OCEAN)
        return TRUE;
    for (other = 0; other < NUM_COUNTIES; other++)
    {
        if (AreCountiesAdjacent(countyId, other) && GetCountyHeader(other)->terrain == COUNTY_TERRAIN_OCEAN)
            return TRUE;
    }
    return FALSE;
}

TEST("RegionCity: Gym ordinals 1-9 are each used exactly once, ordinal 9 only in the Elite Four county")
{
    u32 id, ordinal, used[NUM_GYM_ORDINALS + 1] = {0};

    for (id = 0; id < NUM_COUNTIES; id++)
    {
        ordinal = GetCountyGymOrdinal(id);
        EXPECT_LE(ordinal, NUM_GYM_ORDINALS);
        used[ordinal]++;
        EXPECT_EQ(ordinal == ELITE_FOUR_ORDINAL, IsCountyEliteFour(id));
        if (ordinal != 0)
        {
            EXPECT_EQ(IsCountyReserved(id), FALSE);
            EXPECT_EQ(DoesCountyHaveCity(id), TRUE);
        }
    }
    for (ordinal = 1; ordinal <= NUM_GYM_ORDINALS; ordinal++)
    {
        EXPECT_EQ(used[ordinal], 1);
        EXPECT_EQ(GetCountyGymOrdinal(GetGymCounty(ordinal)), ordinal);
    }
}

TEST("RegionCity: A county has a gym iff its city has the gym building, and the Elite Four has the league instead")
{
    u32 id;

    for (id = 0; id < NUM_COUNTIES; id++)
    {
        u32 ordinal = GetCountyGymOrdinal(id);

        if (!DoesCountyHaveCity(id))
        {
            EXPECT_EQ(ordinal, 0);
            continue;
        }
        EXPECT_EQ(DoesCountyCityHaveBuilding(id, BUILDING_GYM), ordinal >= 1 && ordinal < ELITE_FOUR_ORDINAL);
        EXPECT_EQ(DoesCountyCityHaveBuilding(id, BUILDING_LEAGUE), ordinal == ELITE_FOUR_ORDINAL);
    }
}

TEST("RegionCity: Every city but the starting town has a Pokemon Center, and the start has home healing and a bedroom PC")
{
    u32 id;

    for (id = 0; id < NUM_COUNTIES; id++)
    {
        if (!DoesCountyHaveCity(id))
            continue;
        if (GetCountyHeader(id)->storyIdx == 0)
        {
            EXPECT_EQ(DoesCountyCityHaveBuilding(id, BUILDING_POKEMON_CENTER), FALSE);
            EXPECT_EQ(DoesCountyCityHaveBuilding(id, BUILDING_HOME_HEALING), TRUE);
            EXPECT_EQ(DoesCountyCityHaveBuilding(id, BUILDING_BEDROOM_PC), TRUE);
        }
        else
        {
            EXPECT_EQ(DoesCountyCityHaveBuilding(id, BUILDING_POKEMON_CENTER), TRUE);
            EXPECT_EQ(DoesCountyCityHaveBuilding(id, BUILDING_HOME_HEALING), FALSE);
            EXPECT_EQ(DoesCountyCityHaveBuilding(id, BUILDING_BEDROOM_PC), FALSE);
        }
    }
}

TEST("RegionCity: Inside-city special locations share the city's tile and outside-city ones are route tiles")
{
    u32 id, i;

    for (id = 0; id < NUM_COUNTIES; id++)
    {
        for (i = 0; i < GetCountySpecialLocationCount(id); i++)
        {
            struct SpecialLocation loc = GetCountySpecialLocation(id, i);
            u32 flags = GetRegionTileFlags(loc.pos.x, loc.pos.y);

            EXPECT_LT(loc.kind, NUM_LOCATION_KINDS);
            if (IsSpecialLocationKindInsideCity(loc.kind))
            {
                struct RegionTilePos city;

                EXPECT_EQ(DoesCountyHaveCity(id), TRUE);
                city = GetCountyCityPos(id);
                EXPECT_EQ(loc.pos.x, city.x);
                EXPECT_EQ(loc.pos.y, city.y);
                EXPECT_EQ(flags, TILE_CITY | TILE_SPECIAL);
            }
            else
            {
                EXPECT_EQ(flags, loc.locationClass == LOCATION_CLASS_ENCLAVE ? TILE_SPECIAL : (TILE_ROUTE | TILE_SPECIAL));
            }
        }
    }
}

TEST("RegionCity: There are 2 or 3 ports, each in a city on a coastal or ocean county")
{
    u32 id, ports = 0;

    for (id = 0; id < NUM_COUNTIES; id++)
    {
        if (!GetCountyHeader(id)->port)
            continue;
        ports++;
        EXPECT_EQ(DoesCountyHaveCity(id), TRUE);
        EXPECT_EQ(IsCoastal(id), TRUE);
    }
    EXPECT_GE(ports, 2);
    EXPECT_LE(ports, 3);
}

TEST("RegionCity: Every city has a theme and a name")
{
    u32 id;

    for (id = 0; id < NUM_COUNTIES; id++)
    {
        if (!DoesCountyHaveCity(id))
            continue;
        EXPECT_GT(GetCountyCityTheme(id), CITY_THEME_NONE);
        EXPECT_LT(GetCountyCityTheme(id), NUM_CITY_THEMES);
        EXPECT_NE(GetCountyCityName(id)[0], EOS);
    }
}

TEST("RegionCity: Emerald cities have distinct themes and their Emerald names")
{
    u32 a, b;

    for (a = 0; a < NUM_COUNTIES; a++)
    {
        if (!DoesCountyHaveCity(a))
            continue;
        for (b = a + 1; b < NUM_COUNTIES; b++)
        {
            if (DoesCountyHaveCity(b))
                EXPECT_NE(GetCountyCityTheme(a), GetCountyCityTheme(b));
        }
    }
    EXPECT_EQ(GetCountyCityTheme(COUNTY_ID(1, 3)), CITY_THEME_START);
    EXPECT_EQ(StringCompare(GetCountyCityName(COUNTY_ID(0, 1)), COMPOUND_STRING("RUSTBORO CITY")), 0);
}

TEST("RegionCity: Gym slots strictly increase with the ordinal and callbacks come from earlier blocks")
{
    u32 ordinal;

    for (ordinal = 1; ordinal <= NUM_GYM_ORDINALS; ordinal++)
    {
        struct GymSlot slot = GetGymSlot(ordinal);
        u32 hostIdx = GetCountyHeader(GetGymCounty(ordinal))->storyIdx;

        EXPECT_LE(slot.step, 2);
        if (slot.step == 1)
            EXPECT_LT(hostIdx, slot.block);
        else
            EXPECT_EQ(hostIdx, slot.block);
        if (ordinal > 1)
        {
            struct GymSlot prev = GetGymSlot(ordinal - 1);
            EXPECT(slot.block > prev.block || (slot.block == prev.block && slot.step > prev.step));
        }
    }
}

TEST("RegionCity: Emerald Surf block 11 gives gym 5 as a Petalburg callback beside gym 4, gym 6 is in Fortree, and no ocean county follows")
{
    struct GymSlot g5 = GetGymSlot(5);
    struct GymSlot g6 = GetGymSlot(6);
    u32 fortreeIdx = GetCountyHeader(GetGymCounty(6))->storyIdx;
    u32 afterTerrain = GetCountyHeader(GetCountyByStoryIdx(12))->terrain;

    EXPECT_EQ(GetGymSlot(4).block, 11);
    EXPECT_EQ(GetGymSlot(4).step, 0);
    EXPECT_EQ(g5.block, 11);
    EXPECT_EQ(g5.step, 1);
    EXPECT_EQ(GetGymCounty(5), COUNTY_ID(0, 2));
    EXPECT_EQ(g6.block, 13);
    EXPECT_EQ(g6.step, 2);
    EXPECT_EQ(GetGymCounty(6), COUNTY_ID(3, 1));
    EXPECT_EQ(fortreeIdx, 13);
    EXPECT_NE(afterTerrain, COUNTY_TERRAIN_OCEAN);
}
