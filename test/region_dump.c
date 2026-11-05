#include "global.h"
#include "test/test.h"
#include "region_dump.h"
#include "region_county.h"
#include "region_tile.h"

#define MAX_DUMP_LINES (1 + (REGION_H + COUNTY_ROWS - 1) + 3 * NUM_COUNTIES)
#define MAX_EXTRA_LINES 40

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

static EWRAM_DATA char sExtra[MAX_EXTRA_LINES][REGION_DUMP_LINE_SIZE];
static u32 sNumExtra;
static u32 sLockLines, sEnclaveLines, sPlotLines;

static bool32 IsInterestingExtra(const char *line)
{
    static const char *const needles[] =
    {
        "story=1 ", "story=10 ", "story=12 ", "enclave ", "plot 0 ", "plot 3 ", "plot 49 ", "plot 83 ",
    };
    u32 i;

    for (i = 0; i < ARRAY_COUNT(needles); i++)
    {
        if (StrHas(line, needles[i]))
            return TRUE;
    }
    return FALSE;
}

// The first MAX_DUMP_LINES lines are kept; later ones only counted, and the interesting ones kept.
static void Collect(const char *line)
{
    u32 i;
    char *dest;

    if (sNumLines < MAX_DUMP_LINES)
    {
        dest = sLines[sNumLines];
    }
    else
    {
        sLockLines += StrStartsWith(line, "lock ");
        sEnclaveLines += StrStartsWith(line, "enclave ");
        sPlotLines += StrStartsWith(line, "plot ");
        dest = NULL;
        if (IsInterestingExtra(line) && sNumExtra < MAX_EXTRA_LINES)
            dest = sExtra[sNumExtra++];
    }
    sNumLines++;
    if (dest == NULL)
        return;
    for (i = 0; line[i]; i++)
        dest[i] = line[i];
    dest[i] = '\0';
}

static void Dump(void)
{
    sNumLines = 0;
    sNumExtra = 0;
    sLockLines = sEnclaveLines = sPlotLines = 0;
    DumpRegion(Collect);
}

TEST("RegionDump: Emits a header, the map with county separators, and three lines per county")
{
    Dump();
    EXPECT_GE(sNumLines, 1 + (REGION_H + COUNTY_ROWS - 1) + 3 * NUM_COUNTIES);
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
    EXPECT(StrHas(sLines[base + 3 * COUNTY_ID(4, 0)], "CAPITAL"));
    EXPECT(StrHas(sLines[base + 3 * COUNTY_ID(5, 2)], "ELITE4"));
    EXPECT(StrHas(sLines[base + 3 * COUNTY_ID(3, 3)], "RESERVED"));
    EXPECT(StrEq(sLines[base + 3 * COUNTY_ID(3, 2) + 1], "    city=none"));
    EXPECT(StrEq(sLines[base + 3 * COUNTY_ID(3, 2) + 2], "    routes=0 specials=none"));
}

TEST("RegionDump: City lines carry the name, theme, roles and gym, and special locations their kinds")
{
    u32 base = 1 + (REGION_H + COUNTY_ROWS - 1);

    Dump();
    EXPECT(StrHas(sLines[base + 3 * COUNTY_ID(0, 1) + 1], "RUSTBORO CITY"));
    EXPECT(StrHas(sLines[base + 3 * COUNTY_ID(0, 1) + 1], "gym=1 Rock"));
    EXPECT(StrHas(sLines[base + 3 * COUNTY_ID(5, 2) + 1], "gym=9 none"));
    EXPECT(StrHas(sLines[base + 3 * COUNTY_ID(1, 3) + 1], "gym=0"));
    EXPECT(StrHas(sLines[base + 3 * COUNTY_ID(0, 0) + 2], ":cave@("));
    EXPECT(StrHas(sLines[base + 3 * COUNTY_ID(2, 3) + 2], ":museum@("));
}

static const char *FindLine(const char *prefix, const char *needle)
{
    u32 i;

    for (i = 0; i < sNumExtra; i++)
    {
        if (StrStartsWith(sExtra[i], prefix) && StrHas(sExtra[i], needle))
            return sExtra[i];
    }
    return NULL;
}

TEST("RegionDump: Lock lines name the tile, the gated county and the planner key")
{
    Dump();
    EXPECT_GT(sLockLines, 0);
    // Oldale (story 1) is gated by the Starter, Fortree's unlock (block 11) is the Route 119
    // rival, and the Meteor Falls unlock (block 9) is a blocker.
    EXPECT(FindLine("lock ", "story=1 keys=scripted0") != NULL);
    EXPECT(FindLine("lock ", "story=12 implicit=2") != NULL);
    EXPECT(FindLine("lock ", "story=10 blocker") != NULL);
}

TEST("RegionDump: Enclave lines list every Enclave with its unlock index and map flag")
{
    Dump();
    EXPECT_EQ(sEnclaveLines, 4);
    EXPECT(FindLine("enclave ", "unlock=83 on-map") != NULL);
    EXPECT(FindLine("enclave ", "unlock=83 hidden") != NULL);
}

TEST("RegionDump: One plot line per main-story-index through the Elite Four block")
{
    Dump();
    EXPECT_EQ(sPlotLines, 4 * (GetEliteFourStoryIdx() + 1));
    EXPECT(FindLine("plot 0 ", "scripted0") != NULL);
    EXPECT(FindLine("plot 3 ", "unlock keys=scripted0") != NULL);
    EXPECT(FindLine("plot 49 ", "gym5+hm5@county2") != NULL); // Surf's gym, the Petalburg callback
    EXPECT(FindLine("plot 83 ", "unlock keys=scripted7") != NULL);
}
