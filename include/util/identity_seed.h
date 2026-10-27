#ifndef GUARD_IDENTITY_SEED_H
#define GUARD_IDENTITY_SEED_H

#include "random.h"

/**
 * Deterministic, stateless seeding for procedurally generated content.
 *
 * An identity seed is a hash of the 32-bit original-trainer ID (public and secret halves) and the
 * original-trainer name: the only identity that is stable, always in RAM, and attached to traded
 * Pokémon. Content that must agree across trades (types, moves) is seeded from the Pokémon's own
 * OT ID and name via SeedFromIdentity(); the player's own seed is GetPlayerIdentitySeed().
 *
 * Everything here is a pure function of its arguments and never touches gRngValue/gRng2Value, so
 * generation can be repeated on demand and unit tested. IdentityRng() hands out an independent SFC32
 * state per (salt, ordinal); a salt names a purpose (county, map, NPC, ...) and an ordinal selects
 * one instance of it. Different salts or ordinals give unrelated streams.
 */

// Fills every byte from the first EOS (or from size) to size with EOS.
void PadOtName(u8 *name, u32 size);

// TRUE if all bytes after the first EOS in the first PLAYER_NAME_LENGTH bytes are EOS.
bool32 IsOtNamePadded(const u8 *otName);

// Hashes the first PLAYER_NAME_LENGTH bytes of otName as a fixed block, with no per-character
// loop. Every writer of a player name or Pokémon OT name must therefore EOS-pad it with
// PadOtName(); a debug assert fires otherwise. Trading with other ROMs is unsupported.
u32 SeedFromIdentity(u32 otId, const u8 *otName);

u32 IdentityHash(u32 seed, u32 salt, u32 ordinal);

rng_value_t IdentityRng(u32 seed, u32 salt, u32 ordinal);

u32 GetPlayerIdentitySeed(void);

#endif // GUARD_IDENTITY_SEED_H
