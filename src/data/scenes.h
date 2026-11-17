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
    .stageVar = VAR_SAVE_BIRCH_BEAT,
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
    .stageVar = VAR_SAVE_BIRCH_BEAT,
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
    .stageVar = VAR_LITTLEROOT_INTRO_STATE,
    .numSteps = ARRAY_COUNT(sLittlerootArriveMaleSteps),
    .steps = sLittlerootArriveMaleSteps,
};

static const struct Scene sLittlerootArriveFemale =
{
    .name = "LittlerootArriveFemale",
    .actors = {LOCALID_PLAYER, LOCALID_LITTLEROOT_MOM},
    .numActors = 2,
    .stageVar = VAR_LITTLEROOT_INTRO_STATE,
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
    .stageVar = VAR_SAVE_BIRCH_BEAT,
    .numSteps = ARRAY_COUNT(sRoute101RescueSteps),
    .steps = sRoute101RescueSteps,
};

extern const u8 MossdeepCity_House3_Text_YouWantSuperRod[];
extern const u8 MossdeepCity_House3_Text_SuperRodIsSuper[];
extern const u8 MossdeepCity_House3_Text_TryDroppingRodInWater[];
extern const u8 MossdeepCity_House3_Text_DontYouLikeToFish[];

// The Super Rod fisherman (Mossdeep's subplot, slot 0 of county (5,1)), run from his talk script. It names no
// actor or tile, so any house can host it. The stage var is the subplot's stored stage; FLAG_RECEIVED_SUPER_ROD
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
    .stageVar = VAR_OLDALE_RIVAL_STATE, \
    .numSteps = ARRAY_COUNT(sceneName##Steps), \
    .steps = sceneName##Steps, \
}


ROUTE103_RIVAL_SCENE(sRoute103RivalMay, Route103_Text_MayLetsBattle, Route103_Text_MayDefeated, Route103_Text_MayNotYet);
ROUTE103_RIVAL_SCENE(sRoute103RivalBrendan, Route103_Text_BrendanLetsBattle, Route103_Text_BrendanDefeated, Route103_Text_BrendanNotYet);

static const struct Scene *const sScenes[SCENE_COUNT] =
{
    [SCENE_LITTLEROOT_NEED_POKEMON] = &sLittlerootNeedPokemon,
    [SCENE_LITTLEROOT_TWIN_ASKS] = &sLittlerootTwinAsks,
    [SCENE_LITTLEROOT_ARRIVE_MALE] = &sLittlerootArriveMale,
    [SCENE_LITTLEROOT_ARRIVE_FEMALE] = &sLittlerootArriveFemale,
    [SCENE_ROUTE101_RESCUE] = &sRoute101Rescue,
    [SCENE_MOSSDEEP_SUPER_ROD] = &sMossdeepSuperRod,
    [SCENE_ROUTE103_RIVAL_MAY] = &sRoute103RivalMay,
    [SCENE_ROUTE103_RIVAL_BRENDAN] = &sRoute103RivalBrendan,
};

// The truck drops the player within 4 tiles of the door in both houses. Door warp 1 is Brendan's house, 0 is May's.
static const struct SceneTrigger sSceneTriggers[] =
{
    {MAP_GROUP(MAP_LITTLEROOT_TOWN), MAP_NUM(MAP_LITTLEROOT_TOWN), SCENE_TRIGGER_PROXIMITY, 5, 1, SCENE_LITTLEROOT_ARRIVE_MALE, VAR_LITTLEROOT_INTRO_STATE, 1},
    {MAP_GROUP(MAP_LITTLEROOT_TOWN), MAP_NUM(MAP_LITTLEROOT_TOWN), SCENE_TRIGGER_PROXIMITY, 5, 0, SCENE_LITTLEROOT_ARRIVE_FEMALE, VAR_LITTLEROOT_INTRO_STATE, 2},
    {MAP_GROUP(MAP_LITTLEROOT_TOWN), MAP_NUM(MAP_LITTLEROOT_TOWN), SCENE_TRIGGER_EXIT, 1, 0, SCENE_LITTLEROOT_NEED_POKEMON, VAR_SAVE_BIRCH_BEAT, SAVE_BIRCH_TWIN_GUARDS, DIR_NORTH},
    {MAP_GROUP(MAP_LITTLEROOT_TOWN), MAP_NUM(MAP_LITTLEROOT_TOWN), SCENE_TRIGGER_EXIT, 1, 0, SCENE_LITTLEROOT_TWIN_ASKS, VAR_SAVE_BIRCH_BEAT, SAVE_BIRCH_TWIN_ASKS, DIR_NORTH},
    {MAP_GROUP(MAP_ROUTE101), MAP_NUM(MAP_ROUTE101), SCENE_TRIGGER_ENTER_MAP, 0, 0, SCENE_ROUTE101_RESCUE, VAR_SAVE_BIRCH_BEAT, SAVE_BIRCH_BIRCH_IN_GRASS},
};
