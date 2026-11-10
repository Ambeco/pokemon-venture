#include "global.h"
#include "battle.h"
#include "battle_pyramid.h"
#include "battle_setup.h"
#include "caps.h"
#include "data.h"
#include "dexnav.h"
#include "event_data.h"
#include "evolution_scene.h"
#include "follower_npc.h"
#include "main.h"
#include "menu.h"
#include "money.h"
#include "overworld.h"
#include "palette.h"
#include "party_menu.h"
#include "pokemon.h"
#include "pokemon_storage_system.h"
#include "script.h"
#include "sound.h"
#include "story_mode.h"
#include "string_util.h"
#include "text.h"
#include "text_window.h"
#include "wild_encounter.h"
#include "window.h"

#define POPUP_QUEUE_SIZE 4
#define POPUP_FRAMES     90

enum PopupKind
{
    POPUP_CAUGHT,
    POPUP_ALREADY_HAVE,
    POPUP_NO_ROOM,
};

struct PopupEntry
{
    u16 species;
    u8 kind;
};

static EWRAM_DATA struct PopupEntry sPopupQueue[POPUP_QUEUE_SIZE] = {0};
static EWRAM_DATA u8 sPopupCount = 0;
static EWRAM_DATA u8 sPopupWindowId = 0;
static EWRAM_DATA bool8 sPopupActive = FALSE;
static EWRAM_DATA s16 sPopupTimer = 0;
static EWRAM_DATA s8 sPopupMapGroup = 0;
static EWRAM_DATA s8 sPopupMapNum = 0;
static EWRAM_DATA u8 sTriedEvolving = 0;
static EWRAM_DATA u8 sStartLevel[PARTY_SIZE] = {0};

static const struct WindowTemplate sPopupWindowTemplate = {
    .bg = 0,
    .tilemapLeft = 1,
    .tilemapTop = 16,
    .width = 14,
    .height = 2,
    .paletteNum = 15,
    .baseBlock = 0x008,
};

static const u8 sText_Caught[]      = _("Caught {STR_VAR_1}!");
static const u8 sText_AlreadyHave[] = _("Have better {STR_VAR_1}");
static const u8 sText_NoRoom[]      = _("No room: {STR_VAR_1}");

bool32 IsStoryModeOn(void)
{
    return FlagGet(FLAG_VENTURE_STORY_MODE);
}

// Classes whose fights are scripted story beats. Brendan, May, Wally and Steven are all "Rival".
bool32 StoryMode_IsBossTrainerClass(u32 trainerClass)
{
    switch (trainerClass)
    {
    case TRAINER_CLASS_LEADER:
    case TRAINER_CLASS_ELITE_FOUR:
    case TRAINER_CLASS_CHAMPION:
    case TRAINER_CLASS_RIVAL:
    case TRAINER_CLASS_AQUA_LEADER:
    case TRAINER_CLASS_MAGMA_LEADER:
    case TRAINER_CLASS_AQUA_ADMIN:
    case TRAINER_CLASS_MAGMA_ADMIN:
    case TRAINER_CLASS_RIVAL_EARLY_FRLG:
    case TRAINER_CLASS_RIVAL_LATE_FRLG:
    case TRAINER_CLASS_LEADER_FRLG:
    case TRAINER_CLASS_ELITE_FOUR_FRLG:
    case TRAINER_CLASS_CHAMPION_FRLG:
        return TRUE;
    default:
        return FALSE;
    }
}

// Only a plain single or double wild battle; anything else (partner, Pyramid, legendary, ...) is real.
bool32 StoryMode_CanSkipWildBattleType(u32 battleTypeFlags)
{
    return (battleTypeFlags & ~BATTLE_TYPE_DOUBLE) == 0;
}

bool32 StoryMode_CanSkipTrainerBattleType(u32 battleTypeFlags)
{
    return (battleTypeFlags & BATTLE_TYPE_TRAINER)
        && (battleTypeFlags & ~(BATTLE_TYPE_TRAINER | BATTLE_TYPE_DOUBLE | BATTLE_TYPE_TWO_OPPONENTS)) == 0;
}

u32 StoryMode_CalcExp(u32 expYield, u32 level, bool32 isTrainer)
{
    u32 exp = expYield * level / 5;

    if (isTrainer)
        exp = exp * 150 / 100;
    return exp;
}

u32 StoryMode_CalcPrizeMoney(u32 classMoney, u32 lastMonLevel, bool32 isDouble)
{
    u32 money = 4 * lastMonLevel * (classMoney ? classMoney : 5);

    return isDouble ? money * 2 : money;
}

// Never lets damage take the last HP, so a real (scripted) fight cannot be lost.
s32 StoryMode_ClampPlayerDamage(s32 damage, u32 hp)
{
    if (damage <= 0)
        return damage;
    if (hp <= 1)
        return 0;
    return damage < (s32)hp ? damage : (s32)hp - 1;
}

bool32 StoryMode_IsCatchRedundant(u32 existingIvTotal, u32 newIvTotal)
{
    return existingIvTotal >= newIvTotal;
}

static u32 GetBoxMonIvTotal(struct BoxPokemon *boxMon)
{
    return GetBoxMonData(boxMon, MON_DATA_HP_IV)
         + GetBoxMonData(boxMon, MON_DATA_ATK_IV)
         + GetBoxMonData(boxMon, MON_DATA_DEF_IV)
         + GetBoxMonData(boxMon, MON_DATA_SPEED_IV)
         + GetBoxMonData(boxMon, MON_DATA_SPATK_IV)
         + GetBoxMonData(boxMon, MON_DATA_SPDEF_IV);
}

static bool32 IsSameKind(struct BoxPokemon *boxMon, enum Species species, bool32 shiny)
{
    return GetBoxMonData(boxMon, MON_DATA_SPECIES_OR_EGG) == species
        && (GetBoxMonData(boxMon, MON_DATA_IS_SHINY) != 0) == shiny;
}

// Best IV total among stored copies of the same species, form and shininess, or -1 if none.
static s32 FindBestStoredIvTotal(enum Species species, bool32 shiny)
{
    s32 best = -1;
    u32 i, box;

    for (i = 0; i < PARTY_SIZE; i++)
    {
        struct BoxPokemon *boxMon = &gParties[B_TRAINER_PLAYER][i].box;

        if (IsSameKind(boxMon, species, shiny) && (s32)GetBoxMonIvTotal(boxMon) > best)
            best = GetBoxMonIvTotal(boxMon);
    }
    for (box = 0; box < TOTAL_BOXES_COUNT; box++)
    {
        for (i = 0; i < IN_BOX_COUNT; i++)
        {
            struct BoxPokemon *boxMon = GetBoxedMonPtr(box, i);

            if (IsSameKind(boxMon, species, shiny) && (s32)GetBoxMonIvTotal(boxMon) > best)
                best = GetBoxMonIvTotal(boxMon);
        }
    }
    return best;
}

static void QueuePopup(enum Species species, enum PopupKind kind)
{
    if (sPopupCount >= POPUP_QUEUE_SIZE)
        return;
    sPopupQueue[sPopupCount].species = species;
    sPopupQueue[sPopupCount].kind = kind;
    sPopupCount++;
}

static void GiveWildMon(struct Pokemon *mon)
{
    enum Species species = GetMonData(mon, MON_DATA_SPECIES);
    bool32 shiny = GetMonData(mon, MON_DATA_IS_SHINY) != 0;
    s32 stored = FindBestStoredIvTotal(species, shiny);
    u32 ball = BALL_POKE;

    HandleSetPokedexFlagFromMon(mon, FLAG_SET_SEEN);
    if (stored >= 0 && StoryMode_IsCatchRedundant(stored, GetBoxMonIvTotal(&mon->box)))
    {
        // Still counts as caught for the Pokédex, as the player already owns the species.
        QueuePopup(species, POPUP_ALREADY_HAVE);
        return;
    }

    SetMonData(mon, MON_DATA_POKEBALL, &ball);
    if (GiveCapturedMonToPlayer(mon) == MON_CANT_GIVE)
    {
        QueuePopup(species, POPUP_NO_ROOM);
        return;
    }
    HandleSetPokedexFlagFromMon(mon, FLAG_SET_CAUGHT);
    QueuePopup(species, POPUP_CAUGHT);
}

// Applies the exp and level-up (stats, HP). Moves and evolution are left to the Rare Candy
// level-up menu, which runs afterwards for each slot recorded in sStartLevel.
static void GrantExp(u32 slot, u32 gain)
{
    struct Pokemon *mon = &gParties[B_TRAINER_PLAYER][slot];
    enum Species species = GetMonData(mon, MON_DATA_SPECIES);
    u32 level = GetMonData(mon, MON_DATA_LEVEL);
    u32 exp, cap, newLevel, oldMaxHp, hp;

    if (level >= MAX_LEVEL || gain == 0)
        return;

    cap = gExperienceTables[gSpeciesInfo[species].growthRate][MAX_LEVEL];
    exp = GetMonData(mon, MON_DATA_EXP) + gain;
    if (exp > cap)
        exp = cap;
    SetMonData(mon, MON_DATA_EXP, &exp);

    newLevel = GetLevelFromMonExp(mon);
    if (newLevel <= level)
        return;

    oldMaxHp = GetMonData(mon, MON_DATA_MAX_HP);
    SetMonData(mon, MON_DATA_LEVEL, &newLevel);
    CalculateMonStats(mon);
    hp = GetMonData(mon, MON_DATA_HP) + GetMonData(mon, MON_DATA_MAX_HP) - oldMaxHp;
    SetMonData(mon, MON_DATA_HP, &hp);
    AdjustFriendship(mon, FRIENDSHIP_EVENT_GROW_LEVEL);
    if (sStartLevel[slot] == 0)
        sStartLevel[slot] = level;
}

// Gives the whole healthy party the exp and EVs for one defeated Pokémon.
static void RewardParty(enum Species defeated, u32 level, bool32 isTrainer)
{
    u32 i;
    u32 exp = StoryMode_CalcExp(gSpeciesInfo[defeated].expYield, level, isTrainer);

    for (i = 0; i < PARTY_SIZE; i++)
    {
        struct Pokemon *mon = &gParties[B_TRAINER_PLAYER][i];
        enum Species species = GetMonData(mon, MON_DATA_SPECIES_OR_EGG);

        if (species == SPECIES_NONE || species == SPECIES_EGG || GetMonData(mon, MON_DATA_HP) == 0)
            continue;
        MonGainEVs(mon, defeated);
        GrantExp(i, GetSoftLevelCapExpValue(GetMonData(mon, MON_DATA_LEVEL), exp));
    }
}

// Runs the level-up menu for each slot that gained levels, then any remaining evolutions
// (mirroring TrySpecialOverworldEvo), then hands back to the battle's end callback. The menu and
// evolution scenes return here through their exit callbacks.
static void CB2_StoryModeEvolve(void)
{
    u32 i;

    for (i = 0; i < PARTY_SIZE; i++)
    {
        u8 start = sStartLevel[i];

        if (start == 0)
            continue;
        sStartLevel[i] = 0;
        StoryMode_ShowLevelUpMenu(i, start, GetMonData(&gParties[B_TRAINER_PLAYER][i], MON_DATA_LEVEL), CB2_StoryModeEvolve);
        return;
    }

    for (i = 0; i < PARTY_SIZE; i++)
    {
        bool32 canStopEvo = TRUE;
        struct Pokemon *mon = &gParties[B_TRAINER_PLAYER][i];
        enum Species target;

        if (sTriedEvolving & (1u << i))
            continue;
        sTriedEvolving |= 1u << i;
        target = GetEvolutionTargetSpecies(mon, EVO_MODE_NORMAL, ITEM_NONE, NULL, &canStopEvo, CHECK_EVO);
        if (target == SPECIES_NONE)
            continue;

        GetEvolutionTargetSpecies(mon, EVO_MODE_NORMAL, ITEM_NONE, NULL, &canStopEvo, DO_EVO);
        gCB2_AfterEvolution = CB2_StoryModeEvolve;
        if (gMain.callback2 == CB2_StoryModeEvolve)
            EvolutionScene(mon, target, canStopEvo, i);
        else
            BeginEvolutionScene(mon, target, canStopEvo, i);
        return;
    }
    sTriedEvolving = 0;
    SetMainCallback2(gMain.savedCallback);
}

static void FinishSkippedBattle(void)
{
    gSpecialVar_Result = gBattleOutcome = B_OUTCOME_WON;
    ZeroEnemyPartyMons();
    sTriedEvolving = 0;
    CB2_StoryModeEvolve();
}

bool32 StoryMode_TrySkipWildBattle(void)
{
    u32 i;

    memset(sStartLevel, 0, sizeof(sStartLevel));
    if (!IsStoryModeOn() || !StoryMode_CanSkipWildBattleType(gBattleTypeFlags))
        return FALSE;
    if (CurrentBattlePyramidLocation() != PYRAMID_LOCATION_NONE || IsNPCFollowerWildBattle())
        return FALSE;

    for (i = 0; i < 2; i++)
    {
        struct Pokemon *mon = &gParties[B_TRAINER_OPPONENT_A][i];
        enum Species species = GetMonData(mon, MON_DATA_SPECIES);

        if (species == SPECIES_NONE || (i == 1 && !(gBattleTypeFlags & BATTLE_TYPE_DOUBLE)))
            continue;
        RewardParty(species, GetMonData(mon, MON_DATA_LEVEL), FALSE);
        GiveWildMon(mon);
    }
    gIsFishingEncounter = FALSE;
    gIsSurfingEncounter = FALSE;
    gDexNavSpecies = SPECIES_NONE;
    FinishSkippedBattle();
    return TRUE;
}

// A trainer's party read straight from its data (no battle state is needed, unlike building mons).
static u32 GetTrainerRewardParty(u16 trainerId, const struct TrainerMon **party, bool32 halfTeam)
{
    const struct Trainer *trainer = GetTrainerStructFromId(trainerId);
    u32 count = trainer->partySize;

    *party = trainer->party;
    if (trainer->overrideTrainer)
    {
        const struct Trainer *original = GetTrainerStructFromId(trainer->overrideTrainer);

        *party = original->party;
        if (count == 0)
            count = original->partySize;
    }
    if (halfTeam && count > PARTY_SIZE / 2)
        count = PARTY_SIZE / 2;
    return count;
}

static u32 RewardTrainer(u16 trainerId, bool32 halfTeam)
{
    const struct TrainerMon *party;
    u32 count = GetTrainerRewardParty(trainerId, &party, halfTeam);
    u32 i;

    for (i = 0; i < count; i++)
        RewardParty(party[i].species, party[i].lvl, TRUE);
    if (count == 0)
        return 0;
    return StoryMode_CalcPrizeMoney(gTrainerClasses[GetTrainerClassFromId(trainerId)].money,
                                    party[count - 1].lvl,
                                    FALSE);
}

bool32 StoryMode_TrySkipTrainerBattle(void)
{
    u16 opponentA = TRAINER_BATTLE_PARAM.opponentA;
    u16 opponentB = TRAINER_BATTLE_PARAM.opponentB;
    bool32 two = (gBattleTypeFlags & BATTLE_TYPE_TWO_OPPONENTS) != 0;
    u32 money;

    memset(sStartLevel, 0, sizeof(sStartLevel));
    if (!IsStoryModeOn() || !StoryMode_CanSkipTrainerBattleType(gBattleTypeFlags))
        return FALSE;
    if (opponentA == TRAINER_SECRET_BASE || (two && opponentB == TRAINER_SECRET_BASE))
        return FALSE;
    if (StoryMode_IsBossTrainerClass(GetTrainerClassFromId(opponentA))
     || (two && StoryMode_IsBossTrainerClass(GetTrainerClassFromId(opponentB))))
        return FALSE;

    money = RewardTrainer(opponentA, two);
    if (two)
        money += RewardTrainer(opponentB, two);
    else if ((gBattleTypeFlags & BATTLE_TYPE_DOUBLE) && money != 0)
        money *= 2;
    AddMoney(&gSaveBlock1Ptr->money, money);
    FinishSkippedBattle();
    return TRUE;
}

static void ClosePopup(void)
{
    if (sPopupMapGroup == gSaveBlock1Ptr->location.mapGroup && sPopupMapNum == gSaveBlock1Ptr->location.mapNum)
    {
        ClearStdWindowAndFrameToTransparent(sPopupWindowId, TRUE);
        RemoveWindow(sPopupWindowId);
    }
    // After a map change the windows were already reset, so removing the id would corrupt another window.
    sPopupActive = FALSE;
}

static void ShowPopup(const struct PopupEntry *entry)
{
    const u8 *text = sText_Caught;

    if (entry->kind == POPUP_ALREADY_HAVE)
        text = sText_AlreadyHave;
    else if (entry->kind == POPUP_NO_ROOM)
        text = sText_NoRoom;

    StringCopy(gStringVar1, GetSpeciesName(entry->species));
    StringExpandPlaceholders(gStringVar4, text);
    sPopupWindowId = AddWindow(&sPopupWindowTemplate);
    LoadUserWindowBorderGfx(sPopupWindowId, 0x21D, BG_PLTT_ID(13));
    DrawStdFrameWithCustomTileAndPalette(sPopupWindowId, FALSE, 0x21D, 13);
    AddTextPrinterParameterized(sPopupWindowId, FONT_NORMAL, gStringVar4, 0, 2, 0xFF, NULL);
    PutWindowTilemap(sPopupWindowId);
    CopyWindowToVram(sPopupWindowId, COPYWIN_FULL);
    sPopupActive = TRUE;
    sPopupTimer = POPUP_FRAMES;
    sPopupMapGroup = gSaveBlock1Ptr->location.mapGroup;
    sPopupMapNum = gSaveBlock1Ptr->location.mapNum;
}

void StoryMode_Tick(void)
{
    u32 i;
    struct PopupEntry entry;

    if (sPopupActive)
    {
        if (--sPopupTimer <= 0)
            ClosePopup();
        return;
    }
    if (sPopupCount == 0 || gPaletteFade.active || ArePlayerFieldControlsLocked())
        return;

    entry = sPopupQueue[0];
    for (i = 1; i < sPopupCount; i++)
        sPopupQueue[i - 1] = sPopupQueue[i];
    sPopupCount--;
    ShowPopup(&entry);
}
