#ifndef GUARD_REGION_CITY_H
#define GUARD_REGION_CITY_H

/**
 * Layer 3 city metadata: theme, building roles and gym of each county's city.
 *
 * Read-only data keyed by county, like region_tile.h. Stage 3 (hardcoded Emerald) answers from a
 * per-county ROM table; later the same accessors answer from hashes and the main plot planner, so
 * callers (NPC talk, gym scripts) do not change. Everything that needs a city asserts that the
 * county has one (DoesCountyHaveCity), except GetCountyGymOrdinal(), which is 0 for every county
 * without a gym.
 *
 * Gym ordinals are 1-8 for the gyms and 9 for the Elite Four county, whose league building is a
 * BUILDING_LEAGUE, not a BUILDING_GYM; it has no gym type (TYPE_NONE), and its "leader" is the
 * Champion. Ordinals 1-8 are never in the Elite Four or a reserved
 * county. The starting town (county-story-index 0) has no Pokémon Center;
 * its mother heals the party and its bedroom holds the PC.
 *
 * GetGymSlot() is a main plot planner input (design.md, "Main plot planner"): the time block and
 * step in which gym k is given, and the planner replaces it later. A step-1 slot is a callback, so
 * the gym's county lies in an earlier block. Emerald's Surf exception lives here: gyms 5 and 6
 * share block 12 (Fortree), Petalburg's gym as the step-1 callback and Fortree's own as step 2.
 */

#include "global.h"
#include "constants/pokemon.h"
#include "region_county.h"

#define NUM_GYM_ORDINALS   9
#define ELITE_FOUR_ORDINAL 9

enum CityTheme
{
    CITY_THEME_NONE,
    CITY_THEME_START,
    CITY_THEME_SCARCE,
    CITY_THEME_NATURE,
    CITY_THEME_INDUSTRY,
    CITY_THEME_TINY_ISLAND,
    CITY_THEME_PORT,
    CITY_THEME_ENTERTAINMENT,
    CITY_THEME_HIGHLAND,
    CITY_THEME_FARMLAND,
    CITY_THEME_HOT_SPRINGS,
    CITY_THEME_TREETOP,
    CITY_THEME_CAPITAL_COAST,
    CITY_THEME_CHERISH,
    CITY_THEME_ANCIENT,
    CITY_THEME_OCEAN_SUNRISE,
    CITY_THEME_LEAGUE,
    NUM_CITY_THEMES,
};

// Bit set. A special location inside the city supplies its own role (see enum TownLocationKind).
enum BuildingRole
{
    BUILDING_POKEMON_CENTER = 1 << 0,
    BUILDING_MART           = 1 << 1,
    BUILDING_GYM            = 1 << 2,
    BUILDING_LEAGUE         = 1 << 3,
    BUILDING_HOME_HEALING   = 1 << 4,
    BUILDING_BEDROOM_PC     = 1 << 5,
    BUILDING_MARKET         = 1 << 6,
    BUILDING_MOTEL          = 1 << 7,
    BUILDING_HERB_SHOP      = 1 << 8,
    BUILDING_HOT_SPRINGS    = 1 << 9,
    BUILDING_HALL           = 1 << 10,
    BUILDING_FAN_CLUB       = 1 << 11,
    BUILDING_SCHOOL         = 1 << 12,
    BUILDING_SERVICE        = 1 << 13, // name rater, friendship rater, move tutor, reminder, deleter
    BUILDING_HOUSE          = 1 << 14, // generic house
};

#define NUM_BUILDING_ROLES 15

struct GymSlot
{
    u8 block; // county-story-index whose time block gives the gym
    u8 step;  // 0-2; 1 is the callback step
};

// enum CityTheme.
u32 GetCountyCityTheme(u32 countyId);

// TRUE iff the county's city has a building with the given single BuildingRole.
bool32 DoesCountyCityHaveBuilding(u32 countyId, u32 role);

// 0 = no gym, 1-8 = gym, 9 = Elite Four. Valid for every county.
u32 GetCountyGymOrdinal(u32 countyId);

// Asserts that the county has a gym or the Elite Four. TYPE_NONE for the Elite Four.
enum Type GetCountyGymType(u32 countyId);

// Trainer id of the gym leader (the Champion for the Elite Four). Asserts as GetCountyGymType does.
u16 GetCountyGymLeader(u32 countyId);

// City name from the region map's name table; not a copy, so do not write through it.
const u8 *GetCountyCityName(u32 countyId);

// County whose city holds the gym with this ordinal. Asserts that the ordinal is 1-9.
u32 GetGymCounty(u32 ordinal);

struct GymSlot GetGymSlot(u32 ordinal);

#endif // GUARD_REGION_CITY_H
