#include "global.h"
#include "constants/characters.h"
#include "identity_seed.h"

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

void PadOtName(u8 *name, u32 size)
{
    u32 i = 0;

    while (i < size && name[i] != EOS)
        i++;
    for (; i < size; i++)
        name[i] = EOS;
}

bool32 IsOtNamePadded(const u8 *otName)
{
    u32 i = 0;

    while (i < PLAYER_NAME_LENGTH && otName[i] != EOS)
        i++;
    for (; i < PLAYER_NAME_LENGTH; i++)
        if (otName[i] != EOS)
            return FALSE;
    return TRUE;
}

u32 SeedFromIdentity(u32 otId, const u8 *otName)
{
    // Padding is EOS by contract (PadOtName), so the name is hashed as a fixed 7-byte block.
    u32 h = MixWord(0, otId);
    AGB_ASSERT(IsOtNamePadded(otName));
    h = MixWord(h, T1_READ_32(otName));
    h = MixWord(h, T1_READ_16(otName + 4) | (otName[6] << 16));
    return Finalize(h ^ 11);
}

u32 IdentityHash(u32 seed, u32 salt, u32 ordinal)
{
    u32 h = MixWord(0x9e3779b9, seed);
    h = MixWord(h, salt);
    h = MixWord(h, ordinal);
    return Finalize(h ^ 12);
}

rng_value_t IdentityRng(u32 seed, u32 salt, u32 ordinal)
{
    return LocalRandomSeed(IdentityHash(seed, salt, ordinal));
}

u32 GetPlayerIdentitySeed(void)
{
    return SeedFromIdentity(READ_OTID_FROM_SAVE, gSaveBlock2Ptr->playerName);
}
