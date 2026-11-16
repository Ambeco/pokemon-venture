@ Scripts for scene `custom` steps (SCENE_CUSTOM, include/field/scene.h). Labels start with SceneCustom_.
@ They run as subroutines (end with `return`) and must not move or place objects or name a local id, so scenes
@ stay layout-independent: tools/misc/check_scene_custom.py enforces it as part of `make check`.

SceneCustom_SetBigGuyGirlString::
	special GetPlayerBigGuyGirlString
	return
