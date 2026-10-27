#include "global.h"
#include "gba/isagbprint.h"
#include "region_county.h"
#include "region_tile.h"
#include "region_dump.h"

struct LineWriter
{
    char buf[REGION_DUMP_LINE_SIZE];
    u32 len;
};

static void PutChar(struct LineWriter *w, char c)
{
    if (w->len < REGION_DUMP_LINE_SIZE - 1)
        w->buf[w->len++] = c;
}

static void PutStr(struct LineWriter *w, const char *s)
{
    while (*s)
        PutChar(w, *s++);
}

static void PutNum(struct LineWriter *w, u32 n)
{
    char digits[10];
    u32 count = 0;

    do
    {
        digits[count++] = '0' + n % 10;
        n /= 10;
    } while (n != 0);
    while (count != 0)
        PutChar(w, digits[--count]);
}

static void Flush(struct LineWriter *w, RegionDumpLineFn emit)
{
    w->buf[w->len] = '\0';
    emit(w->buf);
    w->len = 0;
}

static char CellChar(u32 x, u32 y)
{
    u32 flags = GetRegionTileFlags(x, y);

    if ((flags & TILE_CITY) && (flags & TILE_SPECIAL))
        return '@';
    if (flags & TILE_CITY)
        return 'C';
    if (flags & TILE_SPECIAL)
        return 'S';
    if (flags & TILE_ROUTE)
        return '#';
    return '.';
}

static void DumpMap(struct LineWriter *w, RegionDumpLineFn emit)
{
    u32 x, y;

    for (y = 0; y < REGION_H; y++)
    {
        // Cell (x, y) is grid tile (x + 1, y + 1), so a county column starts where x + 1 is a multiple of 5.
        if (y != 0 && (y + 1) % COUNTY_TILES_H == 0)
            Flush(w, emit);
        for (x = 0; x < REGION_W; x++)
        {
            if (x != 0 && (x + 1) % COUNTY_TILES_W == 0)
                PutChar(w, ' ');
            PutChar(w, CellChar(x, y));
        }
        Flush(w, emit);
    }
}

static void DumpCounty(struct LineWriter *w, RegionDumpLineFn emit, u32 id)
{
    static const char sTerrain[] = {'L', 'M', 'O'};
    const struct CountyHeader *h = GetCountyHeader(id);
    u32 i;

    PutStr(w, "county ");
    PutNum(w, id);
    PutStr(w, " (");
    PutNum(w, COUNTY_X(id));
    PutChar(w, ',');
    PutNum(w, COUNTY_Y(id));
    PutStr(w, ") story=");
    PutNum(w, h->storyIdx);
    PutStr(w, " terrain=");
    PutChar(w, sTerrain[h->terrain]);
    PutStr(w, " biome=");
    PutNum(w, GetCountyBiome(id));
    PutStr(w, " pop=");
    PutNum(w, h->population);
    PutStr(w, " key=");
    PutNum(w, h->keyId);
    PutStr(w, " lock=");
    PutNum(w, GetCountyLockId(id));
    if (h->port)
        PutStr(w, " port");
    if (id == GetRegionHeader()->capital)
        PutStr(w, " CAPITAL");
    if (IsCountyEliteFour(id))
        PutStr(w, " ELITE4");
    if (IsCountyReserved(id))
        PutStr(w, " RESERVED");
    Flush(w, emit);

    PutStr(w, "    city=");
    if (DoesCountyHaveCity(id))
    {
        struct RegionTilePos pos = GetCountyCityPos(id);

        PutNum(w, GetCountyCityMapSec(id));
        PutStr(w, "@(");
        PutNum(w, pos.x);
        PutChar(w, ',');
        PutNum(w, pos.y);
        PutChar(w, ')');
    }
    else
    {
        PutStr(w, "none");
    }
    PutStr(w, " routes=");
    PutNum(w, GetCountyRouteTileCount(id));
    PutStr(w, " specials=");
    for (i = 0; i < GetCountySpecialLocationCount(id); i++)
    {
        struct SpecialLocation s = GetCountySpecialLocation(id, i);

        if (i != 0)
            PutChar(w, ' ');
        PutNum(w, s.mapSec);
        PutStr(w, "@(");
        PutNum(w, s.pos.x);
        PutChar(w, ',');
        PutNum(w, s.pos.y);
        PutChar(w, ')');
    }
    if (i == 0)
        PutStr(w, "none");
    Flush(w, emit);
}

void DumpRegion(RegionDumpLineFn emit)
{
    struct LineWriter w = {.len = 0};
    u32 id;

    PutStr(&w, "region: capital=");
    PutNum(&w, GetRegionHeader()->capital);
    PutStr(&w, " reserved=");
    PutNum(&w, GetRegionHeader()->reservedCount);
    PutStr(&w, " arc=");
    PutNum(&w, GetRegionHeader()->storyArc);
    Flush(&w, emit);
    DumpMap(&w, emit);
    for (id = 0; id < NUM_COUNTIES; id++)
        DumpCounty(&w, emit, id);
}

static void EmitToLog(const char *line)
{
    DebugPrintf("%s", line);
}

void PrintRegionToLog(void)
{
    DumpRegion(EmitToLog);
}
