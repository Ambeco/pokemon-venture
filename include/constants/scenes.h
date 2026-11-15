#ifndef GUARD_CONSTANTS_SCENES_H
#define GUARD_CONSTANTS_SCENES_H

// Scene ids for `setvar VAR_0x8004, SCENE_...` before `call Scene_EventScript_Run` (see scene.h).
#define SCENE_LITTLEROOT_NEED_POKEMON 0
#define SCENE_LITTLEROOT_ARRIVE_MALE   1
#define SCENE_LITTLEROOT_ARRIVE_FEMALE 2
#define SCENE_COUNT                   3

// What the driver script does after a step (SceneNextStep leaves one in VAR_RESULT).
#define SCENE_RESULT_NEXT          0 // nothing to do: ask for the next step
#define SCENE_RESULT_WAIT_MOVEMENT 1
#define SCENE_RESULT_SAY           2 // the text is in gStringVar4
#define SCENE_RESULT_LOCK          3
#define SCENE_RESULT_UNLOCK        4
#define SCENE_RESULT_END           5
#define SCENE_RESULT_WAIT_DOOR     6
#define SCENE_RESULT_WARP          7 // the map is changing: wait for it

#endif // GUARD_CONSTANTS_SCENES_H
