#include "global.h"
#include "event_data.h"
#include "test/test.h"
#include "story_mode.h"

TEST("StoryMode: The toggle is the story-mode flag")
{
    FlagClear(FLAG_VENTURE_STORY_MODE);
    EXPECT_EQ(IsStoryModeOn(), FALSE);
    FlagSet(FLAG_VENTURE_STORY_MODE);
    EXPECT_EQ(IsStoryModeOn(), TRUE);
    FlagClear(FLAG_VENTURE_STORY_MODE);
}

TEST("StoryMode: Gym leaders, the Elite Four, the Champion, rivals and team bosses stay real fights")
{
    EXPECT_EQ(StoryMode_IsBossTrainerClass(TRAINER_CLASS_LEADER), TRUE);
    EXPECT_EQ(StoryMode_IsBossTrainerClass(TRAINER_CLASS_ELITE_FOUR), TRUE);
    EXPECT_EQ(StoryMode_IsBossTrainerClass(TRAINER_CLASS_CHAMPION), TRUE);
    EXPECT_EQ(StoryMode_IsBossTrainerClass(TRAINER_CLASS_RIVAL), TRUE);
    EXPECT_EQ(StoryMode_IsBossTrainerClass(TRAINER_CLASS_AQUA_LEADER), TRUE);
    EXPECT_EQ(StoryMode_IsBossTrainerClass(TRAINER_CLASS_MAGMA_LEADER), TRUE);
    EXPECT_EQ(StoryMode_IsBossTrainerClass(TRAINER_CLASS_AQUA_ADMIN), TRUE);
    EXPECT_EQ(StoryMode_IsBossTrainerClass(TRAINER_CLASS_MAGMA_ADMIN), TRUE);
    EXPECT_EQ(StoryMode_IsBossTrainerClass(TRAINER_CLASS_LEADER_FRLG), TRUE);
    EXPECT_EQ(StoryMode_IsBossTrainerClass(TRAINER_CLASS_RIVAL_EARLY_FRLG), TRUE);
}

TEST("StoryMode: Ordinary trainer classes are skippable")
{
    EXPECT_EQ(StoryMode_IsBossTrainerClass(TRAINER_CLASS_HIKER), FALSE);
    EXPECT_EQ(StoryMode_IsBossTrainerClass(TRAINER_CLASS_TEAM_AQUA), FALSE);
    EXPECT_EQ(StoryMode_IsBossTrainerClass(TRAINER_CLASS_TEAM_MAGMA), FALSE);
    EXPECT_EQ(StoryMode_IsBossTrainerClass(TRAINER_CLASS_PKMN_TRAINER_1), FALSE);
}

TEST("StoryMode: Only plain wild battles are skipped")
{
    EXPECT_EQ(StoryMode_CanSkipWildBattleType(0), TRUE);
    EXPECT_EQ(StoryMode_CanSkipWildBattleType(BATTLE_TYPE_DOUBLE), TRUE);
    EXPECT_EQ(StoryMode_CanSkipWildBattleType(BATTLE_TYPE_DOUBLE | BATTLE_TYPE_MULTI | BATTLE_TYPE_INGAME_PARTNER), FALSE);
    EXPECT_EQ(StoryMode_CanSkipWildBattleType(BATTLE_TYPE_PYRAMID), FALSE);
    EXPECT_EQ(StoryMode_CanSkipWildBattleType(BATTLE_TYPE_LEGENDARY), FALSE);
    EXPECT_EQ(StoryMode_CanSkipWildBattleType(BATTLE_TYPE_ROAMER), FALSE);
    EXPECT_EQ(StoryMode_CanSkipWildBattleType(BATTLE_TYPE_SAFARI), FALSE);
    EXPECT_EQ(StoryMode_CanSkipWildBattleType(BATTLE_TYPE_GHOST), FALSE);
    EXPECT_EQ(StoryMode_CanSkipWildBattleType(BATTLE_TYPE_PIKE), FALSE);
    EXPECT_EQ(StoryMode_CanSkipWildBattleType(BATTLE_TYPE_CATCH_TUTORIAL), FALSE);
}

TEST("StoryMode: Only plain trainer battles are skipped")
{
    EXPECT_EQ(StoryMode_CanSkipTrainerBattleType(BATTLE_TYPE_TRAINER), TRUE);
    EXPECT_EQ(StoryMode_CanSkipTrainerBattleType(BATTLE_TYPE_TRAINER | BATTLE_TYPE_DOUBLE), TRUE);
    EXPECT_EQ(StoryMode_CanSkipTrainerBattleType(BATTLE_TYPE_TRAINER | BATTLE_TYPE_DOUBLE | BATTLE_TYPE_TWO_OPPONENTS), TRUE);
    EXPECT_EQ(StoryMode_CanSkipTrainerBattleType(0), FALSE);
    EXPECT_EQ(StoryMode_CanSkipTrainerBattleType(BATTLE_TYPE_TRAINER | BATTLE_TYPE_MULTI | BATTLE_TYPE_INGAME_PARTNER | BATTLE_TYPE_DOUBLE), FALSE);
    EXPECT_EQ(StoryMode_CanSkipTrainerBattleType(BATTLE_TYPE_TRAINER | BATTLE_TYPE_PYRAMID), FALSE);
    EXPECT_EQ(StoryMode_CanSkipTrainerBattleType(BATTLE_TYPE_TRAINER | BATTLE_TYPE_TRAINER_HILL), FALSE);
    EXPECT_EQ(StoryMode_CanSkipTrainerBattleType(BATTLE_TYPE_TRAINER | BATTLE_TYPE_FIRST_BATTLE), FALSE);
}

TEST("StoryMode: Experience is yield * level / 5, with a 1.5x trainer bonus")
{
    EXPECT_EQ(StoryMode_CalcExp(100, 10, FALSE), 200);
    EXPECT_EQ(StoryMode_CalcExp(100, 10, TRUE), 300);
    EXPECT_EQ(StoryMode_CalcExp(0, 50, TRUE), 0);
    EXPECT_EQ(StoryMode_CalcExp(64, 1, FALSE), 12);
}

TEST("StoryMode: Prize money is 4 * last level * class money, doubled in doubles, 5 when the class has none")
{
    EXPECT_EQ(StoryMode_CalcPrizeMoney(10, 20, FALSE), 800);
    EXPECT_EQ(StoryMode_CalcPrizeMoney(10, 20, TRUE), 1600);
    EXPECT_EQ(StoryMode_CalcPrizeMoney(0, 20, FALSE), 400);
}

TEST("StoryMode: Damage to the player is clamped to leave 1 HP")
{
    EXPECT_EQ(StoryMode_ClampPlayerDamage(50, 100), 50);
    EXPECT_EQ(StoryMode_ClampPlayerDamage(99, 100), 99);
    EXPECT_EQ(StoryMode_ClampPlayerDamage(100, 100), 99);
    EXPECT_EQ(StoryMode_ClampPlayerDamage(500, 100), 99);
    EXPECT_EQ(StoryMode_ClampPlayerDamage(5, 1), 0);
    EXPECT_EQ(StoryMode_ClampPlayerDamage(5, 0), 0);
}

TEST("StoryMode: Healing and zero damage pass through the clamp")
{
    EXPECT_EQ(StoryMode_ClampPlayerDamage(0, 10), 0);
    EXPECT_EQ(StoryMode_ClampPlayerDamage(-30, 10), -30);
    EXPECT_EQ(StoryMode_ClampPlayerDamage(-30, 1), -30);
}

TEST("StoryMode: A catch is redundant unless it has a strictly higher IV total")
{
    EXPECT_EQ(StoryMode_IsCatchRedundant(100, 99), TRUE);
    EXPECT_EQ(StoryMode_IsCatchRedundant(100, 100), TRUE);
    EXPECT_EQ(StoryMode_IsCatchRedundant(100, 101), FALSE);
    EXPECT_EQ(StoryMode_IsCatchRedundant(0, 0), TRUE);
}
