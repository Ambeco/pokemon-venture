// Scene data (design_scenes.md, "Scene runner"); included by scene.c.

extern const u8 LittlerootTown_Text_IfYouGoInGrassPokemonWillJumpOut[];
extern const u8 LittlerootTown_Text_DangerousIfYouDontHavePokemon[];

// The little girl (the twin) blocks the way out of Littleroot while the player has no Pokémon: she walks up
// to the player, jumps, warns them, pushes them back a step and returns to her post. It never advances the stage.
static const struct SceneWalk sLittlerootPushBack[] =
{
    {SCENE_ACTOR_1, SCENE_ACTOR_TARGET(SCENE_ACTOR_1, SCENE_SIDE_FRONT)},
    {SCENE_ACTOR_PLAYER, SCENE_ACTOR_TARGET(SCENE_ACTOR_PLAYER, SCENE_SIDE_BEHIND), TRUE},
};

static const struct SceneStep sLittlerootNeedPokemonSteps[] =
{
    SCENE_LOCK_PLAYER(),
    SCENE_FACE(SCENE_ACTOR_1, SCENE_ACTOR_PLAYER, SCENE_SIDE_SELF),
    SCENE_JUMP(SCENE_ACTOR_1),
    SCENE_WALK(SCENE_ACTOR_1, SCENE_ACTOR_PLAYER, SCENE_SIDE_FRONT),
    SCENE_FACE(SCENE_ACTOR_1, SCENE_ACTOR_PLAYER, SCENE_SIDE_SELF),
    SCENE_SAY(LittlerootTown_Text_IfYouGoInGrassPokemonWillJumpOut),
    SCENE_PARALLEL_WALK(sLittlerootPushBack),
    SCENE_SAY(LittlerootTown_Text_DangerousIfYouDontHavePokemon),
    SCENE_WALK(SCENE_ACTOR_1, SCENE_ACTOR_1, SCENE_SIDE_HOME),
    SCENE_UNLOCK_PLAYER(),
};

static const struct Scene sLittlerootNeedPokemon =
{
    .name = "LittlerootNeedPokemon",
    .actors = {LOCALID_PLAYER, LOCALID_LITTLEROOT_TWIN},
    .numActors = 2,
    .stageVar = SCENE_SAVE_BIRCH_BEAT_VAR,
    .numSteps = ARRAY_COUNT(sLittlerootNeedPokemonSteps),
    .steps = sLittlerootNeedPokemonSteps,
};

extern const u8 LittlerootTown_Text_CanYouGoSeeWhatsHappening[];
extern const u8 SceneCustom_SetBigGuyGirlString[];

// Once the player has a Pokémon the twin, standing in the gap, stops them and sends them to Route 101.
static const struct SceneStep sLittlerootTwinAsksSteps[] =
{
    SCENE_LOCK_PLAYER(),
    SCENE_FACE(SCENE_ACTOR_1, SCENE_ACTOR_PLAYER, SCENE_SIDE_SELF),
    SCENE_FACE(SCENE_ACTOR_PLAYER, SCENE_ACTOR_1, SCENE_SIDE_SELF),
    SCENE_CUSTOM(SceneCustom_SetBigGuyGirlString),
    SCENE_SAY(LittlerootTown_Text_CanYouGoSeeWhatsHappening),
    SCENE_FACE_ORIGINAL(SCENE_ACTOR_1),
    SCENE_ADVANCE(SAVE_BIRCH_BIRCH_IN_GRASS),
    SCENE_UNLOCK_PLAYER(),
};

static const struct Scene sLittlerootTwinAsks =
{
    .name = "LittlerootTwinAsks",
    .actors = {LOCALID_PLAYER, LOCALID_LITTLEROOT_TWIN},
    .numActors = 2,
    .stageVar = SCENE_SAVE_BIRCH_BEAT_VAR,
    .numSteps = ARRAY_COUNT(sLittlerootTwinAsksSteps),
    .steps = sLittlerootTwinAsksSteps,
};

extern const u8 LittlerootTown_Text_OurNewHomeLetsGoInside[];

// The mom heads for the tile in front of the door while the player takes the tile she leaves: she is one step ahead.
static const struct SceneWalk sLittlerootApproachDoor[] =
{
    {SCENE_ACTOR_1, SCENE_ACTOR_TARGET(SCENE_ACTOR_ANCHOR, SCENE_SIDE_FRONT)},
    {SCENE_ACTOR_PLAYER, SCENE_ACTOR_TARGET(SCENE_ACTOR_PLAYER, SCENE_SIDE_FRONT)},
};

// Stage 0: the player steps off the moving truck, the mom comes out of the house door (the anchor), says her line,
// and they go in together. Male and female differ only in the truck's hide flag; the door comes from the trigger.
// Unlike the original script the mom and player take turns walking into the door (she is only one step ahead
// there), and there are no fixed delays between the steps.
#define LITTLEROOT_ARRIVAL_STEPS(truckHideFlag) \
{ \
    SCENE_LOCK_PLAYER(), \
    SCENE_HOP(SCENE_ACTOR_PLAYER, DIR_EAST), \
    SCENE_DOOR_OPEN(), \
    SCENE_PLACE(SCENE_ACTOR_1, SCENE_ACTOR_ANCHOR, SCENE_SIDE_SELF), \
    SCENE_WALK(SCENE_ACTOR_1, SCENE_ACTOR_ANCHOR, SCENE_SIDE_FRONT), \
    SCENE_DOOR_CLOSE(), \
    SCENE_WALK(SCENE_ACTOR_1, SCENE_ACTOR_PLAYER, SCENE_SIDE_FRONT), \
    SCENE_FACE(SCENE_ACTOR_1, SCENE_ACTOR_PLAYER, SCENE_SIDE_SELF), \
    SCENE_SAY(LittlerootTown_Text_OurNewHomeLetsGoInside), \
    SCENE_PARALLEL_WALK(sLittlerootApproachDoor), \
    SCENE_FACE(SCENE_ACTOR_PLAYER, SCENE_ACTOR_ANCHOR, SCENE_SIDE_SELF), \
    SCENE_DOOR_OPEN(), \
    SCENE_WALK(SCENE_ACTOR_1, SCENE_ACTOR_ANCHOR, SCENE_SIDE_SELF), \
    SCENE_HIDE(SCENE_ACTOR_1), \
    SCENE_WALK(SCENE_ACTOR_PLAYER, SCENE_ACTOR_ANCHOR, SCENE_SIDE_SELF), \
    SCENE_SET_FLAG(FLAG_HIDE_LITTLEROOT_TOWN_MOM_OUTSIDE), \
    SCENE_ADVANCE(3), \
    SCENE_HIDE(SCENE_ACTOR_PLAYER), \
    SCENE_DOOR_CLOSE(), \
    SCENE_CLEAR_FLAG(FLAG_HIDE_LITTLEROOT_TOWN_FAT_MAN), \
    SCENE_CLEAR_FLAG(FLAG_HIDE_MAP_NAME_POPUP), \
    SCENE_SET_FLAG(truckHideFlag), \
    SCENE_WARP(TRUE), \
}

static const struct SceneStep sLittlerootArriveMaleSteps[] = LITTLEROOT_ARRIVAL_STEPS(FLAG_HIDE_LITTLEROOT_TOWN_BRENDANS_HOUSE_TRUCK);
static const struct SceneStep sLittlerootArriveFemaleSteps[] = LITTLEROOT_ARRIVAL_STEPS(FLAG_HIDE_LITTLEROOT_TOWN_MAYS_HOUSE_TRUCK);

static const struct Scene sLittlerootArriveMale =
{
    .name = "LittlerootArriveMale",
    .actors = {LOCALID_PLAYER, LOCALID_LITTLEROOT_MOM},
    .numActors = 2,
    .stageVar = SCENE_MOVING_IN_BEAT_VAR,
    .numSteps = ARRAY_COUNT(sLittlerootArriveMaleSteps),
    .steps = sLittlerootArriveMaleSteps,
};

static const struct Scene sLittlerootArriveFemale =
{
    .name = "LittlerootArriveFemale",
    .actors = {LOCALID_PLAYER, LOCALID_LITTLEROOT_MOM},
    .numActors = 2,
    .stageVar = SCENE_MOVING_IN_BEAT_VAR,
    .numSteps = ARRAY_COUNT(sLittlerootArriveFemaleSteps),
    .steps = sLittlerootArriveFemaleSteps,
};

extern const u8 Route101_Text_HelpMe[];
extern const u8 Route101_Text_PleaseHelp[];

// Stage SAVE_BIRCH_BIRCH_IN_GRASS of Save Birch, fired when the route loads: Birch and the Zigzagoon appear at their
// template tiles in the grass and circle there while the player watches, then face off and Birch asks for help.
// The no-wild flag keeps spawns and encounters off until Birch's bag script clears it.
static const struct SceneStep sRoute101RescueSteps[] =
{
    SCENE_LOCK_PLAYER(),
    SCENE_SET_FLAG(FLAG_HIDE_MAP_NAME_POPUP),
    SCENE_SET_FLAG(FLAG_SCENE_NO_WILD),
    SCENE_MUSIC(MUS_HELP),
    SCENE_PLACE(SCENE_ACTOR_1, SCENE_ACTOR_1, SCENE_SIDE_HOME),
    SCENE_PLACE(SCENE_ACTOR_2, SCENE_ACTOR_2, SCENE_SIDE_HOME),
    SCENE_SAY(Route101_Text_HelpMe),
    SCENE_CHASE_LOOP(SCENE_ACTOR_1, SCENE_ACTOR_2, SCENE_ACTOR_PLAYER, SCENE_SIDE_SELF, 2, 3, 2),
    SCENE_PAUSE(16),
    SCENE_FACE(SCENE_ACTOR_2, SCENE_ACTOR_1, SCENE_SIDE_SELF),
    SCENE_FACE(SCENE_ACTOR_1, SCENE_ACTOR_2, SCENE_SIDE_SELF),
    SCENE_SAY(Route101_Text_PleaseHelp),
    SCENE_ADVANCE(SAVE_BIRCH_AWAIT_BAG),
    SCENE_UNLOCK_PLAYER(),
};

static const struct Scene sRoute101Rescue =
{
    .name = "Route101Rescue",
    .actors = {LOCALID_PLAYER, LOCALID_ROUTE101_BIRCH, LOCALID_ROUTE101_ZIGZAGOON},
    .numActors = 3,
    .stageVar = SCENE_SAVE_BIRCH_BEAT_VAR,
    .numSteps = ARRAY_COUNT(sRoute101RescueSteps),
    .steps = sRoute101RescueSteps,
};

extern const u8 SceneCustom_FadeOut[];
extern const u8 SceneCustom_ChooseStarter[];
extern const u8 SceneCustom_HideRivalBedroom[];
extern const u8 SceneCustom_HealAndGoToLab[];
extern const u8 Route101_Text_YouSavedMe[];

// Stage SAVE_BIRCH_AWAIT_BAG of Save Birch, run from the bag's talk script: the screen fades, the Zigzagoon goes,
// the player picks the starter (the first battle follows and the scene resumes after it), Birch thanks them and
// sends them to his lab. The no-wild flag is lifted here, where the rescue's music and flags end.
static const struct SceneStep sRoute101BagSteps[] =
{
    SCENE_SET_FLAG(FLAG_SYS_POKEMON_GET),
    SCENE_SET_FLAG(FLAG_RESCUED_BIRCH),
    SCENE_CUSTOM(SceneCustom_FadeOut),
    SCENE_HIDE(SCENE_ACTOR_2),
    SCENE_FACE(SCENE_ACTOR_PLAYER, SCENE_ACTOR_1, SCENE_SIDE_SELF),
    SCENE_CUSTOM(SceneCustom_ChooseStarter),
    SCENE_WALK(SCENE_ACTOR_1, SCENE_ACTOR_PLAYER, SCENE_SIDE_FRONT),
    SCENE_SAY(Route101_Text_YouSavedMe),
    SCENE_SET_FLAG(FLAG_HIDE_ROUTE_101_BIRCH_ZIGZAGOON_BATTLE),
    SCENE_CLEAR_FLAG(FLAG_HIDE_LITTLEROOT_TOWN_BIRCHS_LAB_BIRCH),
    SCENE_SET_FLAG(FLAG_HIDE_ROUTE_101_BIRCH_STARTERS_BAG),
    SCENE_ADVANCE(SAVE_BIRCH_STARTER_TAKEN),
    SCENE_CLEAR_FLAG(FLAG_SCENE_NO_WILD),
    SCENE_CLEAR_FLAG(FLAG_HIDE_MAP_NAME_POPUP),
    SCENE_CUSTOM(SceneCustom_HideRivalBedroom),
    SCENE_CUSTOM(SceneCustom_HealAndGoToLab),
};

static const struct Scene sRoute101Bag =
{
    .name = "Route101Bag",
    .actors = {LOCALID_PLAYER, LOCALID_ROUTE101_BIRCH, LOCALID_ROUTE101_ZIGZAGOON},
    .numActors = 3,
    .stageVar = SCENE_SAVE_BIRCH_BEAT_VAR,
    .numSteps = ARRAY_COUNT(sRoute101BagSteps),
    .steps = sRoute101BagSteps,
};

extern const u8 SceneCustom_AnnounceStarter[];
extern const u8 SceneCustom_NicknameStarter[];
extern const u8 LittlerootTown_ProfessorBirchsLab_Text_WhyNotGiveNicknameToMon[];
extern const u8 LittlerootTown_ProfessorBirchsLab_Text_MightBeGoodIdeaToGoSeeRival[];
extern const u8 LittlerootTown_ProfessorBirchsLab_Text_DontBeThatWay[];
extern const u8 LittlerootTown_ProfessorBirchsLab_Text_GetRivalToTeachYou[];

static const struct SceneStep sLabNicknameSteps[] = {SCENE_CUSTOM(SceneCustom_NicknameStarter)};

static const struct SceneBranch sLabNicknameAnswers[] =
{
    SCENE_BRANCH(NULL, sLabNicknameSteps),
    SCENE_EMPTY_BRANCH(NULL),
};

// Declining twice in a row is the same as accepting; vanilla asks again until the answer is yes.
static const struct SceneBranch sLabDontBeThatWayAnswers[] =
{
    SCENE_EMPTY_BRANCH(NULL),
    SCENE_EMPTY_BRANCH(NULL),
};

static const struct SceneStep sLabDeclineRivalSteps[] =
{
    SCENE_ASK_YES_NO(LittlerootTown_ProfessorBirchsLab_Text_DontBeThatWay, sLabDontBeThatWayAnswers),
};

static const struct SceneBranch sLabRivalAnswers[] =
{
    SCENE_EMPTY_BRANCH(NULL),
    SCENE_BRANCH(NULL, sLabDeclineRivalSteps),
};

// Stage SAVE_BIRCH_STARTER_TAKEN of Save Birch, run from the lab's on-frame script: Birch hands over the starter
// the pick already gave, the player may nickname it, and Birch sends them to the rival.
static const struct SceneStep sLabStarterSteps[] =
{
    SCENE_CUSTOM(SceneCustom_AnnounceStarter),
    SCENE_ASK_YES_NO(LittlerootTown_ProfessorBirchsLab_Text_WhyNotGiveNicknameToMon, sLabNicknameAnswers),
    SCENE_ASK_YES_NO(LittlerootTown_ProfessorBirchsLab_Text_MightBeGoodIdeaToGoSeeRival, sLabRivalAnswers),
    SCENE_SAY(LittlerootTown_ProfessorBirchsLab_Text_GetRivalToTeachYou),
    SCENE_CLEAR_FLAG(FLAG_HIDE_ROUTE_101_BOY),
    SCENE_ADVANCE(SAVE_BIRCH_SEE_RIVAL),
};

static const struct Scene sLabStarter =
{
    .name = "LabStarter",
    .actors = {LOCALID_PLAYER},
    .numActors = 1,
    .stageVar = SCENE_SAVE_BIRCH_BEAT_VAR,
    .numSteps = ARRAY_COUNT(sLabStarterSteps),
    .steps = sLabStarterSteps,
};

extern const u8 MossdeepCity_House3_Text_YouWantSuperRod[];
extern const u8 MossdeepCity_House3_Text_SuperRodIsSuper[];
extern const u8 MossdeepCity_House3_Text_TryDroppingRodInWater[];
extern const u8 MossdeepCity_House3_Text_DontYouLikeToFish[];

// The Super Rod fisherman (Mossdeep's chapter, slot 0 of county (5,1)), run from his talk script. It names no
// actor or tile, so any house can host it. The stage var is the chapter's stored stage; FLAG_RECEIVED_SUPER_ROD
// stays set for the main-plot adapter. A full bag ends the scene at the give, before the flag and the advance.
static const struct SceneStep sSuperRodAcceptSteps[] =
{
    SCENE_SAY(MossdeepCity_House3_Text_SuperRodIsSuper),
    SCENE_GIVE(ITEM_SUPER_ROD, 1),
    SCENE_SET_FLAG(FLAG_RECEIVED_SUPER_ROD),
    SCENE_ADVANCE(1),
    SCENE_SAY(MossdeepCity_House3_Text_TryDroppingRodInWater),
};

static const struct SceneStep sSuperRodDeclineSteps[] =
{
    SCENE_SAY(MossdeepCity_House3_Text_DontYouLikeToFish),
};

static const struct SceneBranch sSuperRodAnswers[] =
{
    SCENE_BRANCH(NULL, sSuperRodAcceptSteps),
    SCENE_BRANCH(NULL, sSuperRodDeclineSteps),
};

static const struct SceneStep sMossdeepSuperRodSteps[] =
{
    SCENE_ASK_YES_NO(MossdeepCity_House3_Text_YouWantSuperRod, sSuperRodAnswers),
};

static const struct Scene sMossdeepSuperRod =
{
    .name = "MossdeepSuperRod",
    .actors = {LOCALID_PLAYER},
    .numActors = 1,
    .stageVar = SCENE_MOSSDEEP_SUPER_ROD_BEAT_VAR,
    .numSteps = ARRAY_COUNT(sMossdeepSuperRodSteps),
    .steps = sMossdeepSuperRodSteps,
};

extern const u8 Route103_Text_BattleNow[];
extern const u8 Route103_Text_NotYet[];
extern const u8 Route103_Text_MayLetsBattle[];
extern const u8 Route103_Text_MayDefeated[];
extern const u8 Route103_Text_MayNotYet[];
extern const u8 Route103_Text_BrendanLetsBattle[];
extern const u8 Route103_Text_BrendanDefeated[];
extern const u8 Route103_Text_BrendanNotYet[];

// The rival's challenge on Route 103, run from his talk script after he faces the player. The player may refuse
// ("Not yet") to heal first; the script then releases and the rival waits. The after-battle talk and his exit stay
// in the script, which tells a win from a refusal by the trainer flag. GetRivalTrainer picks the trainer for the
// starter; the two scenes differ only in the rival's texts.
#define ROUTE103_RIVAL_SCENE(sceneName, letsBattle, defeated, notYet) \
static const struct SceneStep sceneName##FightSteps[] = \
{ \
    SCENE_BATTLE_RIVAL(RIVAL_ENCOUNTER_ROUTE_103, defeated), \
}; \
static const struct SceneStep sceneName##NotYetSteps[] = \
{ \
    SCENE_SAY(notYet), \
}; \
static const struct SceneBranch sceneName##Answers[] = \
{ \
    SCENE_BRANCH(Route103_Text_BattleNow, sceneName##FightSteps), \
    SCENE_BRANCH(Route103_Text_NotYet, sceneName##NotYetSteps), \
}; \
static const struct SceneStep sceneName##Steps[] = \
{ \
    SCENE_ASK_CHOICE(letsBattle, sceneName##Answers), \
}; \
static const struct Scene sceneName = \
{ \
    .name = #sceneName, \
    .actors = {LOCALID_PLAYER}, \
    .numActors = 1, \
    .stageVar = SCENE_RIVAL_DEX_BEAT_VAR, \
    .numSteps = ARRAY_COUNT(sceneName##Steps), \
    .steps = sceneName##Steps, \
}


ROUTE103_RIVAL_SCENE(sRoute103RivalMay, Route103_Text_MayLetsBattle, Route103_Text_MayDefeated, Route103_Text_MayNotYet);
ROUTE103_RIVAL_SCENE(sRoute103RivalBrendan, Route103_Text_BrendanLetsBattle, Route103_Text_BrendanDefeated, Route103_Text_BrendanNotYet);

extern const u8 DewfordTown_Text_GettingItchToFish[];
extern const u8 DewfordTown_Text_GiveYouOneOfMyRods[];
extern const u8 DewfordTown_Text_ThrowInFishingAdvice[];
extern const u8 DewfordTown_Text_ThatsTooBadThen[];

// The Old Rod fisherman (Dewford's chapter, slot 0 of county (0,3)), run from his talk script. He stands outdoors, so
// the scene names no actor or tile. FLAG_RECEIVED_OLD_ROD stays set for the main-plot adapter. A full bag ends the
// scene at the give, before the flag and the advance.
static const struct SceneStep sOldRodAcceptSteps[] =
{
    SCENE_SAY(DewfordTown_Text_GiveYouOneOfMyRods),
    SCENE_GIVE(ITEM_OLD_ROD, 1),
    SCENE_SET_FLAG(FLAG_RECEIVED_OLD_ROD),
    SCENE_ADVANCE(1),
    SCENE_SAY(DewfordTown_Text_ThrowInFishingAdvice),
};

static const struct SceneStep sOldRodDeclineSteps[] =
{
    SCENE_SAY(DewfordTown_Text_ThatsTooBadThen),
};

static const struct SceneBranch sOldRodAnswers[] =
{
    SCENE_BRANCH(NULL, sOldRodAcceptSteps),
    SCENE_BRANCH(NULL, sOldRodDeclineSteps),
};

static const struct SceneStep sDewfordOldRodSteps[] =
{
    SCENE_ASK_YES_NO(DewfordTown_Text_GettingItchToFish, sOldRodAnswers),
};

static const struct Scene sDewfordOldRod =
{
    .name = "DewfordOldRod",
    .actors = {LOCALID_PLAYER},
    .numActors = 1,
    .stageVar = SCENE_DEWFORD_OLD_ROD_BEAT_VAR,
    .numSteps = ARRAY_COUNT(sDewfordOldRodSteps),
    .steps = sDewfordOldRodSteps,
};

extern const u8 Route118_Text_YouAgreeGoodRodIsGood[];
extern const u8 Route118_Text_IdenticalMindsTakeThis[];
extern const u8 Route118_Text_TryYourLuckFishing[];
extern const u8 Route118_Text_DontYouLikeToFish[];

// The Good Rod fisherman on Route 118 (the Weather Institute county's chapter, slot 0), run from his talk script.
// He stands outdoors, so the scene names no actor or tile. FLAG_RECEIVED_GOOD_ROD stays set for the main-plot adapter.
// A full bag ends the scene at the give, before the flag and the advance.
static const struct SceneStep sGoodRodAcceptSteps[] =
{
    SCENE_SAY(Route118_Text_IdenticalMindsTakeThis),
    SCENE_GIVE(ITEM_GOOD_ROD, 1),
    SCENE_SET_FLAG(FLAG_RECEIVED_GOOD_ROD),
    SCENE_ADVANCE(1),
    SCENE_SAY(Route118_Text_TryYourLuckFishing),
};

static const struct SceneStep sGoodRodDeclineSteps[] =
{
    SCENE_SAY(Route118_Text_DontYouLikeToFish),
};

static const struct SceneBranch sGoodRodAnswers[] =
{
    SCENE_BRANCH(NULL, sGoodRodAcceptSteps),
    SCENE_BRANCH(NULL, sGoodRodDeclineSteps),
};

static const struct SceneStep sRoute118GoodRodSteps[] =
{
    SCENE_ASK_YES_NO(Route118_Text_YouAgreeGoodRodIsGood, sGoodRodAnswers),
};

static const struct Scene sRoute118GoodRod =
{
    .name = "Route118GoodRod",
    .actors = {LOCALID_PLAYER},
    .numActors = 1,
    .stageVar = SCENE_ROUTE118_GOOD_ROD_BEAT_VAR,
    .numSteps = ARRAY_COUNT(sRoute118GoodRodSteps),
    .steps = sRoute118GoodRodSteps,
};

extern const u8 Route110_Text_MayLetsBattle[];
extern const u8 Route110_Text_MayDefeated[];
extern const u8 Route110_Text_MayTakeThis[];
extern const u8 Route110_Text_MayExplainItemfinder[];
extern const u8 Route110_Text_BrendanLetsBattle[];
extern const u8 Route110_Text_BrendanDefeated[];
extern const u8 Route110_Text_BrendanTakeThis[];
extern const u8 Route110_Text_BrendanExplainItemfinder[];

// Route 110's rival and the Itemfinder are two chapters in the Route 110 county (slots 0 and 1), run back to back
// from the rival's script after he walks up to the player. The Itemfinder scene is separate so a full bag leaves its
// stage at 0 while the rival stays done; the script then ends without its exit and the next trigger repeats only the
// Itemfinder (it skips the fight when the rival trainer flag is set). The approach and exit stay in the script.
#define ROUTE110_RIVAL_SCENE(sceneName, letsBattle, defeated) \
static const struct SceneStep sceneName##Steps[] = \
{ \
    SCENE_SAY(letsBattle), \
    SCENE_BATTLE_RIVAL(RIVAL_ENCOUNTER_ROUTE_110, defeated), \
    SCENE_ADVANCE(1), \
}; \
static const struct Scene sceneName = \
{ \
    .name = #sceneName, \
    .actors = {LOCALID_PLAYER}, \
    .numActors = 1, \
    .stageVar = SCENE_ROUTE110_RIVAL_BEAT_VAR, \
    .numSteps = ARRAY_COUNT(sceneName##Steps), \
    .steps = sceneName##Steps, \
}

#define ROUTE110_ITEMFINDER_SCENE(sceneName, takeThis, explain) \
static const struct SceneStep sceneName##Steps[] = \
{ \
    SCENE_SAY(takeThis), \
    SCENE_GIVE(ITEM_DOWSING_MACHINE, 1), \
    SCENE_ADVANCE(1), \
    SCENE_SAY(explain), \
}; \
static const struct Scene sceneName = \
{ \
    .name = #sceneName, \
    .actors = {LOCALID_PLAYER}, \
    .numActors = 1, \
    .stageVar = SCENE_ROUTE110_ITEMFINDER_BEAT_VAR, \
    .numSteps = ARRAY_COUNT(sceneName##Steps), \
    .steps = sceneName##Steps, \
}

ROUTE110_RIVAL_SCENE(sRoute110RivalMay, Route110_Text_MayLetsBattle, Route110_Text_MayDefeated);
ROUTE110_RIVAL_SCENE(sRoute110RivalBrendan, Route110_Text_BrendanLetsBattle, Route110_Text_BrendanDefeated);
ROUTE110_ITEMFINDER_SCENE(sRoute110ItemfinderMay, Route110_Text_MayTakeThis, Route110_Text_MayExplainItemfinder);
ROUTE110_ITEMFINDER_SCENE(sRoute110ItemfinderBrendan, Route110_Text_BrendanTakeThis, Route110_Text_BrendanExplainItemfinder);

extern const u8 Route119_Text_MayIntro[];
extern const u8 Route119_Text_MayDefeat[];
extern const u8 Route119_Text_MayPresentForYou[];
extern const u8 Route119_Text_MayExplainFly[];
extern const u8 Route119_Text_BrendanIntro[];
extern const u8 Route119_Text_BrendanDefeat[];
extern const u8 Route119_Text_BrendanIllGiveYouThis[];
extern const u8 Route119_Text_BrendanExplainFly[];

// Route 119's rival (Weather Institute county, slot 1, three stages: 1 after the fight, 2 after the HM Fly). A full
// bag at the give leaves the stage at 1 and FLAG_RECEIVED_HM_FLY clear; the script then ends without its exit and
// the next trigger runs only the gift scene. FLAG_RECEIVED_HM_FLY stays set for the HM adapter.
#define ROUTE119_GIFT_STEPS(present, explain) \
    SCENE_SAY(present), \
    SCENE_GIVE(ITEM_HM_FLY, 1), \
    SCENE_SET_FLAG(FLAG_RECEIVED_HM_FLY), \
    SCENE_ADVANCE(2), \
    SCENE_SAY(explain)

#define ROUTE119_SCENE_DESC(sceneName) \
static const struct Scene sceneName = \
{ \
    .name = #sceneName, \
    .actors = {LOCALID_PLAYER}, \
    .numActors = 1, \
    .stageVar = SCENE_ROUTE119_RIVAL_BEAT_VAR, \
    .numSteps = ARRAY_COUNT(sceneName##Steps), \
    .steps = sceneName##Steps, \
}

#define ROUTE119_RIVAL_SCENES(rivalName, giftName, intro, defeat, present, explain) \
static const struct SceneStep rivalName##Steps[] = \
{ \
    SCENE_SAY(intro), \
    SCENE_BATTLE_RIVAL(RIVAL_ENCOUNTER_ROUTE_119, defeat), \
    SCENE_ADVANCE(1), \
    ROUTE119_GIFT_STEPS(present, explain), \
}; \
ROUTE119_SCENE_DESC(rivalName); \
static const struct SceneStep giftName##Steps[] = \
{ \
    ROUTE119_GIFT_STEPS(present, explain), \
}; \
ROUTE119_SCENE_DESC(giftName)

ROUTE119_RIVAL_SCENES(sRoute119RivalMay, sRoute119FlyMay, Route119_Text_MayIntro, Route119_Text_MayDefeat, Route119_Text_MayPresentForYou, Route119_Text_MayExplainFly);
ROUTE119_RIVAL_SCENES(sRoute119RivalBrendan, sRoute119FlyBrendan, Route119_Text_BrendanIntro, Route119_Text_BrendanDefeat, Route119_Text_BrendanIllGiveYouThis, Route119_Text_BrendanExplainFly);

extern const u8 LilycoveCity_Text_MayShoppingLetsBattle[];
extern const u8 LilycoveCity_Text_MayBattleMe[];
extern const u8 LilycoveCity_Text_MayWontBeBeaten[];
extern const u8 LilycoveCity_Text_MayDefeat[];
extern const u8 LilycoveCity_Text_MayGoingBackToLittleroot[];
extern const u8 LilycoveCity_Text_MayNotRaisingPokemon[];
extern const u8 LilycoveCity_Text_BrendanShoppingLetsBattle[];
extern const u8 LilycoveCity_Text_BrendanBattleMe[];
extern const u8 LilycoveCity_Text_BrendanWontBeBeaten[];
extern const u8 LilycoveCity_Text_BrendanDefeat[];
extern const u8 LilycoveCity_Text_BrendanGoingBackToLittleroot[];
extern const u8 LilycoveCity_Text_BrendanNoConfidence[];

// The Lilycove rival (Lilycove county, slot 2, two stages: 1 after the fight). A "no" sets
// FLAG_DECLINED_RIVAL_BATTLE_LILYCOVE and leaves the stage at 0, so the rival waits and the script picks the "again"
// question next time. The after-fight badge talk, the bedroom flags and the fly-away stay in the script, which tells
// a win from a decline by the trainer flag. FLAG_MET_RIVAL_LILYCOVE (set by the script) stays for the main-plot adapter.
#define LILYCOVE_RIVAL_SCENES(firstName, againName, firstAsk, againAsk, wontBeBeaten, defeat, goingBack, declined) \
static const struct SceneStep firstName##AcceptSteps[] = \
{ \
    SCENE_SAY(wontBeBeaten), \
    SCENE_BATTLE_RIVAL(RIVAL_ENCOUNTER_LILYCOVE, defeat), \
    SCENE_SAY(goingBack), \
    SCENE_ADVANCE(1), \
}; \
static const struct SceneStep firstName##DeclineSteps[] = \
{ \
    SCENE_SET_FLAG(FLAG_DECLINED_RIVAL_BATTLE_LILYCOVE), \
    SCENE_SAY(declined), \
}; \
static const struct SceneBranch firstName##Answers[] = \
{ \
    SCENE_BRANCH(NULL, firstName##AcceptSteps), \
    SCENE_BRANCH(NULL, firstName##DeclineSteps), \
}; \
static const struct SceneStep firstName##Steps[] = {SCENE_ASK_YES_NO(firstAsk, firstName##Answers)}; \
static const struct Scene firstName = \
{ \
    .name = #firstName, \
    .actors = {LOCALID_PLAYER}, \
    .numActors = 1, \
    .stageVar = SCENE_LILYCOVE_RIVAL_BEAT_VAR, \
    .numSteps = ARRAY_COUNT(firstName##Steps), \
    .steps = firstName##Steps, \
}; \
static const struct SceneStep againName##Steps[] = {SCENE_ASK_YES_NO(againAsk, firstName##Answers)}; \
static const struct Scene againName = \
{ \
    .name = #againName, \
    .actors = {LOCALID_PLAYER}, \
    .numActors = 1, \
    .stageVar = SCENE_LILYCOVE_RIVAL_BEAT_VAR, \
    .numSteps = ARRAY_COUNT(againName##Steps), \
    .steps = againName##Steps, \
}

LILYCOVE_RIVAL_SCENES(sLilycoveRivalMay, sLilycoveRivalMayAgain, LilycoveCity_Text_MayShoppingLetsBattle, LilycoveCity_Text_MayBattleMe, LilycoveCity_Text_MayWontBeBeaten, LilycoveCity_Text_MayDefeat, LilycoveCity_Text_MayGoingBackToLittleroot, LilycoveCity_Text_MayNotRaisingPokemon);
LILYCOVE_RIVAL_SCENES(sLilycoveRivalBrendan, sLilycoveRivalBrendanAgain, LilycoveCity_Text_BrendanShoppingLetsBattle, LilycoveCity_Text_BrendanBattleMe, LilycoveCity_Text_BrendanWontBeBeaten, LilycoveCity_Text_BrendanDefeat, LilycoveCity_Text_BrendanGoingBackToLittleroot, LilycoveCity_Text_BrendanNoConfidence);

extern const u8 Route104_Text_MayMinesDecentLetsBattle[];
extern const u8 Route104_Text_MayLetsBattle[];
extern const u8 Route104_Text_MayIntro[];
extern const u8 Route104_Text_MayDefeat[];
extern const u8 Route104_Text_MayHaventRaisedPokemon[];
extern const u8 Route104_Text_BrendanDoingGreatLetsBattle[];
extern const u8 Route104_Text_BrendanLetsBattle[];
extern const u8 Route104_Text_BrendanIntro[];
extern const u8 Route104_Text_BrendanDefeat[];
extern const u8 Route104_Text_BrendanNoConfidence[];
extern const u8 RustboroCity_Text_MayPassedBrineyWantToBattle[];
extern const u8 RustboroCity_Text_MayWantToBattle[];
extern const u8 RustboroCity_Text_MayImNotGoingToLose[];
extern const u8 RustboroCity_Text_MayDefeat[];
extern const u8 RustboroCity_Text_MayOhHaventRaisedPokemonEnough[];
extern const u8 RustboroCity_Text_BrendanPassedBrineyWantToBattle[];
extern const u8 RustboroCity_Text_BrendanWantToBattle[];
extern const u8 RustboroCity_Text_BrendanIWontGoEasy[];
extern const u8 RustboroCity_Text_BrendanDefeat[];
extern const u8 RustboroCity_Text_BrendanNoConfidenceInPokemon[];

// Rival 2 is one chapter (Rustboro county, slot 2, stage 1 after the fight) met at Route 104 or in Rustboro, depending
// on the route taken; both sites fight the same RIVAL_ENCOUNTER_RUSTBORO trainers and write the same slot var. Each
// site and gender has a first-meeting and a repeated question over shared answers: yes says the intro, fights and
// advances, no says the decline. The registration, music, post-fight talk and the site's defeated flag stay in the
// scripts, which tell a win from a decline by the trainer flag.
#define RIVAL2_SCENES(firstName, againName, firstAsk, againAsk, intro, defeat, declined) \
static const struct SceneStep firstName##AcceptSteps[] = \
{ \
    SCENE_SAY(intro), \
    SCENE_BATTLE_RIVAL(RIVAL_ENCOUNTER_RUSTBORO, defeat), \
    SCENE_ADVANCE(1), \
}; \
static const struct SceneStep firstName##DeclineSteps[] = {SCENE_SAY(declined)}; \
static const struct SceneBranch firstName##Answers[] = \
{ \
    SCENE_BRANCH(NULL, firstName##AcceptSteps), \
    SCENE_BRANCH(NULL, firstName##DeclineSteps), \
}; \
static const struct SceneStep firstName##Steps[] = {SCENE_ASK_YES_NO(firstAsk, firstName##Answers)}; \
static const struct Scene firstName = \
{ \
    .name = #firstName, \
    .actors = {LOCALID_PLAYER}, \
    .numActors = 1, \
    .stageVar = SCENE_RUSTBORO_RIVAL_BEAT_VAR, \
    .numSteps = ARRAY_COUNT(firstName##Steps), \
    .steps = firstName##Steps, \
}; \
static const struct SceneStep againName##Steps[] = {SCENE_ASK_YES_NO(againAsk, firstName##Answers)}; \
static const struct Scene againName = \
{ \
    .name = #againName, \
    .actors = {LOCALID_PLAYER}, \
    .numActors = 1, \
    .stageVar = SCENE_RUSTBORO_RIVAL_BEAT_VAR, \
    .numSteps = ARRAY_COUNT(againName##Steps), \
    .steps = againName##Steps, \
}

RIVAL2_SCENES(sRoute104RivalMay, sRoute104RivalMayAgain, Route104_Text_MayMinesDecentLetsBattle, Route104_Text_MayLetsBattle, Route104_Text_MayIntro, Route104_Text_MayDefeat, Route104_Text_MayHaventRaisedPokemon);
RIVAL2_SCENES(sRoute104RivalBrendan, sRoute104RivalBrendanAgain, Route104_Text_BrendanDoingGreatLetsBattle, Route104_Text_BrendanLetsBattle, Route104_Text_BrendanIntro, Route104_Text_BrendanDefeat, Route104_Text_BrendanNoConfidence);
RIVAL2_SCENES(sRustboroRivalMay, sRustboroRivalMayAgain, RustboroCity_Text_MayPassedBrineyWantToBattle, RustboroCity_Text_MayWantToBattle, RustboroCity_Text_MayImNotGoingToLose, RustboroCity_Text_MayDefeat, RustboroCity_Text_MayOhHaventRaisedPokemonEnough);
RIVAL2_SCENES(sRustboroRivalBrendan, sRustboroRivalBrendanAgain, RustboroCity_Text_BrendanPassedBrineyWantToBattle, RustboroCity_Text_BrendanWantToBattle, RustboroCity_Text_BrendanIWontGoEasy, RustboroCity_Text_BrendanDefeat, RustboroCity_Text_BrendanNoConfidenceInPokemon);

static const struct Scene *const sScenes[SCENE_COUNT] =
{
    [SCENE_LITTLEROOT_NEED_POKEMON] = &sLittlerootNeedPokemon,
    [SCENE_LITTLEROOT_TWIN_ASKS] = &sLittlerootTwinAsks,
    [SCENE_LITTLEROOT_ARRIVE_MALE] = &sLittlerootArriveMale,
    [SCENE_LITTLEROOT_ARRIVE_FEMALE] = &sLittlerootArriveFemale,
    [SCENE_ROUTE101_RESCUE] = &sRoute101Rescue,
    [SCENE_MOSSDEEP_SUPER_ROD] = &sMossdeepSuperRod,
    [SCENE_DEWFORD_OLD_ROD] = &sDewfordOldRod,
    [SCENE_ROUTE118_GOOD_ROD] = &sRoute118GoodRod,
    [SCENE_ROUTE110_RIVAL_MAY] = &sRoute110RivalMay,
    [SCENE_ROUTE110_RIVAL_BRENDAN] = &sRoute110RivalBrendan,
    [SCENE_ROUTE110_ITEMFINDER_MAY] = &sRoute110ItemfinderMay,
    [SCENE_ROUTE110_ITEMFINDER_BRENDAN] = &sRoute110ItemfinderBrendan,
    [SCENE_ROUTE119_RIVAL_MAY] = &sRoute119RivalMay,
    [SCENE_ROUTE119_RIVAL_BRENDAN] = &sRoute119RivalBrendan,
    [SCENE_ROUTE119_FLY_MAY] = &sRoute119FlyMay,
    [SCENE_ROUTE119_FLY_BRENDAN] = &sRoute119FlyBrendan,
    [SCENE_LILYCOVE_RIVAL_MAY] = &sLilycoveRivalMay,
    [SCENE_LILYCOVE_RIVAL_MAY_AGAIN] = &sLilycoveRivalMayAgain,
    [SCENE_LILYCOVE_RIVAL_BRENDAN] = &sLilycoveRivalBrendan,
    [SCENE_LILYCOVE_RIVAL_BRENDAN_AGAIN] = &sLilycoveRivalBrendanAgain,
    [SCENE_ROUTE104_RIVAL_MAY] = &sRoute104RivalMay,
    [SCENE_ROUTE104_RIVAL_MAY_AGAIN] = &sRoute104RivalMayAgain,
    [SCENE_ROUTE104_RIVAL_BRENDAN] = &sRoute104RivalBrendan,
    [SCENE_ROUTE104_RIVAL_BRENDAN_AGAIN] = &sRoute104RivalBrendanAgain,
    [SCENE_RUSTBORO_RIVAL_MAY] = &sRustboroRivalMay,
    [SCENE_RUSTBORO_RIVAL_MAY_AGAIN] = &sRustboroRivalMayAgain,
    [SCENE_RUSTBORO_RIVAL_BRENDAN] = &sRustboroRivalBrendan,
    [SCENE_RUSTBORO_RIVAL_BRENDAN_AGAIN] = &sRustboroRivalBrendanAgain,
    [SCENE_ROUTE103_RIVAL_MAY] = &sRoute103RivalMay,
    [SCENE_ROUTE103_RIVAL_BRENDAN] = &sRoute103RivalBrendan,
    [SCENE_ROUTE101_BAG] = &sRoute101Bag,
    [SCENE_LAB_STARTER] = &sLabStarter,
};

// The truck drops the player within 4 tiles of the door in both houses. Door warp 1 is Brendan's house, 0 is May's.
static const struct SceneTrigger sSceneTriggers[] =
{
    {MAP_GROUP(MAP_LITTLEROOT_TOWN), MAP_NUM(MAP_LITTLEROOT_TOWN), SCENE_TRIGGER_PROXIMITY, 5, 1, SCENE_LITTLEROOT_ARRIVE_MALE, SCENE_MOVING_IN_BEAT_VAR, 1},
    {MAP_GROUP(MAP_LITTLEROOT_TOWN), MAP_NUM(MAP_LITTLEROOT_TOWN), SCENE_TRIGGER_PROXIMITY, 5, 0, SCENE_LITTLEROOT_ARRIVE_FEMALE, SCENE_MOVING_IN_BEAT_VAR, 2},
    {MAP_GROUP(MAP_LITTLEROOT_TOWN), MAP_NUM(MAP_LITTLEROOT_TOWN), SCENE_TRIGGER_EXIT, 1, 0, SCENE_LITTLEROOT_NEED_POKEMON, SCENE_SAVE_BIRCH_BEAT_VAR, SAVE_BIRCH_TWIN_GUARDS, DIR_NORTH},
    {MAP_GROUP(MAP_LITTLEROOT_TOWN), MAP_NUM(MAP_LITTLEROOT_TOWN), SCENE_TRIGGER_EXIT, 1, 0, SCENE_LITTLEROOT_TWIN_ASKS, SCENE_SAVE_BIRCH_BEAT_VAR, SAVE_BIRCH_TWIN_ASKS, DIR_NORTH},
    {MAP_GROUP(MAP_ROUTE101), MAP_NUM(MAP_ROUTE101), SCENE_TRIGGER_ENTER_MAP, 0, 0, SCENE_ROUTE101_RESCUE, SCENE_SAVE_BIRCH_BEAT_VAR, SAVE_BIRCH_BIRCH_IN_GRASS},
};
