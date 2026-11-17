@ Scripts for scene `custom` steps (SCENE_CUSTOM, include/field/scene.h). Labels start with SceneCustom_.
@ They run as subroutines (end with `return`) and must not move or place objects or name a local id, so scenes
@ stay layout-independent: tools/misc/check_scene_custom.py enforces it as part of `make check`.

SceneCustom_SetBigGuyGirlString::
	special GetPlayerBigGuyGirlString
	return

SceneCustom_FadeOut::
	fadescreen FADE_TO_BLACK
	return

@ The starter is given by the CB2 that follows the pick; the script resumes after the first battle.
SceneCustom_ChooseStarter::
	special ChooseStarter
	return

SceneCustom_HideRivalBedroom::
	checkplayergender
	call_if_eq VAR_RESULT, MALE, SceneCustom_HideMayInBedroom
	call_if_eq VAR_RESULT, FEMALE, SceneCustom_HideBrendanInBedroom
	return

SceneCustom_HideMayInBedroom:
	setflag FLAG_HIDE_LITTLEROOT_TOWN_MAYS_HOUSE_RIVAL_BEDROOM
	return

SceneCustom_HideBrendanInBedroom:
	setflag FLAG_HIDE_LITTLEROOT_TOWN_BRENDANS_HOUSE_RIVAL_BEDROOM
	return

SceneCustom_HealAndGoToLab::
	special HealPlayerParty
	warp MAP_LITTLEROOT_TOWN_PROFESSOR_BIRCHS_LAB, 6, 5
	waitstate
	return

SceneCustom_AnnounceStarter::
	bufferleadmonspeciesname STR_VAR_1
	message LittlerootTown_ProfessorBirchsLab_Text_LikeYouToHavePokemon
	waitmessage
	playfanfare MUS_OBTAIN_ITEM
	waitfanfare
	return

SceneCustom_NicknameStarter::
	setvar VAR_0x8004, 0
	fadescreen FADE_TO_BLACK
	special ChangePokemonNickname
	return
