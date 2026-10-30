#ifndef GUARD_RANDOMIZATION_H
#define GUARD_RANDOMIZATION_H

#include "constants/randomization.h"

/**
 * Per-save switches for each randomization (RANDOMIZE_*).
 *
 * Every randomized path checks IsRandomized() and falls back to vanilla Emerald data when its bit is
 * off. The mask lives in SaveBlock2, so it survives save and load; a new game copies
 * RANDOMIZATION_NEW_GAME_DEFAULT. The test runner zeroes the save blocks before each test, so tests
 * start with everything off and opt in with SetRandomized().
 */

bool32 IsRandomized(u32 feature);

void SetRandomized(u32 feature, bool32 on);

void InitNewGameRandomization(void);

#endif // GUARD_RANDOMIZATION_H
