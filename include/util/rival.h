#ifndef GUARD_RIVAL_H
#define GUARD_RIVAL_H

#include "constants/rival.h"

// A fixed rival fight (RIVAL_ENCOUNTER_*, constants/rival.h); GetRivalTrainer maps it to the trainer id for the
// current rival and starter.

u8 GetRivalGender(void);
u16 GetRivalTrainer(u8 encounter);
bool8 HasRivalBeenDefeated(u8 encounter);

// Scripts: VAR_0x8004 = the encounter; the result is in VAR_RESULT.
bool8 ScriptHasRivalBeenDefeated(void);

#endif // GUARD_RIVAL_H
