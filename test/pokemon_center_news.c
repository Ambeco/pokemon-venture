#include "global.h"
#include "test/test.h"
#include "test/map_loaded.h"
#include "test/overworld_script.h"
#include "city_hinters.h"
#include "event_object_movement.h"
#include "field_message_box.h"
#include "line_break.h"
#include "overworld.h"
#include "region_county.h"
#include "region_city.h"
#include "script.h"
#include "string_util.h"
#include "chapter.h"
#include "constants/maps.h"
#include "constants/script_commands.h"
#include "constants/region_map_sections.h"
#include "constants/characters.h"
#include "event_data.h"
#include "plot_stage.h"

#define LILYCOVE COUNTY_ID(4, 0)
#define RUSTBORO COUNTY_ID(0, 1)

extern const u8 LilycoveCity_PokemonCenter_1F_EventScript_Boy[];
extern const u8 LilycoveCity_EventScript_Man3[];
extern const u8 LilycoveCity_EventScript_RichBoy[];
extern const u8 RustboroCity_EventScript_Woman[];
extern const u8 RustboroCity_EventScript_Twin[];
extern const u8 RustboroCity_CuttersHouse_EventScript_Cutter[];
extern const u8 MauvilleCity_House1_EventScript_RockSmashDude[];
extern const u8 MauvilleCity_House2_EventScript_AcceptTrade[];

// Talks to the NPC bound to `script` and runs frames until its message has printed.
static void TalkTo(const u8 *script)
{
    u32 i, id = Test_FindObjectByScript(script);

    ASSUME(id < OBJECT_EVENTS_COUNT);
    EXPECT(GetObjectEventScriptPointerByObjectEventId(id) == script);
    gStringVar4[0] = EOS;
    gSpecialVar_LastTalked = gObjectEvents[id].localId;
    ScriptContext_SetupScript(script);
    // The text is in gStringVar4 before `lock` finishes waiting for a walking NPC, so wait for the box itself.
    for (i = 0; i < 300 && IsFieldMessageBoxHidden(); i++)
        Test_RunFrames(1);
    ASSUME(!IsFieldMessageBoxHidden());
    // Long text is still printing for a while, and A pressed meanwhile only speeds it up.
    Test_RunFrames(600);
}

// The message stays up until A is pressed (once per page, at most MAX_PAGES), then the script ends.
#define MAX_PAGES 4
static void ExpectDismissedByA(void)
{
    u32 presses;

    EXPECT(ScriptContext_IsEnabled());
    Test_RunFrames(30);
    EXPECT(ScriptContext_IsEnabled());
    for (presses = 0; presses < MAX_PAGES && ScriptContext_IsEnabled(); presses++)
    {
        Test_PressKeys(A_BUTTON);
        // Later pages print for a while; closemessage and release can wait on the NPC's walking.
        Test_RunFrames(600);
    }
    EXPECT(!ScriptContext_IsEnabled());
}

TEST("PokemonCenterNews: Talking to Lilycove's Pokemon Center Boy shows the county's news until A is pressed")
{
    static EWRAM_DATA u8 sExpect[300];

    Test_LoadMap(MAP_GROUP(MAP_LILYCOVE_CITY_POKEMON_CENTER_1F), MAP_NUM(MAP_LILYCOVE_CITY_POKEMON_CENTER_1F));
    TalkTo(LilycoveCity_PokemonCenter_1F_EventScript_Boy);
    WritePokemonCenterNews(sExpect, GetMainStoryIndex(), LILYCOVE);
    EXPECT_EQ(StringCompare(gStringVar4, sExpect), 0);
    ExpectDismissedByA();
}

TEST("PokemonCenterNews: The special writes the news for the map's county and succeeds")
{
    static EWRAM_DATA u8 sExpect[300];
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

// A city NPC bound by hand as a chapter hinter says the planner's hint for its (slot, beat, selector).
static void ExpectHinterSpeaksIn(u32 county, u32 mapGroup, u32 mapNum, const u8 *script, u32 plannedIndex, u32 x, u32 y)
{
    static EWRAM_DATA u8 sExpect[300];
    struct HinterPlacement out[MAX_COUNTY_HINTERS];
    const struct ChapterDesc *desc;
    const u8 *text;
    u32 count;

    Test_LoadMapAt(mapGroup, mapNum, x, y);
    count = PlanCountyHinters(county, out, ARRAY_COUNT(out));
    ASSUME(count > plannedIndex);
    desc = GetCountyChapter(county, out[plannedIndex].slot);
    ASSUME(desc != NULL);
    text = GetCountySlotHint(county, out[plannedIndex].slot, CHAPTER_CLOSE, out[plannedIndex].selector);
    ASSUME(text != NULL);
    ExpandHintTemplate(sExpect, text, county, &desc->stages[out[plannedIndex].beat].anchor);
    BreakStringForFieldMessage(sExpect);

    TalkTo(script);
    EXPECT_EQ(StringCompare(gStringVar4, sExpect), 0);
    ExpectDismissedByA();
}

static void ExpectHinterSpeaks(const u8 *script, u32 plannedIndex, u32 x, u32 y)
{
    ExpectHinterSpeaksIn(LILYCOVE, MAP_GROUP(MAP_LILYCOVE_CITY), MAP_NUM(MAP_LILYCOVE_CITY), script, plannedIndex, x, y);
}

TEST("PokemonCenterNews: Lilycove's first city hinter says the planner's hint for its slot, beat and selector")
{
    ExpectHinterSpeaks(LilycoveCity_EventScript_Man3, 0, 28, 30);
}

TEST("PokemonCenterNews: Lilycove's second city hinter says the planner's hint for its slot, beat and selector")
{
    ExpectHinterSpeaks(LilycoveCity_EventScript_RichBoy, 1, 16, 11);
}

TEST("PokemonCenterNews: Rustboro's first city hinter says the planner's hint for its slot, beat and selector")
{
    ExpectHinterSpeaksIn(RUSTBORO, MAP_GROUP(MAP_RUSTBORO_CITY), MAP_NUM(MAP_RUSTBORO_CITY), RustboroCity_EventScript_Woman, 0, 22, 34);
}

TEST("PokemonCenterNews: Rustboro's second city hinter says the planner's hint for its slot, beat and selector")
{
    ExpectHinterSpeaksIn(RUSTBORO, MAP_GROUP(MAP_RUSTBORO_CITY), MAP_NUM(MAP_RUSTBORO_CITY), RustboroCity_EventScript_Twin, 1, 21, 46);
}

// `setvar VAR_0x8004, <slot>`, `setvar VAR_0x8005, 1` then `special`, as a script sets a chapter slot to stage 1 after its reward.
static bool32 ScriptSetsSlotToStage1(const u8 *script, u8 slot)
{
    const u8 setStage[] = {SCR_OP_SETVAR, 0x04, 0x80, slot, 0x00, SCR_OP_SETVAR, 0x05, 0x80, 0x01, 0x00, SCR_OP_SPECIAL};
    u32 i, j;

    for (i = 0; i + sizeof(setStage) < 120; i++)
    {
        for (j = 0; j < sizeof(setStage) && script[i + j] == setStage[j]; j++)
            ;
        if (j == sizeof(setStage))
            return TRUE;
    }
    return FALSE;
}

TEST("PokemonCenterNews: The Cutter's script sets slot 0 to stage 1 when it gives the HM")
{
    EXPECT(ScriptSetsSlotToStage1(RustboroCity_CuttersHouse_EventScript_Cutter, 0));
}

TEST("PokemonCenterNews: The Rock Smash dude's script sets slot 0 to stage 1 when it gives the HM")
{
    EXPECT(ScriptSetsSlotToStage1(MauvilleCity_House1_EventScript_RockSmashDude, 0));
}

TEST("PokemonCenterNews: The Coin Case trade sets slot 1 to stage 1 when it gives the case")
{
    EXPECT(ScriptSetsSlotToStage1(MauvilleCity_House2_EventScript_AcceptTrade, 1));
}
