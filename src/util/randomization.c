#include "global.h"
#include "config/randomization.h"
#include "randomization.h"

STATIC_ASSERT((RANDOMIZATION_NEW_GAME_DEFAULT & ~RANDOMIZE_ALL) == 0, RandomizationDefaultHasUnknownBits);

// Exactly one known bit, so a mask or a bit index passed by mistake is caught.
static void AssertSingleFeature(u32 feature)
{
    AGB_ASSERT(feature != 0 && (feature & (feature - 1)) == 0 && (feature & ~RANDOMIZE_ALL) == 0);
}

bool32 IsRandomized(u32 feature)
{
    AssertSingleFeature(feature);
    return (gSaveBlock2Ptr->randomizations & feature) != 0;
}

void SetRandomized(u32 feature, bool32 on)
{
    AssertSingleFeature(feature);
    if (on)
        gSaveBlock2Ptr->randomizations |= feature;
    else
        gSaveBlock2Ptr->randomizations &= ~feature;
}

void InitNewGameRandomization(void)
{
    gSaveBlock2Ptr->randomizations = RANDOMIZATION_NEW_GAME_DEFAULT;
}
