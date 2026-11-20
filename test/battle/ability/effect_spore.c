#include "global.h"
#include "test/battle.h"
#include "battle_util.h"

SINGLE_BATTLE_TEST("Effect Spore only inflicts status on contact")
{
    enum Move move;

    PARAMETRIZE { move = MOVE_SCRATCH; }
    PARAMETRIZE { move = MOVE_SWIFT; }
    GIVEN {
        ASSUME(MoveMakesContact(MOVE_SCRATCH));
        ASSUME(!MoveMakesContact(MOVE_SWIFT));
        PLAYER(SPECIES_WOBBUFFET);
        OPPONENT(SPECIES_BRELOOM) { Ability(ABILITY_EFFECT_SPORE); }
    } WHEN {
        TURN { MOVE(player, move, WITH_RNG(RNG_EFFECT_SPORE, 1)); }
        TURN {}
    } SCENE {
        if (MoveMakesContact(move)) {
            ABILITY_POPUP(opponent, ABILITY_EFFECT_SPORE);
            ANIMATION(ANIM_TYPE_STATUS, B_ANIM_STATUS_PSN, player);
            MESSAGE("Wobbuffet was poisoned!");
            STATUS_ICON(player, poison: TRUE);
        } else {
            NONE_OF {
                ABILITY_POPUP(opponent, ABILITY_EFFECT_SPORE);
                ANIMATION(ANIM_TYPE_STATUS, B_ANIM_STATUS_PSN, player);
                MESSAGE("Wobbuffet was poisoned!");
                STATUS_ICON(player, poison: TRUE);
            }
        }
    }
}

// The battle test PARAMETRIZE needs a battle, so the plain test loops over a table instead.
static const struct {
    u32 config;
    u32 rollMax;
    u32 poison;
    u32 paralysis;
    u32 sleep;
} sEffectSporeOdds[] = {
    { GEN_3, 299, 10, 10, 10 },      // 3.3% each of 300
    { GEN_4, 99, 10, 10, 10 },       // 10% each of 100
    { GEN_5, 99, 9, 10, 11 },
    { GEN_LATEST, 99, 9, 10, 11 },
};

TEST("Effect Spore's roll bands give 3.3% (Gen3), 10% (Gen4) and 9% / 10% / 11% (Gen5+) odds")
{
    u32 i, roll;

    for (i = 0; i < ARRAY_COUNT(sEffectSporeOdds); i++)
    {
        u32 counts[4] = { 0 };
        u32 rollMax = sEffectSporeOdds[i].rollMax;
        u32 config = sEffectSporeOdds[i].config;

        EXPECT_EQ(GetEffectSporeRollMax(config), rollMax);
        for (roll = 0; roll <= rollMax; roll++)
            counts[GetEffectSporeOutcome(roll, config)]++;
        EXPECT_EQ(counts[EFFECT_SPORE_POISON], sEffectSporeOdds[i].poison);
        EXPECT_EQ(counts[EFFECT_SPORE_PARALYSIS], sEffectSporeOdds[i].paralysis);
        EXPECT_EQ(counts[EFFECT_SPORE_SLEEP], sEffectSporeOdds[i].sleep);
        EXPECT_EQ(counts[EFFECT_SPORE_NOTHING], rollMax + 1 - sEffectSporeOdds[i].poison - sEffectSporeOdds[i].paralysis - sEffectSporeOdds[i].sleep);
    }
}

TEST("Effect Spore's bands run poison, paralysis, sleep in order")
{
    EXPECT_EQ(GetEffectSporeOutcome(0, GEN_5), EFFECT_SPORE_POISON);
    EXPECT_EQ(GetEffectSporeOutcome(8, GEN_5), EFFECT_SPORE_POISON);
    EXPECT_EQ(GetEffectSporeOutcome(9, GEN_5), EFFECT_SPORE_PARALYSIS);
    EXPECT_EQ(GetEffectSporeOutcome(18, GEN_5), EFFECT_SPORE_PARALYSIS);
    EXPECT_EQ(GetEffectSporeOutcome(19, GEN_5), EFFECT_SPORE_SLEEP);
    EXPECT_EQ(GetEffectSporeOutcome(29, GEN_5), EFFECT_SPORE_SLEEP);
    EXPECT_EQ(GetEffectSporeOutcome(30, GEN_5), EFFECT_SPORE_NOTHING);
    EXPECT_EQ(GetEffectSporeOutcome(99, GEN_5), EFFECT_SPORE_NOTHING);
}

SINGLE_BATTLE_TEST("Effect Spore paralyzes the attacker on a paralysis roll")
{
    GIVEN {
        ASSUME(MoveMakesContact(MOVE_SCRATCH));
        PLAYER(SPECIES_WOBBUFFET);
        OPPONENT(SPECIES_BRELOOM) { Ability(ABILITY_EFFECT_SPORE); }
    } WHEN {
        TURN { MOVE(player, MOVE_SCRATCH, WITH_RNG(RNG_EFFECT_SPORE, EFFECT_SPORE_PARALYSIS_ROLL)); }
        TURN {}
    } SCENE {
        ABILITY_POPUP(opponent, ABILITY_EFFECT_SPORE);
        ANIMATION(ANIM_TYPE_STATUS, B_ANIM_STATUS_PRZ, player);
        MESSAGE("Wobbuffet is paralyzed, so it may be unable to move!");
        STATUS_ICON(player, paralysis: TRUE);
    }
}

SINGLE_BATTLE_TEST("Effect Spore puts the attacker to sleep on a sleep roll")
{
    u32 config;
    PARAMETRIZE { config = GEN_3; }
    PARAMETRIZE { config = GEN_4; }
    PARAMETRIZE { config = GEN_5; }
    GIVEN {
        WITH_CONFIG(B_ABILITY_TRIGGER_CHANCE, config);
        ASSUME(MoveMakesContact(MOVE_SCRATCH));
        PLAYER(SPECIES_WOBBUFFET);
        OPPONENT(SPECIES_BRELOOM) { Ability(ABILITY_EFFECT_SPORE); }
    } WHEN {
        TURN { MOVE(player, MOVE_SCRATCH, WITH_RNG(RNG_EFFECT_SPORE, EFFECT_SPORE_SLEEP_ROLL)); }
        TURN {}
    } SCENE {
        ABILITY_POPUP(opponent, ABILITY_EFFECT_SPORE);
        ANIMATION(ANIM_TYPE_STATUS, B_ANIM_STATUS_SLP, player);
        MESSAGE("Wobbuffet fell asleep!");
        STATUS_ICON(player, sleep: TRUE);
    }
}

SINGLE_BATTLE_TEST("Effect Spore does nothing on a roll outside every band")
{
    GIVEN {
        ASSUME(MoveMakesContact(MOVE_SCRATCH));
        PLAYER(SPECIES_WOBBUFFET);
        OPPONENT(SPECIES_BRELOOM) { Ability(ABILITY_EFFECT_SPORE); }
    } WHEN {
        TURN { MOVE(player, MOVE_SCRATCH, WITH_RNG(RNG_EFFECT_SPORE, 50)); }
        TURN {}
    } SCENE {
        NONE_OF {
            ABILITY_POPUP(opponent, ABILITY_EFFECT_SPORE);
            MESSAGE("Wobbuffet was poisoned!");
            MESSAGE("Wobbuffet is paralyzed, so it may be unable to move!");
            MESSAGE("Wobbuffet fell asleep!");
        }
    }
}

SINGLE_BATTLE_TEST("Effect Spore will check if it can inflict status onto attacker, not itself")
{
    GIVEN {
        ASSUME(MoveMakesContact(MOVE_SCRATCH));
        PLAYER(SPECIES_WOBBUFFET);
        OPPONENT(SPECIES_BRELOOM) { Status1(STATUS1_BURN); Ability(ABILITY_EFFECT_SPORE); }
    } WHEN {
        TURN { MOVE(player, MOVE_SCRATCH, WITH_RNG(RNG_EFFECT_SPORE, EFFECT_SPORE_SLEEP_ROLL)); }
    } SCENE {
        ABILITY_POPUP(opponent, ABILITY_EFFECT_SPORE);
        ANIMATION(ANIM_TYPE_STATUS, B_ANIM_STATUS_SLP, player);
        MESSAGE("Wobbuffet fell asleep!");
        STATUS_ICON(player, sleep: TRUE);
    }
}
