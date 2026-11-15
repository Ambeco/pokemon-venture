#ifndef GUARD_SCENE_H
#define GUARD_SCENE_H

/**
 * Scene runner (design_scenes.md, "Scene runner").
 *
 * A scene is a list of steps run one at a time by the driver script Scene_EventScript_Run, which loops on
 * `special SceneNextStep`. Each step reads the live object positions, so a walk's goal is relative to where
 * the previous step left everyone. Start a scene from a script with
 *     setvar VAR_0x8004, SCENE_ID
 *     call Scene_EventScript_Run
 *
 * Actors are named by slot: slot 0 is always the player, the others are the local ids in Scene.actors. A step
 * kind that is in the vocabulary but not implemented yet fails loudly, naming the kind, instead of doing nothing.
 * A walk or parallel_walk fails loudly when its goal is blocked or not reachable within the search window.
 *
 * A scene can also start itself: sSceneTriggers (data/scenes.h) lists, per map, a stage var value and an anchor (a
 * door, by warp index) with a trigger kind. SceneTryStartTrigger checks the table when the player takes a step and
 * once after each map load (so teleporting counts), never on a ROM coord_event, so generated maps need none. Distance
 * is Manhattan, in tiles. A triggered scene's anchor is the pseudo-actor SCENE_ACTOR_ANCHOR: SIDE_SELF is the door
 * tile and SIDE_FRONT the tile below it (a door faces south; generated maps will supply the facing). A scene that
 * does not `advance` its stage fires again on the next check while the trigger still holds.
 */

#include "global.h"
#include "constants/scenes.h"

#define SCENE_MAX_ACTORS 4          // slot 0 (the player) plus three
#define SCENE_MAX_PATH 40           // longest walk, in steps
#define SCENE_WINDOW_HALF_WIDTH 8   // search window around the walker: the screen (15x10) plus a margin
#define SCENE_WINDOW_HALF_HEIGHT 6
#define SCENE_PATH_NONE 0xFFFF

enum SceneActorSlot
{
    SCENE_ACTOR_PLAYER,
    SCENE_ACTOR_1,
    SCENE_ACTOR_2,
    SCENE_ACTOR_3,
    SCENE_ACTOR_ANCHOR = SCENE_MAX_ACTORS, // only in a target; not a slot of Scene.actors
};

enum SceneSide
{
    SCENE_SIDE_SELF,
    SCENE_SIDE_FRONT,   // the tile the actor faces
    SCENE_SIDE_BEHIND,
    SCENE_SIDE_LEFT,    // relative to its facing
    SCENE_SIDE_RIGHT,
    SCENE_SIDE_HOME,    // the tile the actor started the map on (its object template's position)
};

enum SceneStepKind
{
    SCENE_STEP_LOCK_PLAYER,
    SCENE_STEP_UNLOCK_PLAYER,
    SCENE_STEP_FACE,
    SCENE_STEP_WALK,
    SCENE_STEP_PARALLEL_WALK,
    SCENE_STEP_SAY,
    SCENE_STEP_HIDE,
    SCENE_STEP_SHOW,
    SCENE_STEP_JUMP,    // jump in place in the direction the actor faces
    SCENE_STEP_ADVANCE,
    SCENE_STEP_DOOR_OPEN,   // the anchor door
    SCENE_STEP_DOOR_CLOSE,
    SCENE_STEP_WARP,        // through the anchor door's warp_def destination
    SCENE_STEP_PLACE,       // put an actor on a tile, spawning it first if it is not on the map
    SCENE_STEP_SET_FLAG,
    SCENE_STEP_CLEAR_FLAG,
    SCENE_STEP_HOP,         // a one-tile jump in a direction with the ledge sound, then a short pause
    // In the vocabulary, not implemented: running one asserts.
    SCENE_STEP_ASK_YES_NO,
    SCENE_STEP_ASK_CHOICE,
    SCENE_STEP_GIVE,
    SCENE_STEP_BATTLE,
    SCENE_STEP_MUSIC,
    SCENE_STEP_CUSTOM,
    SCENE_STEP_KIND_COUNT,
};

struct SceneTarget
{
    u8 actor; // enum SceneActorSlot
    u8 side;  // enum SceneSide
};

struct SceneWalk
{
    u8 actor;
    struct SceneTarget goal;
    bool8 slide; // walk without changing facing
};

struct SceneStep
{
    u8 kind;                          // enum SceneStepKind
    u8 actor;                         // face, hide, show
    struct SceneTarget target;        // face: look toward it; walk: the goal
    u8 numWalks;                      // parallel_walk
    const struct SceneWalk *walks;
    const u8 *text;                   // say
    u8 stage;                         // advance: the value written to the scene's stage var
    bool8 slide;                      // walk: as SceneWalk.slide
    bool8 silent;                     // warp: no door sound or fade-in flourish (warpsilent)
    u16 flag;                         // set_flag, clear_flag
    u8 direction;                     // hop: enum Direction
};

enum SceneTriggerKind
{
    SCENE_TRIGGER_PROXIMITY,        // within radius tiles of the door (Manhattan), the door tile included
    SCENE_TRIGGER_ON_DOOR,          // standing on the door tile
    SCENE_TRIGGER_IN_FRONT_OF_DOOR, // standing on the tile in front of the door
};

struct SceneTrigger
{
    u8 mapGroup;
    u8 mapNum;
    u8 kind;     // enum SceneTriggerKind
    u8 radius;   // proximity only
    u8 doorWarp; // index in the map's warps of the door that is the anchor
    u8 sceneId;
    u16 stageVar;
    u16 stageValue; // the trigger holds while VarGet(stageVar) equals this
};

struct Scene
{
    const char *name; // for failure messages
    u8 actors[SCENE_MAX_ACTORS];      // local ids by slot; slot 0 is ignored (the player)
    u8 numActors;                     // slots in use, counting the player
    u16 stageVar;                     // advance writes here
    u8 numSteps;
    const struct SceneStep *steps;
};

#define SCENE_ACTOR_TARGET(actorSlot, sideValue) {actorSlot, sideValue}
#define SCENE_LOCK_PLAYER()                 {.kind = SCENE_STEP_LOCK_PLAYER}
#define SCENE_UNLOCK_PLAYER()               {.kind = SCENE_STEP_UNLOCK_PLAYER}
#define SCENE_FACE(who, toward, side)       {.kind = SCENE_STEP_FACE, .actor = who, .target = {toward, side}}
#define SCENE_WALK(who, toward, side)       {.kind = SCENE_STEP_WALK, .actor = who, .target = {toward, side}}
#define SCENE_SLIDE(who, toward, side) {.kind = SCENE_STEP_WALK, .actor = who, .target = {toward, side}, .slide = TRUE}
#define SCENE_JUMP(who)                     {.kind = SCENE_STEP_JUMP, .actor = who}
#define SCENE_PARALLEL_WALK(array)          {.kind = SCENE_STEP_PARALLEL_WALK, .numWalks = ARRAY_COUNT(array), .walks = array}
#define SCENE_SAY(string)                   {.kind = SCENE_STEP_SAY, .text = string}
#define SCENE_HIDE(who)                     {.kind = SCENE_STEP_HIDE, .actor = who}
#define SCENE_SHOW(who)                     {.kind = SCENE_STEP_SHOW, .actor = who}
#define SCENE_ADVANCE(toStage)              {.kind = SCENE_STEP_ADVANCE, .stage = toStage}
#define SCENE_DOOR_OPEN()                   {.kind = SCENE_STEP_DOOR_OPEN}
#define SCENE_DOOR_CLOSE()                  {.kind = SCENE_STEP_DOOR_CLOSE}
#define SCENE_WARP(isSilent)                {.kind = SCENE_STEP_WARP, .silent = isSilent}
#define SCENE_PLACE(who, toward, side)      {.kind = SCENE_STEP_PLACE, .actor = who, .target = {toward, side}}
#define SCENE_SET_FLAG(flagId)              {.kind = SCENE_STEP_SET_FLAG, .flag = flagId}
#define SCENE_CLEAR_FLAG(flagId)            {.kind = SCENE_STEP_CLEAR_FLAG, .flag = flagId}
#define SCENE_HOP(who, dir)                 {.kind = SCENE_STEP_HOP, .actor = who, .direction = dir}

// The tile `side` of an actor standing at (x, y) facing `facing`; DIR_NONE counts as DIR_SOUTH.
void SceneResolveSide(s16 x, s16 y, enum Direction facing, u32 side, s16 *outX, s16 *outY);

// The cardinal direction from one tile toward another along the longer axis (vertical on a tie); DIR_NONE if equal.
enum Direction SceneDirectionToward(s16 fromX, s16 fromY, s16 toX, s16 toY);

// May the walker step from (x, y) one tile in `dir`?
typedef bool32 (*SceneStepCheck)(void *context, s16 x, s16 y, enum Direction dir);

// Shortest 4-connected path inside the window around the start, written to dirs as `enum Direction` values.
// Returns its length, or SCENE_PATH_NONE if the goal is outside the window, unreachable or farther than
// SCENE_MAX_PATH steps. A goal equal to the start gives length 0.
u32 SceneFindPath(s16 startX, s16 startY, s16 goalX, s16 goalY, SceneStepCheck canStep, void *context, u8 *dirs);

// Writes the walk-and-end movement script for `dirs` at `out` (SCENE_MAX_PATH + 3 bytes); returns the end.
// slide brackets the walk with lock/unlock facing direction, so the actor keeps its facing.
u8 *SceneBuildWalkScript(u8 *out, const u8 *dirs, u32 length, bool32 slide);

const struct Scene *GetScene(u32 sceneId);

// Does a player at (playerX, playerY) set off a trigger of this kind for a door at (doorX, doorY)? Radius is in
// tiles and only used by SCENE_TRIGGER_PROXIMITY.
bool32 SceneTriggerFires(u32 kind, u32 radius, s16 playerX, s16 playerY, s16 doorX, s16 doorY);

// The first trigger of `table` that holds on the map for the player at (playerX, playerY), or NULL.
const struct SceneTrigger *SceneFindTrigger(const struct SceneTrigger *table, u32 count, u32 mapGroup, u32 mapNum, s16 playerX, s16 playerY);

// SceneFindTrigger over the real trigger table (sSceneTriggers in data/scenes.h).
const struct SceneTrigger *SceneFindTriggerAt(u32 mapGroup, u32 mapNum, s16 playerX, s16 playerY);

// Called after each map load; the next SceneTryStartTrigger then checks even without a step.
void SceneNotifyMapLoaded(void);

// Called by the field input handler; starts a scene and returns TRUE if a trigger holds. Does nothing unless the
// player took a step or a map was just loaded.
bool8 SceneTryStartTrigger(bool32 tookStep);

// Scripts: VAR_0x8004 = scene id.
void SceneBegin(void);
// VAR_RESULT = a SCENE_RESULT_* value (constants/scenes.h).
void SceneNextStep(void);

#endif // GUARD_SCENE_H
