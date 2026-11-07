@ Generic route blocker (design.md, "Blocker McGuffins"). The caller sets VAR_0x8004 to the main-story-index s the
@ blocker holds until, then gotos the entry for the direction the player should be pushed (the way they came from).
@ Once the index is past s the NPC says nothing and the player walks on. Texts come from the pool in
@ gRouteBlockerTexts later; for now one generic line.

Common_EventScript_RouteBlockerPushUp::
	special IsMainStoryIndexPast
	goto_if_eq VAR_RESULT, TRUE, Common_EventScript_NopReturn
	lockall
	msgbox Common_Text_RouteBlocked, MSGBOX_DEFAULT
	closemessage
	applymovement LOCALID_PLAYER, Common_Movement_WalkUp
	waitmovement 0
	releaseall
	end

Common_EventScript_RouteBlockerPushDown::
	special IsMainStoryIndexPast
	goto_if_eq VAR_RESULT, TRUE, Common_EventScript_NopReturn
	lockall
	msgbox Common_Text_RouteBlocked, MSGBOX_DEFAULT
	closemessage
	applymovement LOCALID_PLAYER, Common_Movement_WalkDown
	waitmovement 0
	releaseall
	end

Common_EventScript_RouteBlockerPushLeft::
	special IsMainStoryIndexPast
	goto_if_eq VAR_RESULT, TRUE, Common_EventScript_NopReturn
	lockall
	msgbox Common_Text_RouteBlocked, MSGBOX_DEFAULT
	closemessage
	applymovement LOCALID_PLAYER, Common_Movement_WalkLeft
	waitmovement 0
	releaseall
	end

Common_EventScript_RouteBlockerPushRight::
	special IsMainStoryIndexPast
	goto_if_eq VAR_RESULT, TRUE, Common_EventScript_NopReturn
	lockall
	msgbox Common_Text_RouteBlocked, MSGBOX_DEFAULT
	closemessage
	applymovement LOCALID_PLAYER, Common_Movement_WalkRight
	waitmovement 0
	releaseall
	end

Common_Text_RouteBlocked:
	.string "Sorry, you can't go through here yet.\n"
	.string "Come back when you've sorted out\l"
	.string "things in the area.$"
