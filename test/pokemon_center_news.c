#include "global.h"
#include "test/test.h"
#include "test/overworld_script.h"
#include "city_hinters.h"
#include "event_object_movement.h"
#include "overworld.h"
#include "region_county.h"
#include "region_city.h"
#include "string_util.h"
#include "constants/map_groups.h"
#include "constants/maps.h"
#include "constants/region_map_sections.h"
#include "constants/script_commands.h"
#include "constants/characters.h"
#include "event_data.h"
#include "plot_stage.h"

#define LILYCOVE COUNTY_ID(4, 0)

extern const u8 LilycoveCity_PokemonCenter_1F_EventScript_Boy[];
extern const u8 LilycoveCity_EventScript_Man3[];
extern const u8 LilycoveCity_EventScript_RichBoy[];
extern const u8 Common_EventScript_PokemonCenterNews[];
extern const u8 Common_EventScript_TrySubplotHint[];

static const u8 *ReadPointer(const u8 *p)
{
    return (const u8 *)(p[0] | (p[1] << 8) | (p[2] << 16) | ((u32)p[3] << 24));
}

// Script bytes are read as written by the event.inc macros: goto/call are 1 + 4 bytes, setvar is 1 + 2 + 2.
static const u8 *ReadGotoTarget(const u8 *script)
{
    return script[0] == SCR_OP_GOTO ? ReadPointer(script + 1) : NULL;
}

static bool32 ReadSetvar(const u8 **script, u32 var, u32 *value)
{
    const u8 *s = *script;

    if (s[0] != SCR_OP_SETVAR || (s[1] | (s[2] << 8)) != var)
        return FALSE;
    *value = s[3] | (s[4] << 8);
    *script += 5;
    return TRUE;
}

// Reads the subplot_hinter macro's expansion; FALSE if the script differs.
static bool32 ReadSubplotHinter(const u8 *script, u32 *slot, u32 *beat, u32 *selector)
{
    if (!ReadSetvar(&script, 0x8004, slot) || !ReadSetvar(&script, 0x8005, beat) || !ReadSetvar(&script, 0x8006, selector))
        return FALSE;
    return script[0] == SCR_OP_CALL && ReadPointer(script + 1) == Common_EventScript_TrySubplotHint;
}

static const struct ObjectEventTemplate *FindTemplateByScript(u32 map, const u8 *script)
{
    const struct MapHeader *header = Overworld_GetMapHeaderByGroupAndId(map >> 8, map & 0xFF);
    u32 i;

    for (i = 0; i < header->events->objectEventCount; i++)
    {
        if (header->events->objectEvents[i].script == script)
            return &header->events->objectEvents[i];
    }
    return NULL;
}

TEST("PokemonCenterNews: Lilycove's Pokemon Center Boy is an object event that goes to the shared news script")
{
    const struct ObjectEventTemplate *boy = FindTemplateByScript(MAP_LILYCOVE_CITY_POKEMON_CENTER_1F, LilycoveCity_PokemonCenter_1F_EventScript_Boy);

    ASSUME(boy != NULL);
    EXPECT(ReadGotoTarget(boy->script) == Common_EventScript_PokemonCenterNews);
}

TEST("PokemonCenterNews: The special writes the news for the map's county and succeeds")
{
    static EWRAM_DATA u8 sExpect[200];
    struct MapHeader saved = gMapHeader;

    gMapHeader.regionMapSectionId = MAPSEC_LILYCOVE_CITY;
    gStringVar4[0] = EOS;
    RUN_OVERWORLD_SCRIPT(special ShowPokemonCenterNews;);
    gMapHeader = saved;

    EXPECT_EQ(gSpecialVar_Result, TRUE);
    EXPECT(gStringVar4[0] != EOS);
    WritePokemonCenterNews(sExpect, GetMainStoryIndex(), LILYCOVE);
    EXPECT_EQ(StringCompare(gStringVar4, sExpect), 0);
}

TEST("PokemonCenterNews: Lilycove's two city hinters pass the slot, beat and selector the planner gives")
{
    struct HinterPlacement out[MAX_COUNTY_HINTERS];
    const u8 *scripts[] = { LilycoveCity_EventScript_Man3, LilycoveCity_EventScript_RichBoy };
    u32 i, count = PlanCountyHinters(LILYCOVE, out, ARRAY_COUNT(out));

    ASSUME(count >= ARRAY_COUNT(scripts));
    for (i = 0; i < ARRAY_COUNT(scripts); i++)
    {
        u32 slot = 0, beat = 0, selector = 0;

        EXPECT(FindTemplateByScript(MAP_LILYCOVE_CITY, scripts[i]) != NULL);
        EXPECT(ReadSubplotHinter(scripts[i], &slot, &beat, &selector));
        EXPECT_EQ(slot, out[i].slot);
        EXPECT_EQ(beat, out[i].beat);
        EXPECT_EQ(selector, out[i].selector);
    }
}
