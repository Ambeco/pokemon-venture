#include "global.h"
#include "test/test.h"
#include "region_dump.h"
#include "region_tile.h"

#define MAX_DUMP_LINES 80

static EWRAM_DATA char sLines[MAX_DUMP_LINES][REGION_DUMP_LINE_SIZE];
static u32 sNumLines;

static bool32 StrEq(const char *a, const char *b)
{
    while (*a && *a == *b)
        a++, b++;
    return *a == *b;
}

static bool32 StrStartsWith(const char *s, const char *prefix)
{
    while (*prefix && *s == *prefix)
        s++, prefix++;
    return *prefix == '\0';
}

static u32 StrLen(const char *s)
{
    u32 n = 0;
    while (s[n])
        n++;
    return n;
}

static bool32 StrHas(const char *s, const char *needle)
{
    u32 i, len = StrLen(needle);

    for (i = 0; s[i]; i++)
    {
        if (StrStartsWith(s + i, needle))
            return TRUE;
    }
    return len == 0;
}

static void Collect(const char *line)
{
    u32 i;

    ASSUME(sNumLines < MAX_DUMP_LINES);
    for (i = 0; line[i]; i++)
        sLines[sNumLines][i] = line[i];
    sLines[sNumLines][i] = '\0';
    sNumLines++;
}

static void Dump(void)
{
    sNumLines = 0;
    DumpRegion(Collect);
}

TEST("RegionDump: Emits a header, the map with county separators, and two lines per county")
{
    Dump();
    EXPECT_EQ(sNumLines, 1 + (REGION_H + COUNTY_ROWS - 1) + 2 * NUM_COUNTIES);
    EXPECT(StrStartsWith(sLines[0], "region: capital=4 reserved=3"));
}

TEST("RegionDump: Map rows are 28 cells plus 5 county gaps, and separators are blank")
{
    u32 row, blanks = 0;

    Dump();
    for (row = 1; row < 1 + REGION_H + COUNTY_ROWS - 1; row++)
    {
        u32 len = StrLen(sLines[row]);
        if (len == 0)
            blanks++;
        else
            EXPECT_EQ(len, REGION_W + COUNTY_COLS - 1);
    }
    EXPECT_EQ(blanks, COUNTY_ROWS - 1);
}

TEST("RegionDump: Map glyph counts match the tile accessors")
{
    u32 row, i, cities = 0, routes = 0, expectCities = 0, expectRoutes = 0, id;

    Dump();
    for (row = 1; row < 1 + REGION_H + COUNTY_ROWS - 1; row++)
    {
        for (i = 0; sLines[row][i]; i++)
        {
            cities += sLines[row][i] == 'C' || sLines[row][i] == '@';
            routes += sLines[row][i] == '#';
        }
    }
    for (id = 0; id < NUM_COUNTIES; id++)
    {
        expectCities += DoesCountyHaveCity(id);
        // A route tile shared with an outside-city location draws as a special location, not '#'.
        for (i = 0; i < GetCountyRouteTileCount(id); i++)
        {
            struct RegionTilePos pos = GetCountyRouteTile(id, i);
            expectRoutes += GetRegionTileFlags(pos.x, pos.y) == TILE_ROUTE;
        }
    }
    EXPECT_EQ(cities, expectCities);
    EXPECT_EQ(routes, expectRoutes);
}

TEST("RegionDump: County lines carry roles and the empty county reports none")
{
    u32 base = 1 + (REGION_H + COUNTY_ROWS - 1);

    Dump();
    EXPECT(StrHas(sLines[base + 2 * COUNTY_ID(4, 0)], "CAPITAL"));
    EXPECT(StrHas(sLines[base + 2 * COUNTY_ID(5, 2)], "ELITE4"));
    EXPECT(StrHas(sLines[base + 2 * COUNTY_ID(3, 3)], "RESERVED"));
    EXPECT(StrEq(sLines[base + 2 * COUNTY_ID(3, 2) + 1], "    city=none routes=0 specials=none"));
}
