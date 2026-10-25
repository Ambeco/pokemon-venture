#include "global.h"
#include "test/test.h"
#include "constants/characters.h"
#include "world_seed.h"

#define SALT_A 1
#define SALT_B 2

// OT names are fixed 7-byte blocks padded with EOS.
static const u8 sNameAbc[PLAYER_NAME_LENGTH] = {CHAR_A, CHAR_B, CHAR_C, EOS, EOS, EOS, EOS};
static const u8 sNameAbd[PLAYER_NAME_LENGTH] = {CHAR_A, CHAR_B, CHAR_D, EOS, EOS, EOS, EOS};
static const u8 sNameLong[PLAYER_NAME_LENGTH] = {CHAR_A, CHAR_B, CHAR_C, CHAR_D, CHAR_E, CHAR_F, CHAR_G};

TEST("WorldSeedFromIdentity is deterministic")
{
    EXPECT_EQ(WorldSeedFromIdentity(0x12345678, sNameAbc), WorldSeedFromIdentity(0x12345678, sNameAbc));
}

TEST("WorldSeedFromIdentity depends on the full 32-bit OT ID")
{
    EXPECT_NE(WorldSeedFromIdentity(0x00001234, sNameAbc), WorldSeedFromIdentity(0x00011234, sNameAbc));
    EXPECT_NE(WorldSeedFromIdentity(0x00001234, sNameAbc), WorldSeedFromIdentity(0x00001235, sNameAbc));
}

TEST("WorldSeedFromIdentity depends on the name")
{
    EXPECT_NE(WorldSeedFromIdentity(1, sNameAbc), WorldSeedFromIdentity(1, sNameAbd));
    EXPECT_NE(WorldSeedFromIdentity(1, sNameAbc), WorldSeedFromIdentity(1, sNameLong));
}

TEST("PadOtName fills everything after the first EOS with EOS")
{
    u8 name[PLAYER_NAME_LENGTH + 1] = {CHAR_A, CHAR_B, EOS, 0x11, 0x22, 0x33, 0x44, 0x55};
    u32 i;
    PadOtName(name, sizeof(name));
    EXPECT_EQ(name[0], CHAR_A);
    EXPECT_EQ(name[1], CHAR_B);
    for (i = 2; i < sizeof(name); i++)
        EXPECT_EQ(name[i], EOS);
}

TEST("PadOtName leaves a full-length name intact")
{
    u8 name[PLAYER_NAME_LENGTH + 1] = {CHAR_A, CHAR_B, CHAR_C, CHAR_D, CHAR_E, CHAR_F, CHAR_G, 0x33};
    PadOtName(name, PLAYER_NAME_LENGTH);
    EXPECT_EQ(name[PLAYER_NAME_LENGTH - 1], CHAR_G);
    EXPECT_EQ(name[PLAYER_NAME_LENGTH], 0x33);
}

TEST("IsOtNamePadded detects junk after EOS")
{
    u8 good[PLAYER_NAME_LENGTH] = {CHAR_A, EOS, EOS, EOS, EOS, EOS, EOS};
    u8 bad[PLAYER_NAME_LENGTH] = {CHAR_A, EOS, EOS, 0x11, EOS, EOS, EOS};
    EXPECT(IsOtNamePadded(good));
    EXPECT(!IsOtNamePadded(bad));
}

TEST("SetBoxMonData OT name pads with EOS")
{
    struct Pokemon mon;
    u8 src[PLAYER_NAME_LENGTH] = {CHAR_A, CHAR_B, EOS, 0x11, 0x22, 0x33, 0x44};
    u32 i;
    CreateMon(&mon, SPECIES_WOBBUFFET, 5, 0, OTID_STRUCT_PLAYER_ID);
    SetMonData(&mon, MON_DATA_OT_NAME, src);
    for (i = 2; i < PLAYER_NAME_LENGTH; i++)
        EXPECT_EQ(mon.box.otName[i], EOS);
    EXPECT(IsOtNamePadded(mon.box.otName));
}

TEST("A Pokémon's seed is identical whatever junk its source name carried")
{
    struct Pokemon a, b;
    u8 srcA[PLAYER_NAME_LENGTH] = {CHAR_A, CHAR_B, EOS, 0x11, 0x22, 0x33, 0x44};
    u8 srcB[PLAYER_NAME_LENGTH] = {CHAR_A, CHAR_B, EOS, 0x55, 0x66, 0x77, 0x88};
    CreateMon(&a, SPECIES_WOBBUFFET, 5, 0, OTID_STRUCT_PRESET(0x12345678));
    CreateMon(&b, SPECIES_WOBBUFFET, 5, 0, OTID_STRUCT_PRESET(0x12345678));
    SetMonData(&a, MON_DATA_OT_NAME, srcA);
    SetMonData(&b, MON_DATA_OT_NAME, srcB);
    EXPECT_EQ(WorldSeedFromIdentity(0x12345678, a.box.otName), WorldSeedFromIdentity(0x12345678, b.box.otName));
}

TEST("New-game player name is padded")
{
    EXPECT(IsOtNamePadded(gSaveBlock2Ptr->playerName));
}

TEST("WorldSeedFromIdentity reads an unterminated 7-character OT name safely")
{
    // Pokémon store otName as exactly PLAYER_NAME_LENGTH bytes with no terminator.
    u8 name[PLAYER_NAME_LENGTH] = {CHAR_A, CHAR_B, CHAR_C, CHAR_D, CHAR_E, CHAR_F, CHAR_G};
    EXPECT_EQ(WorldSeedFromIdentity(3, name), WorldSeedFromIdentity(3, sNameLong));
}

TEST("WorldSeedFromIdentity has no collisions over a sweep of IDs")
{
    u32 i, j;
    u32 seeds[64];
    for (i = 0; i < ARRAY_COUNT(seeds); i++)
    {
        seeds[i] = WorldSeedFromIdentity(i * 0x10001, sNameAbc);
        for (j = 0; j < i; j++)
            EXPECT_NE(seeds[i], seeds[j]);
    }
}

TEST("WorldHash separates salts and ordinals")
{
    EXPECT_EQ(WorldHash(5, SALT_A, 0), WorldHash(5, SALT_A, 0));
    EXPECT_NE(WorldHash(5, SALT_A, 0), WorldHash(5, SALT_B, 0));
    EXPECT_NE(WorldHash(5, SALT_A, 0), WorldHash(5, SALT_A, 1));
    EXPECT_NE(WorldHash(5, SALT_A, 0), WorldHash(6, SALT_A, 0));
    // Swapping salt and ordinal must not collide.
    EXPECT_NE(WorldHash(5, 1, 2), WorldHash(5, 2, 1));
}

TEST("WorldHash spreads bits (avalanche on the seed)")
{
    u32 i, flipped = 0;
    for (i = 0; i < 32; i++)
    {
        u32 diff = WorldHash(0, SALT_A, 0) ^ WorldHash(1u << i, SALT_A, 0);
        u32 bit;
        for (bit = 0; bit < 32; bit++)
            flipped += (diff >> bit) & 1;
    }
    // Expect about 16 of 32 output bits per flip, i.e. 512 total.
    EXPECT_GT(flipped, 400);
    EXPECT_LT(flipped, 624);
}

TEST("WorldRng is reproducible and independent of the global RNG")
{
    rng_value_t a = WorldRng(9, SALT_A, 3);
    u32 first = LocalRandom32(&a);
    rng_value_t b;
    Random32();
    Random32();
    b = WorldRng(9, SALT_A, 3);
    EXPECT_EQ(first, LocalRandom32(&b));
}

TEST("WorldRng streams differ per salt and ordinal")
{
    rng_value_t a = WorldRng(9, SALT_A, 0);
    rng_value_t b = WorldRng(9, SALT_B, 0);
    rng_value_t c = WorldRng(9, SALT_A, 1);
    u32 x = LocalRandom32(&a), y = LocalRandom32(&b), z = LocalRandom32(&c);
    EXPECT_NE(x, y);
    EXPECT_NE(x, z);
    EXPECT_NE(y, z);
}

TEST("GetPlayerWorldSeed matches the saved identity")
{
    EXPECT_EQ(GetPlayerWorldSeed(), WorldSeedFromIdentity(READ_OTID_FROM_SAVE, gSaveBlock2Ptr->playerName));
}
