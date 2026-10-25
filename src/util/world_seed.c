#include "global.h"
#include "constants/characters.h"
#include "world_seed.h"

// Murmur3 32-bit mixing.
static u32 MixWord(u32 h, u32 k)
{
    k *= 0xcc9e2d51;
    k = (k << 15) | (k >> 17);
    k *= 0x1b873593;
    h ^= k;
    h = (h << 13) | (h >> 19);
    return h * 5 + 0xe6546b64;
}

static u32 Finalize(u32 h)
{
    h ^= h >> 16;
    h *= 0x85ebca6b;
    h ^= h >> 13;
    h *= 0xc2b2ae35;
    h ^= h >> 16;
    return h;
}

u32 WorldSeedFromIdentity(u32 otId, const u8 *otName)
{
    u32 i;
    u32 h = MixWord(0, otId);

    for (i = 0; i < PLAYER_NAME_LENGTH && otName[i] != EOS; i++)
        h = MixWord(h, otName[i]);

    // Length is mixed in so a name and the same name plus a trailing 0x00 (a space) differ.
    return Finalize(h ^ i);
}

u32 WorldHash(u32 worldSeed, u32 salt, u32 ordinal)
{
    u32 h = MixWord(0x9e3779b9, worldSeed);
    h = MixWord(h, salt);
    h = MixWord(h, ordinal);
    return Finalize(h ^ 12);
}

rng_value_t WorldRng(u32 worldSeed, u32 salt, u32 ordinal)
{
    return LocalRandomSeed(WorldHash(worldSeed, salt, ordinal));
}

u32 GetPlayerWorldSeed(void)
{
    return WorldSeedFromIdentity(READ_OTID_FROM_SAVE, gSaveBlock2Ptr->playerName);
}
