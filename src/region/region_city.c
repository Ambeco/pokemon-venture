#include "global.h"
#include "constants/opponents.h"
#include "region_city.h"
#include "region_map.h"
#include "region_tile.h"

#include "data/region/emerald_cities.h"

static const struct CountyCity *GetCountyCity(u32 countyId)
{
    AGB_ASSERT(countyId < NUM_COUNTIES);
    AGB_ASSERT(DoesCountyHaveCity(countyId));
    return &sEmeraldCities[countyId];
}

u32 GetCountyCityTheme(u32 countyId)
{
    return GetCountyCity(countyId)->theme;
}

bool32 DoesCountyCityHaveBuilding(u32 countyId, u32 role)
{
    AGB_ASSERT(role != 0 && (role & (role - 1)) == 0);
    return (GetCountyCity(countyId)->roles & role) != 0;
}

u32 GetCountyGymOrdinal(u32 countyId)
{
    AGB_ASSERT(countyId < NUM_COUNTIES);
    return sEmeraldCities[countyId].gymOrdinal;
}

enum Type GetCountyGymType(u32 countyId)
{
    AGB_ASSERT(GetCountyGymOrdinal(countyId) != 0);
    return sEmeraldCities[countyId].gymType;
}

u16 GetCountyGymLeader(u32 countyId)
{
    AGB_ASSERT(GetCountyGymOrdinal(countyId) != 0);
    return sEmeraldCities[countyId].gymLeader;
}

const u8 *GetCountyCityName(u32 countyId)
{
    return gRegionMapEntries[GetCountyCityMapSec(countyId)].name;
}

u32 GetGymCounty(u32 ordinal)
{
    u32 countyId;

    AGB_ASSERT(ordinal >= 1 && ordinal <= NUM_GYM_ORDINALS);
    for (countyId = 0; countyId < NUM_COUNTIES; countyId++)
    {
        if (sEmeraldCities[countyId].gymOrdinal == ordinal)
            return countyId;
    }
    AGB_ASSERT(FALSE);
    return 0;
}

struct GymSlot GetGymSlot(u32 ordinal)
{
    AGB_ASSERT(ordinal >= 1 && ordinal <= NUM_GYM_ORDINALS);
    return sEmeraldGymSlots[ordinal - 1];
}
