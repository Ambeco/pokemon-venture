#include "global.h"
#include "line_break.h"
#include "constants/characters.h"
#include "constants/vars.h"
#include "assertf.h"
#include "event_data.h"
#include "global.fieldmap.h"
#include "region_city.h"
#include "region_tile.h"
#include "string_util.h"
#include "subplot.h"

static const u8 gText_Lilycove_Exhibit0FarCurrent[] = _("I heard the new exhibit hall at the\nLILYCOVE MUSEUM is about to open.");
static const u8 gText_Lilycove_Exhibit0FarAfter0[] = _("The LILYCOVE MUSEUM finally opened its\nnew exhibit hall, I heard.");
static const u8 gText_Lilycove_Exhibit0FarAfter1[] = _("Did you hear? The exhibit hall at the\nLILYCOVE MUSEUM is open.");
static const u8 gText_Lilycove_Exhibit0CloseCurrent[] = _("The curator is still busy preparing the\nexhibit hall.");
static const u8 gText_Lilycove_Exhibit0CloseAfter[] = _("The exhibit hall is open at last!\nThe curator must be so relieved.");
static const u8 gText_Lilycove_Exhibit1FarCurrent[] = _("The LILYCOVE MUSEUM's exhibit hall is\nopen, and they want paintings for it!");
static const u8 gText_Lilycove_Exhibit1CloseBefore[] = _("There's talk of a new exhibit hall\n{HINT_ANCHOR}. It's not ready yet, though.");
static const u8 gText_Lilycove_Exhibit1CloseCurrent[] = _("The curator is asking trainers for\npaintings to hang in the exhibit hall.");

static const u8 *const sLilycoveExhibit0FarAfter[] =
{
    gText_Lilycove_Exhibit0FarAfter0,
    gText_Lilycove_Exhibit0FarAfter1,
};

#include "data/region/emerald_subplots.h"

const struct SubplotDesc *GetCountySubplot(u32 countyId, u32 slot)
{
    u32 i;

    assertf(slot < SUBPLOTS_PER_COUNTY, "subplot slot %d >= %d; slot 0 of a county slice is its completion mask", slot, SUBPLOTS_PER_COUNTY)
    {
        return NULL;
    }
    for (i = 0; i < ARRAY_COUNT(sEmeraldSubplotHosts); i++)
    {
        if (sEmeraldSubplotHosts[i].county == countyId && sEmeraldSubplotHosts[i].slot == slot)
            return sEmeraldSubplotHosts[i].desc;
    }
    return NULL;
}

u16 GetCountySlotVar(u32 countyId, u32 slot)
{
    assertf(countyId < COUNTY_VAR_SLICES_FREE, "county %d slice overlaps legacy vars (only counties 0-%d are free); retire legacy vars first", countyId, COUNTY_VAR_SLICES_FREE - 1)
    {
        return 0;
    }
    assertf(slot < SUBPLOTS_PER_COUNTY, "subplot slot %d >= %d", slot, SUBPLOTS_PER_COUNTY)
    {
        return 0;
    }
    return COUNTY_VARS_START + countyId * VARS_PER_COUNTY + 1 + slot;
}

u32 GetSubplotStage(u32 countyId, u32 slot, const struct SubplotDesc *desc)
{
    u32 stage;

    assertf(desc != NULL, "county %d hosts no subplot in slot %d", countyId, slot)
    {
        return 0;
    }
    if (desc->backend == SUBPLOT_STORED)
    {
        stage = VarGet(GetCountySlotVar(countyId, slot));
        assertf(stage < desc->numStages, "county %d slot %d holds stage %d but the subplot has %d", countyId, slot, stage, desc->numStages)
        {
            return 0;
        }
        return stage;
    }
    for (stage = desc->numStages - 1; stage > 0; stage--)
    {
        if (IsStoryPredicateTrue(&desc->stages[stage].done))
            return stage;
    }
    return 0;
}

void SetSubplotStage(u32 countyId, u32 slot, const struct SubplotDesc *desc, u32 stage)
{
    assertf(desc != NULL, "county %d hosts no subplot in slot %d", countyId, slot)
    {
        return;
    }
    assertf(desc->backend == SUBPLOT_STORED, "county %d slot %d is an adapter subplot, which is read-only; did you mean to port it to the stored backend?", countyId, slot)
    {
        return;
    }
    assertf(stage < desc->numStages, "stage %d >= %d stages", stage, desc->numStages)
    {
        return;
    }
    VarSet(GetCountySlotVar(countyId, slot), stage);
}

u32 GetCountySlotStage(u32 countyId, u32 slot)
{
    return GetSubplotStage(countyId, slot, GetCountySubplot(countyId, slot));
}

void SetCountySlotStage(u32 countyId, u32 slot, u32 stage)
{
    SetSubplotStage(countyId, slot, GetCountySubplot(countyId, slot), stage);
}

u32 GetSubplotBeatState(u32 subplotStage, u32 beat)
{
    if (subplotStage < beat)
        return SUBPLOT_BEFORE;
    return subplotStage == beat ? SUBPLOT_CURRENT : SUBPLOT_AFTER;
}

static const u8 *PickFromList(const struct SubplotTextList *list, u32 selector)
{
    if (list->count == 0)
        return NULL;
    return list->texts[selector % list->count];
}

const u8 *GetSubplotHint(const struct SubplotDesc *desc, u32 subplotStage, u32 beat, u32 distance, u32 selector)
{
    const struct SubplotStageHints *hints;
    u32 state;

    assertf(desc != NULL && beat < desc->numStages, "no subplot stage %d", beat)
    {
        return NULL;
    }
    hints = &desc->stages[beat].hints;
    state = GetSubplotBeatState(subplotStage, beat);
    if (distance == SUBPLOT_FAR)
    {
        if (state == SUBPLOT_BEFORE)
            return PickFromList(&hints->farBefore, selector);
        return state == SUBPLOT_CURRENT ? hints->farCurrent : PickFromList(&hints->farAfter, selector);
    }
    if (state == SUBPLOT_BEFORE)
        return hints->closeBefore;
    return state == SUBPLOT_CURRENT ? hints->closeCurrent : hints->closeAfter;
}

const u8 *GetCountySlotHintAbout(u32 countyId, u32 slot, u32 beat, u32 distance, u32 selector)
{
    const struct SubplotDesc *desc = GetCountySubplot(countyId, slot);

    return GetSubplotHint(desc, GetSubplotStage(countyId, slot, desc), beat, distance, selector);
}

const u8 *GetCountySlotHint(u32 countyId, u32 slot, u32 distance, u32 selector)
{
    const struct SubplotDesc *desc = GetCountySubplot(countyId, slot);
    u32 stage = GetSubplotStage(countyId, slot, desc);

    return GetSubplotHint(desc, stage, stage, distance, selector);
}

bool32 DoesSubplotProduce(const struct SubplotDesc *desc, u32 kind, u32 ordinal)
{
    return desc->producesMainGet && desc->mainGet.kind == kind && desc->mainGet.ordinal == ordinal;
}

u32 GetSubplotActors(const struct SubplotDesc *desc, u32 stage, const struct SubplotActor **out, u32 maxOut)
{
    u32 i, total = 0;

    assertf(desc != NULL, "no subplot to list actors for")
    {
        return 0;
    }
    for (i = 0; i < desc->numActors; i++)
    {
        if (stage < desc->actors[i].firstStage || stage > desc->actors[i].lastStage)
            continue;
        if (total < maxOut)
            out[total] = &desc->actors[i];
        total++;
    }
    return total;
}

u32 GetCountyFromCityMapSec(u32 mapSec)
{
    u32 county;

    for (county = 0; county < NUM_COUNTIES; county++)
    {
        if (DoesCountyHaveCity(county) && GetCountyCityMapSec(county) == mapSec)
            return county;
    }
    assertf(FALSE, "MAPSEC %d is no county's city; only a city can host subplots for now", mapSec)
    {
        return 0;
    }
    return 0;
}

static const u8 *const sBuildingRoleNames[] =
{
    COMPOUND_STRING("POKéMON CENTER"), COMPOUND_STRING("MART"), COMPOUND_STRING("GYM"), COMPOUND_STRING("POKéMON LEAGUE"), COMPOUND_STRING("home"), COMPOUND_STRING("bedroom"), COMPOUND_STRING("MARKET"),
    COMPOUND_STRING("MOTEL"), COMPOUND_STRING("HERB SHOP"), COMPOUND_STRING("HOT SPRINGS"), COMPOUND_STRING("HALL"), COMPOUND_STRING("FAN CLUB"), COMPOUND_STRING("SCHOOL"), COMPOUND_STRING("SERVICE house"),
    COMPOUND_STRING("house"),
};

static const u8 *const sLocationKindNames[] =
{
    [ROUTE_LOCATION_CAVE] = COMPOUND_STRING("cave"),
    [ROUTE_LOCATION_TUNNEL] = COMPOUND_STRING("tunnel"),
    [ROUTE_LOCATION_VOLCANO] = COMPOUND_STRING("volcano"),
    [ROUTE_LOCATION_MOUNTAIN] = COMPOUND_STRING("mountain"),
    [ROUTE_LOCATION_FOREST] = COMPOUND_STRING("forest"),
    [ROUTE_LOCATION_RUINS] = COMPOUND_STRING("ruins"),
    [ROUTE_LOCATION_TOWER] = COMPOUND_STRING("tower"),
    [ROUTE_LOCATION_HIDEOUT] = COMPOUND_STRING("hideout"),
    [ROUTE_LOCATION_ISLAND] = COMPOUND_STRING("island"),
    [ROUTE_LOCATION_UNDERWATER] = COMPOUND_STRING("undersea spot"),
    [ROUTE_LOCATION_WRECK] = COMPOUND_STRING("wreck"),
    [ROUTE_LOCATION_PARK] = COMPOUND_STRING("park"),
    [ROUTE_LOCATION_FACILITY] = COMPOUND_STRING("facility"),
    [ROUTE_LOCATION_ROADSIDE_BUILDING] = COMPOUND_STRING("roadside building"),
    [ROUTE_LOCATION_ROUTE_FEATURE] = COMPOUND_STRING("landmark"),
    [TOWN_LOCATION_MUSEUM] = COMPOUND_STRING("MUSEUM"),
    [TOWN_LOCATION_ENTERTAINMENT] = COMPOUND_STRING("entertainment hall"),
    [TOWN_LOCATION_STORE] = COMPOUND_STRING("DEPT. STORE"),
    [TOWN_LOCATION_HARBOR] = COMPOUND_STRING("harbor"),
    [TOWN_LOCATION_INSTITUTION] = COMPOUND_STRING("institute"),
    [TOWN_LOCATION_SACRED_CAVE] = COMPOUND_STRING("sacred cave"),
};

static const u8 sNearThe[] = _("near the ");
static const u8 sNearTheRoadTo[] = _("near the road to ");
static const u8 sNearTheRoad[] = _("near the road");
static const u8 sInCity[] = _("in ");

u32 GetBeatAnchorCounty(u32 hostCounty, const struct BeatAnchor *anchor)
{
    return hostCounty;
}

u32 GetBeatDistance(u32 hostCounty, const struct BeatAnchor *anchor, u32 hinterCounty)
{
    return hinterCounty == GetBeatAnchorCounty(hostCounty, anchor) ? SUBPLOT_CLOSE : SUBPLOT_FAR;
}

u8 *WriteBeatAnchorReference(u8 *dst, u32 hostCounty, const struct BeatAnchor *anchor)
{
    u32 bit;

    if (anchor->role != 0)
    {
        assertf((anchor->role & (anchor->role - 1)) == 0, "anchor role 0x%x has several bits; did you mean one BuildingRole?", anchor->role)
        {
            return dst;
        }
        for (bit = 0; bit < ARRAY_COUNT(sBuildingRoleNames); bit++)
        {
            if (anchor->role == 1u << bit)
                break;
        }
        assertf(bit < ARRAY_COUNT(sBuildingRoleNames), "anchor role 0x%x has no name; did you mean to add it to sBuildingRoleNames?", anchor->role)
        {
            return dst;
        }
        StringAppend(StringCopy(dst, sNearThe), sBuildingRoleNames[bit]);
        return dst + StringLength(dst);
    }
    if (anchor->locationKind != BEAT_LOCATION_NONE)
    {
        assertf(anchor->locationKind < ARRAY_COUNT(sLocationKindNames), "anchor location kind %d >= %d", anchor->locationKind, ARRAY_COUNT(sLocationKindNames))
        {
            return dst;
        }
        StringAppend(StringCopy(dst, sNearThe), sLocationKindNames[anchor->locationKind]);
        return dst + StringLength(dst);
    }
    if (anchor->routeId < NUM_COUNTIES && DoesCountyHaveCity(anchor->routeId))
    {
        StringAppend(StringCopy(dst, sNearTheRoadTo), GetCountyCityName(anchor->routeId));
        return dst + StringLength(dst);
    }
    if (anchor->routeId != ROUTE_ID_NONE)
    {
        StringCopy(dst, sNearTheRoad);
        return dst + StringLength(dst);
    }
    StringAppend(StringCopy(dst, sInCity), GetCountyCityName(hostCounty));
    return dst + StringLength(dst);
}

u8 *ExpandHintTemplate(u8 *dst, const u8 *src, u32 hostCounty, const struct BeatAnchor *anchor)
{
    while (*src != EOS)
    {
        if (src[0] == PLACEHOLDER_BEGIN && src[1] == PLACEHOLDER_ID_HINT_ANCHOR)
        {
            dst = WriteBeatAnchorReference(dst, hostCounty, anchor);
            src += 2;
        }
        else if (src[0] == PLACEHOLDER_BEGIN)
        {
            *dst++ = *src++;
            *dst++ = *src++;
        }
        else
        {
            *dst++ = *src++;
        }
    }
    *dst = EOS;
    return dst;
}

static u32 GetCurrentMapCounty(void)
{
    return GetCountyFromCityMapSec(gMapHeader.regionMapSectionId);
}

// Script special: VAR_0x8004 = slot; VAR_RESULT = stage of the current map's county subplot.
void GetCurrentSubplotStage(void)
{
    gSpecialVar_Result = GetCountySlotStage(GetCurrentMapCounty(), gSpecialVar_0x8004);
}

// Script special: VAR_0x8004 = slot, VAR_0x8005 = stage.
void SetCurrentSubplotStage(void)
{
    SetCountySlotStage(GetCurrentMapCounty(), gSpecialVar_0x8004, gSpecialVar_0x8005);
}

// Script special: VAR_0x8004 = slot, VAR_0x8005 = beat, VAR_0x8006 = the NPC's selector. Puts the hinter's
// text in gStringVar4 and sets VAR_RESULT = TRUE; FALSE (buffer untouched) means "say generic text". The
// hinter stands in the host city, so it is always close.
void ShowCurrentSubplotHint(void)
{
    u32 county = GetCurrentMapCounty();
    u32 slot = gSpecialVar_0x8004;
    u32 beat = gSpecialVar_0x8005;
    const struct SubplotDesc *desc = GetCountySubplot(county, slot);
    const u8 *text;

    gSpecialVar_Result = FALSE;
    assertf(desc != NULL && beat < desc->numStages, "county %d slot %d has no stage %d to hint about; did the script set VAR_0x8004 (slot) and VAR_0x8005 (beat)?", county, slot, beat)
    {
        return;
    }
    text = GetCountySlotHintAbout(county, slot, beat, SUBPLOT_CLOSE, gSpecialVar_0x8006);
    if (text == NULL)
        return;
    ExpandHintTemplate(gStringVar4, text, county, &desc->stages[beat].anchor);
    BreakStringForFieldMessage(gStringVar4);
    gSpecialVar_Result = TRUE;
}

// Script special: VAR_0x8004 = main-story-index s; VAR_RESULT = TRUE once the index is past s.
void IsMainStoryIndexPast(void)
{
    gSpecialVar_Result = GetMainStoryIndex() > gSpecialVar_0x8004;
}
