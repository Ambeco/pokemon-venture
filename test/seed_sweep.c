#include "global.h"
#include "test/test.h"
#include "test/seed_sweep.h"
#include "constants/characters.h"
#include "identity_seed.h"

#define SALT_SWEEP_OTID 0x53574550
#define SALT_SWEEP_NAME 0x4e414d45

u32 SetSweepIdentity(u32 index)
{
    u32 otId = IdentityHash(0, SALT_SWEEP_OTID, index);
    u32 nameBits = IdentityHash(0, SALT_SWEEP_NAME, index);
    u32 len = 1 + nameBits % PLAYER_NAME_LENGTH;
    u32 i;

    nameBits /= PLAYER_NAME_LENGTH;
    for (i = 0; i < len; i++)
    {
        gSaveBlock2Ptr->playerName[i] = CHAR_A + nameBits % 26;
        nameBits = nameBits / 26 + IdentityHash(nameBits, SALT_SWEEP_NAME, i);
    }
    gSaveBlock2Ptr->playerName[len] = EOS;
    PadOtName(gSaveBlock2Ptr->playerName, PLAYER_NAME_LENGTH + 1);
    gSaveBlock2Ptr->playerTrainerId[0] = otId;
    gSaveBlock2Ptr->playerTrainerId[1] = otId >> 8;
    gSaveBlock2Ptr->playerTrainerId[2] = otId >> 16;
    gSaveBlock2Ptr->playerTrainerId[3] = otId >> 24;
    gSeedSweepIndex = index;
    return GetPlayerIdentitySeed();
}

TEST("SeedSweep: SetSweepIdentity sets the player identity it reports")
{
    u32 i, seed;
    FOR_EACH_SWEEP_SEED(i, seed)
    {
        EXPECT_EQ(seed, SeedFromIdentity(READ_OTID_FROM_SAVE, gSaveBlock2Ptr->playerName));
        EXPECT(IsOtNamePadded(gSaveBlock2Ptr->playerName));
        EXPECT_NE(gSaveBlock2Ptr->playerName[0], EOS);
    }
    EXPECT_EQ(i, SEED_SWEEP_COUNT);
}

TEST("SeedSweep: Identities are reproducible per index")
{
    u32 a = SetSweepIdentity(7);
    SetSweepIdentity(8);
    EXPECT_EQ(SetSweepIdentity(7), a);
}

TEST("SeedSweep: Every index gives a distinct seed")
{
    static EWRAM_DATA u32 seeds[SEED_SWEEP_COUNT];
    u32 i, j, seed;
    FOR_EACH_SWEEP_SEED(i, seed)
    {
        for (j = 0; j < i; j++)
            EXPECT_NE(seeds[j], seed);
        seeds[i] = seed;
    }
}

TEST("SeedSweep: Names vary in length and use only capital letters")
{
    u32 i, j, seed, lengths = 0;
    FOR_EACH_SWEEP_SEED(i, seed)
    {
        for (j = 0; j < PLAYER_NAME_LENGTH && gSaveBlock2Ptr->playerName[j] != EOS; j++)
            EXPECT(gSaveBlock2Ptr->playerName[j] >= CHAR_A && gSaveBlock2Ptr->playerName[j] <= CHAR_Z);
        lengths |= 1 << j;
    }
    EXPECT_EQ(lengths, ((1 << PLAYER_NAME_LENGTH) - 1) << 1);
}
