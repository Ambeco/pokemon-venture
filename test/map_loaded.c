#include "global.h"
#include "test/test.h"
#include "test/map_loaded.h"
#include "event_object_movement.h"
#include "main.h"
#include "malloc.h"
#include "overworld.h"
#include "event_data.h"
#include "fieldmap.h"
#include "constants/characters.h"
#include "script.h"
#include "string_util.h"
#include "city_hinters.h"
#include "region_county.h"
#include "plot_stage.h"
#include "constants/map_groups.h"
#include "constants/maps.h"
#include "constants/region_map_sections.h"

#define LILYCOVE COUNTY_ID(4, 0)

extern const u8 LilycoveCity_PokemonCenter_1F_EventScript_Boy[];
void ReinitCallbacks(void);

// Drives the overworld directly so gMain's callbacks stay owned by the test runner.
static void PumpFrame(void)
{
    gMain.newKeys = 0;
    gMain.heldKeys = 0;
    CB2_Overworld();
}

void Test_RunFrames(u32 frames)
{
    u32 i;

    for (i = 0; i < frames; i++)
        PumpFrame();
}

// The map load resets the heap (MoveSaveBlocks_ResetHeap), which would invalidate the runner's own
// heap-allocated state, so it is saved and re-allocated around the load. Anything else the test
// allocated before calling Test_LoadMap is lost.
void Test_LoadMap(u32 mapGroup, u32 mapNum)
{
    static EWRAM_DATA struct FunctionTestRunnerState sSavedRunnerState;
    u32 i;

    sSavedRunnerState = *gFunctionTestRunnerState;

    SetWarpDestinationToMapWarp(mapGroup, mapNum, WARP_ID_NONE);
    WarpIntoMap();
    SetMainCallback2(CB2_LoadMap);
    for (i = 0; i < TEST_MAP_LOAD_MAX_FRAMES && gMain.callback2 != CB2_Overworld; i++)
    {
        if (gMain.callback1)
            gMain.callback1();
        gMain.callback2();
    }
    if (gMain.callback2 != CB2_Overworld)
        Test_ExitWithResult(TEST_RESULT_INVALID, __LINE__, ":L%s:%d: map %d.%d did not reach CB2_Overworld in %d frames", gTestRunnerState.test->filename, __LINE__, mapGroup, mapNum, TEST_MAP_LOAD_MAX_FRAMES);
    // The runner only continues while callback2 is CB2_TestRunner.
    ReinitCallbacks();
    gFunctionTestRunnerState = AllocZeroed(sizeof(*gFunctionTestRunnerState));
    *gFunctionTestRunnerState = sSavedRunnerState;
    // The loaded map's allocations are never freed.
    Test_ExpectLeaks(TRUE);
    Test_RestartAfterTest();
}

u32 Test_FindObjectByScript(const u8 *script)
{
    u32 t, id;

    for (t = 0; t < gMapHeader.events->objectEventCount; t++)
    {
        const struct ObjectEventTemplate *template = &gMapHeader.events->objectEvents[t];

        if (template->script != script)
            continue;
        for (id = 0; id < OBJECT_EVENTS_COUNT; id++)
        {
            if (gObjectEvents[id].active
             && gObjectEvents[id].currentCoords.x == template->x + MAP_OFFSET
             && gObjectEvents[id].currentCoords.y == template->y + MAP_OFFSET)
                return id;
        }
    }
    return OBJECT_EVENTS_COUNT;
}

TEST("MapLoaded: Lilycove's Pokemon Center loads with the Boy bound to the script and the news text prints")
{
    static EWRAM_DATA u8 sExpect[200];
    u32 id;

    Test_LoadMap(MAP_GROUP(MAP_LILYCOVE_CITY_POKEMON_CENTER_1F), MAP_NUM(MAP_LILYCOVE_CITY_POKEMON_CENTER_1F));
    EXPECT_EQ(gMapHeader.regionMapSectionId, MAPSEC_LILYCOVE_CITY);

    id = Test_FindObjectByScript(LilycoveCity_PokemonCenter_1F_EventScript_Boy);
    ASSUME(id < OBJECT_EVENTS_COUNT);
    EXPECT(GetObjectEventScriptPointerByObjectEventId(id) == LilycoveCity_PokemonCenter_1F_EventScript_Boy);

    gStringVar4[0] = EOS;
    gSpecialVar_LastTalked = gObjectEvents[id].localId;
    ScriptContext_SetupScript(LilycoveCity_PokemonCenter_1F_EventScript_Boy);
    Test_RunFrames(30);

    WritePokemonCenterNews(sExpect, GetMainStoryIndex(), LILYCOVE);
    EXPECT_EQ(StringCompare(gStringVar4, sExpect), 0);
}
