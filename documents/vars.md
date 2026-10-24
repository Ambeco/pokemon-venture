# Var inventory

Inventory of Emerald's saved vars (`include/constants/vars.h`, `0x4000`-`0x40FF`), for allocating story-state regions (design.md, "Story state storage"). Counts are word-matches of `VAR_*` in `data/`, `src/`, `test/` and `include/` (excluding `vars.h`). Usage descriptions come from names, comments and reference counts; verify the code before merging or repurposing any var. `vars_frlg.h` defines a second, overlapping FRLG var set (same addresses, `_FRLG` names) that no Emerald-build code references; ignore it.

Totals: 16 temp + 16 object-GFX vars (not persistent), 224 persistent vars `0x4020`-`0x40FF`: 56 unreferenced, 87 referenced only by map scripts, 45 only by C, 49 by both (some are retire candidates, below).

## Free now (56, no references anywhere)

`0x4020` (`RECYCLE_GOODS`), `404E`, `4052` (Dewford state), `4055`, `4056`, `4059`, `405B`, `405C`, `405F` (town/city states), `4061 4062 4064-4068 406A-406E 4070 4073 4075-407A 407C-4081` (unused `ROUTEnnn_STATE`), `4083 408B 4091 409B 409D 40A1 40A8 40B8 40BB 40DB 40DC 40E5`, `40F7-40FF`. Beware: the objects-GFX vars `0x401B`-`0x401D` also have no references but belong to the GFX pool, so leave them.

Freeable with a trivial edit (+10): `GIFT_UNUSED_1-7` `40DE-40E4` (only cleared in `ClearMysteryGiftVars`, `event_data.c:154`), `DISTRIBUTE_EON_TICKET` `403F` (always zero), `TRICK_HOUSE_PUZZLE_7_STATE_2` `40B6` (never set), `SS_TIDAL_SCOTT_STATE` `40D4` (duplicates `FLAG_MET_SCOTT_ON_SS_TIDAL`).

## Keep: engine and system (fixed meaning; merge candidates noted)

| Vars | Use | Merge/reuse idea |
| --- | --- | --- |
| `REPEL_STEP_COUNT 4021`, `FRIENDSHIP_STEP_COUNTER 402A`, `POISON_STEP_COUNTER 402B`, `WONDER_NEWS_STEP_COUNTER 402E`, `ICE_STEP_COUNT 4022` | per-step counters (repel, friendship, poison, news, ice tiles) | pack the small ones as bitfields (friendship wraps at 128, poison at 5) |
| `DAYS 4040`, `PACIFIDLOG_TM_RECEIVED_DAY 40C2` | RTC day count; last day the Pacifidlog TM was given | TM day may become a per-county var |
| `NATIONAL_DEX 4046` | Dex unlock magic; its offset is in the ROM header (`rom_header_gf.c`) | fixed position |
| `STARTER_MON 4023` | 0-2, read by ~47 scripts | keep; could be derived later |
| `ROAMER_POKEMON 40D5` | Latias(0)/Latios(1) | 1 bit |
| `DAILY_SLOTS/WILDS/BLENDER/PLANTED_BERRIES/PICKED_BERRIES/ROULETTE/BP 40E6-40EB, 40F1` | once-a-day limits, cleared on a new day | 7 small counters: pack as nibbles into 2 vars |
| `SECRET_BASE_MAP 4026`, `CURRENT_SECRET_BASE 4054`, `SECRET_BASE_INITIALIZED 4089`, `INIT_SECRET_BASE 4097`, `STEP_COUNTER 40EC`, `LAST_ITEM_USED 40ED`, `LOW/HIGH_TV_FLAGS 40EE/40EF`, `IS_NOT_LOCAL 40F0` | secret bases and their TV shows | `INITIALIZED`, `INIT`, `IS_NOT_LOCAL` are flags; TV flags are 32 bits across two vars |
| `MIRAGE_RND_H/L 4024/4025`, `MIRAGE_TOWER_STATE 40CB` | Mirage Island seed (32 bits), Mirage Tower | seed could be derived from the world seed |
| `CYCLING_ROAD_RECORD_COLLISIONS/TIME_L/TIME_H 4027-4029`, `CYCLING_CHALLENGE_STATE 40A9` | Acro/Mach bike records | pack time and collisions into 2 vars |
| `SEEDOT_SIZE_RECORD 4047`, `LOTAD_SIZE_RECORD 404F` | size-record minigames | one var if the records fit 8 bits each |
| `POKELOT_PRIZE_ITEM/PLACE 4045/404D`, `POKELOT_RND1/RND2 404B/404C` | lottery corner | pack to 2 vars |
| `RESET_RTC_ENABLE 402C` | Pokenav RTC reset | flag |
| `PC_BOX_TO_SEND_MON 4036` | scratch for the PC transfer script | could be a special var (`0x8000`+) |
| `ABNORMAL_WEATHER_LOCATION/STEP_COUNTER/SHOULD_END 4037-4039` | abnormal weather on several routes | tied to Emerald routes; reworked with biomes |
| `CRUISE_STEP_COUNT 404A`, `SS_TIDAL_STATE 40B4` | SS Tidal cruise | tied to Emerald harbors |
| `WALLY/SCOTT_FORTREE/ROXANNE/SCOTT_BF/RIVAL_RAYQUAZA_CALL_STEP_COUNTER 40F2-40F6` | Pokenav match-call step timers | one shared counter with per-call thresholds in flags |
| `ASH_GATHER_COUNT 4048`, `JAGGED_PASS_ASH_WEATHER 40BD` | Route 113 ash and Jagged Pass weather | tied to the Emerald route |
| `DEOXYS_ROCK_STEP_COUNT/LEVEL 4034/4035`, `FARAWAY_ISLAND_STEP_COUNTER 403A`, `REGICE_STEPS_1-3 403B-403D`, `ALTERING_CAVE_WILD_SET 403E` | legendary-island puzzles, Altering Cave | legendary-county content; revisit with Milestone 10 |
| `GIFT_PICHU_SLOT 40DD` | mystery gift Pichu | keep |
| `DEX_UPGRADE_JOHTO_STARTER_STATE 40D3`, `REGISTER_BIRCH_STATE 40DA`, `CABLE_CLUB_STATE 4087`, `CABLE_CLUB_TUTORIAL_STATE 40CD`, `CONTEST_*`, `BRAVO_TRAINER_BATTLE_TOWER_ON 40BC` | post-game gifts, link and contest features | keep; contest/cable club vars are scratch-like and may be packed |

Contest vars: `CONTEST_HALL_STATE 4086`, `CONTEST_TYPE 4088`, `CONTEST_PRIZE_PICKUP 408A`, `LILYCOVE_CONTEST_LOBBY_STATE 4099`.

## Keep: facilities (own content; could share one packed region)

- Battle Frontier: `FRONTIER_MANIAC_FACILITY 402F`, `GAMBLER_CHALLENGE/SET_CHALLENGE/AMOUNT_BET/STATE 4030-4033`, `FRONTIER_BATTLE_MODE 40CE` (94 data refs), `FRONTIER_FACILITY 40CF`, `HAS_ENTERED_BATTLE_FRONTIER 40D0` (a flag in disguise), `TRAINER_HILL_IS_ACTIVE 40D6`. Frontier is a late-game special location; most are scratch parameters that could be special vars.
- Trick House (17): `LEVEL 4044`, `BEING_WATCHED_STATE 40A5`, `FOUND_TRICK_MASTER 40A6`, `ENTRANCE_STATE 40A7`, `PUZZLE_1-8_STATE 40AB-40B2`, `ENTER_FROM_CORRIDOR 40B5`, `PRIZE_PICKUP 40C1`, plus `PUZZLE_7_STATE_2 40B6`. Largest merge target: puzzle states are small and could pack 4 per var, or live in the owning county's region.
- Fan clubs: `FANCLUB_FAN_COUNTER/LOSE_FAN_TIMER 4041/4042` are cast to a 2-byte struct (`trainer_fan_club.c`); keep adjacent. `LILYCOVE_FAN_CLUB_STATE 4095`, `SLATEPORT_FAN_CLUB_STATE 40B7` are story-ish.
- Others: `DEPT_STORE_FLOOR 4043`, `SAFARI_ZONE_STATE 40A4`, `CABLE_CAR_STATION_STATE 40A3`, `GLASS_WORKSHOP_STATE 40BE`, `WEATHER_INSTITUTE_STATE 40B3`, `FOSSIL_RESURRECTION_STATE/WHICH_FOSSIL_REVIVED 40C4/40C5`, `FOSSIL_MANIAC_STATE 40CC`: location-bound minigames and shops; treat like city subplots (below).

## Retire: main arc (replaced by the main-story-index at Milestone 3)

Rival, Birch, Aqua/Magma, Devon, Wally, Elite Four and the other cross-county storylines: `BIRCH_STATE 4049`, `BIRCH_LAB_STATE 4084`, `LITTLEROOT_TOWN_STATE 4050`, `LITTLEROOT_HOUSES_STATE_MAY/BRENDAN 4082/408C`, `LITTLEROOT_RIVAL_STATE 408D`, `LITTLEROOT_INTRO_STATE 4092`, `OLDALE_TOWN_STATE 4051`, `OLDALE_RIVAL_STATE 40C7`, `ROUTE101_STATE 4060`, `ROUTE103`-area rival, `PETALBURG_GYM_STATE 4085` (also the implicit badge counter), `PETALBURG_CITY_STATE 4057`, `RUSTBORO_CITY_STATE 405A`, `SLATEPORT_CITY_STATE 4058`, `MOSSDEEP_CITY_STATE 405D`, `SOOTOPOLIS_CITY_STATE 405E` (67 refs, the busiest), `SOOTOPOLIS_WALLACE_STATE 40D8`, `MT_PYRE_STATE 40B9`, `DEVON_CORP_3F_STATE 408F`, `BRINEY_HOUSE_STATE 4090`, `BRINEY_LOCATION 4096`, `BOARD_BRINEY_BOAT_STATE 408E`, `STEVENS_HOUSE_STATE 40C6`, `SEAFLOOR_CAVERN_STATE 40A2`, `HAS_TALKED_TO_SEAFLOOR_CAVERN_ENTRANCE_GRUNT 40D9`, `SKY_PILLAR_STATE/RAYQUAZA_CRY_DONE 40CA/40D7`, `ELITE_4_STATE 409C`, `MAUVILLE_GYM_STATE 4093`, `LAVARIDGE_TOWN_STATE 4053`, `SCOTT_STATE 40D1`, `SCOTT_PETALBURG_ENCOUNTER 40C9`, `ROUTE104/110/116/118/119/121/128_STATE 4063/4069/406F/4071/4072/4074/407B`. Roughly 40 vars.

## Retire as ported: city and special-location subplots (Milestones 1a and 2)

`SLATEPORT_HARBOR_STATE 40A0`, `SLATEPORT_MUSEUM_1F_STATE 40AA`, `SLATEPORT_OUTSIDE_MUSEUM_STATE 40D2`, `LILYCOVE_MUSEUM_2F_STATE 4094`, `PETALBURG_WOODS_STATE 4098`, `RUSTURF_TUNNEL_STATE 409A`, `METEOR_FALLS_STATE 40BF`, `JAGGED_PASS_STATE 40C8`, `VICTORY_ROAD_1F_STATE 40C3`, `NEW_MAUVILLE_STATE 40BA`, `SOOTOPOLIS_MYSTERY_EVENTS_STATE 40C0`, `MOSSDEEP_SPACE_CENTER_STATE/STAIR_GUARD_STATE 409F/409E`, plus the location-bound minigames above. About 25 vars; several can stay (the Trick House, facilities) as county-owned slots instead of being retired.

## Reading the counts

- Retire-or-reuse candidates: 56 free now + 10 trivially freeable + about 65 (main arc and subplot) as ported = about 130 of 224. The packing ideas above (daily, Trick House, step counters, records, secret base, Frontier scratch) could free about 30 more.
- Before the per-county plan needs a hard number, the unmeasured part is how many of these must stay exactly where they are (ROM-header Dex var, struct-cast pairs, 32-bit pairs).
