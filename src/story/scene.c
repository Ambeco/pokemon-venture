#include "global.h"
#include "assertf.h"
#include "battle_setup.h"
#include "list_menu.h"
#include "malloc.h"
#include "scene.h"
#include "event_data.h"
#include "event_object_movement.h"
#include "field_door.h"
#include "field_player_avatar.h"
#include "field_screen_effect.h"
#include "fieldmap.h"
#include "metatile_behavior.h"
#include "overworld.h"
#include "script.h"
#include "script_menu.h"
#include "script_movement.h"
#include "script_pokemon_util.h"
#include "sound.h"
#include "rival.h"
#include "string_util.h"
#include "text.h"
#include "constants/battle_setup.h"
#include "constants/event_objects.h"
#include "constants/event_object_movement.h"
#include "constants/flags.h"
#include "constants/maps.h"
#include "constants/songs.h"
#include "constants/map_event_ids.h"
#include "constants/scenes.h"
#include "constants/items.h"
#include "constants/script_menu.h"
#include "constants/vars.h"
#include "region_county.h"

#define WINDOW_WIDTH (2 * SCENE_WINDOW_HALF_WIDTH + 1)
#define WINDOW_HEIGHT (2 * SCENE_WINDOW_HALF_HEIGHT + 1)
#define WINDOW_CELLS (WINDOW_WIDTH * WINDOW_HEIGHT)
#define CELL_UNSEEN 0

STATIC_ASSERT(WINDOW_CELLS < 256, SceneQueueIndexFitsU8);

extern const u8 Scene_EventScript_RunTriggered[];

// Slot 0 of Mossdeep's county slice (GetCountySlotVar); test/chapter.c checks they agree.
#define SCENE_MOSSDEEP_SUPER_ROD_BEAT_VAR (COUNTY_VARS_START + COUNTY_ID(5, 1) * VARS_PER_COUNTY + 1)

// Slot 0 of Dewford's county slice, likewise.
#define SCENE_DEWFORD_OLD_ROD_BEAT_VAR (COUNTY_VARS_START + COUNTY_ID(0, 3) * VARS_PER_COUNTY + 1)

// Slot 0 of the Weather Institute county (3, 0), likewise.
#define SCENE_ROUTE118_GOOD_ROD_BEAT_VAR (COUNTY_VARS_START + COUNTY_ID(3, 0) * VARS_PER_COUNTY + 1)

// Slots 0 and 1 of the Route 110 county (2, 2), and slot 1 of the Weather Institute county, likewise.
#define SCENE_ROUTE110_RIVAL_BEAT_VAR (COUNTY_VARS_START + COUNTY_ID(2, 2) * VARS_PER_COUNTY + 1)
#define SCENE_ROUTE110_ITEMFINDER_BEAT_VAR (COUNTY_VARS_START + COUNTY_ID(2, 2) * VARS_PER_COUNTY + 2)
#define SCENE_ROUTE119_RIVAL_BEAT_VAR (COUNTY_VARS_START + COUNTY_ID(3, 0) * VARS_PER_COUNTY + 2)

#include "data/scenes.h"

static EWRAM_DATA u8 sSearchCameFrom[WINDOW_CELLS] = {0}; // the direction taken into each cell, CELL_UNSEEN if not reached
static EWRAM_DATA u8 sSearchQueue[WINDOW_CELLS] = {0};
static EWRAM_DATA u8 sWalkDirs[SCENE_MAX_PATH] = {0}; // one walker's path at a time
static EWRAM_DATA u8 sWalkScript[SCENE_MAX_ACTORS][SCENE_MAX_PATH + 3] = {0};
static EWRAM_DATA u8 sFaceScript[SCENE_MAX_ACTORS][2] = {0};
static EWRAM_DATA u8 sJumpScript[SCENE_MAX_ACTORS][5] = {0};
struct SceneFrame
{
    const struct SceneStep *steps;
    u8 numSteps;
    u8 index; // the next step to run
};

static EWRAM_DATA const struct Scene *sScene = NULL;
static EWRAM_DATA struct SceneFrame sFrames[SCENE_MAX_DEPTH] = {0}; // [0] is the scene's list; a branch pushes one
static EWRAM_DATA u8 sDepth = 0;
static EWRAM_DATA const struct SceneStep *sPendingStep = NULL; // the ask, give, battle or custom step the driver is running
static EWRAM_DATA bool8 sPauseActive = FALSE;
static EWRAM_DATA u8 sPauseLeft = 0;
static EWRAM_DATA bool8 sPlayerLocked = FALSE;
static EWRAM_DATA bool8 sArrivalPending = FALSE;
static EWRAM_DATA bool8 sNextAnchorSet = FALSE; // a trigger chose the anchor for the scene it is about to start
static EWRAM_DATA s16 sNextAnchorX = 0, sNextAnchorY = 0;
static EWRAM_DATA u8 sNextAnchorWarp = 0;
static EWRAM_DATA bool8 sAnchorSet = FALSE;
static EWRAM_DATA s16 sAnchorX = 0, sAnchorY = 0; // the running scene's anchor door, in object (MAP_OFFSET) coordinates
static EWRAM_DATA u8 sAnchorWarp = 0;

static const char *const sStepKindNames[SCENE_STEP_KIND_COUNT] =
{
    [SCENE_STEP_LOCK_PLAYER] = "lock_player",
    [SCENE_STEP_UNLOCK_PLAYER] = "unlock_player",
    [SCENE_STEP_FACE] = "face",
    [SCENE_STEP_FACE_ORIGINAL] = "face_original",
    [SCENE_STEP_WALK] = "walk",
    [SCENE_STEP_PARALLEL_WALK] = "parallel_walk",
    [SCENE_STEP_SAY] = "say",
    [SCENE_STEP_HIDE] = "hide",
    [SCENE_STEP_SHOW] = "show",
    [SCENE_STEP_JUMP] = "jump",
    [SCENE_STEP_ADVANCE] = "advance",
    [SCENE_STEP_DOOR_OPEN] = "door_open",
    [SCENE_STEP_DOOR_CLOSE] = "door_close",
    [SCENE_STEP_WARP] = "warp",
    [SCENE_STEP_PLACE] = "place",
    [SCENE_STEP_SET_FLAG] = "set_flag",
    [SCENE_STEP_CLEAR_FLAG] = "clear_flag",
    [SCENE_STEP_HOP] = "hop",
    [SCENE_STEP_PAUSE] = "pause",
    [SCENE_STEP_CHASE_LOOP] = "chase_loop",
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

bool32 SceneTriggerFires(u32 kind, u32 radius, s16 playerX, s16 playerY, s16 doorX, s16 doorY)
{
    s16 frontX, frontY;
    s32 dx = playerX - doorX, dy = playerY - doorY;

    switch (kind)
    {
    case SCENE_TRIGGER_PROXIMITY:
        return (dx < 0 ? -dx : dx) + (dy < 0 ? -dy : dy) <= (s32)radius;
    case SCENE_TRIGGER_ON_DOOR:
        return dx == 0 && dy == 0;
    case SCENE_TRIGGER_IN_FRONT_OF_DOOR:
        SceneResolveSide(doorX, doorY, DIR_SOUTH, SCENE_SIDE_FRONT, &frontX, &frontY);
        return playerX == frontX && playerY == frontY;
    default:
        assertf(FALSE, "scene trigger kind %d does not exist; did you mean one of enum SceneTriggerKind?", kind)
        {
            return FALSE;
        }
    }
    return FALSE;
}

bool32 SceneExitTriggerFires(enum Direction edge, u32 depth, s16 playerX, s16 playerY, s32 mapWidth, s32 mapHeight)
{
    switch (edge)
    {
    case DIR_NORTH:
        return playerY == (s32)depth;
    case DIR_SOUTH:
        return playerY == mapHeight - 1 - (s32)depth;
    case DIR_WEST:
        return playerX == (s32)depth;
    case DIR_EAST:
        return playerX == mapWidth - 1 - (s32)depth;
    default:
        assertf(FALSE, "exit trigger edge %d is not a cardinal direction; did you mean DIR_NORTH, DIR_SOUTH, DIR_WEST or DIR_EAST?", edge)
        {
            return FALSE;
        }
    }
    return FALSE;
}

// Fires only where the edge row (or column) is walkable straight ahead, so the trigger sits at the gap, not along the whole edge.
static bool32 SceneExitTriggerHolds(const struct SceneTrigger *trigger, s16 playerX, s16 playerY)
{
    s32 width = gMapHeader.mapLayout->width, height = gMapHeader.mapLayout->height;
    s16 edgeX = playerX, edgeY = playerY;

    if (!SceneExitTriggerFires(trigger->edge, trigger->radius, playerX - MAP_OFFSET, playerY - MAP_OFFSET, width, height))
        return FALSE;
    switch (trigger->edge)
    {
    case DIR_NORTH:
        edgeY = MAP_OFFSET;
        break;
    case DIR_SOUTH:
        edgeY = MAP_OFFSET + height - 1;
        break;
    case DIR_WEST:
        edgeX = MAP_OFFSET;
        break;
    default:
        edgeX = MAP_OFFSET + width - 1;
        break;
    }
    return MapGridGetCollisionAt(edgeX, edgeY) == 0;
}

const struct SceneTrigger *SceneFindTrigger(const struct SceneTrigger *table, u32 count, u32 mapGroup, u32 mapNum, s16 playerX, s16 playerY, bool32 mapJustLoaded)
{
    u32 i;

    for (i = 0; i < count; i++)
    {
        const struct SceneTrigger *trigger = &table[i];
        const struct MapEvents *events = gMapHeader.events;
        const struct WarpEvent *door;

        if (trigger->mapGroup != mapGroup || trigger->mapNum != mapNum || VarGet(trigger->stageVar) != trigger->stageValue)
            continue;
        if (trigger->kind == SCENE_TRIGGER_ENTER_MAP)
        {
            if (mapJustLoaded)
                return trigger;
            continue;
        }
        if (trigger->kind == SCENE_TRIGGER_EXIT)
        {
            if (SceneExitTriggerHolds(trigger, playerX, playerY))
                return trigger;
            continue;
        }
        assertf(events != NULL && trigger->doorWarp < events->warpCount, "scene trigger for scene %d names door warp %d, but map %d.%d has %d warps; did you mean another warp index?", trigger->sceneId, trigger->doorWarp, mapGroup, mapNum, events == NULL ? 0 : events->warpCount)
        {
            continue;
        }
        door = &events->warps[trigger->doorWarp];
        if (SceneTriggerFires(trigger->kind, trigger->radius, playerX, playerY, door->x + MAP_OFFSET, door->y + MAP_OFFSET))
            return trigger;
    }
    return NULL;
}

const struct SceneTrigger *SceneFindTriggerAt(u32 mapGroup, u32 mapNum, s16 playerX, s16 playerY, bool32 mapJustLoaded)
{
    return SceneFindTrigger(sSceneTriggers, ARRAY_COUNT(sSceneTriggers), mapGroup, mapNum, playerX, playerY, mapJustLoaded);
}

void SceneNotifyMapLoaded(void)
{
    sArrivalPending = TRUE;
}

bool8 SceneTryStartTrigger(bool32 tookStep)
{
    struct ObjectEvent *player = &gObjectEvents[gPlayerAvatar.objectEventId];
    const struct SceneTrigger *trigger;
    const struct WarpEvent *door;

    bool32 mapJustLoaded = sArrivalPending;

    if (!tookStep && !sArrivalPending)
        return FALSE;
    sArrivalPending = FALSE;
    trigger = SceneFindTriggerAt(gSaveBlock1Ptr->location.mapGroup, gSaveBlock1Ptr->location.mapNum, player->currentCoords.x, player->currentCoords.y, mapJustLoaded);
    if (trigger == NULL)
        return FALSE;
    if (trigger->kind == SCENE_TRIGGER_ENTER_MAP || trigger->kind == SCENE_TRIGGER_EXIT)
    {
        sNextAnchorSet = FALSE;
    }
    else
    {
        door = &gMapHeader.events->warps[trigger->doorWarp];
        sNextAnchorSet = TRUE;
        sNextAnchorX = door->x + MAP_OFFSET;
        sNextAnchorY = door->y + MAP_OFFSET;
        sNextAnchorWarp = trigger->doorWarp;
    }
    gSpecialVar_0x8004 = trigger->sceneId;
    ScriptContext_SetupScript(Scene_EventScript_RunTriggered);
    return TRUE;
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

u32 SceneRingLength(const struct SceneRing *ring)
{
    return 2 * (ring->width + ring->height) - 4;
}

void SceneRingTile(const struct SceneRing *ring, u32 index, s16 *outX, s16 *outY)
{
    u32 w = ring->width, h = ring->height;

    index %= SceneRingLength(ring);
    if (index < w - 1)
    {
        *outX = ring->x + index;
        *outY = ring->y;
    }
    else if (index < w + h - 2)
    {
        *outX = ring->x + w - 1;
        *outY = ring->y + (index - (w - 1));
    }
    else if (index < 2 * w + h - 3)
    {
        *outX = ring->x + w - 1 - (index - (w + h - 2));
        *outY = ring->y + h - 1;
    }
    else
    {
        *outX = ring->x;
        *outY = ring->y + h - 1 - (index - (2 * w + h - 3));
    }
}

u32 SceneRingIndexOf(const struct SceneRing *ring, s16 x, s16 y)
{
    u32 i, length = SceneRingLength(ring);

    for (i = 0; i < length; i++)
    {
        s16 tileX, tileY;

        SceneRingTile(ring, i, &tileX, &tileY);
        if (tileX == x && tileY == y)
            return i;
    }
    return SCENE_RING_NONE;
}

void SceneRingDirs(const struct SceneRing *ring, u32 startIndex, u32 count, u8 *dirs)
{
    u32 i;

    for (i = 0; i < count; i++)
    {
        s16 fromX, fromY, toX, toY;

        SceneRingTile(ring, startIndex + i, &fromX, &fromY);
        SceneRingTile(ring, startIndex + i + 1, &toX, &toY);
        dirs[i] = SceneDirectionToward(fromX, fromY, toX, toY);
    }
}

static s32 AbsDiff(s32 a, s32 b)
{
    return a > b ? a - b : b - a;
}

bool32 SceneFindRing(s16 centerX, s16 centerY, u32 searchRadius, s16 avoidX, s16 avoidY, u32 avoidRadius, SceneTileScore score, void *context, struct SceneRing *out)
{
    static const u8 sizes[][2] = {{3, 3}, {4, 3}, {3, 4}};
    bool32 found = FALSE;
    s32 bestScore = 0, bestDistance = 0;
    s32 ox, oy;
    u32 sizeIndex;

    for (oy = centerY - (s32)searchRadius; oy <= centerY + (s32)searchRadius; oy++)
    {
        for (ox = centerX - (s32)searchRadius; ox <= centerX + (s32)searchRadius; ox++)
        {
            for (sizeIndex = 0; sizeIndex < ARRAY_COUNT(sizes); sizeIndex++)
            {
                struct SceneRing candidate = {ox, oy, sizes[sizeIndex][0], sizes[sizeIndex][1]};
                u32 i, length = SceneRingLength(&candidate);
                s32 total = 0, distance;
                bool32 usable = TRUE;

                for (i = 0; i < length && usable; i++)
                {
                    s16 x, y;
                    s32 tileScore, dx, dy;

                    SceneRingTile(&candidate, i, &x, &y);
                    dx = AbsDiff(x, avoidX);
                    dy = AbsDiff(y, avoidY);
                    tileScore = score(context, x, y);
                    if (tileScore < 0 || (dx > dy ? dx : dy) <= (s32)avoidRadius)
                        usable = FALSE;
                    total += tileScore;
                }
                if (!usable)
                    continue;
                // Twice the distance from the center to the ring's center, in tiles.
                distance = AbsDiff(2 * ox + candidate.width - 1, 2 * centerX) + AbsDiff(2 * oy + candidate.height - 1, 2 * centerY);
                if (!found || total > bestScore || (total == bestScore && distance < bestDistance))
                {
                    found = TRUE;
                    bestScore = total;
                    bestDistance = distance;
                    *out = candidate;
                }
            }
        }
    }
    return found;
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

u8 *SceneBuildWalkScript(u8 *out, const u8 *dirs, u32 length, bool32 slide)
{
    u32 i;

    assertf(length <= SCENE_MAX_PATH, "walk of %d steps is longer than SCENE_MAX_PATH (%d)", length, SCENE_MAX_PATH)
    {
        length = SCENE_MAX_PATH;
    }
    if (slide)
        *out++ = MOVEMENT_ACTION_LOCK_FACING_DIRECTION;
    for (i = 0; i < length; i++)
        *out++ = GetWalkNormalMovementAction(dirs[i]);
    if (slide)
        *out++ = MOVEMENT_ACTION_UNLOCK_FACING_DIRECTION;
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
    // A door's own tile is blocked by the map grid (the player enters it through its warp), but a scene's walkers
    // may step onto its anchor door.
    if (sAnchorSet && nx == sAnchorX && ny == sAnchorY)
        return TRUE;
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
    struct ObjectEvent *object;

    if (target->actor == SCENE_ACTOR_ANCHOR)
    {
        assertf(sAnchorSet && target->side != SCENE_SIDE_HOME, "scene %s targets its anchor door%s; did you mean to start it from a trigger in sSceneTriggers, and a side other than HOME?", scene->name, sAnchorSet ? " by HOME" : ", but a script started it, so it has none")
        {
            *x = sAnchorX;
            *y = sAnchorY;
            return;
        }
        SceneResolveSide(sAnchorX, sAnchorY, DIR_SOUTH, target->side, x, y);
        return;
    }
    object = GetActorObject(scene, target->actor);
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
        single.slide = step->slide;
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
        lengths[i] = SceneFindPath(mover->currentCoords.x, mover->currentCoords.y, goalX, goalY, CanWalkerStep, &search, sWalkDirs);
        mover->range.rangeX = rangeX;
        mover->range.rangeY = rangeY;
        assertf(lengths[i] != SCENE_PATH_NONE, "scene %s step %d (%s): actor slot %d at (%d, %d) cannot reach (%d, %d) (goal collision %d) within %d steps and the on-screen window; did you mean a goal side that is open, or a closer actor?", scene->name, stepIndex, sStepKindNames[step->kind], walks[i].actor, mover->currentCoords.x, mover->currentCoords.y, goalX, goalY, GetCollisionAtCoords(mover, goalX, goalY, SceneDirectionToward(mover->currentCoords.x, mover->currentCoords.y, goalX, goalY)), SCENE_MAX_PATH)
        {
            return FALSE;
        }
        // Built now, while the path is in the shared buffer; nothing starts until every walker has a path.
        if (lengths[i] != 0)
            SceneBuildWalkScript(sWalkScript[i], sWalkDirs, lengths[i], walks[i].slide);
    }
    for (i = 0; i < numWalks; i++)
    {
        if (lengths[i] == 0)
            continue;
        ScriptMovement_StartObjectMovementScript(GetActorLocalId(scene, walks[i].actor), gSaveBlock1Ptr->location.mapNum, gSaveBlock1Ptr->location.mapGroup, sWalkScript[i]);
        started = TRUE;
    }
    return started;
}

static s32 RingTileScore(void *context, s16 x, s16 y)
{
    struct ObjectEvent *mover = context;
    u32 behavior;

    if (GetCollisionAtCoords(mover, x, y, DIR_NORTH) != COLLISION_NONE)
        return -1;
    behavior = MapGridGetMetatileBehaviorAt(x, y);
    return MetatileBehavior_IsTallGrass(behavior) || MetatileBehavior_IsLongGrass(behavior) ? 1 : 0;
}

// The leader and chaser each walk to a tile of a ring found about halfway between the leader and the target, the
// chaser `lag` tiles behind, then run laps of it. Starts nothing and returns FALSE on failure.
static bool32 StartChaseLoop(const struct Scene *scene, const struct SceneStep *step, u32 stepIndex)
{
    struct ObjectEvent *objects[2] = {GetActorObject(scene, step->actor), GetActorObject(scene, step->chaser)};
    struct SceneRing ring;
    u32 startIndex[2], length, i, nearest = 0, nearestDistance = 0xFFFF;
    s16 avoidX, avoidY;
    bool32 ok;
    u8 rangeX[2], rangeY[2];

    GetTargetTile(scene, &step->target, &avoidX, &avoidY);
    for (i = 0; i < 2; i++)
    {
        rangeX[i] = objects[i]->range.rangeX;
        rangeY[i] = objects[i]->range.rangeY;
        // Scripted walks ignore the wander range, but the collision check does not.
        objects[i]->range.rangeX = objects[i]->range.rangeY = 0;
    }
    ok = SceneFindRing((objects[0]->currentCoords.x + avoidX) / 2, (objects[0]->currentCoords.y + avoidY) / 2, 3, avoidX, avoidY, step->avoidRadius, RingTileScore, objects[0], &ring);
    assertf(ok, "scene %s step %d (chase_loop): no free ring of 3x3 to 4x3 tiles within 3 tiles of the midpoint between actor slot %d and (%d, %d), more than %d tiles from it; did you mean an open area there? (route generation must guarantee one)", scene->name, stepIndex, step->actor, avoidX, avoidY, step->avoidRadius)
    {
        ok = FALSE;
    }
    if (ok)
    {
        length = SceneRingLength(&ring);
        for (i = 0; i < length; i++)
        {
            s16 x, y;
            u32 distance;

            SceneRingTile(&ring, i, &x, &y);
            distance = AbsDiff(x, objects[0]->currentCoords.x) + AbsDiff(y, objects[0]->currentCoords.y);
            if (distance < nearestDistance)
            {
                nearestDistance = distance;
                nearest = i;
            }
        }
        startIndex[0] = nearest;
        startIndex[1] = (nearest + length - step->lag % length) % length;
    }
    for (i = 0; ok && i < 2; i++)
    {
        struct WalkSearch search = {objects[i], 1u << (objects[1 - i] - gObjectEvents)};
        s16 goalX, goalY;
        u32 approach, total;

        SceneRingTile(&ring, startIndex[i], &goalX, &goalY);
        approach = SceneFindPath(objects[i]->currentCoords.x, objects[i]->currentCoords.y, goalX, goalY, CanWalkerStep, &search, sWalkDirs);
        total = approach == SCENE_PATH_NONE ? 0 : approach + step->laps * length;
        assertf(approach != SCENE_PATH_NONE && total <= SCENE_MAX_PATH, "scene %s step %d (chase_loop): actor at (%d, %d) cannot reach its ring tile (%d, %d), or needs more than %d steps with %d laps of %d; did you mean fewer laps or a closer ring?", scene->name, stepIndex, objects[i]->currentCoords.x, objects[i]->currentCoords.y, goalX, goalY, SCENE_MAX_PATH, step->laps, length)
        {
            ok = FALSE;
            break;
        }
        SceneRingDirs(&ring, startIndex[i], step->laps * length, sWalkDirs + approach);
        SceneBuildWalkScript(sWalkScript[i], sWalkDirs, total, FALSE);
    }
    if (ok)
    {
        ScriptMovement_StartObjectMovementScript(GetActorLocalId(scene, step->actor), gSaveBlock1Ptr->location.mapNum, gSaveBlock1Ptr->location.mapGroup, sWalkScript[0]);
        ScriptMovement_StartObjectMovementScript(GetActorLocalId(scene, step->chaser), gSaveBlock1Ptr->location.mapNum, gSaveBlock1Ptr->location.mapGroup, sWalkScript[1]);
    }
    for (i = 0; i < 2; i++)
    {
        objects[i]->range.rangeX = rangeX[i];
        objects[i]->range.rangeY = rangeY[i];
    }
    return ok;
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

static void StartJump(const struct Scene *scene, const struct SceneStep *step)
{
    u8 *script = sJumpScript[step->actor];

    *script++ = MOVEMENT_ACTION_DELAY_8;
    *script++ = MOVEMENT_ACTION_DISABLE_JUMP_LANDING_GROUND_EFFECT;
    *script++ = GetJumpInPlaceMovementAction(GetActorObject(scene, step->actor)->facingDirection);
    *script++ = MOVEMENT_ACTION_DELAY_8;
    *script = MOVEMENT_ACTION_STEP_END;
    ScriptMovement_StartObjectMovementScript(GetActorLocalId(scene, step->actor), gSaveBlock1Ptr->location.mapNum, gSaveBlock1Ptr->location.mapGroup, sJumpScript[step->actor]);
}

static void StartHop(const struct Scene *scene, const struct SceneStep *step)
{
    u8 *script = sJumpScript[step->actor]; // a hop and a jump are never in flight for one actor together

    PlaySE(SE_LEDGE);
    *script++ = GetJumpMovementAction(step->direction);
    *script++ = MOVEMENT_ACTION_DELAY_16;
    *script++ = MOVEMENT_ACTION_DELAY_16;
    *script++ = MOVEMENT_ACTION_DELAY_16;
    *script = MOVEMENT_ACTION_STEP_END;
    ScriptMovement_StartObjectMovementScript(GetActorLocalId(scene, step->actor), gSaveBlock1Ptr->location.mapNum, gSaveBlock1Ptr->location.mapGroup, sJumpScript[step->actor]);
}

static void PlaceActor(const struct Scene *scene, const struct SceneStep *step)
{
    u8 localId = GetActorLocalId(scene, step->actor);
    u8 mapNum = gSaveBlock1Ptr->location.mapNum, mapGroup = gSaveBlock1Ptr->location.mapGroup;
    s16 x, y;

    if (GetObjectEventIdByLocalId(localId) >= OBJECT_EVENTS_COUNT)
    {
        // The spawn ignores the object's hide flag, like the addobject command.
        TrySpawnObjectEvent(localId, mapNum, mapGroup);
        assertf(GetObjectEventIdByLocalId(localId) < OBJECT_EVENTS_COUNT, "scene %s: place could not spawn local id %d; did you mean an object that this map's template list has?", scene->name, localId)
        {
            return;
        }
    }
    GetTargetTile(scene, &step->target, &x, &y);
    TryMoveObjectEventToMapCoords(localId, mapNum, mapGroup, x - MAP_OFFSET, y - MAP_OFFSET);
}

static void WarpThroughAnchor(const struct Scene *scene, bool32 silent)
{
    const struct WarpEvent *door;

    assertf(sAnchorSet && sAnchorWarp < gMapHeader.events->warpCount, "scene %s: warp needs an anchor door; did you mean to start it from a trigger in sSceneTriggers?", scene->name)
    {
        return;
    }
    door = &gMapHeader.events->warps[sAnchorWarp];
    SetWarpDestination(door->mapGroup, door->mapNum, door->warpId, -1, -1);
    if (silent)
        DoDiveWarp();
    else
        DoWarp();
    ResetInitialPlayerAvatarState();
}

static void BeginScene(const struct Scene *scene)
{
    sScene = scene;
    sDepth = 0;
    if (scene != NULL)
    {
        sFrames[0].steps = scene->steps;
        sFrames[0].numSteps = scene->numSteps;
        sFrames[0].index = 0;
    }
    sPendingStep = NULL;
    sPlayerLocked = FALSE;
    sPauseActive = FALSE;
    sAnchorSet = sNextAnchorSet;
    sAnchorX = sNextAnchorX;
    sAnchorY = sNextAnchorY;
    sAnchorWarp = sNextAnchorWarp;
    sNextAnchorSet = FALSE;
}

void SceneBegin(void)
{
    BeginScene(GetScene(gSpecialVar_0x8004));
}

void SceneBeginScene(const struct Scene *scene)
{
    BeginScene(scene);
}

// The number of the step being run in the current frame, for failure messages.
#define STEP_NUMBER() (sFrames[sDepth].index - 1)

static void PushBranch(const struct SceneBranch *branch)
{
    if (branch->numSteps == 0)
        return;
    assertf(sDepth + 1 < SCENE_MAX_DEPTH, "scene %s nests branches deeper than SCENE_MAX_DEPTH (%d); did you mean to flatten a branch or raise the limit?", sScene->name, SCENE_MAX_DEPTH)
    {
        return;
    }
    sDepth++;
    sFrames[sDepth].steps = branch->steps;
    sFrames[sDepth].numSteps = branch->numSteps;
    sFrames[sDepth].index = 0;
}

// Ends the scene where it stands, without advancing it (a branch pushed afterwards still runs).
static void AbortScene(void)
{
    sDepth = 0;
    sFrames[0].index = sFrames[0].numSteps;
}

static u32 StartAsk(const struct Scene *scene, const struct SceneStep *step)
{
    u32 i;

    if (step->kind == SCENE_STEP_ASK_YES_NO)
    {
        assertf(step->numBranches == 2 && step->branches != NULL, "scene %s step %d (ask_yes_no) has %d branches; it needs exactly 2, yes then no (use SCENE_EMPTY_BRANCH for one that does nothing)", scene->name, STEP_NUMBER(), step->numBranches)
        {
            return SCENE_RESULT_NEXT;
        }
        StringExpandPlaceholders(gStringVar4, step->text);
        sPendingStep = step;
        return SCENE_RESULT_ASK_YES_NO;
    }
    assertf(step->numBranches >= 2 && step->numBranches <= SCENE_MAX_CHOICES && step->branches != NULL, "scene %s step %d (ask_choice) has %d choices; it needs 2 to %d (for one, use `say`)", scene->name, STEP_NUMBER(), step->numBranches, SCENE_MAX_CHOICES)
    {
        return SCENE_RESULT_NEXT;
    }
    StringExpandPlaceholders(gStringVar4, step->text);
    for (i = 0; i < step->numBranches; i++)
    {
        struct ListMenuItem item;
        u8 *name;

        assertf(step->branches[i].label != NULL, "scene %s step %d (ask_choice): choice %d has no label; did you mean SCENE_BRANCH(label, steps) or SCENE_EMPTY_BRANCH(label)?", scene->name, STEP_NUMBER(), i)
        {
            if (i > 0)
                MultichoiceDynamic_DestroyStack(); // the labels pushed so far
            return SCENE_RESULT_NEXT;
        }
        name = Alloc(100); // like dynmultipush's copy, the menu owns it

        StringExpandPlaceholders(name, step->branches[i].label);
        item.name = name;
        item.id = i;
        MultichoiceDynamic_PushElement(item);
    }
    sPendingStep = step;
    return SCENE_RESULT_ASK_CHOICE;
}

void SceneTakeAnswer(void)
{
    const struct SceneStep *step = sPendingStep;
    u32 answer = gSpecialVar_Result;

    sPendingStep = NULL;
    assertf(step != NULL, "SceneTakeAnswer ran with no ask or give pending; did you call it outside Scene_EventScript_Run?")
    {
        return;
    }
    switch (step->kind)
    {
    case SCENE_STEP_ASK_YES_NO:
        PushBranch(&step->branches[answer ? 0 : 1]);
        break;
    case SCENE_STEP_ASK_CHOICE:
        assertf(answer < step->numBranches, "scene %s ask_choice answered %d but has %d choices; did the menu allow B (answer %d)?", sScene->name, answer, step->numBranches, MULTI_B_PRESSED)
        {
            AbortScene();
            break;
        }
        PushBranch(&step->branches[answer]);
        break;
    case SCENE_STEP_GIVE:
        if (gSpecialVar_0x8007 == TRUE)
            break;
        // The item was not received, so nothing after this may advance the stage, or the item would be lost for good:
        // the bag-full branch (if any) is the last thing the scene runs.
        AbortScene();
        if (step->numBranches > 0 && step->branches != NULL)
            PushBranch(&step->branches[0]);
        break;
    default:
        assertf(FALSE, "SceneTakeAnswer ran for step kind %d, which asks nothing", step->kind)
        {
            break;
        }
    }
}

static u32 StartGive(const struct SceneStep *step)
{
    gSpecialVar_0x8000 = step->item;
    gSpecialVar_0x8001 = step->quantity;
    sPendingStep = step;
    return SCENE_RESULT_GIVE;
}

static u32 PrepareBattle(const struct Scene *scene, const struct SceneStep *step)
{
    if (step->species != 0)
    {
        CreateScriptedWildMon(step->species, step->level, ITEM_NONE);
    }
    else
    {
        assertf(step->text != NULL, "scene %s step %d (battle) has trainer %d but no defeat text; did you mean SCENE_BATTLE_TRAINER(trainer, text)?", scene->name, STEP_NUMBER(), step->trainer)
        {
            return SCENE_RESULT_NEXT;
        }
    }
    sPendingStep = step;
    return SCENE_RESULT_BATTLE;
}

void SceneLoadGive(void)
{
    const struct SceneStep *step = sPendingStep;

    assertf(step != NULL && step->kind == SCENE_STEP_GIVE, "SceneLoadGive ran with no give step pending; did you call it outside Scene_EventScript_Run?")
    {
        return;
    }
    gSpecialVar_0x8000 = step->item;
    gSpecialVar_0x8001 = step->quantity;
}

void SceneStartBattle(void)
{
    const struct SceneStep *step = sPendingStep;

    sPendingStep = NULL;
    assertf(step != NULL && step->kind == SCENE_STEP_BATTLE, "SceneStartBattle ran with no battle step pending; did you call it outside Scene_EventScript_Run?")
    {
        return;
    }
    if (step->species != 0)
    {
        BattleSetup_StartScriptedWildBattle();
        return;
    }
    InitTrainerBattleParameter();
    TRAINER_BATTLE_PARAM.mode = TRAINER_BATTLE_SINGLE_NO_INTRO_TEXT;
    TRAINER_BATTLE_PARAM.playMusicA = TRUE;
    TRAINER_BATTLE_PARAM.opponentA = step->rival ? GetRivalTrainer(step->trainer) : step->trainer;
    TRAINER_BATTLE_PARAM.defeatTextA = (u8 *)step->text;
    PlayTrainerEncounterMusic();
    BattleSetup_StartTrainerBattle();
}

void SceneCallCustom(struct ScriptContext *ctx)
{
    const struct SceneStep *step = sPendingStep;

    sPendingStep = NULL;
    assertf(step != NULL && step->script != NULL, "scene custom step has no script; did you mean SCENE_CUSTOM(EventScript_Name)?")
    {
        return;
    }
    ScriptCall(ctx, step->script);
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
    case SCENE_STEP_FACE_ORIGINAL:
        sFaceScript[step->actor][0] = MOVEMENT_ACTION_FACE_ORIGINAL_DIRECTION;
        sFaceScript[step->actor][1] = MOVEMENT_ACTION_STEP_END;
        ScriptMovement_StartObjectMovementScript(GetActorLocalId(scene, step->actor), gSaveBlock1Ptr->location.mapNum, gSaveBlock1Ptr->location.mapGroup, sFaceScript[step->actor]);
        return SCENE_RESULT_WAIT_MOVEMENT;
    case SCENE_STEP_WALK:
    case SCENE_STEP_PARALLEL_WALK:
        return StartWalks(scene, step, STEP_NUMBER()) ? SCENE_RESULT_WAIT_MOVEMENT : SCENE_RESULT_NEXT;
    case SCENE_STEP_JUMP:
        StartJump(scene, step);
        return SCENE_RESULT_WAIT_MOVEMENT;
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
    case SCENE_STEP_SET_FLAG:
        FlagSet(step->flag);
        return SCENE_RESULT_NEXT;
    case SCENE_STEP_CLEAR_FLAG:
        FlagClear(step->flag);
        return SCENE_RESULT_NEXT;
    case SCENE_STEP_PLACE:
        PlaceActor(scene, step);
        return SCENE_RESULT_NEXT;
    case SCENE_STEP_PAUSE:
        if (!sPauseActive)
        {
            sPauseActive = TRUE;
            sPauseLeft = step->pause;
        }
        if (sPauseLeft == 0)
        {
            sPauseActive = FALSE;
            return SCENE_RESULT_NEXT;
        }
        sPauseLeft--;
        sFrames[sDepth].index--; // stay on this step until the wait is over
        return SCENE_RESULT_DELAY;
    case SCENE_STEP_MUSIC:
        if (step->song != 0)
        {
            Overworld_SetSavedMusic(step->song);
            PlayNewMapMusic(step->song);
        }
        else
        {
            Overworld_ChangeMusicToDefault();
        }
        return SCENE_RESULT_NEXT;
    case SCENE_STEP_CHASE_LOOP:
        return StartChaseLoop(scene, step, STEP_NUMBER()) ? SCENE_RESULT_WAIT_MOVEMENT : SCENE_RESULT_NEXT;
    case SCENE_STEP_HOP:
        StartHop(scene, step);
        return SCENE_RESULT_WAIT_MOVEMENT;
    case SCENE_STEP_DOOR_OPEN:
    case SCENE_STEP_DOOR_CLOSE:
    {
        s16 x, y;
        struct SceneTarget door = {SCENE_ACTOR_ANCHOR, SCENE_SIDE_SELF};

        GetTargetTile(scene, &door, &x, &y);
        if (step->kind == SCENE_STEP_DOOR_OPEN)
        {
            PlaySE(GetDoorSoundEffect(x, y));
            FieldAnimateDoorOpen(x, y);
        }
        else
        {
            FieldAnimateDoorClose(x, y);
        }
        return SCENE_RESULT_WAIT_DOOR;
    }
    case SCENE_STEP_WARP:
        WarpThroughAnchor(scene, step->silent);
        return SCENE_RESULT_WARP;
    case SCENE_STEP_ASK_YES_NO:
    case SCENE_STEP_ASK_CHOICE:
        return StartAsk(scene, step);
    case SCENE_STEP_GIVE:
        return StartGive(step);
    case SCENE_STEP_BATTLE:
        return PrepareBattle(scene, step);
    case SCENE_STEP_CUSTOM:
        sPendingStep = step;
        return SCENE_RESULT_CUSTOM;
    default:
        assertf(FALSE, "scene %s step %d: step kind %s is unknown; did you mean one of enum SceneStepKind, or `custom` for a one-off?", scene->name, STEP_NUMBER(), step->kind < SCENE_STEP_KIND_COUNT ? sStepKindNames[step->kind] : "(unknown)")
        {
            return SCENE_RESULT_NEXT;
        }
    }
    return SCENE_RESULT_NEXT;
}

void SceneNextStep(void)
{
    struct SceneFrame *frame;

    if (sScene == NULL)
    {
        gSpecialVar_Result = SCENE_RESULT_END;
        return;
    }
    while (sDepth > 0 && sFrames[sDepth].index >= sFrames[sDepth].numSteps)
        sDepth--;
    frame = &sFrames[sDepth];
    if (frame->index >= frame->numSteps)
    {
        // A scene that locks the player must unlock before it ends; do it here rather than leave them stuck.
        gSpecialVar_Result = sPlayerLocked ? SCENE_RESULT_UNLOCK : SCENE_RESULT_END;
        sPlayerLocked = FALSE;
        return;
    }
    gSpecialVar_Result = RunStep(sScene, &frame->steps[frame->index++]);
}
