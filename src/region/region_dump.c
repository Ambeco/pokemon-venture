#include "global.h"
#include "gba/isagbprint.h"
#include "data.h"
#include "constants/characters.h"
#include "battle_main.h"
#include "region_city.h"
#include "region_county.h"
#include "region_plot.h"
#include "region_tile.h"
#include "region_unlock.h"
#include "region_dump.h"

#define MAX_PLOT_DUMP_ITEMS 12

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

static void PutHex(struct LineWriter *w, u32 n)
{
    s32 shift;

    PutStr(w, "0x");
    for (shift = 12; shift >= 0; shift -= 4)
        PutChar(w, "0123456789abcdef"[(n >> shift) & 0xF]);
}

// Gen 3 text to ASCII; characters outside letters, digits, space and a little punctuation print as '?'.
static void PutText(struct LineWriter *w, const u8 *text)
{
    if (text == NULL)
    {
        PutChar(w, '?');
        return;
    }
    for (; *text != EOS; text++)
    {
        u8 c = *text;

        if (c == CHAR_SPACE)
            PutChar(w, ' ');
        else if (c >= CHAR_0 && c <= CHAR_9)
            PutChar(w, '0' + c - CHAR_0);
        else if (c >= CHAR_A && c <= CHAR_Z)
            PutChar(w, 'A' + c - CHAR_A);
        else if (c >= CHAR_a && c <= CHAR_z)
            PutChar(w, 'a' + c - CHAR_a);
        else if (c == CHAR_PERIOD)
            PutChar(w, '.');
        else if (c == CHAR_HYPHEN)
            PutChar(w, '-');
        else
            PutChar(w, '?');
    }
}

static const char *const sLocationKindNames[NUM_LOCATION_KINDS] =
{
    [ROUTE_LOCATION_CAVE] = "cave",
    [ROUTE_LOCATION_TUNNEL] = "tunnel",
    [ROUTE_LOCATION_VOLCANO] = "volcano",
    [ROUTE_LOCATION_MOUNTAIN] = "mountain",
    [ROUTE_LOCATION_FOREST] = "forest",
    [ROUTE_LOCATION_RUINS] = "ruins",
    [ROUTE_LOCATION_TOWER] = "tower",
    [ROUTE_LOCATION_HIDEOUT] = "hideout",
    [ROUTE_LOCATION_ISLAND] = "island",
    [ROUTE_LOCATION_UNDERWATER] = "underwater",
    [ROUTE_LOCATION_WRECK] = "wreck",
    [ROUTE_LOCATION_PARK] = "park",
    [ROUTE_LOCATION_FACILITY] = "facility",
    [ROUTE_LOCATION_ROADSIDE_BUILDING] = "roadside",
    [ROUTE_LOCATION_ROUTE_FEATURE] = "feature",
    [TOWN_LOCATION_MUSEUM] = "museum",
    [TOWN_LOCATION_ENTERTAINMENT] = "entertain",
    [TOWN_LOCATION_STORE] = "store",
    [TOWN_LOCATION_HARBOR] = "harbor",
    [TOWN_LOCATION_INSTITUTION] = "institution",
    [TOWN_LOCATION_SACRED_CAVE] = "sacred-cave",
};

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
        u32 ordinal = GetCountyGymOrdinal(id);
        u32 roles = 0;

        PutText(w, GetCountyCityName(id));
        PutStr(w, " mapsec=");
        PutNum(w, GetCountyCityMapSec(id));
        PutStr(w, " @(");
        PutNum(w, pos.x);
        PutChar(w, ',');
        PutNum(w, pos.y);
        PutStr(w, ") theme=");
        PutNum(w, GetCountyCityTheme(id));
        PutStr(w, " roles=");
        for (i = 0; i < 16; i++)
            roles |= DoesCountyCityHaveBuilding(id, 1 << i) << i;
        PutHex(w, roles);
        PutStr(w, " gym=");
        PutNum(w, ordinal);
        if (ordinal != 0)
        {
            PutChar(w, ' ');
            if (GetCountyGymType(id) == TYPE_NONE)
                PutStr(w, "none");
            else
                PutText(w, gTypesInfo[GetCountyGymType(id)].name);
            PutChar(w, ' ');
            PutText(w, GetTrainerNameFromId(GetCountyGymLeader(id)));
            PutChar(w, '(');
            PutNum(w, GetCountyGymLeader(id));
            PutChar(w, ')');
        }
    }
    else
    {
        PutStr(w, "none");
    }
    Flush(w, emit);

    PutStr(w, "    routes=");
    PutNum(w, GetCountyRouteTileCount(id));
    PutStr(w, " specials=");
    for (i = 0; i < GetCountySpecialLocationCount(id); i++)
    {
        struct SpecialLocation s = GetCountySpecialLocation(id, i);

        if (i != 0)
            PutChar(w, ' ');
        PutNum(w, s.mapSec);
        PutChar(w, ':');
        PutStr(w, sLocationKindNames[s.kind]);
        PutStr(w, "@(");
        PutNum(w, s.pos.x);
        PutChar(w, ',');
        PutNum(w, s.pos.y);
        PutChar(w, ')');
        PutChar(w, "tldpe"[s.locationClass]); // town, landmark, detour, pitstop, enclave
    }
    if (i == 0)
        PutStr(w, "none");
    Flush(w, emit);
}

static const char *const sPlotKindNames[NUM_PLOT_KINDS] =
{
    [PLOT_KIND_GYM] = "gym",
    [PLOT_KIND_RIVAL] = "rival",
    [PLOT_KIND_WALLY] = "wally",
    [PLOT_KIND_ARC] = "arc",
    [PLOT_KIND_DEVON] = "devon",
    [PLOT_KIND_SPECIAL] = "special",
    [PLOT_KIND_UTILITY] = "utility",
    [PLOT_KIND_SCRIPTED] = "scripted",
};

static void PutKeyRef(struct LineWriter *w, u32 kind, u32 ordinal)
{
    PutStr(w, sPlotKindNames[kind]);
    PutNum(w, ordinal);
}

static void PutUnlock(struct LineWriter *w, const struct PlotUnlock *unlock)
{
    u32 i;

    switch (unlock->type)
    {
    case PLOT_UNLOCK_OPEN:
        PutStr(w, "open");
        break;
    case PLOT_UNLOCK_BLOCKER:
        PutStr(w, "blocker");
        break;
    case PLOT_UNLOCK_COUNTY_PRIVATE:
        PutStr(w, "county-private id=");
        PutNum(w, unlock->mcguffinId);
        break;
    case PLOT_UNLOCK_IMPLICIT:
        PutStr(w, "implicit=");
        PutNum(w, unlock->implicit);
        break;
    default:
        PutStr(w, "keys=");
        for (i = 0; i < unlock->numKeys; i++)
        {
            if (i != 0)
                PutChar(w, '+');
            PutKeyRef(w, unlock->keys[i].kind, unlock->keys[i].ordinal);
        }
        break;
    }
}

static void PutPos(struct LineWriter *w, u32 x, u32 y)
{
    PutStr(w, "@(");
    PutNum(w, x);
    PutChar(w, ',');
    PutNum(w, y);
    PutChar(w, ')');
}

static void DumpLocks(struct LineWriter *w, RegionDumpLineFn emit)
{
    u32 a, b;

    for (a = 0; a < NUM_COUNTIES; a++)
    {
        for (b = a + 1; b < NUM_COUNTIES; b++)
        {
            struct RegionTilePos pos;
            struct PlotUnlock key;
            u32 gated;

            if (!AreCountiesAdjacent(a, b) || !GetRouteLockKey(a, b, &pos, &gated, &key))
                continue;
            PutStr(w, "lock ");
            PutNum(w, a);
            PutChar(w, '-');
            PutNum(w, b);
            PutChar(w, ' ');
            PutPos(w, pos.x, pos.y);
            PutStr(w, " gates=");
            PutNum(w, gated);
            PutStr(w, " story=");
            PutNum(w, GetCountyHeader(gated)->storyIdx);
            PutChar(w, ' ');
            PutUnlock(w, &key);
            Flush(w, emit);
        }
    }
}

static void DumpEnclaves(struct LineWriter *w, RegionDumpLineFn emit)
{
    u32 id, i;

    for (id = 0; id < NUM_COUNTIES; id++)
    {
        for (i = 0; i < GetCountySpecialLocationCount(id); i++)
        {
            struct SpecialLocation s;
            u32 unlockIndex;

            if (GetCountySpecialLocationClass(id, i) != LOCATION_CLASS_ENCLAVE)
                continue;
            s = GetCountySpecialLocation(id, i);
            PutStr(w, "enclave county=");
            PutNum(w, id);
            PutStr(w, " mapsec=");
            PutNum(w, s.mapSec);
            PutChar(w, ':');
            PutStr(w, sLocationKindNames[s.kind]);
            PutStr(w, " unlock=");
            if (GetCountyUnlockIndex(id, &unlockIndex))
                PutNum(w, unlockIndex);
            else
                PutStr(w, "start");
            PutStr(w, IsLocationOnMap(&s) ? " on-map" : " hidden");
            Flush(w, emit);
        }
    }
}

// One line per main-story-index: the items of the vanilla list, or the unlock key at step 3.
static void DumpPlot(struct LineWriter *w, RegionDumpLineFn emit)
{
    struct PlotSlot slots[MAX_PLOT_DUMP_ITEMS];
    u32 index, i;

    for (index = 0; index < 4 * (GetEliteFourStoryIdx() + 1); index++)
    {
        PutStr(w, "plot ");
        PutNum(w, index);
        PutStr(w, " x=");
        PutNum(w, index >> 2);
        PutStr(w, " step=");
        PutNum(w, index & 3);
        PutStr(w, ":");
        if ((index & 3) == 3)
        {
            struct PlotUnlock unlock = GetPlotBlockUnlock(index >> 2);

            PutStr(w, " unlock ");
            PutUnlock(w, &unlock);
        }
        else
        {
            u32 count = ResolveVanillaPlotList(index, slots, MAX_PLOT_DUMP_ITEMS);

            for (i = 0; i < count; i++)
            {
                PutChar(w, ' ');
                PutKeyRef(w, slots[i].kind, slots[i].ordinal);
                if (slots[i].hm != PLOT_HM_NONE)
                {
                    PutStr(w, "+hm");
                    PutNum(w, slots[i].hm);
                }
                if ((index & 3) == 1)
                {
                    PutStr(w, "@county");
                    PutNum(w, slots[i].host);
                }
            }
            if (count == 0)
                PutStr(w, " -");
        }
        Flush(w, emit);
    }
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
    DumpLocks(&w, emit);
    DumpEnclaves(&w, emit);
    DumpPlot(&w, emit);
}

static void EmitToLog(const char *line)
{
    DebugPrintf("%s", line);
}

void PrintRegionToLog(void)
{
    DumpRegion(EmitToLog);
}
