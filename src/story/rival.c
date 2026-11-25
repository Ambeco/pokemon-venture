#include "global.h"
#include "rival.h"
#include "event_data.h"
#include "assertf.h"
#include "constants/flags.h"
#include "constants/global.h"
#include "constants/opponents.h"
#include "constants/rival.h"
#include "constants/vars.h"

/**
 * Which trainer the rival fields in each fixed encounter.
 *
 * Every rival fight asks this instead of naming a trainer, so randomizing starters or the rival later changes only
 * GetRivalGender and the table lookup. The rival is the opposite gender of the player. The table is indexed by the
 * player's starter (VAR_STARTER_MON: 0 Treecko, 1 Torchic, 2 Mudkip), which picks the rival's counter party.
 */

#define NUM_STARTERS 3

static const u16 sRivalTrainers[RIVAL_ENCOUNTER_COUNT][2][NUM_STARTERS] =
{
    [RIVAL_ENCOUNTER_ROUTE_103] =
    {
        [MALE] = {TRAINER_BRENDAN_ROUTE_103_TREECKO, TRAINER_BRENDAN_ROUTE_103_TORCHIC, TRAINER_BRENDAN_ROUTE_103_MUDKIP},
        [FEMALE] = {TRAINER_MAY_ROUTE_103_TREECKO, TRAINER_MAY_ROUTE_103_TORCHIC, TRAINER_MAY_ROUTE_103_MUDKIP},
    },
    [RIVAL_ENCOUNTER_ROUTE_110] =
    {
        [MALE] = {TRAINER_BRENDAN_ROUTE_110_TREECKO, TRAINER_BRENDAN_ROUTE_110_TORCHIC, TRAINER_BRENDAN_ROUTE_110_MUDKIP},
        [FEMALE] = {TRAINER_MAY_ROUTE_110_TREECKO, TRAINER_MAY_ROUTE_110_TORCHIC, TRAINER_MAY_ROUTE_110_MUDKIP},
    },
    [RIVAL_ENCOUNTER_ROUTE_119] =
    {
        [MALE] = {TRAINER_BRENDAN_ROUTE_119_TREECKO, TRAINER_BRENDAN_ROUTE_119_TORCHIC, TRAINER_BRENDAN_ROUTE_119_MUDKIP},
        [FEMALE] = {TRAINER_MAY_ROUTE_119_TREECKO, TRAINER_MAY_ROUTE_119_TORCHIC, TRAINER_MAY_ROUTE_119_MUDKIP},
    },
    [RIVAL_ENCOUNTER_LILYCOVE] =
    {
        [MALE] = {TRAINER_BRENDAN_LILYCOVE_TREECKO, TRAINER_BRENDAN_LILYCOVE_TORCHIC, TRAINER_BRENDAN_LILYCOVE_MUDKIP},
        [FEMALE] = {TRAINER_MAY_LILYCOVE_TREECKO, TRAINER_MAY_LILYCOVE_TORCHIC, TRAINER_MAY_LILYCOVE_MUDKIP},
    },
};

u8 GetRivalGender(void)
{
    return gSaveBlock2Ptr->playerGender == MALE ? FEMALE : MALE;
}

u16 GetRivalTrainer(u8 encounter)
{
    u16 starter = VarGet(VAR_STARTER_MON);

    assertf(encounter < RIVAL_ENCOUNTER_COUNT, "rival encounter %d does not exist (there are %d); did you add it to constants/rival.h and sRivalTrainers?", encounter, RIVAL_ENCOUNTER_COUNT)
    {
        return sRivalTrainers[0][0][0];
    }
    assertf(starter < NUM_STARTERS, "VAR_STARTER_MON is %d but the rival table has %d starters; did you add a starter without a row per rival?", starter, NUM_STARTERS)
    {
        starter = 0;
    }
    return sRivalTrainers[encounter][GetRivalGender()][starter];
}

bool8 HasRivalBeenDefeated(u8 encounter)
{
    return FlagGet(TRAINER_FLAGS_START + GetRivalTrainer(encounter));
}

bool8 ScriptHasRivalBeenDefeated(void)
{
    return HasRivalBeenDefeated(gSpecialVar_0x8004);
}
