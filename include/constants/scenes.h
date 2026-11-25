#ifndef GUARD_CONSTANTS_SCENES_H
#define GUARD_CONSTANTS_SCENES_H

// Scene ids for `setvar VAR_0x8004, SCENE_...` before `call Scene_EventScript_Run` (see scene.h).
#define SCENE_LITTLEROOT_NEED_POKEMON 0
#define SCENE_LITTLEROOT_ARRIVE_MALE   1
#define SCENE_LITTLEROOT_ARRIVE_FEMALE 2
#define SCENE_ROUTE101_RESCUE         3
#define SCENE_MOSSDEEP_SUPER_ROD      4
#define SCENE_LITTLEROOT_TWIN_ASKS    5
#define SCENE_ROUTE103_RIVAL_MAY     6
#define SCENE_ROUTE103_RIVAL_BRENDAN 7
#define SCENE_ROUTE101_BAG           8
#define SCENE_LAB_STARTER            9
#define SCENE_DEWFORD_OLD_ROD       10
#define SCENE_ROUTE118_GOOD_ROD     11
#define SCENE_ROUTE110_RIVAL_MAY         12
#define SCENE_ROUTE110_RIVAL_BRENDAN     13
#define SCENE_ROUTE110_ITEMFINDER_MAY    14
#define SCENE_ROUTE110_ITEMFINDER_BRENDAN 15
#define SCENE_ROUTE119_RIVAL_MAY         16
#define SCENE_ROUTE119_RIVAL_BRENDAN     17
#define SCENE_ROUTE119_FLY_MAY           18
#define SCENE_ROUTE119_FLY_BRENDAN       19
#define SCENE_LILYCOVE_RIVAL_MAY            20
#define SCENE_LILYCOVE_RIVAL_MAY_AGAIN      21
#define SCENE_LILYCOVE_RIVAL_BRENDAN        22
#define SCENE_LILYCOVE_RIVAL_BRENDAN_AGAIN  23
#define SCENE_COUNT                  24

// What the driver script does after a step (SceneNextStep leaves one in VAR_RESULT).
#define SCENE_RESULT_NEXT          0 // nothing to do: ask for the next step
#define SCENE_RESULT_WAIT_MOVEMENT 1
#define SCENE_RESULT_SAY           2 // the text is in gStringVar4
#define SCENE_RESULT_LOCK          3
#define SCENE_RESULT_UNLOCK        4
#define SCENE_RESULT_END           5
#define SCENE_RESULT_WAIT_DOOR     6
#define SCENE_RESULT_WARP          7 // the map is changing: wait for it
#define SCENE_RESULT_DELAY         8 // wait 8 frames, then ask for the next step
#define SCENE_RESULT_ASK_YES_NO    9 // the question is in gStringVar4; the answer goes to SceneTakeAnswer
#define SCENE_RESULT_ASK_CHOICE    10 // the prompt is in gStringVar4 and the labels are on the dynamic multichoice stack
#define SCENE_RESULT_GIVE          11 // VAR_0x8000 item, VAR_0x8001 quantity
#define SCENE_RESULT_BATTLE        12
#define SCENE_RESULT_CUSTOM        13

#endif // GUARD_CONSTANTS_SCENES_H
