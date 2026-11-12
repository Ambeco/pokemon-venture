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

static const u8 gText_Lilycove_Exhibit0FarCurrent[] = _("I heard the new exhibit hall at the MUSEUM in {HINT_CITY} is about to open.");
static const u8 gText_Lilycove_Exhibit0FarAfter0[] = _("The MUSEUM in {HINT_CITY} finally opened its new exhibit hall, I heard.");
static const u8 gText_Lilycove_Exhibit0FarAfter1[] = _("Did you hear? The exhibit hall at the MUSEUM in {HINT_CITY} is open.");
static const u8 gText_Lilycove_Exhibit0CloseCurrent[] = _("The curator is still busy preparing the\nexhibit hall.");
static const u8 gText_Lilycove_Exhibit0CloseAfter[] = _("The exhibit hall is open at last!\nThe curator must be so relieved.");
static const u8 gText_Lilycove_Exhibit1FarCurrent[] = _("The exhibit hall at the MUSEUM in {HINT_CITY} is open, and they want paintings for it!");
static const u8 gText_Lilycove_Exhibit1CloseBefore[] = _("There's talk of a new exhibit hall\n{HINT_ANCHOR}.\pIt's not ready yet, though.");
static const u8 gText_Lilycove_Exhibit1CloseCurrent[] = _("The curator is asking trainers for\npaintings to hang in the exhibit hall.");

static const u8 gText_Rustboro_Cut0FarCurrent[] = _("I heard a man in {HINT_CITY} is looking for a skilled TRAINER to give an HM to.");
static const u8 gText_Rustboro_Cut0FarAfter0[] = _("A man in {HINT_CITY} finally found a TRAINER worthy of his HIDDEN MACHINE.");
static const u8 gText_Rustboro_Cut0FarAfter1[] = _("Did you hear? Someone in {HINT_CITY} got an HM from a man who just stared at them.");
static const u8 gText_Rustboro_Cut0CloseCurrent[] = _("The man {HINT_ANCHOR} keeps staring at passing TRAINERS. What is he up to?");
static const u8 gText_Rustboro_Cut0CloseAfter[] = _("The man {HINT_ANCHOR} found his TRAINER. He was so pleased.");
static const u8 gText_Rustboro_Cut1FarCurrent[] = _("There's a TRAINER in {HINT_CITY} who was given an HM by a stranger, I heard.");
static const u8 gText_Rustboro_Cut1CloseBefore[] = _("People say the man {HINT_ANCHOR} has an HM to give away.");
static const u8 gText_Rustboro_Cut1CloseCurrent[] = _("That man {HINT_ANCHOR} gave away his HM. He says CUT can clear thin trees.");

static const u8 *const sLilycoveStage0Far[] = {gText_Lilycove_Exhibit0FarCurrent};
static const u8 *const sLilycoveStage0Close[] = {gText_Lilycove_Exhibit0CloseCurrent, gText_Lilycove_Exhibit1CloseBefore};
static const u8 *const sLilycoveStage1Far[] = {gText_Lilycove_Exhibit1FarCurrent, gText_Lilycove_Exhibit0FarAfter0, gText_Lilycove_Exhibit0FarAfter1};
static const u8 *const sLilycoveStage1Close[] = {gText_Lilycove_Exhibit1CloseCurrent, gText_Lilycove_Exhibit0CloseAfter};

static const u8 *const sRustboroStage0Far[] = {gText_Rustboro_Cut0FarCurrent};
static const u8 *const sRustboroStage0Close[] = {gText_Rustboro_Cut0CloseCurrent, gText_Rustboro_Cut1CloseBefore};
static const u8 *const sRustboroStage1Far[] = {gText_Rustboro_Cut1FarCurrent, gText_Rustboro_Cut0FarAfter0, gText_Rustboro_Cut0FarAfter1};
static const u8 *const sRustboroStage1Close[] = {gText_Rustboro_Cut1CloseCurrent, gText_Rustboro_Cut0CloseAfter};

#include "data/region/emerald_subplots.h"

const struct SubplotHost *GetSubplotHosts(u32 *numHosts)
{
    *numHosts = ARRAY_COUNT(sEmeraldSubplotHosts);
    return sEmeraldSubplotHosts;
}

const struct SubplotDesc *GetCountySubplot(u32 countyId, u32 slot)
{
    u32 i;

    assertf(slot < SUBPLOTS_PER_COUNTY, "subplot slot %d >= %d; var 0 of a county slice is its chain position", slot, SUBPLOTS_PER_COUNTY)
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

// Position on the line BEFORE, 0, 1, ..., AFTER, so ranges can end at a pseudo-stage.
static u32 StageRank(u32 stage)
{
    if (stage == SUBPLOT_STAGE_BEFORE)
        return 0;
    return stage == SUBPLOT_STAGE_AFTER ? 0x200 : stage + 1;
}

bool32 DoesSubplotHintCover(const struct SubplotHint *hint, u32 subplotStage)
{
    u32 rank = StageRank(subplotStage);

    return StageRank(hint->firstStage) <= rank && rank <= StageRank(hint->lastStage);
}

const struct SubplotHint *FindSubplotHint(const struct SubplotDesc *desc, u32 subplotStage)
{
    u32 i;

    for (i = 0; i < desc->numHints; i++)
    {
        if (DoesSubplotHintCover(&desc->hints[i], subplotStage))
            return &desc->hints[i];
    }
    return NULL;
}

const u8 *GetSubplotHint(const struct SubplotDesc *desc, u32 subplotStage, u32 distance, u32 selector)
{
    const struct SubplotHint *hint;
    const struct SubplotTextList *list;

    assertf(desc != NULL, "no subplot to hint about")
    {
        return NULL;
    }
    hint = FindSubplotHint(desc, subplotStage);
    if (hint == NULL)
        return NULL;
    list = distance == SUBPLOT_FAR ? &hint->far : &hint->close;
    if (list->count == 0)
        return NULL;
    return list->texts[selector % list->count];
}

const u8 *GetCountySlotHint(u32 countyId, u32 slot, u32 distance, u32 selector)
{
    const struct SubplotDesc *desc = GetCountySubplot(countyId, slot);

    return GetSubplotHint(desc, GetSubplotStage(countyId, slot, desc), distance, selector);
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

u32 ResolveSubplotKey(const struct SubplotKeyRef *ref, u32 hostCounty)
{
    switch (ref->type)
    {
    case SUBPLOT_KEY_NONE:
        return MCGUFFIN_ID_NONE;
    case SUBPLOT_KEY_HOST_POOL:
        return GetCountyMcGuffinId(hostCounty, ref->a);
    case SUBPLOT_KEY_ITEM:
        return GetPlotItemMcGuffinId(ref->a, ref->b);
    }
    assertf(FALSE, "unknown SubplotKeyType %d; did you mean to add a case to ResolveSubplotKey?", ref->type)
    {
        return MCGUFFIN_ID_NONE;
    }
    return MCGUFFIN_ID_NONE;
}

u32 GetSubplotHintCount(const struct SubplotDesc *desc, u32 beat, u32 badgesHeld)
{
    assertf(beat < desc->numStages, "no subplot stage %d", beat)
    {
        return 0;
    }
    switch (desc->stages[beat].hintCount)
    {
    case SUBPLOT_COUNT_NONE:
        return 0;
    case SUBPLOT_COUNT_BADGES_SHORT:
        return desc->minBadges > badgesHeld ? desc->minBadges - badgesHeld : 0;
    }
    assertf(FALSE, "unknown SubplotHintCount %d; did you mean to add a case to GetSubplotHintCount?", desc->stages[beat].hintCount)
    {
        return 0;
    }
    return 0;
}

void BufferSubplotHintCount(const struct SubplotDesc *desc, u32 beat)
{
    assertf(beat < desc->numStages, "no subplot stage %d; a pseudo-stage has no hint count", beat)
    {
        return;
    }
    if (desc->stages[beat].hintCount != SUBPLOT_COUNT_NONE)
        ConvertIntToDecimalStringN(gStringVar1, GetSubplotHintCount(desc, beat, GetBadgeCount()), STR_CONV_MODE_LEFT_ALIGN, 3);
}

u32 GetSubplotNeededRoles(const struct SubplotDesc *desc)
{
    u32 stage, roles = desc->requiredRoles;

    for (stage = 0; stage < desc->numStages; stage++)
        roles |= desc->stages[stage].anchor.role | desc->stages[stage].extraRoles;
    return roles;
}

bool32 IsMcGuffinProducedBy(const struct SubplotHost *hosts, u32 numHosts, u32 mcguffinId)
{
    u32 i;

    for (i = 0; i < numHosts; i++)
    {
        const struct SubplotDesc *desc = hosts[i].desc;

        if (desc->producedKey.type == SUBPLOT_KEY_NONE || ResolveSubplotKey(&desc->producedKey, hosts[i].county) != mcguffinId)
            continue;
        if (GetSubplotStage(hosts[i].county, hosts[i].slot, desc) >= desc->producedKeyStage)
            return TRUE;
    }
    return FALSE;
}

bool32 IsSubplotAvailable(const struct SubplotDesc *desc, u32 hostCounty, const struct SubplotHost *hosts, u32 numHosts)
{
    u32 i;

    if (GetBadgeCount() < desc->minBadges)
        return FALSE;
    for (i = 0; i < SUBPLOT_MAX_REQUIRED_KEYS; i++)
    {
        if (desc->requiredKeys[i].type != SUBPLOT_KEY_NONE
         && !IsMcGuffinProducedBy(hosts, numHosts, ResolveSubplotKey(&desc->requiredKeys[i], hostCounty)))
            return FALSE;
    }
    return TRUE;
}

bool32 IsCountySlotAvailable(u32 countyId, u32 slot)
{
    u32 numHosts;
    const struct SubplotHost *hosts = GetSubplotHosts(&numHosts);
    const struct SubplotDesc *desc = GetCountySubplot(countyId, slot);

    assertf(desc != NULL, "county %d hosts no subplot in slot %d", countyId, slot)
    {
        return FALSE;
    }
    return IsSubplotAvailable(desc, countyId, hosts, numHosts);
}

static bool32 HasEarlierProducer(u32 mcguffinId, u32 county, u32 slot, const struct SubplotHost *hosts, u32 numHosts)
{
    u32 i, storyIdx = GetCountyHeader(county)->storyIdx;

    for (i = 0; i < numHosts; i++)
    {
        const struct SubplotDesc *desc = hosts[i].desc;
        u32 hostIdx = GetCountyHeader(hosts[i].county)->storyIdx;

        if (desc->producedKey.type == SUBPLOT_KEY_NONE || ResolveSubplotKey(&desc->producedKey, hosts[i].county) != mcguffinId)
            continue;
        if (hostIdx < storyIdx || (hosts[i].county == county && hosts[i].slot < slot))
            return TRUE;
    }
    return FALSE;
}

u32 CheckSubplotPlacement(const struct SubplotDesc *desc, u32 county, u32 slot, const struct SubplotHost *hosts, u32 numHosts)
{
    u32 needed = GetSubplotNeededRoles(desc), bit, i;

    if (!DoesCountyHaveCity(county))
        return SUBPLOT_PLACEMENT_NO_CITY;
    for (bit = 0; bit < NUM_BUILDING_ROLES; bit++)
    {
        if ((needed & (1u << bit)) && !DoesCountyCityHaveBuilding(county, 1u << bit))
            return SUBPLOT_PLACEMENT_MISSING_ROLE;
    }
    if (desc->producesMainGet && desc->minBadges > 0)
    {
        struct PlotSlot get = GetVanillaPlotSlot(desc->mainGet.kind, desc->mainGet.ordinal);
        struct GymSlot gym = GetGymSlot(desc->minBadges);

        if (4 * get.block + get.step <= 4 * gym.block + gym.step)
            return SUBPLOT_PLACEMENT_BEFORE_BADGES;
    }
    for (i = 0; i < SUBPLOT_MAX_REQUIRED_KEYS; i++)
    {
        if (desc->requiredKeys[i].type != SUBPLOT_KEY_NONE
         && !HasEarlierProducer(ResolveSubplotKey(&desc->requiredKeys[i], county), county, slot, hosts, numHosts))
            return SUBPLOT_PLACEMENT_KEY_NOT_EARLIER;
    }
    return SUBPLOT_PLACEMENT_OK;
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
        else if (src[0] == PLACEHOLDER_BEGIN && src[1] == PLACEHOLDER_ID_HINT_CITY)
        {
            dst = StringCopy(dst, GetCountyCityName(hostCounty));
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

// Script special: VAR_0x8004 = slot, VAR_0x8005 = beat (the stage whose anchor and count the text names),
// VAR_0x8006 = the NPC's selector. Puts the hinter's text for the subplot's current stage in gStringVar4 and
// sets VAR_RESULT = TRUE; FALSE (buffer untouched) means "say generic text". The hinter stands in the host
// city, so it is always close.
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
    text = GetCountySlotHint(county, slot, SUBPLOT_CLOSE, gSpecialVar_0x8006);
    if (text == NULL)
        return;
    BufferSubplotHintCount(desc, beat);
    ExpandHintTemplate(gStringVar4, text, county, &desc->stages[beat].anchor);
    BreakStringForFieldMessage(gStringVar4);
    gSpecialVar_Result = TRUE;
}

// Script special: VAR_0x8004 = slot; VAR_RESULT = TRUE iff the current map's county subplot has its badges and keys.
void IsCurrentSubplotAvailable(void)
{
    gSpecialVar_Result = IsCountySlotAvailable(GetCurrentMapCounty(), gSpecialVar_0x8004);
}

// Script special: VAR_0x8004 = main-story-index s; VAR_RESULT = TRUE once the index is past s.
void IsMainStoryIndexPast(void)
{
    gSpecialVar_Result = GetMainStoryIndex() > gSpecialVar_0x8004;
}
