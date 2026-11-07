#include "global.h"
#include "assertf.h"
#include "city_hinters.h"
#include "region_city.h"
#include "plot_stage.h"
#include "region_plot.h"
#include "region_unlock.h"
#include "script.h"
#include "string_util.h"
#include "event_data.h"
#include "subplot.h"

#define NUM_MAIN_INDICES (NUM_COUNTIES * 4)

bool32 GetMainBeatHost(u32 index, u32 *county)
{
    struct PlotSlot items[MAX_GETS_PER_LIST];
    u32 count, step = index & 3;

    if (index >= NUM_MAIN_INDICES)
        return FALSE;
    if (step == 3)
    {
        if ((index >> 2) > GetEliteFourStoryIdx() || GetPlotBlockUnlock(index >> 2).type != PLOT_UNLOCK_BLOCKER)
            return FALSE;
        *county = GetCountyByStoryIdx(index >> 2);
        return TRUE;
    }
    count = ResolveVanillaPlotList(index, items, ARRAY_COUNT(items));
    if (count == 0)
        return FALSE;
    *county = GetCountyByStoryIdx(items[0].host);
    return TRUE;
}

u32 GetNextMainBeat(u32 index)
{
    u32 next, county;

    for (next = index + 1; next < NUM_MAIN_INDICES; next++)
    {
        if (GetMainBeatHost(next, &county))
            return next;
    }
    return HINTER_NO_BEAT;
}

u32 GetHinterRoleCapacity(u32 role)
{
    if (role == 0)
        return 4;
    if (role == BUILDING_GYM)
        return 3;
    if (role == BUILDING_SERVICE)
        return 1;
    return 2;
}

#define NUM_BUILDING_ROLES 15

// Room left in each building of the city, index 0 = outdoors, index n = role 1 << (n - 1).
struct HinterRoom
{
    u8 left[NUM_BUILDING_ROLES + 1];
};

static void InitRoom(struct HinterRoom *room, u32 countyId)
{
    u32 bit;

    room->left[0] = GetHinterRoleCapacity(0);
    for (bit = 0; bit < NUM_BUILDING_ROLES; bit++)
        room->left[bit + 1] = DoesCountyCityHaveBuilding(countyId, 1 << bit) ? GetHinterRoleCapacity(1 << bit) : 0;
}

// Takes one slot from the first of `roles` (any building when roles is 0) with room, outdoors first for
// roles 0. Returns FALSE and leaves *role alone if none has room.
static bool32 TakeRoom(struct HinterRoom *room, u32 roles, u32 *role)
{
    u32 n;

    if (roles == 0 && room->left[0] != 0)
    {
        room->left[0]--;
        *role = 0;
        return TRUE;
    }
    for (n = 1; n <= NUM_BUILDING_ROLES; n++)
    {
        if ((roles == 0 || (roles & (1u << (n - 1)))) && room->left[n] != 0)
        {
            room->left[n]--;
            *role = 1u << (n - 1);
            return TRUE;
        }
    }
    return FALSE;
}

static bool32 AddHinter(struct HinterPlacement *out, u32 maxOut, u32 *total, struct HinterRoom *room, u32 countyId, u32 roles, struct HinterPlacement rec)
{
    u32 role = 0;
    bool32 found = TakeRoom(room, roles, &role);

    assertf(found, "county %d has no room for hinter #%d (roles 0x%x): the city lacks those buildings or they are full; did you mean to widen hinterRoles or lower numHinters?", countyId, *total, roles)
    {
        return FALSE;
    }
    rec.role = role;
    rec.selector = *total;
    if (*total < maxOut)
        out[*total] = rec;
    (*total)++;
    return TRUE;
}

u32 PlanCountyHinters(u32 countyId, struct HinterPlacement *out, u32 maxOut)
{
    struct HinterRoom room;
    u32 total = 0, slot, stage, i, index, next, host;

    InitRoom(&room, countyId);
    for (slot = 0; slot < SUBPLOTS_PER_COUNTY; slot++)
    {
        const struct SubplotDesc *desc = GetCountySubplot(countyId, slot);

        if (desc == NULL)
            continue;
        for (stage = 0; stage < desc->numStages; stage++)
        {
            const struct SubplotStage *s = &desc->stages[stage];

            assertf(s->hinterRoles == 0 || (s->hinterRoles & ~0x7FFF) == 0, "county %d slot %d stage %d: hinterRoles 0x%x has bits that are no BuildingRole", countyId, slot, stage, s->hinterRoles)
            {
                continue;
            }
            for (i = 0; i < s->numHinters; i++)
            {
                struct HinterPlacement rec = {HINTER_SUBPLOT, slot, stage, GetBeatDistance(countyId, &s->anchor, countyId), 0, countyId, 0};

                if (!AddHinter(out, maxOut, &total, &room, countyId, s->hinterRoles, rec))
                    return total;
            }
        }
    }
    for (index = 0; index < NUM_MAIN_INDICES; index++)
    {
        if (!GetMainBeatHost(index, &host) || host != countyId)
            continue;
        next = GetNextMainBeat(index);
        if (next == HINTER_NO_BEAT)
            continue; // the last beat has nothing left to hint at
        GetMainBeatHost(next, &host);
        {
            struct HinterPlacement rec = {HINTER_MAIN, index, next, host == countyId ? SUBPLOT_CLOSE : SUBPLOT_FAR, 0, host, 0};

            if (!AddHinter(out, maxOut, &total, &room, countyId, 0, rec))
                return total;
        }
    }
    return total;
}

struct MainBeatPhrase
{
    const u8 *lead; // follows "I heard " or "Word around town is "; the anchor reference comes next
    const u8 *tail;
};

static const struct MainBeatPhrase sMainBeatPhrases[NUM_PLOT_KINDS] =
{
    [PLOT_KIND_GYM] = {COMPOUND_STRING("the GYM "), COMPOUND_STRING(" is taking on challengers.")},
    [PLOT_KIND_RIVAL] = {COMPOUND_STRING("your rival was looking for you "), COMPOUND_STRING(".")},
    [PLOT_KIND_WALLY] = {COMPOUND_STRING("a frail boy named WALLY was seen "), COMPOUND_STRING(".")},
    [PLOT_KIND_ARC] = {COMPOUND_STRING("something odd is going on "), COMPOUND_STRING(".")},
    [PLOT_KIND_DEVON] = {COMPOUND_STRING("DEVON CORP. has an errand to run "), COMPOUND_STRING(".")},
    [PLOT_KIND_SPECIAL] = {COMPOUND_STRING("someone "), COMPOUND_STRING(" is giving away a handy gadget.")},
    [PLOT_KIND_UTILITY] = {COMPOUND_STRING("someone "), COMPOUND_STRING(" is handing out a useful tool.")},
    [PLOT_KIND_SCRIPTED] = {COMPOUND_STRING("big things are happening "), COMPOUND_STRING(".")},
};

static const struct MainBeatPhrase sMainBeatBlockerPhrase =
    {COMPOUND_STRING("somebody "), COMPOUND_STRING(" is blocking the road onward.")};

static const u8 sFarOpening[] = _("I heard ");
static const u8 sCloseOpening[] = _("Word around town is ");

u8 *WriteMainBeatHint(u8 *dst, u32 index, u32 distance)
{
    struct PlotSlot items[MAX_GETS_PER_LIST];
    const struct MainBeatPhrase *phrase;
    struct BeatAnchor anchor = {BEAT_ANCHOR_NPC, ROUTE_ID_NONE, ROUTE_ID_NONE, BEAT_LOCATION_NONE, 0};
    u32 host;

    assertf(GetMainBeatHost(index, &host), "main-story-index %d has no host, so nothing to hint at; did you mean GetNextMainBeat()?", index)
    {
        return dst;
    }
    if ((index & 3) == 3)
    {
        phrase = &sMainBeatBlockerPhrase;
    }
    else
    {
        ResolveVanillaPlotList(index, items, ARRAY_COUNT(items));
        assertf(items[0].kind < NUM_PLOT_KINDS && sMainBeatPhrases[items[0].kind].lead != NULL, "no main-beat hint phrase for plot kind %d (index %d); did you mean to add it to sMainBeatPhrases?", items[0].kind, index)
        {
            return dst;
        }
        phrase = &sMainBeatPhrases[items[0].kind];
    }
    dst = StringCopy(dst, distance == SUBPLOT_CLOSE ? sCloseOpening : sFarOpening);
    dst = StringCopy(dst, phrase->lead);
    dst = WriteBeatAnchorReference(dst, host, &anchor);
    return StringCopy(dst, phrase->tail);
}

// Script special: VAR_0x8004 = the hinter's county-hosted main-story-index (HinterPlacement.slot),
// VAR_0x8005 = its target beat. Puts the hint in gStringVar4 and sets VAR_RESULT = TRUE only while the
// player has finished the hinter's own beat but not yet the target; else FALSE (say generic text).
void ShowMainBeatHint(void)
{
    u32 own = gSpecialVar_0x8004, target = gSpecialVar_0x8005, host, hinterHost, current = GetMainStoryIndex();

    gSpecialVar_Result = FALSE;
    if (current <= own || current > target)
        return;
    if (!GetMainBeatHost(own, &hinterHost) || !GetMainBeatHost(target, &host))
        return;
    WriteMainBeatHint(gStringVar4, target, host == hinterHost ? SUBPLOT_CLOSE : SUBPLOT_FAR);
    gSpecialVar_Result = TRUE;
}
