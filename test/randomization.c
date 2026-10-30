#include "global.h"
#include "test/test.h"
#include "config/randomization.h"
#include "identity_seed.h"
#include "randomization.h"

TEST("Tests start with every randomization off")
{
    EXPECT_EQ(gSaveBlock2Ptr->randomizations, 0);
}

TEST("Tests start with a fixed player identity")
{
    u32 i;
    EXPECT_EQ(READ_OTID_FROM_SAVE, 0);
    for (i = 0; i < PLAYER_NAME_LENGTH; i++)
        EXPECT_EQ(gSaveBlock2Ptr->playerName[i], 0);
}

TEST("SetRandomized turns one randomization on without touching others")
{
    SetRandomized(RANDOMIZE_TYPES, TRUE);
    EXPECT(IsRandomized(RANDOMIZE_TYPES));
    EXPECT(!IsRandomized(RANDOMIZE_MOVES));
    EXPECT_EQ(gSaveBlock2Ptr->randomizations, RANDOMIZE_TYPES);
}

TEST("SetRandomized turns one randomization off without touching others")
{
    gSaveBlock2Ptr->randomizations = RANDOMIZE_ALL;
    SetRandomized(RANDOMIZE_ENCOUNTERS, FALSE);
    EXPECT(!IsRandomized(RANDOMIZE_ENCOUNTERS));
    EXPECT_EQ(gSaveBlock2Ptr->randomizations, RANDOMIZE_ALL & ~RANDOMIZE_ENCOUNTERS);
}

TEST("Each randomization bit is distinct and within RANDOMIZE_ALL")
{
    static const u32 features[] = {
        RANDOMIZE_NPC_TEXT, RANDOMIZE_SUBPLOTS, RANDOMIZE_ARC_CUTSCENES, RANDOMIZE_TYPES,
        RANDOMIZE_ENCOUNTERS, RANDOMIZE_MOVES, RANDOMIZE_TRAINERS,
    };
    u32 i, seen = 0;
    for (i = 0; i < ARRAY_COUNT(features); i++)
    {
        EXPECT_EQ(seen & features[i], 0);
        seen |= features[i];
    }
    EXPECT_EQ(seen, RANDOMIZE_ALL);
}

TEST("A new game takes the configured default randomizations")
{
    gSaveBlock2Ptr->randomizations = RANDOMIZE_ALL;
    InitNewGameRandomization();
    EXPECT_EQ(gSaveBlock2Ptr->randomizations, RANDOMIZATION_NEW_GAME_DEFAULT);
}

TEST("The randomization mask fits in its save field")
{
    EXPECT_EQ(RANDOMIZE_ALL & ~0xFFFF, 0);
}
