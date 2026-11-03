// Hardcoded Emerald tile roles (design.md, "Development approach", stage 2). Which places go in
// which county is in Emerald.md; positions inside a county are placeholders nudged toward Hoenn.
// A special location's kind decides whether it is inside the city (then it must sit on the city's
// tile) or outside (then it is also a route tile). Route tiles come from region_route.c. Landmarks are the places with a map icon in Emerald (Mt. Chimney, Mt. Pyre, Southern Island, Safari Zone, Battle Frontier). Pitstops are the default. Detours are the cityless counties' routed places (Meteor Falls, Shoal Cave, Route 119, Route 117) the story dungeons (Magma and Aqua Hideouts, Sky Pillar, Seafloor Cavern, New Mauville) and a few interesting ones that also give a crowded county more route tiles for its pitstops (Sealed Chamber, Mirage Tower, Desert Ruins).

#define NO_TILE 0xFF

// Local tile (0-4, 0-3) inside a county.
#define T(lx, ly) ((lx) + (ly) * COUNTY_TILES_W)

// Inside-city locations share the city's tile. A pitstop has no stored tile: it is placed on a free
// route tile once the routes are known (GetPitstopLocalTile).
#define SPECIAL(lx, ly, sec, kind)  {T(lx, ly), sec, kind, LOCATION_CLASS_TOWN}
#define LANDMARK(lx, ly, sec, kind) {T(lx, ly), sec, kind, LOCATION_CLASS_LANDMARK}
#define DETOUR(lx, ly, sec, kind)   {T(lx, ly), sec, kind, LOCATION_CLASS_DETOUR}
#define PITSTOP(sec, kind)          {NO_TILE, sec, kind, LOCATION_CLASS_PITSTOP}

struct SpecialTile
{
    u8 tile;
    u8 mapSec;
    u8 kind;
    u8 locationClass;
};

struct CountyTiles
{
    u8 cityTile;
    u8 cityMapSec;
    u8 numSpecials;
    struct SpecialTile specials[MAX_SPECIAL_LOCATIONS_PER_COUNTY];
};

// Indexed by COUNTY_ID(x, y). County (3,2) is deliberately empty (Emerald.md).
static const struct CountyTiles sEmeraldTiles[NUM_COUNTIES] =
{
    // y=0
    [COUNTY_ID(0, 0)] = {NO_TILE, MAPSEC_NONE, 2,
        {DETOUR(3, 1, MAPSEC_METEOR_FALLS, ROUTE_LOCATION_CAVE), PITSTOP(MAPSEC_DESERT_UNDERPASS, ROUTE_LOCATION_CAVE)}},
    [COUNTY_ID(1, 0)] = {T(2, 2), MAPSEC_FALLARBOR_TOWN, 1, {PITSTOP(MAPSEC_ROUTE_113, ROUTE_LOCATION_ROADSIDE_BUILDING)}},
    [COUNTY_ID(2, 0)] = {T(2, 2), MAPSEC_LAVARIDGE_TOWN, 6,
        {LANDMARK(1, 1, MAPSEC_MT_CHIMNEY, ROUTE_LOCATION_VOLCANO), PITSTOP(MAPSEC_FIERY_PATH, ROUTE_LOCATION_TUNNEL),
         PITSTOP(MAPSEC_JAGGED_PASS, ROUTE_LOCATION_MOUNTAIN), DETOUR(4, 1, MAPSEC_MAGMA_HIDEOUT, ROUTE_LOCATION_HIDEOUT),
         DETOUR(0, 2, MAPSEC_MIRAGE_TOWER, ROUTE_LOCATION_TOWER), DETOUR(0, 3, MAPSEC_DESERT_RUINS, ROUTE_LOCATION_RUINS)}},
    [COUNTY_ID(3, 0)] = {T(1, 2), MAPSEC_FORTREE_CITY, 1, {PITSTOP(MAPSEC_ANCIENT_TOMB, ROUTE_LOCATION_RUINS)}},
    [COUNTY_ID(4, 0)] = {T(1, 2), MAPSEC_LILYCOVE_CITY, 3,
        {PITSTOP(MAPSEC_SCORCHED_SLAB, ROUTE_LOCATION_CAVE), LANDMARK(4, 2, MAPSEC_SAFARI_ZONE, ROUTE_LOCATION_PARK),
         DETOUR(3, 3, MAPSEC_AQUA_HIDEOUT, ROUTE_LOCATION_HIDEOUT)}},
    [COUNTY_ID(5, 0)] = {NO_TILE, MAPSEC_NONE, 1, {DETOUR(2, 2, MAPSEC_SHOAL_CAVE, ROUTE_LOCATION_CAVE)}},
    // y=1
    [COUNTY_ID(0, 1)] = {T(2, 1), MAPSEC_RUSTBORO_CITY, 1, {SPECIAL(2, 1, MAPSEC_RUSTBORO_CITY, TOWN_LOCATION_INSTITUTION)}},
    [COUNTY_ID(1, 1)] = {T(3, 2), MAPSEC_VERDANTURF_TOWN, 1, {PITSTOP(MAPSEC_RUSTURF_TUNNEL, ROUTE_LOCATION_TUNNEL)}},
    [COUNTY_ID(2, 1)] = {T(2, 2), MAPSEC_MAUVILLE_CITY, 3,
        {DETOUR(1, 1, MAPSEC_NEW_MAUVILLE, ROUTE_LOCATION_FACILITY), PITSTOP(MAPSEC_TRAINER_HILL, ROUTE_LOCATION_ROADSIDE_BUILDING),
         SPECIAL(2, 2, MAPSEC_MAUVILLE_CITY, TOWN_LOCATION_ENTERTAINMENT)}},
    [COUNTY_ID(3, 1)] = {NO_TILE, MAPSEC_NONE, 2,
        {PITSTOP(MAPSEC_ROUTE_123, ROUTE_LOCATION_ROADSIDE_BUILDING), DETOUR(3, 2, MAPSEC_ROUTE_119, ROUTE_LOCATION_FACILITY)}},
    [COUNTY_ID(4, 1)] = {NO_TILE, MAPSEC_NONE, 1, {LANDMARK(2, 2, MAPSEC_MT_PYRE, ROUTE_LOCATION_MOUNTAIN)}},
    [COUNTY_ID(5, 1)] = {T(2, 1), MAPSEC_MOSSDEEP_CITY, 1, {SPECIAL(2, 1, MAPSEC_MOSSDEEP_CITY, TOWN_LOCATION_INSTITUTION)}},
    // y=2
    [COUNTY_ID(0, 2)] = {T(2, 1), MAPSEC_PETALBURG_CITY, 1, {PITSTOP(MAPSEC_PETALBURG_WOODS, ROUTE_LOCATION_FOREST)}},
    [COUNTY_ID(1, 2)] = {T(1, 1), MAPSEC_OLDALE_TOWN, 1, {PITSTOP(MAPSEC_ALTERING_CAVE, ROUTE_LOCATION_CAVE)}},
    [COUNTY_ID(2, 2)] = {NO_TILE, MAPSEC_NONE, 2,
        {PITSTOP(MAPSEC_ROUTE_110, ROUTE_LOCATION_ROUTE_FEATURE), DETOUR(3, 2, MAPSEC_ROUTE_117, ROUTE_LOCATION_ROADSIDE_BUILDING)}},
    [COUNTY_ID(3, 2)] = {NO_TILE, MAPSEC_NONE, 0, {}},
    [COUNTY_ID(4, 2)] = {T(2, 1), MAPSEC_SOOTOPOLIS_CITY, 1, {SPECIAL(2, 1, MAPSEC_CAVE_OF_ORIGIN, TOWN_LOCATION_SACRED_CAVE)}},
    [COUNTY_ID(5, 2)] = {T(2, 1), MAPSEC_EVER_GRANDE_CITY, 1, {DETOUR(3, 3, MAPSEC_SEAFLOOR_CAVERN, ROUTE_LOCATION_UNDERWATER)}},
    // y=3
    [COUNTY_ID(0, 3)] = {T(2, 2), MAPSEC_DEWFORD_TOWN, 2,
        {PITSTOP(MAPSEC_GRANITE_CAVE, ROUTE_LOCATION_CAVE), PITSTOP(MAPSEC_ISLAND_CAVE, ROUTE_LOCATION_CAVE)}},
    [COUNTY_ID(1, 3)] = {T(2, 2), MAPSEC_LITTLEROOT_TOWN, 1, {PITSTOP(MAPSEC_ABANDONED_SHIP, ROUTE_LOCATION_WRECK)}},
    [COUNTY_ID(2, 3)] = {T(2, 1), MAPSEC_SLATEPORT_CITY, 1, {SPECIAL(2, 1, MAPSEC_SLATEPORT_CITY, TOWN_LOCATION_MUSEUM)}},
    [COUNTY_ID(3, 3)] = {NO_TILE, MAPSEC_NONE, 2,
        {DETOUR(2, 2, MAPSEC_SEALED_CHAMBER, ROUTE_LOCATION_UNDERWATER), LANDMARK(3, 3, MAPSEC_SOUTHERN_ISLAND, ROUTE_LOCATION_ISLAND)}},
    [COUNTY_ID(4, 3)] = {T(2, 2), MAPSEC_PACIFIDLOG_TOWN, 1, {DETOUR(3, 1, MAPSEC_SKY_PILLAR, ROUTE_LOCATION_TOWER)}},
    [COUNTY_ID(5, 3)] = {NO_TILE, MAPSEC_NONE, 3,
        {PITSTOP(MAPSEC_MIRAGE_ISLAND, ROUTE_LOCATION_ISLAND), LANDMARK(2, 2, MAPSEC_BATTLE_FRONTIER, ROUTE_LOCATION_ISLAND),
         PITSTOP(MAPSEC_ARTISAN_CAVE, ROUTE_LOCATION_CAVE)}},
};
