#ifndef GUARD_STORY_MODE_H
#define GUARD_STORY_MODE_H

/**
 * Story mode: a debug toggle for quickly playing through the story.
 *
 * While FLAG_VENTURE_STORY_MODE is set, ordinary wild and trainer battles are skipped as wins:
 * the party gets experience, EVs, level-ups, moves and evolutions, trainers pay prize money, and a
 * skipped wild mon is given to the player unless storage already holds a copy of the same species
 * (forms are separate species), shininess and an equal or better IV total. A corner popup reports
 * each catch. Scripted fights stay real so their scripts get tested: boss trainer classes, double
 * partner battles, Pyramid, Trainer Hill, legendary, roamer, Safari, ghost and tutorial battles.
 * The player's side cannot lose its last HP in a real fight (see StoryMode_ClampPlayerDamage).
 *
 * Quirks: every healthy party member gets the full experience (the Gen 6 Exp. Share model with no
 * sent-in split) and no level-difference scaling. A full learnset replaces the first move when four
 * are known, as there is no field prompt. A trainer's prize uses the last party Pokémon's level and
 * never doubles for Amulet Coin or Happy Hour. A trainer's party is read from its data without
 * pools, so a pooled trainer pays out for its first partySize entries. The corner popup waits for
 * the field to be visible.
 */

#include "constants/battle.h"
#include "constants/trainers.h"

bool32 IsStoryModeOn(void);

bool32 StoryMode_IsBossTrainerClass(u32 trainerClass);
bool32 StoryMode_CanSkipWildBattleType(u32 battleTypeFlags);
bool32 StoryMode_CanSkipTrainerBattleType(u32 battleTypeFlags);
u32 StoryMode_CalcExp(u32 expYield, u32 level, bool32 isTrainer);
u32 StoryMode_CalcPrizeMoney(u32 classMoney, u32 lastMonLevel, bool32 isDouble);
s32 StoryMode_ClampPlayerDamage(s32 damage, u32 hp);

// A copy at least as good already exists, so the new catch is not kept.
bool32 StoryMode_IsCatchRedundant(u32 existingIvTotal, u32 newIvTotal);

// Wild: returns TRUE if it took over the battle and set the end callback. Call after the usual
// setup (savedCallback and gBattleTypeFlags). Trainer: same.
bool32 StoryMode_TrySkipWildBattle(void);
bool32 StoryMode_TrySkipTrainerBattle(void);

// Called every overworld frame; shows queued catch popups once the field is visible.
void StoryMode_Tick(void);

#endif // GUARD_STORY_MODE_H
