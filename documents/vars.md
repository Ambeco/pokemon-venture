# Var inventory

Inventory of Emerald's saved vars (`include/constants/vars.h`, `0x4000`-`0x40FF`), for allocating story-state regions (design.md, "Story state storage"). Counts are word-matches of `VAR_*` in `data/`, `src/`, `test/` and `include/` (excluding `vars.h`). Usage descriptions come from names, comments and reference counts; verify the code before merging or repurposing any var. `vars_frlg.h` defines a second, overlapping FRLG var set (same addresses, `_FRLG` names) that no Emerald-build code references; ignore it.

Totals: 16 temp + 16 object-GFX vars (not persistent), 224 persistent vars `0x4020`-`0x40FF`, each in exactly one bucket: 56 free now, 10 freeable with a trivial edit, 106 keep (65 minigame/facility + 41 engine), 52 retire (39 main arc + 13 subplot). The 106 keep vars are the size of the minigame and engine blocks; `test/vars.c` lists every persistent var with its block.

## Free now (56, no references anywhere)

`0x4020` (`RECYCLE_GOODS`), `404E`, `4052` (Dewford state), `4055`, `4056`, `4059`, `405B`, `405C`, `405F` (town/city states), `4061 4062 4064-4068 406A-406E 4070 4073 4075-407A 407C-4081` (unused `ROUTEnnn_STATE`), `4083 408B 4091 409B 409D 40A1 40A8 40B8 40BB 40DB 40DC 40E5`, `40F7-40FF`. Beware: the objects-GFX vars `0x401B`-`0x401D` also have no references but belong to the GFX pool, so leave them.

Freeable with a trivial edit (+10): `GIFT_UNUSED_1-7` `40DE-40E4` (only cleared in `ClearMysteryGiftVars`, `event_data.c:154`), `DISTRIBUTE_EON_TICKET` `403F` (always zero), `TRICK_HOUSE_PUZZLE_7_STATE_2` `40B6` (never set), `SS_TIDAL_SCOTT_STATE` `40D4` (duplicates `FLAG_MET_SCOTT_ON_SS_TIDAL`). Until those edits land they are still referenced, so they stay in the legacy block with the retire vars.

## Keep: minigame and facility (65, own content; each gets a hardcoded range)

Rule (design.md, "Story state storage"): content with a lot of non-script code (wanted in nearly every run, hard to fork), or many steps doable out of order. Location-bound content that Emerald.md counts as a county chain var (fan clubs, Glass Workshop, fossils, Mirage Tower, the weekly TM) is classed here instead, because each appears at most once per run and keeps its own code; this only lowers the per-county need. Block order follows the groups below, so a group can be packed or removed as a unit.

| Group | Vars | Merge/reuse idea |
| --- | --- | --- |
| Battle Frontier (10) | `FRONTIER_MANIAC_FACILITY 402F`, `FRONTIER_GAMBLER_CHALLENGE/SET_CHALLENGE/AMOUNT_BET/STATE 4030-4033`, `FRONTIER_BATTLE_MODE 40CE` (94 data refs), `FRONTIER_FACILITY 40CF`, `HAS_ENTERED_BATTLE_FRONTIER 40D0` (a flag in disguise), `TRAINER_HILL_IS_ACTIVE 40D6`, `BRAVO_TRAINER_BATTLE_TOWER_ON 40BC` | late-game special location; most are scratch parameters that could be special vars |
| Trick House (14) | `LEVEL 4044`, `BEING_WATCHED_STATE 40A5`, `FOUND_TRICK_MASTER 40A6`, `ENTRANCE_STATE 40A7`, `PUZZLE_1-8_STATE 40AB-40B2`, `ENTER_FROM_CORRIDOR 40B5`, `PRIZE_PICKUP 40C1` | largest merge target: puzzle states are small and could pack 4 per var |
| Contests (4) | `CONTEST_HALL_STATE 4086`, `CONTEST_TYPE 4088`, `CONTEST_PRIZE_PICKUP 408A`, `LILYCOVE_CONTEST_LOBBY_STATE 4099` | scratch-like; may be packed |
| Lottery corner (4) | `POKELOT_PRIZE_ITEM 4045`, `POKELOT_RND1/RND2 404B/404C` (adjacent pair), `POKELOT_PRIZE_PLACE 404D` | pack to 2 vars |
| Records (2 + 4) | `SEEDOT_SIZE_RECORD 4047`, `LOTAD_SIZE_RECORD 404F`; `CYCLING_ROAD_RECORD_COLLISIONS 4027`, `TIME_L/TIME_H 4028/4029` (32-bit pair), `CYCLING_CHALLENGE_STATE 40A9` | size records: one var if each fits 8 bits; cycling: pack time and collisions into 2 vars |
| Fan clubs (4) | `FANCLUB_FAN_COUNTER/LOSE_FAN_TIMER 4041/4042` (cast to a 2-byte struct in `field_specials.c`, so adjacent), `LILYCOVE_FAN_CLUB_STATE 4095`, `SLATEPORT_FAN_CLUB_STATE 40B7` | |
| Cruise and harbor (2) | `CRUISE_STEP_COUNT 404A`, `SS_TIDAL_STATE 40B4` | tied to Emerald harbors |
| Legendary-island puzzles (7) | `DEOXYS_ROCK_STEP_COUNT/LEVEL 4034/4035`, `FARAWAY_ISLAND_STEP_COUNTER 403A`, `REGICE_STEPS_1-3 403B-403D`, `ALTERING_CAVE_WILD_SET 403E` | legendary-county content; revisit with Milestone 10 |
| Mirage Island and Tower (3) | `MIRAGE_RND_H/L 4024/4025` (32-bit seed pair), `MIRAGE_TOWER_STATE 40CB` | seed could be derived from the world seed |
| Ash and Glass (3) | `ASH_GATHER_COUNT 4048`, `JAGGED_PASS_ASH_WEATHER 40BD`, `GLASS_WORKSHOP_STATE 40BE` | tied to Emerald routes |
| Fossils and TM (4) | `FOSSIL_RESURRECTION_STATE/WHICH_FOSSIL_REVIVED 40C4/40C5`, `FOSSIL_MANIAC_STATE 40CC`, `PACIFIDLOG_TM_RECEIVED_DAY 40C2` | fossils are revived in one county, biased toward the gym 5-8 timeline; TM day may become a per-county var |
| Shops and stations (3) | `DEPT_STORE_FLOOR 4043`, `SAFARI_ZONE_STATE 40A4`, `CABLE_CAR_STATION_STATE 40A3`, `WEATHER_INSTITUTE_STATE 40B3` | location-bound; treat like city subplots (below) |

## Keep: engine and system (41, fixed meaning; merge candidates noted)

| Vars | Use | Merge/reuse idea |
| --- | --- | --- |
| `REPEL_STEP_COUNT 4021`, `ICE_STEP_COUNT 4022`, `FRIENDSHIP_STEP_COUNTER 402A`, `POISON_STEP_COUNTER 402B`, `WONDER_NEWS_STEP_COUNTER 402E` | per-step counters (repel, ice tiles, friendship, poison, news) | pack the small ones as bitfields (friendship wraps at 128, poison at 5) |
| `DAYS 4040` | RTC day count | |
| `NATIONAL_DEX 4046` | Dex unlock magic; its offset in the ROM header is computed from the constant (`rom_header_gf.c:118`) | position is free |
| `STARTER_MON 4023` | 0-2, read by ~47 scripts | could be derived later |
| `ROAMER_POKEMON 40D5` | Latias(0)/Latios(1) | 1 bit |
| `DAILY_SLOTS/WILDS/BLENDER/PLANTED_BERRIES/PICKED_BERRIES/ROULETTE 40E6-40EB`, `DAILY_BP 40F1` | once-a-day limits, cleared on a new day | 7 small counters: pack as nibbles into 2 vars |
| `SECRET_BASE_MAP 4026`, `CURRENT_SECRET_BASE 4054`, `SECRET_BASE_INITIALIZED 4089`, `INIT_SECRET_BASE 4097`, `STEP_COUNTER 40EC`, `LAST_ITEM_USED 40ED`, `LOW/HIGH_TV_FLAGS 40EE/40EF`, `IS_NOT_LOCAL 40F0` | secret bases and their TV shows | `INITIALIZED`, `INIT`, `IS_NOT_LOCAL` are flags; TV flags are 32 bits across two vars (keep adjacent) |
| `RESET_RTC_ENABLE 402C` | Pokenav RTC reset | flag |
| `ENIGMA_BERRY_AVAILABLE 402D` | Mystery Event Enigma Berry gift (read by the Petalburg gym script) | flag |
| `PC_BOX_TO_SEND_MON 4036` | scratch for the PC transfer script | could be a special var (`0x8000`+) |
| `ABNORMAL_WEATHER_LOCATION/STEP_COUNTER/SHOULD_END 4037-4039` | abnormal weather on several routes | tied to Emerald routes; reworked with biomes |
| `WALLY/SCOTT_FORTREE/ROXANNE/SCOTT_BF/RIVAL_RAYQUAZA_CALL_STEP_COUNTER 40F2-40F6` | Pokenav match-call step timers | one shared counter with per-call thresholds in flags |
| `GIFT_PICHU_SLOT 40DD` | mystery gift Pichu | |
| `DEX_UPGRADE_JOHTO_STARTER_STATE 40D3`, `REGISTER_BIRCH_STATE 40DA`, `CABLE_CLUB_STATE 4087`, `CABLE_CLUB_TUTORIAL_STATE 40CD` | post-game gifts, Pokenav registration, link features | scratch-like; may be packed |

## Retire: main arc (39, replaced by the main-story-index at Milestone 3)

Rival, Birch, Aqua/Magma, Devon, Wally, Elite Four and the other cross-county storylines: `BIRCH_STATE 4049`, `BIRCH_LAB_STATE 4084`, `LITTLEROOT_TOWN_STATE 4050`, `LITTLEROOT_HOUSES_STATE_MAY/BRENDAN 4082/408C`, `LITTLEROOT_RIVAL_STATE 408D`, `LITTLEROOT_INTRO_STATE 4092`, `OLDALE_TOWN_STATE 4051`, `OLDALE_RIVAL_STATE 40C7`, `ROUTE101_STATE 4060`, `PETALBURG_GYM_STATE 4085` (also the implicit badge counter), `PETALBURG_CITY_STATE 4057`, `RUSTBORO_CITY_STATE 405A`, `SLATEPORT_CITY_STATE 4058`, `MOSSDEEP_CITY_STATE 405D`, `SOOTOPOLIS_CITY_STATE 405E` (67 refs, the busiest), `SOOTOPOLIS_WALLACE_STATE 40D8`, `MT_PYRE_STATE 40B9`, `DEVON_CORP_3F_STATE 408F`, `BRINEY_HOUSE_STATE 4090`, `BRINEY_LOCATION 4096`, `BOARD_BRINEY_BOAT_STATE 408E`, `STEVENS_HOUSE_STATE 40C6`, `SEAFLOOR_CAVERN_STATE 40A2`, `HAS_TALKED_TO_SEAFLOOR_CAVERN_ENTRANCE_GRUNT 40D9`, `SKY_PILLAR_STATE/RAYQUAZA_CRY_DONE 40CA/40D7`, `ELITE_4_STATE 409C`, `MAUVILLE_GYM_STATE 4093`, `LAVARIDGE_TOWN_STATE 4053`, `SCOTT_STATE 40D1`, `SCOTT_PETALBURG_ENCOUNTER 40C9`, `ROUTE104/110/116/118/119/121/128_STATE 4063/4069/406F/4071/4072/4074/407B`.

## Retire as ported: city and special-location subplots (13, Milestones 1a and 2)

`SLATEPORT_HARBOR_STATE 40A0`, `SLATEPORT_MUSEUM_1F_STATE 40AA`, `SLATEPORT_OUTSIDE_MUSEUM_STATE 40D2`, `LILYCOVE_MUSEUM_2F_STATE 4094`, `PETALBURG_WOODS_STATE 4098`, `RUSTURF_TUNNEL_STATE 409A`, `METEOR_FALLS_STATE 40BF`, `JAGGED_PASS_STATE 40C8`, `VICTORY_ROAD_1F_STATE 40C3`, `NEW_MAUVILLE_STATE 40BA`, `SOOTOPOLIS_MYSTERY_EVENTS_STATE 40C0`, `MOSSDEEP_SPACE_CENTER_STATE/STAIR_GUARD_STATE 409F/409E`. The location-bound minigames above stay as minigame-block vars instead of being retired.

## Slice size

- Supply: 224 persistent vars - 106 keep = 118 for county slices and the story vars (the main-story-index plus about 4 optional-story vars, so 5).
- Demand per county, from `Emerald.md` ("Per-county storage needs"): one completion mask plus at most 2 chain vars (Slateport and Lilycove; the Trick House's 5 moved to the minigame block), so 3. One whole var is left for authored subplots (not yet counted).
- Result: 4 vars per county x 24 counties = 96, leaving 22 reserve (the 5 story vars, 17 spare). 5 per county (120) does not fit: 224 - 106 - 5 = 113 < 120.
- Until the legacy vars (62 = 52 retire + 10 trivial) are retired, they occupy the last 40 county vars and the 22 reserve, so only the first 14 county slices are free to use.
- Packing the keep vars (daily, Trick House, step counters, records, secret base, Frontier scratch: about 30 vars) would allow 5 per county; not planned.
