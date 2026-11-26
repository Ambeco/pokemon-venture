#include "global.h"
#include "test/test.h"
#include "test/map_loaded.h"
#include "test/overworld_script.h"
#include "battle.h"
#include "battle_setup.h"
#include "field_specials.h"
#include "item.h"
#include "pokemon.h"
#include "script_menu.h"
#include "event_data.h"
#include "event_object_movement.h"
#include "field_message_box.h"
#include "fieldmap.h"
#include "field_player_avatar.h"
#include "scene.h"
#include "rival.h"
#include "chapter.h"
#include "region_county.h"
#include "script.h"
#include "string_util.h"
#include "main.h"
#include "overworld.h"
#include "constants/flags.h"
#include "constants/battle_setup.h"
#include "constants/characters.h"
#include "constants/event_objects.h"
#include "constants/items.h"
#include "constants/opponents.h"
#include "constants/species.h"
#include "constants/event_object_movement.h"
#include "constants/map_event_ids.h"
#include "constants/maps.h"
#include "constants/scenes.h"

extern const u8 Scene_EventScript_RunTriggered[];
extern const u8 LittlerootTown_Text_CanYouGoSeeWhatsHappening[];
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

// Both Route 103 rival scenes ask "battle or not yet"; the first branch fights whichever trainer the rival API picks.
TEST("Scene: Each Route 103 rival scene asks a choice whose first branch fights the rival")
{
    static const u16 sceneIds[] = {SCENE_ROUTE103_RIVAL_MAY, SCENE_ROUTE103_RIVAL_BRENDAN};
    u32 i;

    for (i = 0; i < ARRAY_COUNT(sceneIds); i++)
    {
        const struct Scene *scene = GetScene(sceneIds[i]);
        const struct SceneStep *ask = &scene->steps[0];

        ASSUME(scene->numSteps == 1);
        EXPECT_EQ(ask->kind, SCENE_STEP_ASK_CHOICE);
        ASSUME(ask->numBranches == 2);
        EXPECT(ask->branches[0].label != NULL && ask->branches[1].label != NULL);
        EXPECT_EQ(ask->branches[0].numSteps, 1);
        EXPECT_EQ(ask->branches[0].steps[0].kind, SCENE_STEP_BATTLE);
        EXPECT(ask->branches[0].steps[0].rival);
        EXPECT_EQ(ask->branches[0].steps[0].trainer, RIVAL_ENCOUNTER_ROUTE_103);
        EXPECT(ask->branches[0].steps[0].text != NULL);
        EXPECT_EQ(ask->branches[1].numSteps, 1);
        EXPECT_EQ(ask->branches[1].steps[0].kind, SCENE_STEP_SAY);
    }
}

TEST("Rival: GetRivalTrainer picks the opposite gender's trainer for the player's starter")
{
    static const u16 may[] = {TRAINER_MAY_ROUTE_103_TREECKO, TRAINER_MAY_ROUTE_103_TORCHIC, TRAINER_MAY_ROUTE_103_MUDKIP};
    static const u16 brendan[] = {TRAINER_BRENDAN_ROUTE_103_TREECKO, TRAINER_BRENDAN_ROUTE_103_TORCHIC, TRAINER_BRENDAN_ROUTE_103_MUDKIP};
    u32 starter;

    for (starter = 0; starter < 3; starter++)
    {
        VarSet(VAR_STARTER_MON, starter);
        gSaveBlock2Ptr->playerGender = MALE;
        EXPECT_EQ(GetRivalGender(), FEMALE);
        EXPECT_EQ(GetRivalTrainer(RIVAL_ENCOUNTER_ROUTE_103), may[starter]);
        gSaveBlock2Ptr->playerGender = FEMALE;
        EXPECT_EQ(GetRivalGender(), MALE);
        EXPECT_EQ(GetRivalTrainer(RIVAL_ENCOUNTER_ROUTE_103), brendan[starter]);
    }
}

TEST("Rival: HasRivalBeenDefeated reads the flag of the trainer picked now")
{
    VarSet(VAR_STARTER_MON, 1);
    gSaveBlock2Ptr->playerGender = MALE;
    FlagClear(TRAINER_FLAGS_START + TRAINER_MAY_ROUTE_103_TORCHIC);
    EXPECT(!HasRivalBeenDefeated(RIVAL_ENCOUNTER_ROUTE_103));
    FlagSet(TRAINER_FLAGS_START + TRAINER_MAY_ROUTE_103_TORCHIC);
    EXPECT(HasRivalBeenDefeated(RIVAL_ENCOUNTER_ROUTE_103));
    FlagClear(TRAINER_FLAGS_START + TRAINER_MAY_ROUTE_103_TORCHIC);
}

// Shared by the message tests (test builds are short of EWRAM).
static EWRAM_DATA u8 sSeen[2][100];
static EWRAM_DATA u8 sExpect[2][100];

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
    struct ObjectEvent *twin, *player;
    u32 shown;

    Test_LoadMapAt(MAP_GROUP(MAP_LITTLEROOT_TOWN), MAP_NUM(MAP_LITTLEROOT_TOWN), 10, 1);
    player = &gObjectEvents[gPlayerAvatar.objectEventId];
    twin = &gObjectEvents[GetObjectEventIdByLocalId(LOCALID_LITTLEROOT_TWIN)];
    ASSUME(GetObjectEventIdByLocalId(LOCALID_LITTLEROOT_TWIN) < OBJECT_EVENTS_COUNT);
    ASSUME(twin->initialCoords.x == 7 + MAP_OFFSET && twin->initialCoords.y == 2 + MAP_OFFSET);
    ObjectEventTurn(player, DIR_NORTH);

    VarSet(VAR_SAVE_BIRCH_BEAT, SAVE_BIRCH_TWIN_GUARDS);
    EXPECT(SceneTryStartTrigger(TRUE)); // the exit trigger, not a ROM coord event
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

TEST("Scene: SceneExitTriggerFires measures the depth from the named edge")
{
    EXPECT(SceneExitTriggerFires(DIR_NORTH, 1, 5, 1, 20, 20));
    EXPECT(!SceneExitTriggerFires(DIR_NORTH, 1, 5, 0, 20, 20));
    EXPECT(!SceneExitTriggerFires(DIR_NORTH, 1, 5, 2, 20, 20));
    EXPECT(SceneExitTriggerFires(DIR_NORTH, 0, 5, 0, 20, 20));
    EXPECT(SceneExitTriggerFires(DIR_SOUTH, 1, 5, 18, 20, 20));
    EXPECT(SceneExitTriggerFires(DIR_SOUTH, 0, 5, 19, 20, 20));
    EXPECT(SceneExitTriggerFires(DIR_WEST, 2, 2, 7, 20, 30));
    EXPECT(SceneExitTriggerFires(DIR_EAST, 1, 18, 7, 20, 30));
    EXPECT(!SceneExitTriggerFires(DIR_EAST, 1, 19, 7, 20, 30));
}

TEST("Scene: Littleroot's exit triggers sit at the north gap only, and follow the stage")
{
    u32 group = MAP_GROUP(MAP_LITTLEROOT_TOWN), num = MAP_NUM(MAP_LITTLEROOT_TOWN);
    const struct SceneTrigger *trigger;

    Test_LoadMapAt(group, num, 10, 3);
    VarSet(VAR_SAVE_BIRCH_BEAT, SAVE_BIRCH_TWIN_GUARDS);
    trigger = SceneFindTriggerAt(group, num, 10 + MAP_OFFSET, 1 + MAP_OFFSET, FALSE);
    ASSUME(trigger != NULL);
    EXPECT_EQ(trigger->sceneId, SCENE_LITTLEROOT_NEED_POKEMON);
    EXPECT(SceneFindTriggerAt(group, num, 11 + MAP_OFFSET, 1 + MAP_OFFSET, FALSE) != NULL);
    EXPECT(SceneFindTriggerAt(group, num, 10 + MAP_OFFSET, 2 + MAP_OFFSET, FALSE) == NULL); // one tile too deep
    EXPECT(SceneFindTriggerAt(group, num, 10 + MAP_OFFSET, 0 + MAP_OFFSET, FALSE) == NULL);
    EXPECT(SceneFindTriggerAt(group, num, 3 + MAP_OFFSET, 1 + MAP_OFFSET, FALSE) == NULL); // the edge ahead is trees
    VarSet(VAR_SAVE_BIRCH_BEAT, SAVE_BIRCH_TWIN_ASKS);
    trigger = SceneFindTriggerAt(group, num, 11 + MAP_OFFSET, 1 + MAP_OFFSET, FALSE);
    ASSUME(trigger != NULL);
    EXPECT_EQ(trigger->sceneId, SCENE_LITTLEROOT_TWIN_ASKS);
    VarSet(VAR_SAVE_BIRCH_BEAT, SAVE_BIRCH_BIRCH_IN_GRASS);
    EXPECT(SceneFindTriggerAt(group, num, 11 + MAP_OFFSET, 1 + MAP_OFFSET, FALSE) == NULL);
}

TEST("Scene: the twin stops a player who has a Pokémon, asks them to go and look, and sends them to Route 101")
{
    struct ObjectEvent *twin, *player;

    VarSet(VAR_SAVE_BIRCH_BEAT, SAVE_BIRCH_TWIN_ASKS); // before the load: OnTransition places the twin by the stage
    Test_LoadMapAt(MAP_GROUP(MAP_LITTLEROOT_TOWN), MAP_NUM(MAP_LITTLEROOT_TOWN), 11, 1);
    player = &gObjectEvents[gPlayerAvatar.objectEventId];
    twin = &gObjectEvents[GetObjectEventIdByLocalId(LOCALID_LITTLEROOT_TWIN)];
    ASSUME(GetObjectEventIdByLocalId(LOCALID_LITTLEROOT_TWIN) < OBJECT_EVENTS_COUNT);
    ASSUME(twin->initialCoords.x == 10 + MAP_OFFSET && twin->initialCoords.y == 1 + MAP_OFFSET);
    ObjectEventTurn(player, DIR_NORTH);

    EXPECT(SceneTryStartTrigger(TRUE));
    EXPECT_EQ(RunScriptDismissingMessages(sSeen, 1), 1);
    EXPECT(!ScriptContext_IsEnabled());
    gSaveBlock2Ptr->playerGender = MALE;
    GetPlayerBigGuyGirlString();
    StringExpandPlaceholders(sExpect[0], LittlerootTown_Text_CanYouGoSeeWhatsHappening);
    EXPECT_EQ(StringCompareN(sSeen[0], sExpect[0], 99), 0);
    EXPECT_EQ(VarGet(VAR_SAVE_BIRCH_BEAT), SAVE_BIRCH_BIRCH_IN_GRASS);
    EXPECT_EQ(player->currentCoords.x, 11 + MAP_OFFSET);
    EXPECT_EQ(player->currentCoords.y, 1 + MAP_OFFSET);
    EXPECT_EQ((u32)player->facingDirection, DIR_WEST); // turned to the twin, who stands at (10, 1)
    EXPECT_EQ((u32)twin->facingDirection, DIR_NORTH);  // and she is back to facing her post
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
    VarSet(VAR_MOVING_IN_BEAT, 1);
    EXPECT(SceneFindTriggerAt(MAP_GROUP(MAP_LITTLEROOT_TOWN), MAP_NUM(MAP_LITTLEROOT_TOWN), 3 + MAP_OFFSET, 10 + MAP_OFFSET, FALSE) != NULL);
    EXPECT(SceneFindTriggerAt(MAP_GROUP(MAP_LITTLEROOT_TOWN), MAP_NUM(MAP_LITTLEROOT_TOWN), 12 + MAP_OFFSET, 10 + MAP_OFFSET, FALSE) == NULL);
    EXPECT(SceneFindTriggerAt(MAP_GROUP(MAP_LITTLEROOT_TOWN) + 1, MAP_NUM(MAP_LITTLEROOT_TOWN), 3 + MAP_OFFSET, 10 + MAP_OFFSET, FALSE) == NULL);
    VarSet(VAR_MOVING_IN_BEAT, 2);
    EXPECT(SceneFindTriggerAt(MAP_GROUP(MAP_LITTLEROOT_TOWN), MAP_NUM(MAP_LITTLEROOT_TOWN), 12 + MAP_OFFSET, 10 + MAP_OFFSET, FALSE) != NULL);
    EXPECT(SceneFindTriggerAt(MAP_GROUP(MAP_LITTLEROOT_TOWN), MAP_NUM(MAP_LITTLEROOT_TOWN), 3 + MAP_OFFSET, 10 + MAP_OFFSET, FALSE) == NULL);
    EXPECT(SceneFindTriggerAt(MAP_GROUP(MAP_LITTLEROOT_TOWN), MAP_NUM(MAP_LITTLEROOT_TOWN), 14 + MAP_OFFSET, 20 + MAP_OFFSET, FALSE) == NULL);
    VarSet(VAR_MOVING_IN_BEAT, 3); // past stage 0: no trigger
    EXPECT(SceneFindTriggerAt(MAP_GROUP(MAP_LITTLEROOT_TOWN), MAP_NUM(MAP_LITTLEROOT_TOWN), 12 + MAP_OFFSET, 10 + MAP_OFFSET, FALSE) == NULL);
}

// Drops the player where the truck does, with the intro flags InsideOfTruck sets, and lets the arrival trigger start
// the scene by itself. Returns whether the first message was the mom's line; ends with the destination map loaded.
// (Compares against gStringVar2 on the spot: the test build has no EWRAM to spare for buffers.)
static bool32 RunArrival(u32 introState, u32 dropX, u32 hideFlagToSet, u32 hideFlagToClear)
{
    u32 frame;
    bool32 wasShowing = FALSE, gotText = FALSE, textMatched = FALSE;

    VarSet(VAR_MOVING_IN_BEAT, introState);
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
    EXPECT_GE(VarGet(VAR_MOVING_IN_BEAT), 3);
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
    EXPECT_GE(VarGet(VAR_MOVING_IN_BEAT), 3);
    EXPECT(FlagGet(FLAG_HIDE_LITTLEROOT_TOWN_MOM_OUTSIDE));
    EXPECT(!FlagGet(FLAG_HIDE_LITTLEROOT_TOWN_FAT_MAN));
    EXPECT(FlagGet(FLAG_HIDE_LITTLEROOT_TOWN_MAYS_HOUSE_TRUCK));
}

TEST("Scene: a trigger does not fire after stage 0 or without a step or map load")
{
    Test_LoadMapAt(MAP_GROUP(MAP_LITTLEROOT_TOWN), MAP_NUM(MAP_LITTLEROOT_TOWN), 3, 10);
    VarSet(VAR_MOVING_IN_BEAT, 6);
    Test_RunFieldFrames(30);
    EXPECT(!ScriptContext_IsEnabled());
    VarSet(VAR_MOVING_IN_BEAT, 1);
    Test_RunFieldFrames(30); // the arrival check was consumed by the first frame; standing still does not recheck
    EXPECT(!ScriptContext_IsEnabled());
}

TEST("Scene: a ring has 8 or 10 tiles numbered clockwise from its top-left corner, and its step directions close the loop")
{
    static EWRAM_DATA u8 sDirs[10];
    struct SceneRing ring = {10, 20, 4, 3};
    s16 x, y;
    u32 i;

    EXPECT_EQ(SceneRingLength(&ring), 10);
    SceneRingTile(&ring, 0, &x, &y);
    EXPECT_EQ(x, 10); EXPECT_EQ(y, 20);
    SceneRingTile(&ring, 3, &x, &y);
    EXPECT_EQ(x, 13); EXPECT_EQ(y, 20);
    SceneRingTile(&ring, 5, &x, &y);
    EXPECT_EQ(x, 13); EXPECT_EQ(y, 22);
    SceneRingTile(&ring, 8, &x, &y);
    EXPECT_EQ(x, 10); EXPECT_EQ(y, 22);
    SceneRingTile(&ring, 10, &x, &y); // wraps
    EXPECT_EQ(x, 10); EXPECT_EQ(y, 20);
    EXPECT_EQ(SceneRingIndexOf(&ring, 12, 22), 6);
    EXPECT_EQ(SceneRingIndexOf(&ring, 11, 21), SCENE_RING_NONE); // the inside is not on the ring
    EXPECT_EQ(SceneRingIndexOf(&ring, 9, 20), SCENE_RING_NONE);

    SceneRingDirs(&ring, 0, 10, sDirs);
    EXPECT_EQ(sDirs[0], DIR_EAST); EXPECT_EQ(sDirs[2], DIR_EAST);
    EXPECT_EQ(sDirs[3], DIR_SOUTH); EXPECT_EQ(sDirs[4], DIR_SOUTH);
    EXPECT_EQ(sDirs[5], DIR_WEST); EXPECT_EQ(sDirs[7], DIR_WEST);
    EXPECT_EQ(sDirs[8], DIR_NORTH); EXPECT_EQ(sDirs[9], DIR_NORTH);
    x = 10; y = 20;
    for (i = 0; i < 10; i++)
        MoveCoords(sDirs[i], &x, &y);
    EXPECT_EQ(x, 10); EXPECT_EQ(y, 20);

    ring.width = 3;
    EXPECT_EQ(SceneRingLength(&ring), 8);
    SceneRingDirs(&ring, 6, 4, sDirs); // starts mid-ring and runs past the wrap
    SceneRingTile(&ring, 6, &x, &y);
    for (i = 0; i < 4; i++)
        MoveCoords(sDirs[i], &x, &y);
    EXPECT_EQ(SceneRingIndexOf(&ring, x, y), 2); // 6 + 4 = 10, which wraps to 2
}

static s32 OpenGroundScore(void *context, s16 x, s16 y)
{
    return 0;
}

// Higher the farther east, blocked on the one tile `context` points to.
static s32 EastwardScore(void *context, s16 x, s16 y)
{
    s16 *blocked = context;

    if (blocked != NULL && x == blocked[0] && y == blocked[1])
        return -1;
    return x;
}

static s32 BlockedScore(void *context, s16 x, s16 y)
{
    return -1;
}

TEST("Scene: SceneFindRing on open ground picks the 3x3 ring centered on the search center")
{
    struct SceneRing ring;

    ASSUME(SceneFindRing(30, 30, 3, 0, 0, 0, OpenGroundScore, NULL, &ring));
    EXPECT_EQ(ring.x, 29); EXPECT_EQ(ring.y, 29);
    EXPECT_EQ(ring.width, 3); EXPECT_EQ(ring.height, 3);
}

TEST("Scene: SceneFindRing keeps every ring tile more than the avoid radius from the avoid point")
{
    struct SceneRing ring;
    u32 i;

    ASSUME(SceneFindRing(30, 30, 3, 31, 30, 1, OpenGroundScore, NULL, &ring)); // the avoid point is next to the center
    for (i = 0; i < SceneRingLength(&ring); i++)
    {
        s16 x, y;
        s32 dx, dy;

        SceneRingTile(&ring, i, &x, &y);
        dx = x > 31 ? x - 31 : 31 - x;
        dy = y > 30 ? y - 30 : 30 - y;
        EXPECT((dx > dy ? dx : dy) > 1);
    }
}

TEST("Scene: SceneFindRing prefers the highest scoring ring, and skips rings that use a blocked tile")
{
    struct SceneRing ring;
    s16 blocked[2] = {33, 30};

    ASSUME(SceneFindRing(30, 30, 3, 0, 0, 0, EastwardScore, NULL, &ring));
    EXPECT_EQ(ring.x, 33); // as far east as the search allows
    EXPECT_EQ(SceneRingLength(&ring), 10); // and a longer ring has more tiles to score
    ASSUME(SceneFindRing(30, 30, 3, 0, 0, 0, EastwardScore, blocked, &ring));
    EXPECT(SceneRingIndexOf(&ring, 33, 30) == SCENE_RING_NONE);
    EXPECT(ring.x + ring.width > 33); // still the eastern edge: blocking one tile only rules out rings through it
}

TEST("Scene: SceneFindRing fails when every tile is blocked or the avoid radius covers the whole search area")
{
    struct SceneRing ring;

    EXPECT(!SceneFindRing(30, 30, 3, 0, 0, 0, BlockedScore, NULL, &ring));
    EXPECT(!SceneFindRing(30, 30, 3, 30, 30, 20, OpenGroundScore, NULL, &ring));
}

TEST("Scene: the Route 101 rescue trigger fires only on a map load while Birch is in the grass")
{
    u32 group = MAP_GROUP(MAP_ROUTE101), num = MAP_NUM(MAP_ROUTE101);

    Test_LoadMapAt(group, num, 10, 19);
    VarSet(VAR_SAVE_BIRCH_BEAT, SAVE_BIRCH_BIRCH_IN_GRASS);
    EXPECT(SceneFindTriggerAt(group, num, 10 + MAP_OFFSET, 19 + MAP_OFFSET, TRUE) != NULL);
    EXPECT(SceneFindTriggerAt(group, num, 10 + MAP_OFFSET, 19 + MAP_OFFSET, FALSE) == NULL);
    EXPECT(SceneFindTriggerAt(group + 1, num, 10 + MAP_OFFSET, 19 + MAP_OFFSET, TRUE) == NULL);
    VarSet(VAR_SAVE_BIRCH_BEAT, SAVE_BIRCH_AWAIT_BAG);
    EXPECT(SceneFindTriggerAt(group, num, 10 + MAP_OFFSET, 19 + MAP_OFFSET, TRUE) == NULL);
}

// In-ROM run of the rescue: place, music, chase_loop, pause and face all run on the real Route 101.
static u32 TileDistance(struct ObjectEvent *a, struct ObjectEvent *b)
{
    return (a->currentCoords.x > b->currentCoords.x ? a->currentCoords.x - b->currentCoords.x : b->currentCoords.x - a->currentCoords.x)
         + (a->currentCoords.y > b->currentCoords.y ? a->currentCoords.y - b->currentCoords.y : b->currentCoords.y - a->currentCoords.y);
}

TEST("Scene: Birch's rescue on Route 101 places Birch and the Zigzagoon, circles, asks for help and advances")
{
    u8 sSeen[2][100]; // on the stack: the test build has no EWRAM to spare
    struct ObjectEvent *birch, *zigzagoon;
    u32 shown;

    Test_LoadMapAt(MAP_GROUP(MAP_ROUTE101), MAP_NUM(MAP_ROUTE101), 10, 19);
    VarSet(VAR_SAVE_BIRCH_BEAT, SAVE_BIRCH_BIRCH_IN_GRASS);
    FlagClear(FLAG_SCENE_NO_WILD);
    gSpecialVar_0x8004 = SCENE_ROUTE101_RESCUE;
    ScriptContext_SetupScript(Scene_EventScript_RunTriggered);
    shown = RunScriptDismissingMessages(sSeen, 2);

    EXPECT(!ScriptContext_IsEnabled());
    EXPECT_EQ(shown, 2);
    EXPECT_EQ(VarGet(VAR_SAVE_BIRCH_BEAT), SAVE_BIRCH_AWAIT_BAG);
    EXPECT(FlagGet(FLAG_SCENE_NO_WILD));
    EXPECT(FlagGet(FLAG_HIDE_MAP_NAME_POPUP));
    ASSUME(GetObjectEventIdByLocalId(LOCALID_ROUTE101_BIRCH) < OBJECT_EVENTS_COUNT);
    ASSUME(GetObjectEventIdByLocalId(LOCALID_ROUTE101_ZIGZAGOON) < OBJECT_EVENTS_COUNT);
    birch = &gObjectEvents[GetObjectEventIdByLocalId(LOCALID_ROUTE101_BIRCH)];
    zigzagoon = &gObjectEvents[GetObjectEventIdByLocalId(LOCALID_ROUTE101_ZIGZAGOON)];
    EXPECT(!birch->invisible);
    // They finished the laps on the ring, a few tiles apart, and not on top of the player.
    EXPECT(TileDistance(birch, zigzagoon) <= 4);
    EXPECT(TileDistance(birch, &gObjectEvents[gPlayerAvatar.objectEventId]) > 2);
}

static void ExpectChaseKeepsMoving(s16 startX)
{
    struct ObjectEvent *birch, *zigzagoon;
    u32 frame, birchSteps = 0, zigzagoonSteps = 0;
    s16 birchX, birchY, zigX, zigY;

    Test_LoadMapAt(MAP_GROUP(MAP_ROUTE101), MAP_NUM(MAP_ROUTE101), startX, 19);
    VarSet(VAR_SAVE_BIRCH_BEAT, SAVE_BIRCH_BIRCH_IN_GRASS);
    FlagClear(FLAG_SCENE_NO_WILD);
    gSpecialVar_0x8004 = SCENE_ROUTE101_RESCUE;
    ScriptContext_SetupScript(Scene_EventScript_RunTriggered);
    birch = zigzagoon = NULL;
    for (frame = 0; frame < 1500 && ScriptContext_IsEnabled(); frame++)
    {
        if (birch == NULL && GetObjectEventIdByLocalId(LOCALID_ROUTE101_BIRCH) < OBJECT_EVENTS_COUNT && GetObjectEventIdByLocalId(LOCALID_ROUTE101_ZIGZAGOON) < OBJECT_EVENTS_COUNT)
        {
            birch = &gObjectEvents[GetObjectEventIdByLocalId(LOCALID_ROUTE101_BIRCH)];
            zigzagoon = &gObjectEvents[GetObjectEventIdByLocalId(LOCALID_ROUTE101_ZIGZAGOON)];
            birchX = birch->currentCoords.x; birchY = birch->currentCoords.y;
            zigX = zigzagoon->currentCoords.x; zigY = zigzagoon->currentCoords.y;
        }
        if (frame % 20 == 19)
            Test_PressKeys(A_BUTTON);
        else
            Test_RunFrames(1);
        if (birch == NULL)
            continue;
        if (birch->currentCoords.x != birchX || birch->currentCoords.y != birchY)
            birchSteps++;
        if (zigzagoon->currentCoords.x != zigX || zigzagoon->currentCoords.y != zigY)
            zigzagoonSteps++;
        birchX = birch->currentCoords.x; birchY = birch->currentCoords.y;
        zigX = zigzagoon->currentCoords.x; zigY = zigzagoon->currentCoords.y;
    }
    ASSUME(birch != NULL);
    // Two laps of a ring of 8 or 10 tiles, plus the approach: a stalled loop leaves the second lap unwalked.
    EXPECT_GE(birchSteps, 16);
    EXPECT_GE(zigzagoonSteps, 16);
}

TEST("Scene: Birch and the Zigzagoon keep moving through every lap of the chase, wherever the player enters")
{
    s16 x;

    for (x = 8; x <= 13; x++)
        ExpectChaseKeepsMoving(x);
}

// Scenes that are not in the scene table: const data here, run with SceneBeginScene.
extern const u8 Scene_EventScript_RunPrepared[];

// Runs a prepared scene for a fixed number of frames, pressing the keys of `cycle` in turn, one every 20 frames
// and repeating the cycle. Keys pressed while no menu is up only advance messages or do nothing. Not until the
// script ends: ScriptContext_IsEnabled is false while the script waits on a menu.
static void RunSceneWithKeys(const struct Scene *scene, const u16 *cycle, u32 cycleLength)
{
    u32 frame;

    gSpecialVar_Result = 0xFFFF;
    SceneBeginScene(scene);
    ScriptContext_SetupScript(Scene_EventScript_RunPrepared);
    for (frame = 0; frame < 900 && !(gSpecialVar_Result == SCENE_RESULT_END && !ScriptContext_IsEnabled()); frame++)
    {
        if (frame % 20 == 19)
            Test_PressKeys(cycle[(frame / 20) % cycleLength]);
        else
            Test_RunFrames(1);
    }
    EXPECT_EQ(gSpecialVar_Result, SCENE_RESULT_END); // the driver's last step result: the scene finished
}

// For an ask_choice scene: waits until the menu is up (its labels have left the dynamic multichoice stack), moves
// the cursor down `downs` entries, confirms, and runs on to the end of the scene.
static void RunSceneChoosing(const struct Scene *scene, u32 downs)
{
    u32 frame;
    bool32 sawLabels = FALSE, menuUp = FALSE;

    gSpecialVar_Result = 0xFFFF;
    SceneBeginScene(scene);
    ScriptContext_SetupScript(Scene_EventScript_RunPrepared);
    for (frame = 0; frame < 600 && !menuUp; frame++)
    {
        Test_RunFrames(1);
        if (MultichoiceDynamic_PeekElement() != NULL)
            sawLabels = TRUE;
        else if (sawLabels)
            menuUp = TRUE;
    }
    ASSUME(menuUp);
    Test_RunFrames(10);
    for (frame = 0; frame < downs; frame++)
    {
        Test_PressKeys(DPAD_DOWN);
        Test_RunFrames(3);
    }
    Test_PressKeys(A_BUTTON);
    for (frame = 0; frame < 300 && !(gSpecialVar_Result == SCENE_RESULT_END && !ScriptContext_IsEnabled()); frame++)
        Test_RunFrames(1);
    EXPECT_EQ(gSpecialVar_Result, SCENE_RESULT_END);
}

// Runs the prepared scene's script command by command, not frame by frame, until it waits (as it does for a battle).
static void RunSceneUntilWait(const struct Scene *scene)
{
    u32 i;

    SceneBeginScene(scene);
    ScriptContext_SetupScript(Scene_EventScript_RunPrepared);
    for (i = 0; i < 500 && ScriptContext_RunScript(); i++)
        ;
}

static void LoadTestMap(void)
{
    u32 flag;

    Test_LoadMapAt(MAP_GROUP(MAP_LITTLEROOT_TOWN), MAP_NUM(MAP_LITTLEROOT_TOWN), 10, 1);
    for (flag = FLAG_TEMP_1; flag <= FLAG_TEMP_5; flag++)
        FlagClear(flag);
    VarSet(VAR_TEMP_1, 0);
}

#define TEST_SCENE(sceneName, stepsArray) \
    static const struct Scene sceneName = \
    { \
        .name = #sceneName, \
        .actors = {LOCALID_PLAYER}, \
        .numActors = 1, \
        .stageVar = VAR_TEMP_1, \
        .numSteps = ARRAY_COUNT(stepsArray), \
        .steps = stepsArray, \
    }

static const u16 sKeysA[] = {A_BUTTON};
static const u16 sKeysB[] = {B_BUTTON};

static const u8 sTestQuestion[] = _("Question?");
static const u8 sTestInnerQuestion[] = _("Inner question?");
static const u8 sTestPrompt[] = _("Which one?");
static const u8 sTestLabelOne[] = _("One");
static const u8 sTestLabelTwo[] = _("Two");
static const u8 sTestLabelThree[] = _("Three");
static const u8 sTestDefeatText[] = _("Beaten.");

static const struct SceneStep sSetFlag1[] = {SCENE_SET_FLAG(FLAG_TEMP_1)};
static const struct SceneStep sSetFlag2[] = {SCENE_SET_FLAG(FLAG_TEMP_2)};
static const struct SceneStep sSetFlag3[] = {SCENE_SET_FLAG(FLAG_TEMP_3)};
static const struct SceneStep sSetFlag4[] = {SCENE_SET_FLAG(FLAG_TEMP_4)};

static const struct SceneBranch sYesNoBranches[] = {SCENE_BRANCH(NULL, sSetFlag1), SCENE_BRANCH(NULL, sSetFlag2)};
static const struct SceneStep sYesNoSteps[] =
{
    SCENE_ASK_YES_NO(sTestQuestion, sYesNoBranches),
    SCENE_SET_FLAG(FLAG_TEMP_5),
    SCENE_ADVANCE(1),
};
TEST_SCENE(sYesNoScene, sYesNoSteps);

TEST("Scene: ask_yes_no runs the yes branch on A, then carries on with the steps after it")
{
    LoadTestMap();
    RunSceneWithKeys(&sYesNoScene, sKeysA, ARRAY_COUNT(sKeysA));
    EXPECT(FlagGet(FLAG_TEMP_1));
    EXPECT(!FlagGet(FLAG_TEMP_2));
    EXPECT(FlagGet(FLAG_TEMP_5));
    EXPECT_EQ(VarGet(VAR_TEMP_1), 1);
}

TEST("Scene: ask_yes_no runs the no branch on B, then carries on with the steps after it")
{
    LoadTestMap();
    RunSceneWithKeys(&sYesNoScene, sKeysB, ARRAY_COUNT(sKeysB));
    EXPECT(!FlagGet(FLAG_TEMP_1));
    EXPECT(FlagGet(FLAG_TEMP_2));
    EXPECT(FlagGet(FLAG_TEMP_5));
    EXPECT_EQ(VarGet(VAR_TEMP_1), 1);
}

static const struct SceneBranch sInnerBranches[] = {SCENE_BRANCH(NULL, sSetFlag3), SCENE_BRANCH(NULL, sSetFlag4)};
static const struct SceneStep sInnerSteps[] =
{
    SCENE_ASK_YES_NO(sTestInnerQuestion, sInnerBranches),
    SCENE_SET_FLAG(FLAG_TEMP_1),
};
static const struct SceneBranch sOuterBranches[] = {SCENE_BRANCH(NULL, sInnerSteps), SCENE_EMPTY_BRANCH(NULL)};
static const struct SceneStep sNestedSteps[] =
{
    SCENE_ASK_YES_NO(sTestQuestion, sOuterBranches),
    SCENE_SET_FLAG(FLAG_TEMP_5),
};
TEST_SCENE(sNestedScene, sNestedSteps);

TEST("Scene: a branch can ask again, and the steps after each question still run")
{
    LoadTestMap();
    RunSceneWithKeys(&sNestedScene, sKeysA, ARRAY_COUNT(sKeysA));
    EXPECT(FlagGet(FLAG_TEMP_3));
    EXPECT(!FlagGet(FLAG_TEMP_4));
    EXPECT(FlagGet(FLAG_TEMP_1)); // the rest of the outer branch
    EXPECT(FlagGet(FLAG_TEMP_5)); // the rest of the scene
}

TEST("Scene: a no answer to the outer question skips the inner question and its branch")
{
    LoadTestMap();
    RunSceneWithKeys(&sNestedScene, sKeysB, ARRAY_COUNT(sKeysB));
    EXPECT(!FlagGet(FLAG_TEMP_3));
    EXPECT(!FlagGet(FLAG_TEMP_4));
    EXPECT(!FlagGet(FLAG_TEMP_1));
    EXPECT(FlagGet(FLAG_TEMP_5));
}

static const struct SceneBranch sChoiceBranches[] =
{
    SCENE_BRANCH(sTestLabelOne, sSetFlag1),
    SCENE_BRANCH(sTestLabelTwo, sSetFlag2),
    SCENE_BRANCH(sTestLabelThree, sSetFlag3),
};
static const struct SceneStep sChoiceSteps[] =
{
    SCENE_ASK_CHOICE(sTestPrompt, sChoiceBranches),
    SCENE_SET_FLAG(FLAG_TEMP_5),
};
TEST_SCENE(sChoiceScene, sChoiceSteps);

TEST("Scene: ask_choice runs the first branch when A confirms the first entry")
{
    LoadTestMap();
    RunSceneChoosing(&sChoiceScene, 0);
    EXPECT(FlagGet(FLAG_TEMP_1));
    EXPECT(!FlagGet(FLAG_TEMP_2));
    EXPECT(!FlagGet(FLAG_TEMP_3));
    EXPECT(FlagGet(FLAG_TEMP_5));
}

TEST("Scene: ask_choice runs only the middle branch when the cursor is moved down once")
{
    LoadTestMap();
    RunSceneChoosing(&sChoiceScene, 1);
    EXPECT(!FlagGet(FLAG_TEMP_1));
    EXPECT(FlagGet(FLAG_TEMP_2));
    EXPECT(!FlagGet(FLAG_TEMP_3));
    EXPECT(FlagGet(FLAG_TEMP_5));
}

TEST("Scene: ask_choice runs only the last branch when the cursor is moved down to it")
{
    LoadTestMap();
    RunSceneChoosing(&sChoiceScene, 2);
    EXPECT(!FlagGet(FLAG_TEMP_1));
    EXPECT(!FlagGet(FLAG_TEMP_2));
    EXPECT(FlagGet(FLAG_TEMP_3));
    EXPECT(FlagGet(FLAG_TEMP_5));
}

// Std_ObtainItem never returns under the headless overworld tests (probably showitemdescription or the fanfare wait
// without a VBlank), so a give scene is stepped from C, standing in for the driver: the item is not really added
// (a script does that); `received` is what the std script leaves in VAR_0x8007.
static void StepGiveSceneByHand(const struct Scene *scene, bool32 received, u32 *givenItem, u32 *givenQuantity)
{
    u32 i;

    SceneBeginScene(scene);
    for (i = 0; i < 50; i++)
    {
        SceneNextStep();
        if (gSpecialVar_Result == SCENE_RESULT_GIVE)
        {
            *givenItem = gSpecialVar_0x8000;
            *givenQuantity = gSpecialVar_0x8001;
            gSpecialVar_0x8007 = received;
            SceneTakeAnswer();
        }
        else if (gSpecialVar_Result == SCENE_RESULT_ASK_YES_NO)
        {
            gSpecialVar_Result = TRUE; // yes
            SceneTakeAnswer();
        }
        else if (gSpecialVar_Result == SCENE_RESULT_END)
        {
            return;
        }
    }
    EXPECT(FALSE); // the scene never ended
}

static const struct SceneStep sGiveSteps[] =
{
    SCENE_GIVE(ITEM_POTION, 2),
    SCENE_SET_FLAG(FLAG_TEMP_5),
    SCENE_ADVANCE(1),
};
TEST_SCENE(sGiveScene, sGiveSteps);

TEST("Scene: give hands the item and quantity to the std script, and carries on and advances once it was received")
{
    u32 item = 0, quantity = 0;

    LoadTestMap();
    StepGiveSceneByHand(&sGiveScene, TRUE, &item, &quantity);
    EXPECT_EQ(item, ITEM_POTION);
    EXPECT_EQ(quantity, 2);
    EXPECT(FlagGet(FLAG_TEMP_5));
    EXPECT_EQ(VarGet(VAR_TEMP_1), 1);
}

TEST("Scene: give with a full bag ends the scene without running later steps or advancing")
{
    u32 item = 0, quantity = 0;

    LoadTestMap();
    StepGiveSceneByHand(&sGiveScene, FALSE, &item, &quantity);
    EXPECT_EQ(item, ITEM_POTION);
    EXPECT(!FlagGet(FLAG_TEMP_5));
    EXPECT_EQ(VarGet(VAR_TEMP_1), 0);
}

TEST("Scene: give with a full bag runs its bag-full branch and then ends the scene")
{
    u32 item = 0, quantity = 0;
    struct SceneBranch full[] = {SCENE_BRANCH(NULL, sSetFlag1)};
    struct SceneStep steps[] = {SCENE_GIVE_OR_ELSE(ITEM_POTION, 1, full), SCENE_SET_FLAG(FLAG_TEMP_5), SCENE_ADVANCE(1)};
    struct Scene scene = {.name = "GiveFullBranch", .actors = {LOCALID_PLAYER}, .numActors = 1, .stageVar = VAR_TEMP_1, .numSteps = ARRAY_COUNT(steps), .steps = steps};

    LoadTestMap();
    StepGiveSceneByHand(&scene, FALSE, &item, &quantity);
    EXPECT(FlagGet(FLAG_TEMP_1));
    EXPECT(!FlagGet(FLAG_TEMP_5));
    EXPECT_EQ(VarGet(VAR_TEMP_1), 0);
}

TEST("Scene: a full-bag give inside a yes branch ends the whole scene: nothing after the branch or the question runs")
{
    u32 item = 0, quantity = 0;
    struct SceneStep inner[] = {SCENE_GIVE(ITEM_POTION, 1), SCENE_SET_FLAG(FLAG_TEMP_4)};
    struct SceneBranch branches[] = {SCENE_BRANCH(NULL, inner), SCENE_EMPTY_BRANCH(NULL)};
    struct SceneStep steps[] = {SCENE_ASK_YES_NO(sTestQuestion, branches), SCENE_SET_FLAG(FLAG_TEMP_5), SCENE_ADVANCE(1)};
    struct Scene scene = {.name = "GiveFullNested", .actors = {LOCALID_PLAYER}, .numActors = 1, .stageVar = VAR_TEMP_1, .numSteps = ARRAY_COUNT(steps), .steps = steps};

    LoadTestMap();
    StepGiveSceneByHand(&scene, FALSE, &item, &quantity);
    EXPECT_EQ(item, ITEM_POTION);
    EXPECT(!FlagGet(FLAG_TEMP_4));
    EXPECT(!FlagGet(FLAG_TEMP_5));
    EXPECT_EQ(VarGet(VAR_TEMP_1), 0);
}

TEST("Scene: give with a received item and a bag-full branch skips the branch")
{
    u32 item = 0, quantity = 0;
    struct SceneBranch full[] = {SCENE_BRANCH(NULL, sSetFlag1)};
    struct SceneStep steps[] = {SCENE_GIVE_OR_ELSE(ITEM_POTION, 1, full), SCENE_SET_FLAG(FLAG_TEMP_5), SCENE_ADVANCE(1)};
    struct Scene scene = {.name = "GiveFullBranchReceived", .actors = {LOCALID_PLAYER}, .numActors = 1, .stageVar = VAR_TEMP_1, .numSteps = ARRAY_COUNT(steps), .steps = steps};

    LoadTestMap();
    StepGiveSceneByHand(&scene, TRUE, &item, &quantity);
    EXPECT(!FlagGet(FLAG_TEMP_1));
    EXPECT(FlagGet(FLAG_TEMP_5));
    EXPECT_EQ(VarGet(VAR_TEMP_1), 1);
}

TEST("Scene: custom runs its script as a subroutine and the scene carries on afterwards")
{
    const u8 *script = OVERWORLD_SCRIPT(setvar VAR_TEMP_2, 7; return);
    struct SceneStep steps[] = {SCENE_CUSTOM(script), SCENE_SET_FLAG(FLAG_TEMP_5)};
    struct Scene scene = {.name = "Custom", .actors = {LOCALID_PLAYER}, .numActors = 1, .stageVar = VAR_TEMP_1, .numSteps = ARRAY_COUNT(steps), .steps = steps};

    LoadTestMap();
    VarSet(VAR_TEMP_2, 0);
    RunSceneWithKeys(&scene, sKeysA, ARRAY_COUNT(sKeysA));
    EXPECT_EQ(VarGet(VAR_TEMP_2), 7);
    EXPECT(FlagGet(FLAG_TEMP_5));
}

// The battle itself needs the battle engine and transitions, which the overworld tests do not run: these check
// what the step sets up and that the scene waits for the battle.
TEST("Scene: a trainer battle step configures a single no-intro battle against its trainer and waits")
{
    struct SceneStep steps[] = {SCENE_BATTLE_TRAINER(TRAINER_ROXANNE_1, sTestDefeatText), SCENE_SET_FLAG(FLAG_TEMP_5)};
    struct Scene scene = {.name = "TrainerBattle", .actors = {LOCALID_PLAYER}, .numActors = 1, .stageVar = VAR_TEMP_1, .numSteps = ARRAY_COUNT(steps), .steps = steps};

    LoadTestMap();
    RunSceneUntilWait(&scene);
    EXPECT_EQ((u32)TRAINER_BATTLE_PARAM.opponentA, TRAINER_ROXANNE_1);
    EXPECT_EQ((u32)TRAINER_BATTLE_PARAM.mode, TRAINER_BATTLE_SINGLE_NO_INTRO_TEXT);
    EXPECT(TRAINER_BATTLE_PARAM.defeatTextA == sTestDefeatText);
    EXPECT(gBattleTypeFlags & BATTLE_TYPE_TRAINER);
    EXPECT(!FlagGet(FLAG_TEMP_5)); // not before the battle has been fought
}

TEST("Scene: a wild battle step creates the wild Pokémon and waits")
{
    struct SceneStep steps[] = {SCENE_BATTLE_WILD(SPECIES_RALTS, 7), SCENE_SET_FLAG(FLAG_TEMP_5)};
    struct Scene scene = {.name = "WildBattle", .actors = {LOCALID_PLAYER}, .numActors = 1, .stageVar = VAR_TEMP_1, .numSteps = ARRAY_COUNT(steps), .steps = steps};

    LoadTestMap();
    RunSceneUntilWait(&scene);
    EXPECT_EQ(GetMonData(&gParties[B_TRAINER_OPPONENT_A][0], MON_DATA_SPECIES), SPECIES_RALTS);
    EXPECT_EQ(GetMonData(&gParties[B_TRAINER_OPPONENT_A][0], MON_DATA_LEVEL), 7);
    EXPECT(!FlagGet(FLAG_TEMP_5));
}

extern const u8 MossdeepCity_House3_Text_TryDroppingRodInWater[];
extern const u8 MossdeepCity_House3_Text_DontYouLikeToFish[];

// Steps a real give-a-rod scene by hand (the headless tests cannot run Std_ObtainItem): `yes` answers the
// question, `received` is what the give leaves in VAR_0x8007. Returns what the give asked for in the out params, 0 if it never did.
static void StepGiveScene(u32 sceneId, bool32 yes, bool32 received, u32 *givenItem, u32 *givenQuantity)
{
    u32 i;

    *givenItem = *givenQuantity = 0;
    SceneBeginScene(GetScene(sceneId));
    for (i = 0; i < 50; i++)
    {
        SceneNextStep();
        if (gSpecialVar_Result == SCENE_RESULT_GIVE)
        {
            *givenItem = gSpecialVar_0x8000;
            *givenQuantity = gSpecialVar_0x8001;
            gSpecialVar_0x8007 = received;
            SceneTakeAnswer();
        }
        else if (gSpecialVar_Result == SCENE_RESULT_ASK_YES_NO)
        {
            gSpecialVar_Result = yes;
            SceneTakeAnswer();
        }
        else if (gSpecialVar_Result == SCENE_RESULT_END)
        {
            return;
        }
    }
    EXPECT(FALSE); // the scene never ended
}

TEST("Scene: Mossdeep's Super Rod scene gives the rod on yes, sets the flag and the chapter stage, and says the last line")
{
    u32 item, quantity;

    LoadTestMap();
    FlagClear(FLAG_RECEIVED_SUPER_ROD);
    VarSet(GetCountySlotVar(COUNTY_ID(5, 1), 0), 0);
    StepGiveScene(SCENE_MOSSDEEP_SUPER_ROD, TRUE, TRUE, &item, &quantity);
    EXPECT_EQ(item, ITEM_SUPER_ROD);
    EXPECT_EQ(quantity, 1);
    EXPECT(FlagGet(FLAG_RECEIVED_SUPER_ROD));
    EXPECT_EQ(GetCountySlotStage(COUNTY_ID(5, 1), 0), 1);
    EXPECT_EQ(StringCompare(gStringVar4, MossdeepCity_House3_Text_TryDroppingRodInWater), 0);
}

TEST("Scene: Mossdeep's Super Rod scene with a full bag keeps the flag clear and the stage at 0")
{
    u32 item, quantity;

    LoadTestMap();
    FlagClear(FLAG_RECEIVED_SUPER_ROD);
    VarSet(GetCountySlotVar(COUNTY_ID(5, 1), 0), 0);
    StepGiveScene(SCENE_MOSSDEEP_SUPER_ROD, TRUE, FALSE, &item, &quantity);
    EXPECT_EQ(item, ITEM_SUPER_ROD);
    EXPECT(!FlagGet(FLAG_RECEIVED_SUPER_ROD));
    EXPECT_EQ(GetCountySlotStage(COUNTY_ID(5, 1), 0), 0);
}

TEST("Scene: Mossdeep's Super Rod scene on no gives nothing and changes nothing")
{
    u32 item, quantity;

    LoadTestMap();
    FlagClear(FLAG_RECEIVED_SUPER_ROD);
    VarSet(GetCountySlotVar(COUNTY_ID(5, 1), 0), 0);
    StepGiveScene(SCENE_MOSSDEEP_SUPER_ROD, FALSE, TRUE, &item, &quantity);
    EXPECT_EQ(item, 0);
    EXPECT(!FlagGet(FLAG_RECEIVED_SUPER_ROD));
    EXPECT_EQ(GetCountySlotStage(COUNTY_ID(5, 1), 0), 0);
    EXPECT_EQ(StringCompare(gStringVar4, MossdeepCity_House3_Text_DontYouLikeToFish), 0);
}

extern const u8 DewfordTown_Text_ThrowInFishingAdvice[];
extern const u8 DewfordTown_Text_ThatsTooBadThen[];

#define DEWFORD COUNTY_ID(0, 3)

TEST("Scene: Dewford's Old Rod scene gives the rod on yes, sets the flag and the chapter stage, and says the last line")
{
    u32 item, quantity;

    LoadTestMap();
    FlagClear(FLAG_RECEIVED_OLD_ROD);
    VarSet(GetCountySlotVar(DEWFORD, 0), 0);
    StepGiveScene(SCENE_DEWFORD_OLD_ROD, TRUE, TRUE, &item, &quantity);
    EXPECT_EQ(item, ITEM_OLD_ROD);
    EXPECT_EQ(quantity, 1);
    EXPECT(FlagGet(FLAG_RECEIVED_OLD_ROD));
    EXPECT_EQ(GetCountySlotStage(DEWFORD, 0), 1);
    EXPECT_EQ(StringCompare(gStringVar4, DewfordTown_Text_ThrowInFishingAdvice), 0);
}

TEST("Scene: Dewford's Old Rod scene with a full bag keeps the flag clear and the stage at 0")
{
    u32 item, quantity;

    LoadTestMap();
    FlagClear(FLAG_RECEIVED_OLD_ROD);
    VarSet(GetCountySlotVar(DEWFORD, 0), 0);
    StepGiveScene(SCENE_DEWFORD_OLD_ROD, TRUE, FALSE, &item, &quantity);
    EXPECT_EQ(item, ITEM_OLD_ROD);
    EXPECT(!FlagGet(FLAG_RECEIVED_OLD_ROD));
    EXPECT_EQ(GetCountySlotStage(DEWFORD, 0), 0);
}

TEST("Scene: Dewford's Old Rod scene on no gives nothing and changes nothing")
{
    u32 item, quantity;

    LoadTestMap();
    FlagClear(FLAG_RECEIVED_OLD_ROD);
    VarSet(GetCountySlotVar(DEWFORD, 0), 0);
    StepGiveScene(SCENE_DEWFORD_OLD_ROD, FALSE, TRUE, &item, &quantity);
    EXPECT_EQ(item, 0);
    EXPECT(!FlagGet(FLAG_RECEIVED_OLD_ROD));
    EXPECT_EQ(GetCountySlotStage(DEWFORD, 0), 0);
    EXPECT_EQ(StringCompare(gStringVar4, DewfordTown_Text_ThatsTooBadThen), 0);
}

extern const u8 Route118_Text_TryYourLuckFishing[];
extern const u8 Route118_Text_DontYouLikeToFish[];

#define WEATHER_COUNTY COUNTY_ID(3, 0)

TEST("Scene: Route 118's Good Rod scene gives the rod on yes, sets the flag and the chapter stage, and says the last line")
{
    u32 item, quantity;

    LoadTestMap();
    FlagClear(FLAG_RECEIVED_GOOD_ROD);
    VarSet(GetCountySlotVar(WEATHER_COUNTY, 0), 0);
    StepGiveScene(SCENE_ROUTE118_GOOD_ROD, TRUE, TRUE, &item, &quantity);
    EXPECT_EQ(item, ITEM_GOOD_ROD);
    EXPECT_EQ(quantity, 1);
    EXPECT(FlagGet(FLAG_RECEIVED_GOOD_ROD));
    EXPECT_EQ(GetCountySlotStage(WEATHER_COUNTY, 0), 1);
    EXPECT_EQ(StringCompare(gStringVar4, Route118_Text_TryYourLuckFishing), 0);
}

TEST("Scene: Route 118's Good Rod scene with a full bag keeps the flag clear and the stage at 0")
{
    u32 item, quantity;

    LoadTestMap();
    FlagClear(FLAG_RECEIVED_GOOD_ROD);
    VarSet(GetCountySlotVar(WEATHER_COUNTY, 0), 0);
    StepGiveScene(SCENE_ROUTE118_GOOD_ROD, TRUE, FALSE, &item, &quantity);
    EXPECT_EQ(item, ITEM_GOOD_ROD);
    EXPECT(!FlagGet(FLAG_RECEIVED_GOOD_ROD));
    EXPECT_EQ(GetCountySlotStage(WEATHER_COUNTY, 0), 0);
}

TEST("Scene: Route 118's Good Rod scene on no gives nothing and changes nothing")
{
    u32 item, quantity;

    LoadTestMap();
    FlagClear(FLAG_RECEIVED_GOOD_ROD);
    VarSet(GetCountySlotVar(WEATHER_COUNTY, 0), 0);
    StepGiveScene(SCENE_ROUTE118_GOOD_ROD, FALSE, TRUE, &item, &quantity);
    EXPECT_EQ(item, 0);
    EXPECT(!FlagGet(FLAG_RECEIVED_GOOD_ROD));
    EXPECT_EQ(GetCountySlotStage(WEATHER_COUNTY, 0), 0);
    EXPECT_EQ(StringCompare(gStringVar4, Route118_Text_DontYouLikeToFish), 0);
}

extern const u8 Route110_Text_MayExplainItemfinder[];
extern const u8 Route110_Text_BrendanExplainItemfinder[];
extern const u8 Route119_Text_MayExplainFly[];
extern const u8 Route119_Text_BrendanExplainFly[];

#define ROUTE110_COUNTY COUNTY_ID(2, 2)

// Steps a rival or gift scene by hand: a battle is acknowledged as won (headless tests cannot run one), a give
// leaves `received` in VAR_0x8007, and the number of battle steps seen and the last given item are returned.
static void StepRivalScene(u32 sceneId, bool32 received, u32 *numBattles, u32 *givenItem)
{
    u32 i;

    *numBattles = *givenItem = 0;
    SceneBeginScene(GetScene(sceneId));
    for (i = 0; i < 50; i++)
    {
        SceneNextStep();
        if (gSpecialVar_Result == SCENE_RESULT_GIVE)
        {
            *givenItem = gSpecialVar_0x8000;
            gSpecialVar_0x8007 = received;
            SceneTakeAnswer();
        }
        else if (gSpecialVar_Result == SCENE_RESULT_BATTLE)
        {
            (*numBattles)++;
        }
        else if (gSpecialVar_Result == SCENE_RESULT_END)
        {
            return;
        }
    }
    EXPECT(FALSE); // the scene never ended
}

TEST("Scene: Route 110's rival scenes say the challenge, fight the Route 110 rival and advance the rival slot only")
{
    static const u16 sceneIds[] = {SCENE_ROUTE110_RIVAL_MAY, SCENE_ROUTE110_RIVAL_BRENDAN};
    u32 i, numBattles, item;

    for (i = 0; i < ARRAY_COUNT(sceneIds); i++)
    {
        const struct Scene *scene = GetScene(sceneIds[i]);

        LoadTestMap();
        VarSet(GetCountySlotVar(ROUTE110_COUNTY, 0), 0);
        VarSet(GetCountySlotVar(ROUTE110_COUNTY, 1), 0);
        ASSUME(scene->numSteps == 3);
        EXPECT_EQ(scene->steps[1].kind, SCENE_STEP_BATTLE);
        EXPECT(scene->steps[1].rival);
        EXPECT_EQ(scene->steps[1].trainer, RIVAL_ENCOUNTER_ROUTE_110);
        StepRivalScene(sceneIds[i], TRUE, &numBattles, &item);
        EXPECT_EQ(numBattles, 1);
        EXPECT_EQ(item, 0);
        EXPECT_EQ(GetCountySlotStage(ROUTE110_COUNTY, 0), 1);
        EXPECT_EQ(GetCountySlotStage(ROUTE110_COUNTY, 1), 0);
    }
}

TEST("Scene: Route 110's Itemfinder scenes give the Itemfinder, advance the Itemfinder slot only and end on the explanation")
{
    static const u16 sceneIds[] = {SCENE_ROUTE110_ITEMFINDER_MAY, SCENE_ROUTE110_ITEMFINDER_BRENDAN};
    static const u8 *const explains[] = {Route110_Text_MayExplainItemfinder, Route110_Text_BrendanExplainItemfinder};
    u32 i, numBattles, item;

    for (i = 0; i < ARRAY_COUNT(sceneIds); i++)
    {
        LoadTestMap();
        VarSet(GetCountySlotVar(ROUTE110_COUNTY, 0), 0);
        VarSet(GetCountySlotVar(ROUTE110_COUNTY, 1), 0);
        StepRivalScene(sceneIds[i], TRUE, &numBattles, &item);
        EXPECT_EQ(numBattles, 0);
        EXPECT_EQ(item, ITEM_DOWSING_MACHINE);
        EXPECT_EQ(GetCountySlotStage(ROUTE110_COUNTY, 1), 1);
        EXPECT_EQ(GetCountySlotStage(ROUTE110_COUNTY, 0), 0);
        EXPECT_EQ(StringCompareN(gStringVar4, explains[i], 8), 0); // names are expanded after the speaker
    }
}

TEST("Scene: Route 110's Itemfinder scenes with a full bag keep the Itemfinder stage at 0")
{
    static const u16 sceneIds[] = {SCENE_ROUTE110_ITEMFINDER_MAY, SCENE_ROUTE110_ITEMFINDER_BRENDAN};
    u32 i, numBattles, item;

    for (i = 0; i < ARRAY_COUNT(sceneIds); i++)
    {
        LoadTestMap();
        VarSet(GetCountySlotVar(ROUTE110_COUNTY, 1), 0);
        StepRivalScene(sceneIds[i], FALSE, &numBattles, &item);
        EXPECT_EQ(item, ITEM_DOWSING_MACHINE);
        EXPECT_EQ(GetCountySlotStage(ROUTE110_COUNTY, 1), 0);
    }
}

TEST("Scene: Route 119's rival scenes fight, advance to 1 after the fight and to 2 after the HM Fly, and set its flag")
{
    static const u16 sceneIds[] = {SCENE_ROUTE119_RIVAL_MAY, SCENE_ROUTE119_RIVAL_BRENDAN};
    static const u8 *const explains[] = {Route119_Text_MayExplainFly, Route119_Text_BrendanExplainFly};
    u32 i, numBattles, item;

    for (i = 0; i < ARRAY_COUNT(sceneIds); i++)
    {
        LoadTestMap();
        FlagClear(FLAG_RECEIVED_HM_FLY);
        VarSet(GetCountySlotVar(WEATHER_COUNTY, 1), 0);
        VarSet(GetCountySlotVar(WEATHER_COUNTY, 0), 0);
        StepRivalScene(sceneIds[i], TRUE, &numBattles, &item);
        EXPECT_EQ(numBattles, 1);
        EXPECT_EQ(GetScene(sceneIds[i])->steps[1].trainer, RIVAL_ENCOUNTER_ROUTE_119);
        EXPECT_EQ(item, ITEM_HM_FLY);
        EXPECT(FlagGet(FLAG_RECEIVED_HM_FLY));
        EXPECT_EQ(GetCountySlotStage(WEATHER_COUNTY, 1), 2);
        EXPECT_EQ(GetCountySlotStage(WEATHER_COUNTY, 0), 0);
        EXPECT_EQ(StringCompareN(gStringVar4, explains[i], 8), 0); // names are expanded after the speaker
    }
}

TEST("Scene: Route 119's rival scenes with a full bag stop at stage 1 with the HM Fly flag clear, and the gift scenes then finish it without another fight")
{
    static const u16 rivalIds[] = {SCENE_ROUTE119_RIVAL_MAY, SCENE_ROUTE119_RIVAL_BRENDAN};
    static const u16 giftIds[] = {SCENE_ROUTE119_FLY_MAY, SCENE_ROUTE119_FLY_BRENDAN};
    u32 i, numBattles, item;

    for (i = 0; i < ARRAY_COUNT(rivalIds); i++)
    {
        LoadTestMap();
        FlagClear(FLAG_RECEIVED_HM_FLY);
        VarSet(GetCountySlotVar(WEATHER_COUNTY, 1), 0);
        StepRivalScene(rivalIds[i], FALSE, &numBattles, &item);
        EXPECT_EQ(numBattles, 1);
        EXPECT(!FlagGet(FLAG_RECEIVED_HM_FLY));
        EXPECT_EQ(GetCountySlotStage(WEATHER_COUNTY, 1), 1);
        StepRivalScene(giftIds[i], FALSE, &numBattles, &item);
        EXPECT_EQ(numBattles, 0);
        EXPECT(!FlagGet(FLAG_RECEIVED_HM_FLY));
        EXPECT_EQ(GetCountySlotStage(WEATHER_COUNTY, 1), 1);
        StepRivalScene(giftIds[i], TRUE, &numBattles, &item);
        EXPECT_EQ(numBattles, 0);
        EXPECT_EQ(item, ITEM_HM_FLY);
        EXPECT(FlagGet(FLAG_RECEIVED_HM_FLY));
        EXPECT_EQ(GetCountySlotStage(WEATHER_COUNTY, 1), 2);
    }
}

TEST("Rival: GetRivalTrainer picks the Route 110 and Route 119 trainers for each starter and gender")
{
    static const u16 may110[] = {TRAINER_MAY_ROUTE_110_TREECKO, TRAINER_MAY_ROUTE_110_TORCHIC, TRAINER_MAY_ROUTE_110_MUDKIP};
    static const u16 brendan110[] = {TRAINER_BRENDAN_ROUTE_110_TREECKO, TRAINER_BRENDAN_ROUTE_110_TORCHIC, TRAINER_BRENDAN_ROUTE_110_MUDKIP};
    static const u16 may119[] = {TRAINER_MAY_ROUTE_119_TREECKO, TRAINER_MAY_ROUTE_119_TORCHIC, TRAINER_MAY_ROUTE_119_MUDKIP};
    static const u16 brendan119[] = {TRAINER_BRENDAN_ROUTE_119_TREECKO, TRAINER_BRENDAN_ROUTE_119_TORCHIC, TRAINER_BRENDAN_ROUTE_119_MUDKIP};
    u32 starter;

    for (starter = 0; starter < 3; starter++)
    {
        VarSet(VAR_STARTER_MON, starter);
        gSaveBlock2Ptr->playerGender = MALE;
        EXPECT_EQ(GetRivalTrainer(RIVAL_ENCOUNTER_ROUTE_110), may110[starter]);
        EXPECT_EQ(GetRivalTrainer(RIVAL_ENCOUNTER_ROUTE_119), may119[starter]);
        gSaveBlock2Ptr->playerGender = FEMALE;
        EXPECT_EQ(GetRivalTrainer(RIVAL_ENCOUNTER_ROUTE_110), brendan110[starter]);
        EXPECT_EQ(GetRivalTrainer(RIVAL_ENCOUNTER_ROUTE_119), brendan119[starter]);
    }
}

extern const u8 LilycoveCity_Text_MayShoppingLetsBattle[];
extern const u8 LilycoveCity_Text_MayBattleMe[];
extern const u8 LilycoveCity_Text_MayGoingBackToLittleroot[];
extern const u8 LilycoveCity_Text_MayNotRaisingPokemon[];
extern const u8 LilycoveCity_Text_BrendanShoppingLetsBattle[];
extern const u8 LilycoveCity_Text_BrendanBattleMe[];
extern const u8 LilycoveCity_Text_BrendanGoingBackToLittleroot[];
extern const u8 LilycoveCity_Text_BrendanNoConfidence[];

#define LILYCOVE_COUNTY COUNTY_ID(4, 0)

// Steps a Lilycove rival scene by hand, answering its question `yes` (the battle counts as won).
static void StepLilycoveRival(u32 sceneId, bool32 yes, u32 *numBattles)
{
    u32 i;

    *numBattles = 0;
    SceneBeginScene(GetScene(sceneId));
    for (i = 0; i < 50; i++)
    {
        SceneNextStep();
        if (gSpecialVar_Result == SCENE_RESULT_ASK_YES_NO)
        {
            gSpecialVar_Result = yes;
            SceneTakeAnswer();
        }
        else if (gSpecialVar_Result == SCENE_RESULT_BATTLE)
        {
            (*numBattles)++;
        }
        else if (gSpecialVar_Result == SCENE_RESULT_END)
        {
            return;
        }
    }
    EXPECT(FALSE); // the scene never ended
}

TEST("Scene: Lilycove's rival scenes on yes fight the Lilycove rival, say he is going home and advance slot 2 to stage 1")
{
    static const u16 sceneIds[] = {SCENE_LILYCOVE_RIVAL_MAY, SCENE_LILYCOVE_RIVAL_MAY_AGAIN, SCENE_LILYCOVE_RIVAL_BRENDAN, SCENE_LILYCOVE_RIVAL_BRENDAN_AGAIN};
    static const u8 *const goingBack[] = {LilycoveCity_Text_MayGoingBackToLittleroot, LilycoveCity_Text_MayGoingBackToLittleroot, LilycoveCity_Text_BrendanGoingBackToLittleroot, LilycoveCity_Text_BrendanGoingBackToLittleroot};
    u32 i, numBattles;

    for (i = 0; i < ARRAY_COUNT(sceneIds); i++)
    {
        LoadTestMap();
        FlagClear(FLAG_DECLINED_RIVAL_BATTLE_LILYCOVE);
        VarSet(GetCountySlotVar(LILYCOVE_COUNTY, 2), 0);
        StepLilycoveRival(sceneIds[i], TRUE, &numBattles);
        EXPECT_EQ(numBattles, 1);
        EXPECT(!FlagGet(FLAG_DECLINED_RIVAL_BATTLE_LILYCOVE));
        EXPECT_EQ(GetCountySlotStage(LILYCOVE_COUNTY, 2), 1);
        EXPECT_EQ(GetCountySlotStage(LILYCOVE_COUNTY, 0), 0);
        EXPECT_EQ(GetCountySlotStage(LILYCOVE_COUNTY, 1), 0);
        EXPECT_EQ(StringCompareN(gStringVar4, goingBack[i], 8), 0); // names are expanded after the speaker
    }
}

TEST("Scene: Lilycove's rival scenes on no set the declined flag, fight nothing and leave the stage at 0")
{
    static const u16 sceneIds[] = {SCENE_LILYCOVE_RIVAL_MAY, SCENE_LILYCOVE_RIVAL_MAY_AGAIN, SCENE_LILYCOVE_RIVAL_BRENDAN, SCENE_LILYCOVE_RIVAL_BRENDAN_AGAIN};
    static const u8 *const declined[] = {LilycoveCity_Text_MayNotRaisingPokemon, LilycoveCity_Text_MayNotRaisingPokemon, LilycoveCity_Text_BrendanNoConfidence, LilycoveCity_Text_BrendanNoConfidence};
    u32 i, numBattles;

    for (i = 0; i < ARRAY_COUNT(sceneIds); i++)
    {
        LoadTestMap();
        FlagClear(FLAG_DECLINED_RIVAL_BATTLE_LILYCOVE);
        VarSet(GetCountySlotVar(LILYCOVE_COUNTY, 2), 0);
        StepLilycoveRival(sceneIds[i], FALSE, &numBattles);
        EXPECT_EQ(numBattles, 0);
        EXPECT(FlagGet(FLAG_DECLINED_RIVAL_BATTLE_LILYCOVE));
        EXPECT_EQ(GetCountySlotStage(LILYCOVE_COUNTY, 2), 0);
        EXPECT_EQ(StringCompareN(gStringVar4, declined[i], 8), 0); // names are expanded after the speaker
    }
}

TEST("Scene: Lilycove's first and repeated rival scenes ask different questions and share their answers")
{
    EXPECT(GetScene(SCENE_LILYCOVE_RIVAL_MAY)->steps[0].text == LilycoveCity_Text_MayShoppingLetsBattle);
    EXPECT(GetScene(SCENE_LILYCOVE_RIVAL_MAY_AGAIN)->steps[0].text == LilycoveCity_Text_MayBattleMe);
    EXPECT(GetScene(SCENE_LILYCOVE_RIVAL_BRENDAN)->steps[0].text == LilycoveCity_Text_BrendanShoppingLetsBattle);
    EXPECT(GetScene(SCENE_LILYCOVE_RIVAL_BRENDAN_AGAIN)->steps[0].text == LilycoveCity_Text_BrendanBattleMe);
    EXPECT(GetScene(SCENE_LILYCOVE_RIVAL_MAY)->steps[0].branches == GetScene(SCENE_LILYCOVE_RIVAL_MAY_AGAIN)->steps[0].branches);
    EXPECT(GetScene(SCENE_LILYCOVE_RIVAL_BRENDAN)->steps[0].branches == GetScene(SCENE_LILYCOVE_RIVAL_BRENDAN_AGAIN)->steps[0].branches);
}

TEST("Rival: GetRivalTrainer picks the Lilycove trainers for each starter and gender")
{
    static const u16 may[] = {TRAINER_MAY_LILYCOVE_TREECKO, TRAINER_MAY_LILYCOVE_TORCHIC, TRAINER_MAY_LILYCOVE_MUDKIP};
    static const u16 brendan[] = {TRAINER_BRENDAN_LILYCOVE_TREECKO, TRAINER_BRENDAN_LILYCOVE_TORCHIC, TRAINER_BRENDAN_LILYCOVE_MUDKIP};
    u32 starter;

    for (starter = 0; starter < 3; starter++)
    {
        VarSet(VAR_STARTER_MON, starter);
        gSaveBlock2Ptr->playerGender = MALE;
        EXPECT_EQ(GetRivalTrainer(RIVAL_ENCOUNTER_LILYCOVE), may[starter]);
        gSaveBlock2Ptr->playerGender = FEMALE;
        EXPECT_EQ(GetRivalTrainer(RIVAL_ENCOUNTER_LILYCOVE), brendan[starter]);
    }
}

#define RUSTBORO_COUNTY COUNTY_ID(0, 1)

TEST("Scene: Rival 2's Route 104 and Rustboro scenes on yes fight the Rustboro trainers and advance Rustboro's slot 2 to stage 1")
{
    static const u16 sceneIds[] =
    {
        SCENE_ROUTE104_RIVAL_MAY, SCENE_ROUTE104_RIVAL_MAY_AGAIN, SCENE_ROUTE104_RIVAL_BRENDAN, SCENE_ROUTE104_RIVAL_BRENDAN_AGAIN,
        SCENE_RUSTBORO_RIVAL_MAY, SCENE_RUSTBORO_RIVAL_MAY_AGAIN, SCENE_RUSTBORO_RIVAL_BRENDAN, SCENE_RUSTBORO_RIVAL_BRENDAN_AGAIN,
    };
    u32 i, numBattles;

    for (i = 0; i < ARRAY_COUNT(sceneIds); i++)
    {
        LoadTestMap();
        VarSet(GetCountySlotVar(RUSTBORO_COUNTY, 2), 0);
        StepLilycoveRival(sceneIds[i], TRUE, &numBattles);
        EXPECT_EQ(numBattles, 1);
        EXPECT_EQ(GetCountySlotStage(RUSTBORO_COUNTY, 2), 1);
        EXPECT_EQ(GetCountySlotStage(RUSTBORO_COUNTY, 0), 0);
        EXPECT_EQ(GetCountySlotStage(RUSTBORO_COUNTY, 1), 0);
        EXPECT_EQ(GetScene(sceneIds[i])->stageVar, GetCountySlotVar(RUSTBORO_COUNTY, 2));
    }
}

TEST("Scene: Rival 2's Route 104 and Rustboro scenes on no fight nothing and leave the stage at 0")
{
    static const u16 sceneIds[] =
    {
        SCENE_ROUTE104_RIVAL_MAY, SCENE_ROUTE104_RIVAL_MAY_AGAIN, SCENE_ROUTE104_RIVAL_BRENDAN, SCENE_ROUTE104_RIVAL_BRENDAN_AGAIN,
        SCENE_RUSTBORO_RIVAL_MAY, SCENE_RUSTBORO_RIVAL_MAY_AGAIN, SCENE_RUSTBORO_RIVAL_BRENDAN, SCENE_RUSTBORO_RIVAL_BRENDAN_AGAIN,
    };
    u32 i, numBattles;

    for (i = 0; i < ARRAY_COUNT(sceneIds); i++)
    {
        LoadTestMap();
        VarSet(GetCountySlotVar(RUSTBORO_COUNTY, 2), 0);
        StepLilycoveRival(sceneIds[i], FALSE, &numBattles);
        EXPECT_EQ(numBattles, 0);
        EXPECT_EQ(GetCountySlotStage(RUSTBORO_COUNTY, 2), 0);
    }
}

TEST("Scene: Rival 2's first and repeated questions share their answers at each site")
{
    EXPECT(GetScene(SCENE_ROUTE104_RIVAL_MAY)->steps[0].branches == GetScene(SCENE_ROUTE104_RIVAL_MAY_AGAIN)->steps[0].branches);
    EXPECT(GetScene(SCENE_ROUTE104_RIVAL_BRENDAN)->steps[0].branches == GetScene(SCENE_ROUTE104_RIVAL_BRENDAN_AGAIN)->steps[0].branches);
    EXPECT(GetScene(SCENE_RUSTBORO_RIVAL_MAY)->steps[0].branches == GetScene(SCENE_RUSTBORO_RIVAL_MAY_AGAIN)->steps[0].branches);
    EXPECT(GetScene(SCENE_RUSTBORO_RIVAL_BRENDAN)->steps[0].branches == GetScene(SCENE_RUSTBORO_RIVAL_BRENDAN_AGAIN)->steps[0].branches);
    EXPECT(GetScene(SCENE_ROUTE104_RIVAL_MAY)->steps[0].text != GetScene(SCENE_ROUTE104_RIVAL_MAY_AGAIN)->steps[0].text);
    EXPECT(GetScene(SCENE_RUSTBORO_RIVAL_BRENDAN)->steps[0].text != GetScene(SCENE_RUSTBORO_RIVAL_BRENDAN_AGAIN)->steps[0].text);
}

TEST("Rival: GetRivalTrainer picks the Rustboro trainers for each starter and gender")
{
    static const u16 may[] = {TRAINER_MAY_RUSTBORO_TREECKO, TRAINER_MAY_RUSTBORO_TORCHIC, TRAINER_MAY_RUSTBORO_MUDKIP};
    static const u16 brendan[] = {TRAINER_BRENDAN_RUSTBORO_TREECKO, TRAINER_BRENDAN_RUSTBORO_TORCHIC, TRAINER_BRENDAN_RUSTBORO_MUDKIP};
    u32 starter;

    for (starter = 0; starter < 3; starter++)
    {
        VarSet(VAR_STARTER_MON, starter);
        gSaveBlock2Ptr->playerGender = MALE;
        EXPECT_EQ(GetRivalTrainer(RIVAL_ENCOUNTER_RUSTBORO), may[starter]);
        gSaveBlock2Ptr->playerGender = FEMALE;
        EXPECT_EQ(GetRivalTrainer(RIVAL_ENCOUNTER_RUSTBORO), brendan[starter]);
    }
}
