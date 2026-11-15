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
    // In the vocabulary, not implemented: running one asserts.
    SCENE_STEP_DOOR_OPEN,
    SCENE_STEP_DOOR_CLOSE,
    SCENE_STEP_WARP,
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

// Scripts: VAR_0x8004 = scene id.
void SceneBegin(void);
// VAR_RESULT = a SCENE_RESULT_* value (constants/scenes.h).
void SceneNextStep(void);

#endif // GUARD_SCENE_H
