#include "global.h"
#include "constants/vars.h"
#include "assertf.h"
#include "event_data.h"
#include "global.fieldmap.h"
#include "region_city.h"
#include "region_tile.h"
#include "subplot.h"

static const u8 gText_LilycoveMuseumHint_NotShown[] = _("I heard the new exhibit hall at the\nMUSEUM is about to open.");
static const u8 gText_LilycoveMuseumHint_Shown[] = _("The MUSEUM's exhibit hall is open now.\nThey want paintings for it!");
static const u8 gText_LilycoveMuseumState_NotShown[] = _("The curator is still preparing the\nexhibit hall.");
static const u8 gText_LilycoveMuseumState_Shown[] = _("The curator is asking trainers for\npaintings to hang in the exhibit hall.");

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

const u8 *GetSubplotText(const struct SubplotDesc *desc, u32 stage, u32 kind)
{
    assertf(desc != NULL && stage < desc->numStages, "no subplot stage %d", stage)
    {
        return NULL;
    }
    return kind == SUBPLOT_TEXT_HINT ? desc->stages[stage].hintText : desc->stages[stage].stateText;
}

const u8 *GetCountySlotText(u32 countyId, u32 slot, u32 kind)
{
    const struct SubplotDesc *desc = GetCountySubplot(countyId, slot);

    return GetSubplotText(desc, GetSubplotStage(countyId, slot, desc), kind);
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
