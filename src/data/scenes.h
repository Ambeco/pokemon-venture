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
    .stageVar = VAR_LITTLEROOT_TOWN_STATE,
    .numSteps = ARRAY_COUNT(sLittlerootNeedPokemonSteps),
    .steps = sLittlerootNeedPokemonSteps,
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

static const struct Scene *const sScenes[SCENE_COUNT] =
{
    [SCENE_LITTLEROOT_NEED_POKEMON] = &sLittlerootNeedPokemon,
    [SCENE_LITTLEROOT_ARRIVE_MALE] = &sLittlerootArriveMale,
    [SCENE_LITTLEROOT_ARRIVE_FEMALE] = &sLittlerootArriveFemale,
};

// The truck drops the player within 4 tiles of the door in both houses. Door warp 1 is Brendan's house, 0 is May's.
static const struct SceneTrigger sSceneTriggers[] =
{
    {MAP_GROUP(MAP_LITTLEROOT_TOWN), MAP_NUM(MAP_LITTLEROOT_TOWN), SCENE_TRIGGER_PROXIMITY, 5, 1, SCENE_LITTLEROOT_ARRIVE_MALE, VAR_LITTLEROOT_INTRO_STATE, 1},
    {MAP_GROUP(MAP_LITTLEROOT_TOWN), MAP_NUM(MAP_LITTLEROOT_TOWN), SCENE_TRIGGER_PROXIMITY, 5, 0, SCENE_LITTLEROOT_ARRIVE_FEMALE, VAR_LITTLEROOT_INTRO_STATE, 2},
};
