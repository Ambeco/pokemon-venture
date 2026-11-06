#include "global.h"
#include "constants/vars.h"
#include "assertf.h"
#include "event_data.h"
#include "global.fieldmap.h"
#include "region_city.h"
#include "region_tile.h"
#include "subplot.h"

static const u8 gText_Lilycove_Exhibit0FarCurrent[] = _("I heard the new exhibit hall at the\nLILYCOVE MUSEUM is about to open.");
static const u8 gText_Lilycove_Exhibit0FarAfter0[] = _("The LILYCOVE MUSEUM finally opened its\nnew exhibit hall, I heard.");
static const u8 gText_Lilycove_Exhibit0FarAfter1[] = _("Did you hear? The exhibit hall at the\nLILYCOVE MUSEUM is open.");
static const u8 gText_Lilycove_Exhibit0CloseCurrent[] = _("The curator is still busy preparing the\nexhibit hall.");
static const u8 gText_Lilycove_Exhibit0CloseAfter[] = _("The exhibit hall is open at last!\nThe curator must be so relieved.");
static const u8 gText_Lilycove_Exhibit1FarCurrent[] = _("The LILYCOVE MUSEUM's exhibit hall is\nopen, and they want paintings for it!");
static const u8 gText_Lilycove_Exhibit1CloseBefore[] = _("There's talk of a new exhibit hall\nhere. It's not ready yet, though.");
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
