#ifndef GUARD_WORLD_SEED_H
#define GUARD_WORLD_SEED_H

#include "random.h"

/**
 * Deterministic, stateless seeding for procedurally generated content.
 *
 * A world seed is a hash of the 32-bit original-trainer ID (public and secret halves) and the
 * original-trainer name: the only identity that is stable, always in RAM, and attached to traded
 * Pokémon. Content that must agree across trades (types, moves) is seeded from the Pokémon's own
 * OT ID and name via WorldSeedFromIdentity(); the player's own world uses GetPlayerWorldSeed().
 *
 * Everything here is a pure function of its arguments and never touches gRngValue/gRng2Value, so
 * generation can be repeated on demand and unit tested. WorldRng() hands out an independent SFC32
 * state per (salt, ordinal); a salt names a purpose (county, map, NPC, ...) and an ordinal selects
 * one instance of it. Different salts or ordinals give unrelated streams.
 */

// Hashes up to PLAYER_NAME_LENGTH characters of otName, stopping at EOS. Bytes after EOS are ignored.
u32 WorldSeedFromIdentity(u32 otId, const u8 *otName);

u32 WorldHash(u32 worldSeed, u32 salt, u32 ordinal);

rng_value_t WorldRng(u32 worldSeed, u32 salt, u32 ordinal);

u32 GetPlayerWorldSeed(void);

#endif // GUARD_WORLD_SEED_H
