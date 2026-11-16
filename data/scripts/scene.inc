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
	case SCENE_RESULT_DELAY, Scene_EventScript_Delay
	case SCENE_RESULT_ASK_YES_NO, Scene_EventScript_AskYesNo
	case SCENE_RESULT_ASK_CHOICE, Scene_EventScript_AskChoice
	case SCENE_RESULT_GIVE, Scene_EventScript_Give
	case SCENE_RESULT_BATTLE, Scene_EventScript_Battle
	case SCENE_RESULT_CUSTOM, Scene_EventScript_Custom
	end

@ Entry for SceneTryStartTrigger, which sets VAR_0x8004 and starts this as a top-level script.
Scene_EventScript_RunTriggered::
	call Scene_EventScript_Run
	end

@ Entry for a scene set up with SceneBeginScene (tests).
Scene_EventScript_RunPrepared::
	call Scene_EventScript_NextStep
	end

@ The answer lands in VAR_RESULT, which the next SceneNextStep would overwrite: SceneTakeAnswer reads it first.
Scene_EventScript_AskYesNo:
	msgbox gStringVar4, MSGBOX_YESNO
	special SceneTakeAnswer
	closemessage
	goto Scene_EventScript_NextStep

Scene_EventScript_AskChoice:
	message gStringVar4
	waitmessage
	dynmultichoice 1, 1, TRUE, 0xFF, 0, DYN_MULTICHOICE_CB_NONE, NULL
	special SceneTakeAnswer
	closemessage
	goto Scene_EventScript_NextStep

@ Std_ObtainItem leaves its success in VAR_0x8007, which SceneTakeAnswer reads. It says nothing when the bag is full.
@ `switch VAR_RESULT` above copies the result into VAR_0x8000, so the item is loaded again here.
Scene_EventScript_Give:
	special SceneLoadGive
	callstd STD_OBTAIN_ITEM
	goto_if_eq VAR_0x8007, FALSE, Scene_EventScript_GiveBagFull
	special SceneTakeAnswer
	closemessage
	goto Scene_EventScript_NextStep

Scene_EventScript_GiveBagFull:
	msgbox gText_TooBadBagIsFull, MSGBOX_DEFAULT
	special SceneTakeAnswer
	closemessage
	goto Scene_EventScript_NextStep

Scene_EventScript_Battle:
	special SceneStartBattle
	waitstate
	goto Scene_EventScript_NextStep

Scene_EventScript_Custom:
	callnative SceneCallCustom
	goto Scene_EventScript_NextStep

Scene_EventScript_WaitDoor:
	waitdooranim
	goto Scene_EventScript_NextStep

Scene_EventScript_Delay:
	delay 8
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
