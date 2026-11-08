/* Harness for tests that need a real map loaded in the overworld.
 *
 * Test_LoadMap(group, num) warps the player into the map and runs the load
 * callbacks until CB2_Overworld is reached, so object events, the camera and
 * the script context are live. Test_RunFrames(n) then runs n overworld frames,
 * e.g. after ScriptContext_SetupScript(npcScript) to run an NPC's script in
 * the normal (global) script context.
 *
 * The map load resets the heap: call Test_LoadMap before allocating anything
 * in the test. Leaks are expected afterwards (Test_ExpectLeaks is set).
 * There is no VBlank, so nothing reaches VRAM or OAM; only game state
 * (scripts, objects, strings) can be checked. */
#ifndef GUARD_TEST_MAP_LOADED
#define GUARD_TEST_MAP_LOADED

#include "global.h"

#define TEST_MAP_LOAD_MAX_FRAMES 1200

void Test_LoadMap(u32 mapGroup, u32 mapNum);
void Test_RunFrames(u32 frames);

// Returns the object event id, or OBJECT_EVENTS_COUNT if no active object stands where the template does.
u32 Test_FindObjectByScript(const u8 *script);

#endif
