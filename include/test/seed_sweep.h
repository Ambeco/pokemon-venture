#ifndef GUARD_TEST_SEED_SWEEP_H
#define GUARD_TEST_SEED_SWEEP_H

/**
 * Seed sweeps: rerun a test body under many player identities.
 *
 * SetSweepIdentity(i) writes the i-th sweep identity (OT ID and an EOS-padded 1-7 letter name) into
 * the save block and returns GetPlayerIdentitySeed(), so code that reads the player's seed sees it.
 * The runner zeroes the save before each test, so a sweep needs no cleanup. Identities are fixed per
 * index, so a failure reproduces; a failing test prints the sweep index it was on.
 *
 *     FOR_EACH_SWEEP_SEED(i, seed)
 *     {
 *         EXPECT(SomeInvariant());
 *     }
 */

#define SEED_SWEEP_COUNT 64

extern s32 gSeedSweepIndex;

u32 SetSweepIdentity(u32 index);

#define FOR_EACH_SWEEP_SEED(index, seed) \
    for ((index) = 0; (index) < SEED_SWEEP_COUNT && ((seed) = SetSweepIdentity(index), TRUE); (index)++)

#endif // GUARD_TEST_SEED_SWEEP_H
