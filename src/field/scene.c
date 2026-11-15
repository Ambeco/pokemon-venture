#include "global.h"
#include "assertf.h"
#include "scene.h"
#include "event_data.h"
#include "event_object_movement.h"
#include "field_player_avatar.h"
#include "script_movement.h"
#include "string_util.h"
#include "text.h"
#include "constants/event_objects.h"
#include "constants/event_object_movement.h"
#include "constants/map_event_ids.h"
#include "constants/scenes.h"
#include "constants/vars.h"

#define WINDOW_WIDTH (2 * SCENE_WINDOW_HALF_WIDTH + 1)
#define WINDOW_HEIGHT (2 * SCENE_WINDOW_HALF_HEIGHT + 1)
#define WINDOW_CELLS (WINDOW_WIDTH * WINDOW_HEIGHT)
#define CELL_UNSEEN 0

STATIC_ASSERT(WINDOW_CELLS < 256, SceneQueueIndexFitsU8);

#include "data/scenes.h"

static EWRAM_DATA u8 sSearchCameFrom[WINDOW_CELLS] = {0}; // the direction taken into each cell, CELL_UNSEEN if not reached
static EWRAM_DATA u8 sSearchQueue[WINDOW_CELLS] = {0};
static EWRAM_DATA u8 sWalkDirs[SCENE_MAX_ACTORS][SCENE_MAX_PATH] = {0};
static EWRAM_DATA u8 sWalkScript[SCENE_MAX_ACTORS][SCENE_MAX_PATH + 1] = {0};
static EWRAM_DATA u8 sFaceScript[SCENE_MAX_ACTORS][2] = {0};
static EWRAM_DATA u8 sSceneId = 0;
static EWRAM_DATA u8 sStepIndex = 0;
static EWRAM_DATA bool8 sPlayerLocked = FALSE;

static const char *const sStepKindNames[SCENE_STEP_KIND_COUNT] =
{
    [SCENE_STEP_LOCK_PLAYER] = "lock_player",
    [SCENE_STEP_UNLOCK_PLAYER] = "unlock_player",
    [SCENE_STEP_FACE] = "face",
    [SCENE_STEP_WALK] = "walk",
    [SCENE_STEP_PARALLEL_WALK] = "parallel_walk",
    [SCENE_STEP_SAY] = "say",
    [SCENE_STEP_HIDE] = "hide",
    [SCENE_STEP_SHOW] = "show",
    [SCENE_STEP_ADVANCE] = "advance",
    [SCENE_STEP_DOOR_OPEN] = "door_open",
    [SCENE_STEP_DOOR_CLOSE] = "door_close",
    [SCENE_STEP_WARP] = "warp",
    [SCENE_STEP_ASK_YES_NO] = "ask_yes_no",
    [SCENE_STEP_ASK_CHOICE] = "ask_choice",
    [SCENE_STEP_GIVE] = "give",
    [SCENE_STEP_BATTLE] = "battle",
    [SCENE_STEP_MUSIC] = "music",
    [SCENE_STEP_CUSTOM] = "custom",
};

const struct Scene *GetScene(u32 sceneId)
{
    assertf(sceneId < SCENE_COUNT, "scene id %d is not a scene (there are %d); did you set VAR_0x8004 before calling Scene_EventScript_Run?", sceneId, SCENE_COUNT)
    {
        return NULL;
    }
    return sScenes[sceneId];
}

void SceneResolveSide(s16 x, s16 y, enum Direction facing, u32 side, s16 *outX, s16 *outY)
{
    enum Direction dir = facing == DIR_NONE ? DIR_SOUTH : facing;

    *outX = x;
    *outY = y;
    switch (side)
    {
    case SCENE_SIDE_SELF:
        return;
    case SCENE_SIDE_FRONT:
        break;
    case SCENE_SIDE_BEHIND:
        dir = GetOppositeDirection(dir);
        break;
    case SCENE_SIDE_LEFT:
        dir = GetNinetyDegreeDirection(dir, FALSE);
        break;
    case SCENE_SIDE_RIGHT:
        dir = GetNinetyDegreeDirection(dir, TRUE);
        break;
    default:
        assertf(FALSE, "scene side %d does not exist; did you mean one of enum SceneSide?", side)
        {
            return;
        }
    }
    MoveCoords(dir, outX, outY);
}

enum Direction SceneDirectionToward(s16 fromX, s16 fromY, s16 toX, s16 toY)
{
    s32 dx = toX - fromX, dy = toY - fromY;
    s32 ax = dx < 0 ? -dx : dx, ay = dy < 0 ? -dy : dy;

    if (ax == 0 && ay == 0)
        return DIR_NONE;
    if (ay >= ax)
        return dy > 0 ? DIR_SOUTH : DIR_NORTH;
    return dx > 0 ? DIR_EAST : DIR_WEST;
}

u32 SceneFindPath(s16 startX, s16 startY, s16 goalX, s16 goalY, SceneStepCheck canStep, void *context, u8 *dirs)
{
    static const enum Direction order[] = {DIR_NORTH, DIR_SOUTH, DIR_WEST, DIR_EAST};
    s16 originX = startX - SCENE_WINDOW_HALF_WIDTH, originY = startY - SCENE_WINDOW_HALF_HEIGHT;
    s32 goalCol = goalX - originX, goalRow = goalY - originY;
    u32 head = 0, tail = 0, i, length;
    u8 goalCell, cell;

    if (goalCol < 0 || goalCol >= WINDOW_WIDTH || goalRow < 0 || goalRow >= WINDOW_HEIGHT)
        return SCENE_PATH_NONE;
    if (startX == goalX && startY == goalY)
        return 0;

    for (i = 0; i < WINDOW_CELLS; i++)
        sSearchCameFrom[i] = CELL_UNSEEN;
    goalCell = goalRow * WINDOW_WIDTH + goalCol;
    cell = SCENE_WINDOW_HALF_HEIGHT * WINDOW_WIDTH + SCENE_WINDOW_HALF_WIDTH;
    sSearchCameFrom[cell] = DIR_NONE + 1; // the start counts as reached; any non-zero mark will do
    sSearchQueue[tail++] = cell;

    while (head < tail && sSearchCameFrom[goalCell] == CELL_UNSEEN)
    {
        u8 current = sSearchQueue[head++];
        s16 x = originX + current % WINDOW_WIDTH, y = originY + current / WINDOW_WIDTH;

        for (i = 0; i < ARRAY_COUNT(order); i++)
        {
            s16 nx = x, ny = y;
            s32 col, row;
            u8 next;

            MoveCoords(order[i], &nx, &ny);
            col = nx - originX;
            row = ny - originY;
            if (col < 0 || col >= WINDOW_WIDTH || row < 0 || row >= WINDOW_HEIGHT)
                continue;
            next = row * WINDOW_WIDTH + col;
            if (sSearchCameFrom[next] != CELL_UNSEEN || !canStep(context, x, y, order[i]))
                continue;
            sSearchCameFrom[next] = order[i];
            sSearchQueue[tail++] = next;
        }
    }

    if (sSearchCameFrom[goalCell] == CELL_UNSEEN)
        return SCENE_PATH_NONE;

    for (length = 0, cell = goalCell; cell != SCENE_WINDOW_HALF_HEIGHT * WINDOW_WIDTH + SCENE_WINDOW_HALF_WIDTH; length++)
    {
        enum Direction taken = sSearchCameFrom[cell];
        s16 bx = 0, by = 0;

        if (length >= SCENE_MAX_PATH)
            return SCENE_PATH_NONE;
        dirs[length] = taken; // reversed below
        MoveCoords(GetOppositeDirection(taken), &bx, &by);
        cell = (cell / WINDOW_WIDTH + by) * WINDOW_WIDTH + (cell % WINDOW_WIDTH + bx);
    }
    for (i = 0; i < length / 2; i++)
    {
        u8 tmp = dirs[i];
        dirs[i] = dirs[length - 1 - i];
        dirs[length - 1 - i] = tmp;
    }
    return length;
}

u8 *SceneBuildWalkScript(u8 *out, const u8 *dirs, u32 length)
{
    u32 i;

    assertf(length <= SCENE_MAX_PATH, "walk of %d steps is longer than SCENE_MAX_PATH (%d)", length, SCENE_MAX_PATH)
    {
        length = SCENE_MAX_PATH;
    }
    for (i = 0; i < length; i++)
        *out++ = GetWalkNormalMovementAction(dirs[i]);
    *out = MOVEMENT_ACTION_STEP_END;
    return out;
}

struct WalkSearch
{
    struct ObjectEvent *mover;
    u32 ignoredObjects; // bitmask of object event ids that may be walking out of the way (parallel walks)
};

static bool32 CanWalkerStep(void *context, s16 x, s16 y, enum Direction dir)
{
    struct WalkSearch *search = context;
    struct ObjectEvent *mover = search->mover;
    s16 nx = x, ny = y;
    enum Collision collision;
    u32 other;

    MoveCoords(dir, &nx, &ny);
    collision = GetCollisionAtCoords(mover, nx, ny, dir);
    if (collision == COLLISION_NONE)
        return TRUE;
    if (collision != COLLISION_OBJECT_EVENT)
        return FALSE;
    other = GetObjectObjectCollidesWith(mover, nx, ny, FALSE);
    return other < OBJECT_EVENTS_COUNT && (search->ignoredObjects & (1u << other)) != 0;
}

static u8 GetActorLocalId(const struct Scene *scene, u32 slot)
{
    assertf(slot < scene->numActors, "scene %s uses actor slot %d but lists only %d actors; did you mean to add it to Scene.actors and numActors?", scene->name, slot, scene->numActors)
    {
        return LOCALID_PLAYER;
    }
    return slot == SCENE_ACTOR_PLAYER ? LOCALID_PLAYER : scene->actors[slot];
}

static struct ObjectEvent *GetActorObject(const struct Scene *scene, u32 slot)
{
    u8 localId = GetActorLocalId(scene, slot);
    u8 id = GetObjectEventIdByLocalId(localId);

    assertf(id < OBJECT_EVENTS_COUNT, "scene %s: actor slot %d (local id %d) is not on the map; did you mean to run the scene on the map that has it, or to place it first?", scene->name, slot, localId)
    {
        return &gObjectEvents[gPlayerAvatar.objectEventId];
    }
    return &gObjectEvents[id];
}

static void GetTargetTile(const struct Scene *scene, const struct SceneTarget *target, s16 *x, s16 *y)
{
    struct ObjectEvent *object = GetActorObject(scene, target->actor);

    if (target->side == SCENE_SIDE_HOME)
    {
        *x = object->initialCoords.x;
        *y = object->initialCoords.y;
        return;
    }
    SceneResolveSide(object->currentCoords.x, object->currentCoords.y, object->facingDirection, target->side, x, y);
}

// Starts nothing and returns FALSE if a goal cannot be reached; walks starts once every path is found.
static bool32 StartWalks(const struct Scene *scene, const struct SceneStep *step, u32 stepIndex)
{
    struct SceneWalk single;
    const struct SceneWalk *walks = step->walks;
    u32 numWalks = step->numWalks, i;
    u32 lengths[SCENE_MAX_ACTORS];
    u32 ignored = 0;
    bool32 started = FALSE;

    if (step->kind == SCENE_STEP_WALK)
    {
        single.actor = step->actor;
        single.goal = step->target;
        walks = &single;
        numWalks = 1;
    }
    assertf(numWalks <= SCENE_MAX_ACTORS && walks != NULL, "scene %s step %d (%s) lists %d walks; at most %d, and the list must not be NULL", scene->name, stepIndex, sStepKindNames[step->kind], numWalks, SCENE_MAX_ACTORS)
    {
        return FALSE;
    }
    if (step->kind == SCENE_STEP_PARALLEL_WALK)
    {
        for (i = 0; i < numWalks; i++)
            ignored |= 1u << (GetActorObject(scene, walks[i].actor) - gObjectEvents);
    }

    for (i = 0; i < numWalks; i++)
    {
        struct ObjectEvent *mover = GetActorObject(scene, walks[i].actor);
        struct WalkSearch search = {mover, ignored & ~(1u << (mover - gObjectEvents))};
        s16 goalX, goalY;

        u8 rangeX = mover->range.rangeX, rangeY = mover->range.rangeY;

        GetTargetTile(scene, &walks[i].goal, &goalX, &goalY);
        // Scripted walks ignore an NPC's wander range (applymovement does), but the collision check does not.
        mover->range.rangeX = mover->range.rangeY = 0;
        lengths[i] = SceneFindPath(mover->currentCoords.x, mover->currentCoords.y, goalX, goalY, CanWalkerStep, &search, sWalkDirs[i]);
        mover->range.rangeX = rangeX;
        mover->range.rangeY = rangeY;
        assertf(lengths[i] != SCENE_PATH_NONE, "scene %s step %d (%s): actor slot %d at (%d, %d) cannot reach (%d, %d) within %d steps and the on-screen window; did you mean a goal side that is open, or a closer actor?", scene->name, stepIndex, sStepKindNames[step->kind], walks[i].actor, mover->currentCoords.x, mover->currentCoords.y, goalX, goalY, SCENE_MAX_PATH)
        {
            return FALSE;
        }
    }
    for (i = 0; i < numWalks; i++)
    {
        if (lengths[i] == 0)
            continue;
        SceneBuildWalkScript(sWalkScript[i], sWalkDirs[i], lengths[i]);
        ScriptMovement_StartObjectMovementScript(GetActorLocalId(scene, walks[i].actor), gSaveBlock1Ptr->location.mapNum, gSaveBlock1Ptr->location.mapGroup, sWalkScript[i]);
        started = TRUE;
    }
    return started;
}

static bool32 StartFace(const struct Scene *scene, const struct SceneStep *step)
{
    struct ObjectEvent *object = GetActorObject(scene, step->actor);
    s16 x, y;
    enum Direction dir;

    GetTargetTile(scene, &step->target, &x, &y);
    dir = SceneDirectionToward(object->currentCoords.x, object->currentCoords.y, x, y);
    if (dir == DIR_NONE)
        return FALSE;
    sFaceScript[step->actor][0] = GetFaceDirectionMovementAction(dir);
    sFaceScript[step->actor][1] = MOVEMENT_ACTION_STEP_END;
    ScriptMovement_StartObjectMovementScript(GetActorLocalId(scene, step->actor), gSaveBlock1Ptr->location.mapNum, gSaveBlock1Ptr->location.mapGroup, sFaceScript[step->actor]);
    return TRUE;
}

void SceneBegin(void)
{
    u32 id = gSpecialVar_0x8004;

    GetScene(id);
    sSceneId = id;
    sStepIndex = 0;
    sPlayerLocked = FALSE;
}

static u32 RunStep(const struct Scene *scene, const struct SceneStep *step)
{
    switch (step->kind)
    {
    case SCENE_STEP_LOCK_PLAYER:
        sPlayerLocked = TRUE;
        return SCENE_RESULT_LOCK;
    case SCENE_STEP_UNLOCK_PLAYER:
        sPlayerLocked = FALSE;
        return SCENE_RESULT_UNLOCK;
    case SCENE_STEP_FACE:
        return StartFace(scene, step) ? SCENE_RESULT_WAIT_MOVEMENT : SCENE_RESULT_NEXT;
    case SCENE_STEP_WALK:
    case SCENE_STEP_PARALLEL_WALK:
        return StartWalks(scene, step, sStepIndex - 1) ? SCENE_RESULT_WAIT_MOVEMENT : SCENE_RESULT_NEXT;
    case SCENE_STEP_SAY:
        StringExpandPlaceholders(gStringVar4, step->text);
        return SCENE_RESULT_SAY;
    case SCENE_STEP_HIDE:
    case SCENE_STEP_SHOW:
        GetActorObject(scene, step->actor)->invisible = step->kind == SCENE_STEP_HIDE;
        return SCENE_RESULT_NEXT;
    case SCENE_STEP_ADVANCE:
        VarSet(scene->stageVar, step->stage);
        return SCENE_RESULT_NEXT;
    default:
        assertf(FALSE, "scene %s step %d: step kind %s is in the vocabulary but not implemented yet; did you mean to use `custom` or add its implementation to scene.c?", scene->name, sStepIndex - 1, step->kind < SCENE_STEP_KIND_COUNT ? sStepKindNames[step->kind] : "(unknown)")
        {
            return SCENE_RESULT_NEXT;
        }
    }
    return SCENE_RESULT_NEXT;
}

void SceneNextStep(void)
{
    const struct Scene *scene = GetScene(sSceneId);

    if (scene == NULL)
    {
        gSpecialVar_Result = SCENE_RESULT_END;
        return;
    }
    if (sStepIndex >= scene->numSteps)
    {
        // A scene that locks the player must unlock before it ends; do it here rather than leave them stuck.
        gSpecialVar_Result = sPlayerLocked ? SCENE_RESULT_UNLOCK : SCENE_RESULT_END;
        sPlayerLocked = FALSE;
        return;
    }
    gSpecialVar_Result = RunStep(scene, &scene->steps[sStepIndex++]);
}
