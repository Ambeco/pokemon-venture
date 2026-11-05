// Hardcoded Emerald city metadata (design.md, "Development approach", stage 3); the catalogs are in
// Emerald.md. Roles are best fits from the cities' buildings, not final.

struct CountyCity
{
    u8 theme;
    u16 roles;
    u8 gymOrdinal;
    u8 gymType;
    u16 gymLeader;
};

#define CITY_COMMON (BUILDING_POKEMON_CENTER | BUILDING_MART)

#define GYM(ordinal, type, leader) .gymOrdinal = (ordinal), .gymType = (type), .gymLeader = (leader)

// Indexed by COUNTY_ID(x, y); counties without a city are all zero.
static const struct CountyCity sEmeraldCities[NUM_COUNTIES] =
{
    // y=0
    [COUNTY_ID(1, 0)] = {.theme = CITY_THEME_FARMLAND, .roles = CITY_COMMON | BUILDING_SERVICE},
    [COUNTY_ID(2, 0)] = {.theme = CITY_THEME_HOT_SPRINGS,
        .roles = CITY_COMMON | BUILDING_GYM | BUILDING_HERB_SHOP | BUILDING_HOT_SPRINGS,
        GYM(4, TYPE_FIRE, TRAINER_FLANNERY_1)},
    [COUNTY_ID(4, 0)] = {.theme = CITY_THEME_CAPITAL_COAST,
        .roles = CITY_COMMON | BUILDING_MOTEL | BUILDING_FAN_CLUB | BUILDING_SERVICE},
    // y=1
    [COUNTY_ID(0, 1)] = {.theme = CITY_THEME_INDUSTRY, .roles = CITY_COMMON | BUILDING_GYM | BUILDING_SCHOOL,
        GYM(1, TYPE_ROCK, TRAINER_ROXANNE_1)},
    [COUNTY_ID(1, 1)] = {.theme = CITY_THEME_HIGHLAND, .roles = CITY_COMMON},
    [COUNTY_ID(2, 1)] = {.theme = CITY_THEME_ENTERTAINMENT, .roles = CITY_COMMON | BUILDING_GYM,
        GYM(3, TYPE_ELECTRIC, TRAINER_WATTSON_1)},
    [COUNTY_ID(3, 1)] = {.theme = CITY_THEME_TREETOP, .roles = CITY_COMMON | BUILDING_GYM | BUILDING_SERVICE,
        GYM(6, TYPE_FLYING, TRAINER_WINONA_1)},
    [COUNTY_ID(5, 1)] = {.theme = CITY_THEME_CHERISH, .roles = CITY_COMMON | BUILDING_GYM,
        GYM(7, TYPE_PSYCHIC, TRAINER_TATE_AND_LIZA_1)},
    // y=2
    [COUNTY_ID(0, 2)] = {.theme = CITY_THEME_NATURE, .roles = CITY_COMMON | BUILDING_GYM,
        GYM(5, TYPE_NORMAL, TRAINER_NORMAN_1)},
    [COUNTY_ID(1, 2)] = {.theme = CITY_THEME_SCARCE, .roles = CITY_COMMON},
    [COUNTY_ID(4, 2)] = {.theme = CITY_THEME_ANCIENT, .roles = CITY_COMMON | BUILDING_GYM,
        GYM(8, TYPE_WATER, TRAINER_JUAN_1)},
    [COUNTY_ID(5, 2)] = {.theme = CITY_THEME_LEAGUE, .roles = CITY_COMMON | BUILDING_LEAGUE,
        GYM(ELITE_FOUR_ORDINAL, TYPE_NONE, TRAINER_WALLACE)},
    // y=3
    [COUNTY_ID(0, 3)] = {.theme = CITY_THEME_TINY_ISLAND,
        .roles = BUILDING_POKEMON_CENTER | BUILDING_GYM | BUILDING_HALL,
        GYM(2, TYPE_FIGHTING, TRAINER_BRAWLY_1)},
    [COUNTY_ID(1, 3)] = {.theme = CITY_THEME_START, .roles = BUILDING_HOME_HEALING | BUILDING_BEDROOM_PC},
    [COUNTY_ID(2, 3)] = {.theme = CITY_THEME_PORT,
        .roles = CITY_COMMON | BUILDING_MARKET | BUILDING_FAN_CLUB | BUILDING_SERVICE},
    [COUNTY_ID(4, 3)] = {.theme = CITY_THEME_OCEAN_SUNRISE, .roles = BUILDING_POKEMON_CENTER | BUILDING_SERVICE},
};

// Planner inputs, indexed by gym ordinal - 1. Block = county-story-index. Gyms 4 and 5 are the
// Emerald-only Surf exception: both in the Surf block 11, Petalburg's as the step-1 callback.
static const struct GymSlot sEmeraldGymSlots[NUM_GYM_ORDINALS] =
{
    {3, 0},   // 1 Rustboro
    {4, 0},   // 2 Dewford
    {7, 2},   // 3 Mauville
    {11, 0},  // 4 Lavaridge
    {11, 1},  // 5 Petalburg (callback)
    {13, 2},  // 6 Fortree
    {17, 0},  // 7 Mossdeep
    {19, 1},  // 8 Sootopolis (callback from Pacifidlog's block, after Rayquaza ends the fight)
    {20, 2},  // 9 Ever Grande (League lobby)
};
