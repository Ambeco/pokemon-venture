#include "global.h"
#include "test/test.h"
#include "test/map_loaded.h"
#include "event_data.h"
#include "event_object_movement.h"
#include "field_message_box.h"
#include "fieldmap.h"
#include "field_player_avatar.h"
#include "scene.h"
#include "script.h"
#include "string_util.h"
#include "main.h"
#include "overworld.h"
#include "constants/characters.h"
#include "constants/event_object_movement.h"
#include "constants/map_event_ids.h"
#include "constants/maps.h"
#include "constants/scenes.h"

extern const u8 LittlerootTown_EventScript_NeedPokemonTriggerLeft[];
extern const u8 LittlerootTown_Text_OurNewHomeLetsGoInside[];
extern const u8 LittlerootTown_Text_IfYouGoInGrassPokemonWillJumpOut[];
extern const u8 LittlerootTown_Text_DangerousIfYouDontHavePokemon[];

// A grid of walls around the origin: bit set = blocked. Rows are y = -SCENE_WINDOW_HALF_HEIGHT ..., columns x likewise.
struct FakeGrid
{
    s16 originX, originY;
    u16 blocked[2 * SCENE_WINDOW_HALF_HEIGHT + 1]; // bit i = column i
};

static bool32 FakeCanStep(void *context, s16 x, s16 y, enum Direction dir)
{
    struct FakeGrid *grid = context;
    s16 col, row;

    MoveCoords(dir, &x, &y);
    col = x - grid->originX;
    row = y - grid->originY;
    if (col < 0 || col >= 2 * SCENE_WINDOW_HALF_WIDTH + 1 || row < 0 || row >= 2 * SCENE_WINDOW_HALF_HEIGHT + 1)
        return FALSE;
    return !(grid->blocked[row] & (1 << col));
}

static void ExpectPathReaches(s16 x, s16 y, s16 goalX, s16 goalY, const u8 *dirs, u32 length, struct FakeGrid *grid)
{
    u32 i;

    for (i = 0; i < length; i++)
    {
        EXPECT(FakeCanStep(grid, x, y, dirs[i]));
        MoveCoords(dirs[i], &x, &y);
    }
    EXPECT_EQ(x, goalX);
    EXPECT_EQ(y, goalY);
}

TEST("Scene: SceneResolveSide gives the tile on each side relative to the facing")
{
    s16 x, y;

    SceneResolveSide(5, 5, DIR_NORTH, SCENE_SIDE_SELF, &x, &y);
    EXPECT_EQ(x, 5); EXPECT_EQ(y, 5);
    SceneResolveSide(5, 5, DIR_NORTH, SCENE_SIDE_FRONT, &x, &y);
    EXPECT_EQ(x, 5); EXPECT_EQ(y, 4);
    SceneResolveSide(5, 5, DIR_NORTH, SCENE_SIDE_BEHIND, &x, &y);
    EXPECT_EQ(x, 5); EXPECT_EQ(y, 6);
    SceneResolveSide(5, 5, DIR_NORTH, SCENE_SIDE_LEFT, &x, &y);
    EXPECT_EQ(x, 4); EXPECT_EQ(y, 5);
    SceneResolveSide(5, 5, DIR_NORTH, SCENE_SIDE_RIGHT, &x, &y);
    EXPECT_EQ(x, 6); EXPECT_EQ(y, 5);
    SceneResolveSide(5, 5, DIR_EAST, SCENE_SIDE_LEFT, &x, &y);
    EXPECT_EQ(x, 5); EXPECT_EQ(y, 4);
    SceneResolveSide(5, 5, DIR_WEST, SCENE_SIDE_FRONT, &x, &y);
    EXPECT_EQ(x, 4); EXPECT_EQ(y, 5);
    SceneResolveSide(5, 5, DIR_NONE, SCENE_SIDE_FRONT, &x, &y); // no facing counts as south
    EXPECT_EQ(x, 5); EXPECT_EQ(y, 6);
}

TEST("Scene: SceneDirectionToward picks the longer axis, vertical on a tie, and none for the same tile")
{
    EXPECT_EQ(SceneDirectionToward(0, 0, 3, 1), DIR_EAST);
    EXPECT_EQ(SceneDirectionToward(0, 0, -3, 1), DIR_WEST);
    EXPECT_EQ(SceneDirectionToward(0, 0, 1, 3), DIR_SOUTH);
    EXPECT_EQ(SceneDirectionToward(0, 0, 1, -3), DIR_NORTH);
    EXPECT_EQ(SceneDirectionToward(0, 0, 2, 2), DIR_SOUTH);
    EXPECT_EQ(SceneDirectionToward(4, 4, 4, 4), DIR_NONE);
}

TEST("Scene: SceneFindPath is as short as the Manhattan distance on open ground")
{
    static EWRAM_DATA u8 sDirs[SCENE_MAX_PATH];
    struct FakeGrid grid = {-SCENE_WINDOW_HALF_WIDTH, -SCENE_WINDOW_HALF_HEIGHT, {0}};
    u32 length = SceneFindPath(0, 0, 3, -4, FakeCanStep, &grid, sDirs);

    EXPECT_EQ(length, 7);
    ExpectPathReaches(0, 0, 3, -4, sDirs, length, &grid);
}

TEST("Scene: SceneFindPath walks around a wall through its gap")
{
    static EWRAM_DATA u8 sDirs[SCENE_MAX_PATH];
    struct FakeGrid grid = {-SCENE_WINDOW_HALF_WIDTH, -SCENE_WINDOW_HALF_HEIGHT, {0}};
    u32 row, length;

    // A wall across column x = 2 (grid column 10) with a gap at y = 5 (grid row 11).
    for (row = 0; row < 2 * SCENE_WINDOW_HALF_HEIGHT + 1; row++)
        grid.blocked[row] = 1 << (2 + SCENE_WINDOW_HALF_WIDTH);
    grid.blocked[5 + SCENE_WINDOW_HALF_HEIGHT] = 0;
    length = SceneFindPath(0, 0, 4, 0, FakeCanStep, &grid, sDirs);
    EXPECT_EQ(length, 4 + 2 * 5);
    ExpectPathReaches(0, 0, 4, 0, sDirs, length, &grid);
}

TEST("Scene: SceneFindPath fails for a blocked, enclosed or out-of-window goal and gives 0 for the start")
{
    static EWRAM_DATA u8 sDirs[SCENE_MAX_PATH];
    struct FakeGrid grid = {-SCENE_WINDOW_HALF_WIDTH, -SCENE_WINDOW_HALF_HEIGHT, {0}};

    EXPECT_EQ(SceneFindPath(0, 0, 0, 0, FakeCanStep, &grid, sDirs), 0);
    EXPECT_EQ(SceneFindPath(0, 0, SCENE_WINDOW_HALF_WIDTH + 1, 0, FakeCanStep, &grid, sDirs), SCENE_PATH_NONE);
    EXPECT_EQ(SceneFindPath(0, 0, 0, SCENE_WINDOW_HALF_HEIGHT + 1, FakeCanStep, &grid, sDirs), SCENE_PATH_NONE);
    // The goal tile itself is blocked.
    grid.blocked[0 + SCENE_WINDOW_HALF_HEIGHT] = 1 << (3 + SCENE_WINDOW_HALF_WIDTH);
    EXPECT_EQ(SceneFindPath(0, 0, 3, 0, FakeCanStep, &grid, sDirs), SCENE_PATH_NONE);
    // The goal is walled in on all four sides.
    grid.blocked[0 + SCENE_WINDOW_HALF_HEIGHT] = (1 << (2 + SCENE_WINDOW_HALF_WIDTH)) | (1 << (4 + SCENE_WINDOW_HALF_WIDTH));
    grid.blocked[-1 + SCENE_WINDOW_HALF_HEIGHT] = 1 << (3 + SCENE_WINDOW_HALF_WIDTH);
    grid.blocked[1 + SCENE_WINDOW_HALF_HEIGHT] = 1 << (3 + SCENE_WINDOW_HALF_WIDTH);
    EXPECT_EQ(SceneFindPath(0, 0, 3, 0, FakeCanStep, &grid, sDirs), SCENE_PATH_NONE);
}

TEST("Scene: SceneFindPath fails for a path longer than SCENE_MAX_PATH")
{
    static EWRAM_DATA u8 sDirs[SCENE_MAX_PATH];
    struct FakeGrid grid = {-SCENE_WINDOW_HALF_WIDTH, -SCENE_WINDOW_HALF_HEIGHT, {0}};
    u32 col, row;

    // Walls at every other column force a snake through the whole window: far more than SCENE_MAX_PATH steps.
    for (col = 1; col < 2 * SCENE_WINDOW_HALF_WIDTH + 1; col += 2)
    {
        for (row = 0; row < 2 * SCENE_WINDOW_HALF_HEIGHT + 1; row++)
        {
            bool32 gapAtTop = (col / 2) % 2 == 0;
            if (gapAtTop ? row != 0 : row != 2 * SCENE_WINDOW_HALF_HEIGHT)
                grid.blocked[row] |= 1 << col;
        }
    }
    EXPECT_EQ(SceneFindPath(-SCENE_WINDOW_HALF_WIDTH, 0, SCENE_WINDOW_HALF_WIDTH, 0, FakeCanStep, &grid, sDirs), SCENE_PATH_NONE);
}

TEST("Scene: SceneBuildWalkScript writes one walk action per step and ends with step_end")
{
    static EWRAM_DATA u8 sOut[SCENE_MAX_PATH + 3];
    const u8 dirs[] = {DIR_NORTH, DIR_EAST, DIR_SOUTH, DIR_WEST};
    u8 *end = SceneBuildWalkScript(sOut, dirs, ARRAY_COUNT(dirs), FALSE);

    EXPECT(end == sOut + ARRAY_COUNT(dirs));
    EXPECT_EQ(sOut[0], GetWalkNormalMovementAction(DIR_NORTH));
    EXPECT_EQ(sOut[1], GetWalkNormalMovementAction(DIR_EAST));
    EXPECT_EQ(sOut[2], GetWalkNormalMovementAction(DIR_SOUTH));
    EXPECT_EQ(sOut[3], GetWalkNormalMovementAction(DIR_WEST));
    EXPECT_EQ(sOut[4], MOVEMENT_ACTION_STEP_END);
}

TEST("Scene: SceneBuildWalkScript brackets a slide with lock and unlock facing")
{
    static EWRAM_DATA u8 sOut[SCENE_MAX_PATH + 3];
    const u8 dirs[] = {DIR_SOUTH};
    u8 *end = SceneBuildWalkScript(sOut, dirs, ARRAY_COUNT(dirs), TRUE);

    EXPECT(end == sOut + 3);
    EXPECT_EQ(sOut[0], MOVEMENT_ACTION_LOCK_FACING_DIRECTION);
    EXPECT_EQ(sOut[1], GetWalkNormalMovementAction(DIR_SOUTH));
    EXPECT_EQ(sOut[2], MOVEMENT_ACTION_UNLOCK_FACING_DIRECTION);
    EXPECT_EQ(sOut[3], MOVEMENT_ACTION_STEP_END);
}

TEST("Scene: Every scene id has a scene with steps and a name")
{
    u32 id;

    for (id = 0; id < SCENE_COUNT; id++)
    {
        const struct Scene *scene = GetScene(id);

        ASSUME(scene != NULL);
        EXPECT(scene->numSteps > 0);
        EXPECT(scene->name != NULL);
        EXPECT(scene->numActors >= 1 && scene->numActors <= SCENE_MAX_ACTORS);
    }
}

// Runs the overworld until the script ends, pressing A whenever a message is up, and remembers the
// first text of each message box in `seen` (up to maxSeen).
static u32 RunScriptDismissingMessages(u8 seen[][100], u32 maxSeen)
{
    u32 frame, count = 0;
    bool32 wasShowing = FALSE;

    for (frame = 0; frame < 2400 && ScriptContext_IsEnabled(); frame++)
    {
        bool32 showing = !IsFieldMessageBoxHidden();

        if (showing && !wasShowing && count < maxSeen)
            StringCopyN(seen[count++], gStringVar4, 100 - 1), seen[count - 1][99] = EOS;
        wasShowing = showing;
        if (frame % 90 == 89)
            Test_PressKeys(A_BUTTON);
        else
            Test_RunFrames(1);
    }
    return count;
}

TEST("Scene: Littleroot's little girl walks up to the player, warns them, pushes them back and returns to her post")
{
    static EWRAM_DATA u8 sSeen[2][100];
    static EWRAM_DATA u8 sExpect[2][100];
    struct ObjectEvent *twin, *player;
    u32 shown;

    Test_LoadMapAt(MAP_GROUP(MAP_LITTLEROOT_TOWN), MAP_NUM(MAP_LITTLEROOT_TOWN), 10, 1);
    player = &gObjectEvents[gPlayerAvatar.objectEventId];
    twin = &gObjectEvents[GetObjectEventIdByLocalId(LOCALID_LITTLEROOT_TWIN)];
    ASSUME(GetObjectEventIdByLocalId(LOCALID_LITTLEROOT_TWIN) < OBJECT_EVENTS_COUNT);
    ASSUME(twin->initialCoords.x == 7 + MAP_OFFSET && twin->initialCoords.y == 2 + MAP_OFFSET);
    ObjectEventTurn(player, DIR_NORTH);

    ScriptContext_SetupScript(LittlerootTown_EventScript_NeedPokemonTriggerLeft);
    shown = RunScriptDismissingMessages(sSeen, 2);

    EXPECT(!ScriptContext_IsEnabled());
    EXPECT_EQ(shown, 2);
    StringExpandPlaceholders(sExpect[0], LittlerootTown_Text_IfYouGoInGrassPokemonWillJumpOut);
    StringExpandPlaceholders(sExpect[1], LittlerootTown_Text_DangerousIfYouDontHavePokemon);
    EXPECT_EQ(StringCompareN(sSeen[0], sExpect[0], 99), 0);
    EXPECT_EQ(StringCompareN(sSeen[1], sExpect[1], 99), 0);
    // The player was pushed one tile back from the trigger and the girl is back at her post.
    EXPECT_EQ(player->currentCoords.x, 10 + MAP_OFFSET);
    EXPECT_EQ(player->currentCoords.y, 2 + MAP_OFFSET);
    EXPECT_EQ(twin->currentCoords.x, 7 + MAP_OFFSET);
    EXPECT_EQ(twin->currentCoords.y, 2 + MAP_OFFSET);
    EXPECT_EQ((u32)player->facingDirection, DIR_NORTH); // slid, so still facing north
}

TEST("Scene: SceneTriggerFires proximity is Manhattan distance and includes the door tile and the radius edge")
{
    EXPECT(SceneTriggerFires(SCENE_TRIGGER_PROXIMITY, 5, 10, 10, 10, 10));
    EXPECT(SceneTriggerFires(SCENE_TRIGGER_PROXIMITY, 5, 13, 12, 10, 10));  // 3 + 2 = 5: on the edge
    EXPECT(SceneTriggerFires(SCENE_TRIGGER_PROXIMITY, 5, 5, 10, 10, 10));   // 5 + 0
    EXPECT(!SceneTriggerFires(SCENE_TRIGGER_PROXIMITY, 5, 13, 13, 10, 10)); // 6: outside
    EXPECT(!SceneTriggerFires(SCENE_TRIGGER_PROXIMITY, 5, 10, 4, 10, 10));
    EXPECT(SceneTriggerFires(SCENE_TRIGGER_PROXIMITY, 0, 10, 10, 10, 10));
    EXPECT(!SceneTriggerFires(SCENE_TRIGGER_PROXIMITY, 0, 10, 11, 10, 10));
}

TEST("Scene: SceneTriggerFires distinguishes standing on the door from standing in front of it")
{
    EXPECT(SceneTriggerFires(SCENE_TRIGGER_ON_DOOR, 0, 7, 8, 7, 8));
    EXPECT(!SceneTriggerFires(SCENE_TRIGGER_ON_DOOR, 0, 7, 9, 7, 8));
    EXPECT(SceneTriggerFires(SCENE_TRIGGER_IN_FRONT_OF_DOOR, 0, 7, 9, 7, 8));
    EXPECT(!SceneTriggerFires(SCENE_TRIGGER_IN_FRONT_OF_DOOR, 0, 7, 8, 7, 8));
    EXPECT(!SceneTriggerFires(SCENE_TRIGGER_IN_FRONT_OF_DOOR, 0, 7, 7, 7, 8)); // above the door is not in front
}

TEST("Scene: Littleroot's arrival triggers cover the truck drop-off tiles and nothing far away")
{
    const struct MapEvents *events;

    Test_LoadMapAt(MAP_GROUP(MAP_LITTLEROOT_TOWN), MAP_NUM(MAP_LITTLEROOT_TOWN), 10, 12);
    events = gMapHeader.events;
    // Brendan's door is warp 1 at (5, 8) and the truck drops him at (3, 10); May's is warp 0 at (14, 8), dropped at (12, 10).
    ASSUME(events->warps[1].x == 5 && events->warps[1].y == 8 && events->warps[0].x == 14 && events->warps[0].y == 8);
    VarSet(VAR_LITTLEROOT_INTRO_STATE, 1);
    EXPECT(SceneFindTriggerAt(MAP_GROUP(MAP_LITTLEROOT_TOWN), MAP_NUM(MAP_LITTLEROOT_TOWN), 3 + MAP_OFFSET, 10 + MAP_OFFSET) != NULL);
    EXPECT(SceneFindTriggerAt(MAP_GROUP(MAP_LITTLEROOT_TOWN), MAP_NUM(MAP_LITTLEROOT_TOWN), 12 + MAP_OFFSET, 10 + MAP_OFFSET) == NULL);
    EXPECT(SceneFindTriggerAt(MAP_GROUP(MAP_LITTLEROOT_TOWN) + 1, MAP_NUM(MAP_LITTLEROOT_TOWN), 3 + MAP_OFFSET, 10 + MAP_OFFSET) == NULL);
    VarSet(VAR_LITTLEROOT_INTRO_STATE, 2);
    EXPECT(SceneFindTriggerAt(MAP_GROUP(MAP_LITTLEROOT_TOWN), MAP_NUM(MAP_LITTLEROOT_TOWN), 12 + MAP_OFFSET, 10 + MAP_OFFSET) != NULL);
    EXPECT(SceneFindTriggerAt(MAP_GROUP(MAP_LITTLEROOT_TOWN), MAP_NUM(MAP_LITTLEROOT_TOWN), 3 + MAP_OFFSET, 10 + MAP_OFFSET) == NULL);
    EXPECT(SceneFindTriggerAt(MAP_GROUP(MAP_LITTLEROOT_TOWN), MAP_NUM(MAP_LITTLEROOT_TOWN), 14 + MAP_OFFSET, 20 + MAP_OFFSET) == NULL);
    VarSet(VAR_LITTLEROOT_INTRO_STATE, 3); // past stage 0: no trigger
    EXPECT(SceneFindTriggerAt(MAP_GROUP(MAP_LITTLEROOT_TOWN), MAP_NUM(MAP_LITTLEROOT_TOWN), 12 + MAP_OFFSET, 10 + MAP_OFFSET) == NULL);
}

// Drops the player where the truck does, with the intro flags InsideOfTruck sets, and lets the arrival trigger start
// the scene by itself. Returns whether the first message was the mom's line; ends with the destination map loaded.
// (Compares against gStringVar2 on the spot: the test build has no EWRAM to spare for buffers.)
static bool32 RunArrival(u32 introState, u32 dropX, u32 hideFlagToSet, u32 hideFlagToClear)
{
    u32 frame;
    bool32 wasShowing = FALSE, gotText = FALSE, textMatched = FALSE;

    VarSet(VAR_LITTLEROOT_INTRO_STATE, introState);
    FlagSet(hideFlagToSet);
    FlagClear(hideFlagToClear);
    FlagSet(FLAG_HIDE_LITTLEROOT_TOWN_MOM_OUTSIDE);
    FlagSet(FLAG_HIDE_LITTLEROOT_TOWN_FAT_MAN);
    FlagSet(FLAG_HIDE_LITTLEROOT_TOWN_BIRCH); // new-game flags: Birch and the rival stand on the truck row otherwise
    FlagSet(FLAG_HIDE_LITTLEROOT_TOWN_RIVAL);
    Test_LoadMapAt(MAP_GROUP(MAP_LITTLEROOT_TOWN), MAP_NUM(MAP_LITTLEROOT_TOWN), dropX, 10);
    // The warp-in fade never finishes without VBlank, so the field controls stay locked: unlock them.
    UnlockPlayerFieldControls();
    for (frame = 0; frame < 60 && !ScriptContext_IsEnabled(); frame++)
        Test_RunFieldFrames(1);
    ASSUME(ScriptContext_IsEnabled()); // the trigger started the scene
    for (frame = 0; frame < 3000 && ScriptContext_IsEnabled(); frame++)
    {
        bool32 showing = !IsFieldMessageBoxHidden();

        if (showing && !wasShowing && !gotText)
        {
            StringExpandPlaceholders(gStringVar2, LittlerootTown_Text_OurNewHomeLetsGoInside);
            textMatched = StringCompareN(gStringVar4, gStringVar2, 60) == 0; // StringCompare compared far past the text here; 60 characters is enough to tell the lines apart
            gotText = TRUE;
        }
        wasShowing = showing;
        if (frame % 90 == 89)
            Test_PressKeys(A_BUTTON);
        else
            Test_RunFrames(1);
        if (gMain.callback2 == CB2_LoadMap)
            break;
    }
    ASSUME(gotText);
    Test_FinishWarp();
    return textMatched;
}

TEST("Scene: Littleroot's arrival starts by itself for the boy, walks his mom in and warps into Brendan's house")
{
    EXPECT(RunArrival(1, 3, FLAG_HIDE_LITTLEROOT_TOWN_MAYS_HOUSE_TRUCK, FLAG_HIDE_LITTLEROOT_TOWN_BRENDANS_HOUSE_TRUCK));
    EXPECT_EQ((u32)gSaveBlock1Ptr->location.mapGroup, MAP_GROUP(MAP_LITTLEROOT_TOWN_BRENDANS_HOUSE_1F));
    EXPECT_EQ((u32)gSaveBlock1Ptr->location.mapNum, MAP_NUM(MAP_LITTLEROOT_TOWN_BRENDANS_HOUSE_1F));
    EXPECT_GE(VarGet(VAR_LITTLEROOT_INTRO_STATE), 3);
    EXPECT(FlagGet(FLAG_HIDE_LITTLEROOT_TOWN_MOM_OUTSIDE));
    EXPECT(!FlagGet(FLAG_HIDE_LITTLEROOT_TOWN_FAT_MAN));
    EXPECT(!FlagGet(FLAG_HIDE_MAP_NAME_POPUP));
    EXPECT(FlagGet(FLAG_HIDE_LITTLEROOT_TOWN_BRENDANS_HOUSE_TRUCK));
}

TEST("Scene: Littleroot's arrival starts by itself for the girl, walks her mom in and warps into May's house")
{
    EXPECT(RunArrival(2, 12, FLAG_HIDE_LITTLEROOT_TOWN_BRENDANS_HOUSE_TRUCK, FLAG_HIDE_LITTLEROOT_TOWN_MAYS_HOUSE_TRUCK));
    EXPECT_EQ((u32)gSaveBlock1Ptr->location.mapGroup, MAP_GROUP(MAP_LITTLEROOT_TOWN_MAYS_HOUSE_1F));
    EXPECT_EQ((u32)gSaveBlock1Ptr->location.mapNum, MAP_NUM(MAP_LITTLEROOT_TOWN_MAYS_HOUSE_1F));
    EXPECT_GE(VarGet(VAR_LITTLEROOT_INTRO_STATE), 3);
    EXPECT(FlagGet(FLAG_HIDE_LITTLEROOT_TOWN_MOM_OUTSIDE));
    EXPECT(!FlagGet(FLAG_HIDE_LITTLEROOT_TOWN_FAT_MAN));
    EXPECT(FlagGet(FLAG_HIDE_LITTLEROOT_TOWN_MAYS_HOUSE_TRUCK));
}

TEST("Scene: a trigger does not fire after stage 0 or without a step or map load")
{
    Test_LoadMapAt(MAP_GROUP(MAP_LITTLEROOT_TOWN), MAP_NUM(MAP_LITTLEROOT_TOWN), 3, 10);
    VarSet(VAR_LITTLEROOT_INTRO_STATE, 6);
    Test_RunFieldFrames(30);
    EXPECT(!ScriptContext_IsEnabled());
    VarSet(VAR_LITTLEROOT_INTRO_STATE, 1);
    Test_RunFieldFrames(30); // the arrival check was consumed by the first frame; standing still does not recheck
    EXPECT(!ScriptContext_IsEnabled());
}
