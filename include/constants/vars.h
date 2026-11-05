#ifndef GUARD_CONSTANTS_VARS_H
#define GUARD_CONSTANTS_VARS_H

#include "constants/vars_frlg.h"

#define VARS_START 0x4000

// temporary vars
// The first 0x10 vars are temporary--they are cleared every time a map is loaded.
#define TEMP_VARS_START            0x4000
#define VAR_TEMP_0                 (TEMP_VARS_START + 0x0)
#define VAR_TEMP_1                 (TEMP_VARS_START + 0x1)
#define VAR_TEMP_2                 (TEMP_VARS_START + 0x2)
#define VAR_TEMP_3                 (TEMP_VARS_START + 0x3) // Note: Used when the player checks a TV
#define VAR_TEMP_4                 (TEMP_VARS_START + 0x4)
#define VAR_TEMP_5                 (TEMP_VARS_START + 0x5)
#define VAR_TEMP_6                 (TEMP_VARS_START + 0x6)
#define VAR_TEMP_7                 (TEMP_VARS_START + 0x7)
#define VAR_TEMP_8                 (TEMP_VARS_START + 0x8)
#define VAR_TEMP_9                 (TEMP_VARS_START + 0x9)
#define VAR_TEMP_A                 (TEMP_VARS_START + 0xA)
#define VAR_TEMP_B                 (TEMP_VARS_START + 0xB)
#define VAR_TEMP_C                 (TEMP_VARS_START + 0xC)
#define VAR_TEMP_D                 (TEMP_VARS_START + 0xD)
#define VAR_TEMP_E                 (TEMP_VARS_START + 0xE)
#define VAR_TEMP_F                 (TEMP_VARS_START + 0xF)
#define TEMP_VARS_END              VAR_TEMP_F
#define NUM_TEMP_VARS              (TEMP_VARS_END - TEMP_VARS_START + 1)

// object gfx id vars
// These 0x10 vars are used to dynamically control a map object's sprite.
// For example, the rival's sprite id is dynamically set based on the player's gender.
// See VarGetObjectEventGraphicsId().
#define VAR_OBJ_GFX_ID_0           0x4010
#define VAR_OBJ_GFX_ID_1           0x4011
#define VAR_OBJ_GFX_ID_2           0x4012
#define VAR_OBJ_GFX_ID_3           0x4013
#define VAR_OBJ_GFX_ID_4           0x4014
#define VAR_OBJ_GFX_ID_5           0x4015
#define VAR_OBJ_GFX_ID_6           0x4016
#define VAR_OBJ_GFX_ID_7           0x4017
#define VAR_OBJ_GFX_ID_8           0x4018
#define VAR_OBJ_GFX_ID_9           0x4019
#define VAR_OBJ_GFX_ID_A           0x401A
#define VAR_OBJ_GFX_ID_B           0x401B
#define VAR_OBJ_GFX_ID_C           0x401C
#define VAR_OBJ_GFX_ID_D           0x401D
#define VAR_OBJ_GFX_ID_E           0x401E
#define VAR_OBJ_GFX_ID_F           0x401F

// Persistent vars are split into four blocks (see documents/vars.md and design.md, "Story state storage"):
// county slices, a reserve, minigame/facility vars and engine vars. test/vars.c checks that they do not overlap.
// Pairs read as one 32-bit value or one struct (FANCLUB_*, MIRAGE_RND_*, CYCLING_ROAD_RECORD_TIME_*,
// POKELOT_RND*, SECRET_BASE_*_TV_FLAGS) must stay adjacent and in order.
#define PERSISTENT_VARS_START      0x4020

// county slices: VARS_PER_COUNTY whole vars per county, read by no other county
#define COUNTY_VARS_START          PERSISTENT_VARS_START
#define NUM_COUNTY_VAR_SLICES      24
#define VARS_PER_COUNTY            4
#define COUNTY_VARS_COUNT          (NUM_COUNTY_VAR_SLICES * VARS_PER_COUNTY)
#define COUNTY_VARS_END            (COUNTY_VARS_START + COUNTY_VARS_COUNT - 1)

// reserve: main-story-index, optional-story vars, spare
#define RESERVED_VARS_START        (COUNTY_VARS_END + 1)
#define RESERVED_VARS_COUNT        22
#define RESERVED_VARS_END          (RESERVED_VARS_START + RESERVED_VARS_COUNT - 1)

#define MINIGAME_VARS_START        (RESERVED_VARS_END + 1)
#define MINIGAME_VARS_COUNT        65
#define MINIGAME_VARS_END          (MINIGAME_VARS_START + MINIGAME_VARS_COUNT - 1)

#define ENGINE_VARS_START          (MINIGAME_VARS_END + 1)
#define ENGINE_VARS_COUNT          41
#define ENGINE_VARS_END            (ENGINE_VARS_START + ENGINE_VARS_COUNT - 1)

// legacy: Emerald story vars not yet retired. They overlay the last county slices and the reserve
// until the main arc and subplots are ported; retiring one frees its address for county use.
#define LEGACY_VARS_COUNT          61
#define LEGACY_VARS_START          (RESERVED_VARS_END + 1 - LEGACY_VARS_COUNT)
#define LEGACY_VARS_END            RESERVED_VARS_END

// minigame and facility vars
#define VAR_FRONTIER_MANIAC_FACILITY                     (MINIGAME_VARS_START + 0x0)
#define VAR_FRONTIER_GAMBLER_CHALLENGE                   (MINIGAME_VARS_START + 0x1)
#define VAR_FRONTIER_GAMBLER_SET_CHALLENGE               (MINIGAME_VARS_START + 0x2)
#define VAR_FRONTIER_GAMBLER_AMOUNT_BET                  (MINIGAME_VARS_START + 0x3)
#define VAR_FRONTIER_GAMBLER_STATE                       (MINIGAME_VARS_START + 0x4)
#define VAR_FRONTIER_BATTLE_MODE                         (MINIGAME_VARS_START + 0x5)
#define VAR_FRONTIER_FACILITY                            (MINIGAME_VARS_START + 0x6)
#define VAR_HAS_ENTERED_BATTLE_FRONTIER                  (MINIGAME_VARS_START + 0x7) // Var is used like a flag.
#define VAR_TRAINER_HILL_IS_ACTIVE                       (MINIGAME_VARS_START + 0x8)
#define VAR_BRAVO_TRAINER_BATTLE_TOWER_ON                (MINIGAME_VARS_START + 0x9)

#define VAR_TRICK_HOUSE_LEVEL                            (MINIGAME_VARS_START + 0xA)
#define VAR_TRICK_HOUSE_BEING_WATCHED_STATE              (MINIGAME_VARS_START + 0xB)
#define VAR_TRICK_HOUSE_FOUND_TRICK_MASTER               (MINIGAME_VARS_START + 0xC)
#define VAR_TRICK_HOUSE_ENTRANCE_STATE                   (MINIGAME_VARS_START + 0xD)
#define VAR_TRICK_HOUSE_PUZZLE_1_STATE                   (MINIGAME_VARS_START + 0xE)
#define VAR_TRICK_HOUSE_PUZZLE_2_STATE                   (MINIGAME_VARS_START + 0xF)
#define VAR_TRICK_HOUSE_PUZZLE_3_STATE                   (MINIGAME_VARS_START + 0x10)
#define VAR_TRICK_HOUSE_PUZZLE_4_STATE                   (MINIGAME_VARS_START + 0x11)
#define VAR_TRICK_HOUSE_PUZZLE_5_STATE                   (MINIGAME_VARS_START + 0x12)
#define VAR_TRICK_HOUSE_PUZZLE_6_STATE                   (MINIGAME_VARS_START + 0x13)
#define VAR_TRICK_HOUSE_PUZZLE_7_STATE                   (MINIGAME_VARS_START + 0x14)
#define VAR_TRICK_HOUSE_PUZZLE_8_STATE                   (MINIGAME_VARS_START + 0x15)
#define VAR_TRICK_HOUSE_ENTER_FROM_CORRIDOR              (MINIGAME_VARS_START + 0x16)
#define VAR_TRICK_HOUSE_PRIZE_PICKUP                     (MINIGAME_VARS_START + 0x17)

#define VAR_CONTEST_HALL_STATE                           (MINIGAME_VARS_START + 0x18)
#define VAR_CONTEST_TYPE                                 (MINIGAME_VARS_START + 0x19)
#define VAR_CONTEST_PRIZE_PICKUP                         (MINIGAME_VARS_START + 0x1A)
#define VAR_LILYCOVE_CONTEST_LOBBY_STATE                 (MINIGAME_VARS_START + 0x1B)

#define VAR_POKELOT_PRIZE_ITEM                           (MINIGAME_VARS_START + 0x1C)
#define VAR_POKELOT_PRIZE_PLACE                          (MINIGAME_VARS_START + 0x1D)
#define VAR_POKELOT_RND1                                 (MINIGAME_VARS_START + 0x1E)
#define VAR_POKELOT_RND2                                 (MINIGAME_VARS_START + 0x1F)

#define VAR_SEEDOT_SIZE_RECORD                           (MINIGAME_VARS_START + 0x20)
#define VAR_LOTAD_SIZE_RECORD                            (MINIGAME_VARS_START + 0x21)
#define VAR_CYCLING_ROAD_RECORD_COLLISIONS               (MINIGAME_VARS_START + 0x22)
#define VAR_CYCLING_ROAD_RECORD_TIME_L                   (MINIGAME_VARS_START + 0x23)
#define VAR_CYCLING_ROAD_RECORD_TIME_H                   (MINIGAME_VARS_START + 0x24)
#define VAR_CYCLING_CHALLENGE_STATE                      (MINIGAME_VARS_START + 0x25)

#define VAR_FANCLUB_FAN_COUNTER                          (MINIGAME_VARS_START + 0x26)
#define VAR_FANCLUB_LOSE_FAN_TIMER                       (MINIGAME_VARS_START + 0x27)
#define VAR_LILYCOVE_FAN_CLUB_STATE                      (MINIGAME_VARS_START + 0x28)
#define VAR_SLATEPORT_FAN_CLUB_STATE                     (MINIGAME_VARS_START + 0x29)

#define VAR_CRUISE_STEP_COUNT                            (MINIGAME_VARS_START + 0x2A)
#define VAR_SS_TIDAL_STATE                               (MINIGAME_VARS_START + 0x2B)

#define VAR_DEOXYS_ROCK_STEP_COUNT                       (MINIGAME_VARS_START + 0x2C)
#define VAR_DEOXYS_ROCK_LEVEL                            (MINIGAME_VARS_START + 0x2D)
#define VAR_FARAWAY_ISLAND_STEP_COUNTER                  (MINIGAME_VARS_START + 0x2E)
#define VAR_REGICE_STEPS_1                               (MINIGAME_VARS_START + 0x2F)
#define VAR_REGICE_STEPS_2                               (MINIGAME_VARS_START + 0x30)
#define VAR_REGICE_STEPS_3                               (MINIGAME_VARS_START + 0x31)
#define VAR_ALTERING_CAVE_WILD_SET                       (MINIGAME_VARS_START + 0x32)

#define VAR_MIRAGE_RND_H                                 (MINIGAME_VARS_START + 0x33)
#define VAR_MIRAGE_RND_L                                 (MINIGAME_VARS_START + 0x34)
#define VAR_MIRAGE_TOWER_STATE                           (MINIGAME_VARS_START + 0x35)

#define VAR_ASH_GATHER_COUNT                             (MINIGAME_VARS_START + 0x36)
#define VAR_JAGGED_PASS_ASH_WEATHER                      (MINIGAME_VARS_START + 0x37)
#define VAR_GLASS_WORKSHOP_STATE                         (MINIGAME_VARS_START + 0x38)

#define VAR_FOSSIL_RESURRECTION_STATE                    (MINIGAME_VARS_START + 0x39)
#define VAR_WHICH_FOSSIL_REVIVED                         (MINIGAME_VARS_START + 0x3A)
#define VAR_FOSSIL_MANIAC_STATE                          (MINIGAME_VARS_START + 0x3B)
#define VAR_PACIFIDLOG_TM_RECEIVED_DAY                   (MINIGAME_VARS_START + 0x3C)

#define VAR_DEPT_STORE_FLOOR                             (MINIGAME_VARS_START + 0x3D)
#define VAR_SAFARI_ZONE_STATE                            (MINIGAME_VARS_START + 0x3E) // 0: In or out of SZ, 1: Player exiting SZ, 2: Player entering SZ
#define VAR_CABLE_CAR_STATION_STATE                      (MINIGAME_VARS_START + 0x3F)
#define VAR_WEATHER_INSTITUTE_STATE                      (MINIGAME_VARS_START + 0x40)

// engine and system vars
#define VAR_REPEL_STEP_COUNT                             (ENGINE_VARS_START + 0x0)
#define VAR_ICE_STEP_COUNT                               (ENGINE_VARS_START + 0x1)
#define VAR_FRIENDSHIP_STEP_COUNTER                      (ENGINE_VARS_START + 0x2)
#define VAR_POISON_STEP_COUNTER                          (ENGINE_VARS_START + 0x3)
#define VAR_WONDER_NEWS_STEP_COUNTER                     (ENGINE_VARS_START + 0x4)

#define VAR_DAYS                                         (ENGINE_VARS_START + 0x5)
#define VAR_NATIONAL_DEX                                 (ENGINE_VARS_START + 0x6)
#define VAR_STARTER_MON                                  (ENGINE_VARS_START + 0x7) // 0=Treecko, 1=Torchic, 2=Mudkip
#define VAR_ROAMER_POKEMON                               (ENGINE_VARS_START + 0x8) // 0 = Latias, 1 = Latios

#define VAR_DAILY_SLOTS                                  (ENGINE_VARS_START + 0x9)
#define VAR_DAILY_WILDS                                  (ENGINE_VARS_START + 0xA)
#define VAR_DAILY_BLENDER                                (ENGINE_VARS_START + 0xB)
#define VAR_DAILY_PLANTED_BERRIES                        (ENGINE_VARS_START + 0xC)
#define VAR_DAILY_PICKED_BERRIES                         (ENGINE_VARS_START + 0xD)
#define VAR_DAILY_ROULETTE                               (ENGINE_VARS_START + 0xE)
#define VAR_DAILY_BP                                     (ENGINE_VARS_START + 0xF)

#define VAR_SECRET_BASE_MAP                              (ENGINE_VARS_START + 0x10)
#define VAR_CURRENT_SECRET_BASE                          (ENGINE_VARS_START + 0x11) // was probably allocated for VAR_FALLARBOR_TOWN_STATE at one point
#define VAR_SECRET_BASE_INITIALIZED                      (ENGINE_VARS_START + 0x12)
#define VAR_INIT_SECRET_BASE                             (ENGINE_VARS_START + 0x13)
#define VAR_SECRET_BASE_STEP_COUNTER                     (ENGINE_VARS_START + 0x14) // Used by Secret Base TV programs
#define VAR_SECRET_BASE_LAST_ITEM_USED                   (ENGINE_VARS_START + 0x15) // Used by Secret Base TV programs
#define VAR_SECRET_BASE_LOW_TV_FLAGS                     (ENGINE_VARS_START + 0x16) // Used by Secret Base TV programs
#define VAR_SECRET_BASE_HIGH_TV_FLAGS                    (ENGINE_VARS_START + 0x17) // Used by Secret Base TV programs
#define VAR_SECRET_BASE_IS_NOT_LOCAL                     (ENGINE_VARS_START + 0x18) // Set to TRUE while in another player's secret base.

#define VAR_RESET_RTC_ENABLE                             (ENGINE_VARS_START + 0x19)
#define VAR_ENIGMA_BERRY_AVAILABLE                       (ENGINE_VARS_START + 0x1A)
#define VAR_PC_BOX_TO_SEND_MON                           (ENGINE_VARS_START + 0x1B)

#define VAR_ABNORMAL_WEATHER_LOCATION                    (ENGINE_VARS_START + 0x1C)
#define VAR_ABNORMAL_WEATHER_STEP_COUNTER                (ENGINE_VARS_START + 0x1D)
#define VAR_SHOULD_END_ABNORMAL_WEATHER                  (ENGINE_VARS_START + 0x1E)

#define VAR_WALLY_CALL_STEP_COUNTER                      (ENGINE_VARS_START + 0x1F)
#define VAR_SCOTT_FORTREE_CALL_STEP_COUNTER              (ENGINE_VARS_START + 0x20)
#define VAR_ROXANNE_CALL_STEP_COUNTER                    (ENGINE_VARS_START + 0x21)
#define VAR_SCOTT_BF_CALL_STEP_COUNTER                   (ENGINE_VARS_START + 0x22)
#define VAR_RIVAL_RAYQUAZA_CALL_STEP_COUNTER             (ENGINE_VARS_START + 0x23)

#define VAR_GIFT_PICHU_SLOT                              (ENGINE_VARS_START + 0x24)
#define VAR_DEX_UPGRADE_JOHTO_STARTER_STATE              (ENGINE_VARS_START + 0x25)
#define VAR_REGISTER_BIRCH_STATE                         (ENGINE_VARS_START + 0x26)
#define VAR_CABLE_CLUB_STATE                             (ENGINE_VARS_START + 0x27)
#define VAR_CABLE_CLUB_TUTORIAL_STATE                    (ENGINE_VARS_START + 0x28)

// legacy Emerald story vars (to be retired)
#define VAR_DISTRIBUTE_EON_TICKET                        (LEGACY_VARS_START + 0x0) // This var is read and written, but is always zero. The only way to obtain the Eon Ticket in Emerald is via Record Mixing
#define VAR_BIRCH_STATE                                  (LEGACY_VARS_START + 0x1)
#define VAR_LITTLEROOT_TOWN_STATE                        (LEGACY_VARS_START + 0x2)
#define VAR_OLDALE_TOWN_STATE                            (LEGACY_VARS_START + 0x3)
#define VAR_LAVARIDGE_TOWN_STATE                         (LEGACY_VARS_START + 0x4)
#define VAR_PETALBURG_CITY_STATE                         (LEGACY_VARS_START + 0x5)
#define VAR_SLATEPORT_CITY_STATE                         (LEGACY_VARS_START + 0x6)
#define VAR_RUSTBORO_CITY_STATE                          (LEGACY_VARS_START + 0x7)
#define VAR_MOSSDEEP_CITY_STATE                          (LEGACY_VARS_START + 0x8)
#define VAR_SOOTOPOLIS_CITY_STATE                        (LEGACY_VARS_START + 0x9)
#define VAR_ROUTE101_STATE                               (LEGACY_VARS_START + 0xA)
#define VAR_ROUTE104_STATE                               (LEGACY_VARS_START + 0xB)
#define VAR_ROUTE110_STATE                               (LEGACY_VARS_START + 0xC)
#define VAR_ROUTE116_STATE                               (LEGACY_VARS_START + 0xD)
#define VAR_ROUTE118_STATE                               (LEGACY_VARS_START + 0xE)
#define VAR_ROUTE119_STATE                               (LEGACY_VARS_START + 0xF)
#define VAR_ROUTE121_STATE                               (LEGACY_VARS_START + 0x10)
#define VAR_ROUTE128_STATE                               (LEGACY_VARS_START + 0x11)
#define VAR_LITTLEROOT_HOUSES_STATE_MAY                  (LEGACY_VARS_START + 0x12)
#define VAR_BIRCH_LAB_STATE                              (LEGACY_VARS_START + 0x13)
#define VAR_PETALBURG_GYM_STATE                          (LEGACY_VARS_START + 0x14) // 0-1: Wally tutorial, 2-6: 0-4 badges, 7: Defeated Norman, 8: Rematch Norman
#define VAR_LITTLEROOT_HOUSES_STATE_BRENDAN              (LEGACY_VARS_START + 0x15)
#define VAR_LITTLEROOT_RIVAL_STATE                       (LEGACY_VARS_START + 0x16)
#define VAR_BOARD_BRINEY_BOAT_STATE                      (LEGACY_VARS_START + 0x17)
#define VAR_DEVON_CORP_3F_STATE                          (LEGACY_VARS_START + 0x18)
#define VAR_BRINEY_HOUSE_STATE                           (LEGACY_VARS_START + 0x19)
#define VAR_LITTLEROOT_INTRO_STATE                       (LEGACY_VARS_START + 0x1A)
#define VAR_MAUVILLE_GYM_STATE                           (LEGACY_VARS_START + 0x1B)
#define VAR_GIFT_UNUSED_7                                (LEGACY_VARS_START + 0x1C) // var is written to, but never read
#define VAR_BRINEY_LOCATION                              (LEGACY_VARS_START + 0x1D)
#define VAR_PETALBURG_WOODS_STATE                        (LEGACY_VARS_START + 0x1E)
#define VAR_RUSTURF_TUNNEL_STATE                         (LEGACY_VARS_START + 0x1F)
#define VAR_ELITE_4_STATE                                (LEGACY_VARS_START + 0x20)
#define VAR_MOSSDEEP_SPACE_CENTER_STAIR_GUARD_STATE      (LEGACY_VARS_START + 0x21)
#define VAR_MOSSDEEP_SPACE_CENTER_STATE                  (LEGACY_VARS_START + 0x22)
#define VAR_SLATEPORT_HARBOR_STATE                       (LEGACY_VARS_START + 0x23)
#define VAR_SEAFLOOR_CAVERN_STATE                        (LEGACY_VARS_START + 0x24)
#define VAR_SLATEPORT_MUSEUM_1F_STATE                    (LEGACY_VARS_START + 0x25)
#define VAR_TRICK_HOUSE_PUZZLE_7_STATE_2                 (LEGACY_VARS_START + 0x26) // Leftover from RS, never set
#define VAR_MT_PYRE_STATE                                (LEGACY_VARS_START + 0x27)
#define VAR_NEW_MAUVILLE_STATE                           (LEGACY_VARS_START + 0x28)
#define VAR_METEOR_FALLS_STATE                           (LEGACY_VARS_START + 0x29)
#define VAR_SOOTOPOLIS_MYSTERY_EVENTS_STATE              (LEGACY_VARS_START + 0x2A)
#define VAR_VICTORY_ROAD_1F_STATE                        (LEGACY_VARS_START + 0x2B)
#define VAR_STEVENS_HOUSE_STATE                          (LEGACY_VARS_START + 0x2C)
#define VAR_OLDALE_RIVAL_STATE                           (LEGACY_VARS_START + 0x2D)
#define VAR_JAGGED_PASS_STATE                            (LEGACY_VARS_START + 0x2E)
#define VAR_SCOTT_PETALBURG_ENCOUNTER                    (LEGACY_VARS_START + 0x2F)
#define VAR_SKY_PILLAR_STATE                             (LEGACY_VARS_START + 0x30)
#define VAR_SCOTT_STATE                                  (LEGACY_VARS_START + 0x31)
#define VAR_SLATEPORT_OUTSIDE_MUSEUM_STATE               (LEGACY_VARS_START + 0x32)
#define VAR_SS_TIDAL_SCOTT_STATE                         (LEGACY_VARS_START + 0x33) // Always equal to FLAG_MET_SCOTT_ON_SS_TIDAL
#define VAR_SKY_PILLAR_RAYQUAZA_CRY_DONE                 (LEGACY_VARS_START + 0x34)
#define VAR_SOOTOPOLIS_WALLACE_STATE                     (LEGACY_VARS_START + 0x35)
#define VAR_HAS_TALKED_TO_SEAFLOOR_CAVERN_ENTRANCE_GRUNT (LEGACY_VARS_START + 0x36)
#define VAR_GIFT_UNUSED_1                                (LEGACY_VARS_START + 0x37) // Var is written to, but never read
#define VAR_GIFT_UNUSED_2                                (LEGACY_VARS_START + 0x38) // Var is written to, but never read
#define VAR_GIFT_UNUSED_3                                (LEGACY_VARS_START + 0x39) // Var is written to, but never read
#define VAR_GIFT_UNUSED_4                                (LEGACY_VARS_START + 0x3A) // Var is written to, but never read
#define VAR_GIFT_UNUSED_5                                (LEGACY_VARS_START + 0x3B) // Var is written to, but never read
#define VAR_GIFT_UNUSED_6                                (LEGACY_VARS_START + 0x3C) // Var is written to, but never read

#define VARS_END                                         0x40FF
#define VARS_COUNT                                       (VARS_END - VARS_START + 1)

#define SPECIAL_VARS_START            0x8000
// special vars
// They are commonly used as parameters to commands, or return values from commands.
#define VAR_0x8000                    0x8000
#define VAR_0x8001                    0x8001
#define VAR_0x8002                    0x8002
#define VAR_0x8003                    0x8003
#define VAR_0x8004                    0x8004
#define VAR_0x8005                    0x8005
#define VAR_0x8006                    0x8006
#define VAR_0x8007                    0x8007
#define VAR_0x8008                    0x8008
#define VAR_0x8009                    0x8009
#define VAR_0x800A                    0x800A
#define VAR_0x800B                    0x800B
#define VAR_FACING                    0x800C
#define VAR_RESULT                    0x800D
#define VAR_ITEM_ID                   0x800E
#define VAR_LAST_TALKED               0x800F
#define VAR_CONTEST_RANK              0x8010
#define VAR_CONTEST_CATEGORY          0x8011
#define VAR_MON_BOX_ID                0x8012
#define VAR_MON_BOX_POS               0x8013
#define VAR_UNUSED_0x8014             0x8014
#define VAR_TRAINER_BATTLE_OPPONENT_A 0x8015 // Alias of TRAINER_BATTLE_PARAM.opponentA

#define SPECIAL_VARS_END              0x8015

// If an overworld trigger uses this pseudo-variable as the trigger check,
// then the script will be run using RunScriptImmediately instead of in the
// global script context. This means it will run faster, but cannot do any
// cutscenes nor call a wait command. Used for weather effects in vanilla.
#define TRIGGER_RUN_IMMEDIATELY   0

// Temp var aliases
#define VAR_TEMP_CHALLENGE_STATUS  VAR_TEMP_0

#define VAR_TEMP_MIXED_RECORDS         VAR_TEMP_0
#define VAR_TEMP_RECORD_MIX_GIFT_ITEM  VAR_TEMP_1

#define VAR_TEMP_PLAYING_PYRAMID_MUSIC  VAR_TEMP_E

#define VAR_TEMP_FRONTIER_TUTOR_SELECTION  VAR_TEMP_D
#define VAR_TEMP_FRONTIER_TUTOR_ID         VAR_TEMP_E

#define VAR_TEMP_TRANSFERRED_SPECIES  VAR_TEMP_1

#if TESTING
#define TESTING_VARS_START                  0x9000
#define TESTING_VAR_DIFFICULTY              (TESTING_VARS_START + 0x0)
#define TESTING_VAR_TRAINER_SLIDES          (TESTING_VARS_START + 0x1)
#define TESTING_VAR_UNUSED_2                (TESTING_VARS_START + 0x2)
#define TESTING_VAR_UNUSED_3                (TESTING_VARS_START + 0x3)
#define TESTING_VAR_UNUSED_4                (TESTING_VARS_START + 0x4)
#define TESTING_VAR_UNUSED_5                (TESTING_VARS_START + 0x5)
#define TESTING_VAR_UNUSED_6                (TESTING_VARS_START + 0x6)
#define TESTING_VAR_UNUSED_7                (TESTING_VARS_START + 0x7)
#endif // TESTING

#endif // GUARD_CONSTANTS_VARS_H
