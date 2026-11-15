@ Scene runner driver (include/field/scene.h): VAR_0x8004 = scene id, then `call Scene_EventScript_Run`.
@ Each step is decoded by SceneNextStep, which leaves a SCENE_RESULT_* in VAR_RESULT.
Scene_EventScript_Run::
	special SceneBegin
Scene_EventScript_NextStep:
	special SceneNextStep
	switch VAR_RESULT
	case SCENE_RESULT_NEXT, Scene_EventScript_NextStep
	case SCENE_RESULT_WAIT_MOVEMENT, Scene_EventScript_WaitMovement
	case SCENE_RESULT_SAY, Scene_EventScript_Say
	case SCENE_RESULT_LOCK, Scene_EventScript_Lock
	case SCENE_RESULT_UNLOCK, Scene_EventScript_Unlock
	case SCENE_RESULT_END, Scene_EventScript_End
	case SCENE_RESULT_WAIT_DOOR, Scene_EventScript_WaitDoor
	case SCENE_RESULT_WARP, Scene_EventScript_Warp
	end

@ Entry for SceneTryStartTrigger, which sets VAR_0x8004 and starts this as a top-level script.
Scene_EventScript_RunTriggered::
	call Scene_EventScript_Run
	end

Scene_EventScript_WaitDoor:
	waitdooranim
	goto Scene_EventScript_NextStep

Scene_EventScript_Warp:
	waitstate
	goto Scene_EventScript_NextStep

Scene_EventScript_WaitMovement:
	waitmovementall
	goto Scene_EventScript_NextStep

Scene_EventScript_Say:
	msgbox gStringVar4, MSGBOX_DEFAULT
	closemessage
	goto Scene_EventScript_NextStep

Scene_EventScript_Lock:
	lockall
	goto Scene_EventScript_NextStep

Scene_EventScript_Unlock:
	releaseall
	goto Scene_EventScript_NextStep

Scene_EventScript_End:
	return
