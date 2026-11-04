#ifndef GUARD_REGION_COUNTY_H
#define GUARD_REGION_COUNTY_H

/**
 * Layer 1-3 region data: one 3-byte CountyHeader per county plus a small RegionHeader.
 *
 * The 24 headers are cached for the whole session, so a field is stored only if it is an output of
 * a global constraint (terrain shape, the story-index walk, the settlement quota) that cannot be
 * recomputed from the other headers plus a per-county hash. Everything else is derived on demand:
 * city name and tile, special locations (kind and tile, picked by a coprime stride like cities), island-ness (neighbors'
 * terrain), gym type and gym/HM/arc-beat hosts (main plot planner), which county is the Elite Four
 * and which are reserved. A county is generated from its own header and its neighbors' headers,
 * never from a neighbor's generated content.
 *
 * Stage 1 (hardcoded Emerald): GetRegionHeader() returns a ROM table. Later it returns a RAM copy
 * filled by the generator; callers do not change. Per-tile data (city and special-location
 * tiles, names) is deliberately not here; see region_tile.h.
 *
 * Quirks: a biome is stored relative to the region's 4x3 biome subset (see GetCountyBiome); the
 * spare bits of CountyHeader are reserved and must stay zero; a key's lock is always in the next story index, so only keys are stored; a county may hold one key and one
 * lock at once (a dead end that was itself reached by a jump), but never two of either.
 */

#include "global.h"

#define COUNTY_COLS  6
#define COUNTY_ROWS  4
#define NUM_COUNTIES (COUNTY_COLS * COUNTY_ROWS)

#define COUNTY_TILES_W 5
#define COUNTY_TILES_H 4

// Region map size in cells (MAP_WIDTH/MAP_HEIGHT in region_map.c).
#define REGION_W 28
#define REGION_H 15

#define COUNTY_ID(x, y) ((y) * COUNTY_COLS + (x))
#define COUNTY_X(id)    ((id) % COUNTY_COLS)
#define COUNTY_Y(id)    ((id) / COUNTY_COLS)

enum CountyTerrain
{
    COUNTY_TERRAIN_LAND,
    COUNTY_TERRAIN_MOUNTAIN,
    COUNTY_TERRAIN_OCEAN,
};

// The 25-biome grid: humidity row 0 (driest) to 4 (wettest), temperature column 0 (hottest) to 4.
#define BIOME_GRID(humidity, temperature) ((humidity) * 5 + (temperature))
#define BIOME_HUMIDITY(biome)             ((biome) / 5)
#define BIOME_TEMPERATURE(biome)          ((biome) % 5)

enum Biome
{
    BIOME_TROPICAL_DESERT = BIOME_GRID(0, 0),
    BIOME_THORN_SCRUB,
    BIOME_WARM_TEMPERATE_DESERT,
    BIOME_COOL_TEMPERATE_DESERT_SCRUB,
    BIOME_ARCTIC_ALPINE_DESERT,
    BIOME_THORN_FOREST = BIOME_GRID(1, 0),
    BIOME_SAVANNA,
    BIOME_TEMPERATE_GRASSLAND,
    BIOME_ALPINE_GRASSLANDS,
    BIOME_TUNDRA,
    BIOME_TEMPERATE_DECIDUOUS_FOREST = BIOME_GRID(2, 0),
    BIOME_TEMPERATE_WOODLAND,
    BIOME_TEMPERATE_SHRUBLANDS,
    BIOME_SALT_MARSH,
    BIOME_TAIGA,
    BIOME_TROPICAL_SEASONAL_RAINFOREST = BIOME_GRID(3, 0),
    BIOME_MANGROVE_SWAMP,
    BIOME_TEMPERATE_FRESHWATER_SWAMP_FOREST,
    BIOME_TEMPERATE_EVERGREEN_FOREST,
    BIOME_ELFIN_WOODLAND,
    BIOME_TROPICAL_RAINFOREST = BIOME_GRID(4, 0),
    BIOME_TROPICAL_FRESHWATER_SWAMP_FOREST,
    BIOME_TEMPERATE_GIANT_RAINFOREST,
    BIOME_MONTANE_RAINFOREST,
    BIOME_BOG,
};

#define NUM_BIOMES_IN_GRID   25
#define BIOME_SUBSET_HUMIDITY_LEVELS 4
#define BIOME_SUBSET_TEMPERATURE_LEVELS 3
#define NUM_BIOMES_IN_SUBSET (BIOME_SUBSET_HUMIDITY_LEVELS * BIOME_SUBSET_TEMPERATURE_LEVELS)

enum StoryArc
{
    STORY_ARC_EMERALD,
};

struct CountyHeader
{
    u32 terrain:2;          // enum CountyTerrain
    u32 biome:4;            // index into the region's 4x3 subset, row-major from its driest, hottest corner
    u32 storyIdx:5;         // county-story-index, unique over 0..NUM_COUNTIES-1
    u32 population:2;       // 0 = no settlement, 1 village, 2 town, 3 city
    u32 keyId:3;            // jump ordinal 1-7 whose key this county holds, 0 = none; its lock is at storyIdx + 1
    u32 port:1;
    u32 enclaveOnly:1;      // reserved end-game county holding only Enclaves: no city, no routes (design.md, "Enclaves")
    u32 reserved:6;
} __attribute__((packed));

STATIC_ASSERT(sizeof(struct CountyHeader) == 3, CountyHeaderIs3Bytes);

struct RegionHeader
{
    u8 capital;             // county id; always population 3
    u8 reservedCount;       // R, 3 or 4: the last R story indices are reserved legendary counties
    u8 biomeHumidityBase;   // lowest humidity row of the biome subset, 0-1
    u8 biomeTemperatureBase;// hottest temperature column of the biome subset, 0-2
    u8 storyArc;            // enum StoryArc
    struct CountyHeader counties[NUM_COUNTIES];
};

const struct RegionHeader *GetRegionHeader(void);

const struct CountyHeader *GetCountyHeader(u32 countyId);

// Jump ordinal whose lock opens this county (the key of the county at storyIdx - 1), 0 = none.
u32 GetCountyLockId(u32 countyId);

// Global grid id (enum Biome) of the county's biome, expanded through the region's subset.
u32 GetCountyBiome(u32 countyId);

// County whose storyIdx is idx.
u32 GetCountyByStoryIdx(u32 idx);

// Story index of the Elite Four county: NUM_COUNTIES - 1 - reservedCount.
u32 GetEliteFourStoryIdx(void);

bool32 IsCountyReserved(u32 countyId);

bool32 IsCountyEnclaveOnly(u32 countyId);

bool32 IsCountyEliteFour(u32 countyId);

bool32 AreCountiesAdjacent(u32 a, u32 b);

// Region-map cell bounds (inclusive) of a county's tiles. Edge counties are one tile smaller
// because the outer 30x16 tile ring is off limits (design.md, "County layout").
void GetCountyRegionBounds(u32 countyId, u32 *left, u32 *top, u32 *right, u32 *bottom);

// County containing a cell of the 28x15 region map.
u32 GetCountyFromRegionCell(u32 x, u32 y);

#endif // GUARD_REGION_COUNTY_H
