# Var inventory

Inventory of Emerald's saved vars (`include/constants/vars.h`, `0x4000`-`0x40FF`), for allocating story-state regions (design_story.md, "Story state storage"). Counts are word-matches of `VAR_*` in `data/`, `src/`, `test/` and `include/` (excluding `vars.h`). Usage descriptions come from names, comments and reference counts; verify the code before merging or repurposing any var. `vars_frlg.h` defines a second, overlapping FRLG var set (same addresses, `_FRLG` names) that no Emerald-build code references; ignore it.

Totals: 16 temp + 16 object-GFX vars (not persistent), 224 persistent vars `0x4020`-`0x40FF`. Addresses below are Emerald's original ones (before renumbering), and the bucket counts are of Emerald's original vars: 56 free, 10 freeable with a trivial edit, 106 keep (65 minigame/facility + 41 engine), 52 retire (39 main arc + 13 chapter; 1 already retired). Since then 21 vars were removed or moved to flags (see "Moved to flags"), so the blocks now hold 59 minigame + 37 engine + 43 legacy vars; `test/vars.c` lists every persistent var with its block.

## Free now (56, no references anywhere)

`0x4020` (`RECYCLE_GOODS`), `404E`, `4052` (Dewford state), `4055`, `4056`, `4059`, `405B`, `405C`, `405F` (town/city states), `4061 4062 4064-4068 406A-406E 4070 4073 4075-407A 407C-4081` (unused `ROUTEnnn_STATE`), `4083 408B 4091 409B 409D 40A1 40A8 40B8 40BB 40DB 40DC 40E5`, `40F7-40FF`. Beware: the objects-GFX vars `0x401B`-`0x401D` also have no references but belong to the GFX pool, so leave them.

## Moved to flags or removed (21, done)

Vars that held at most two states, or none, no longer exist; their flags are `FLAG_*` at `0x22`-`0x2A` in `flags.h`. A script-only `0`/`1` var maps to `setflag`/`clearflag`/`goto_if_set`/`goto_if_unset`.

- Deleted: `GIFT_UNUSED_1-7`, `DISTRIBUTE_EON_TICKET` (`ShouldDistributeEonTicket` now returns FALSE), `TRICK_HOUSE_PUZZLE_7_STATE_2` (only set by an unused R/S teleport pad, so that script and its table entries went too), `RESET_RTC_ENABLE` (always set with `FLAG_SYS_RESET_RTC_ENABLE`, so its `0x920` check added nothing).
- Flags: `ENIGMA_BERRY_AVAILABLE`, `SECRET_BASE_IS_NOT_LOCAL`, `JAGGED_PASS_ASH_WEATHER`, `TRICK_HOUSE_ENTER_FROM_CORRIDOR`, `TRICK_HOUSE_PRIZE_PICKUP`, `BRINEY_HOUSE_STATE` (`FLAG_BRINEY_HOUSE_PEEKO_POS`), `HAS_TALKED_TO_SEAFLOOR_CAVERN_ENTRANCE_GRUNT`, `ROAMER_POKEMON` (`FLAG_ROAMER_IS_LATIOS`).
- Flag plus a temp var, because `map_script_2` and `coord_event` can only test a var: `SS_TIDAL_SCOTT_STATE` (`FLAG_MET_SCOTT_ON_SS_TIDAL`, `VAR_TEMP_0`) and `HAS_ENTERED_BATTLE_FRONTIER` (`VAR_TEMP_0`): the map's on-transition script sets the temp var from the flag, and the scene sets both.
- Flag plus a temp var, retired from the main arc: `ROUTE128_STATE` (`FLAG_KYOGRE_AWAKENED_SCENE`, `VAR_TEMP_0`; set when Kyogre escapes, cleared by Route 128's scene). The Seafloor Cavern sets 1 and the scene sets 2, and only 1 was ever tested.
- Not converted although two-state, since a `map_script_2`/`coord_event` guard would need a temp-var shim on every map using it: `SECRET_BASE_INITIALIZED`, `INIT_SECRET_BASE` (shared by all secret-base maps, and read in C), `SHOULD_END_ABNORMAL_WEATHER` (8 route maps), `TRAINER_HILL_IS_ACTIVE`, `TRICK_HOUSE_BEING_WATCHED_STATE`, `TRICK_HOUSE_FOUND_TRICK_MASTER`, and the coord-event legacy states (`ROUTE110/118/119/121_STATE`, `METEOR_FALLS_STATE`, `OLDALE_TOWN_STATE`, `PETALBURG_WOODS_STATE`, `SEAFLOOR_CAVERN_STATE`, `SLATEPORT_MUSEUM_1F_STATE`, `SKY_PILLAR_RAYQUAZA_CRY_DONE`, `SCOTT_PETALBURG_ENCOUNTER`, `DEVON_CORP_3F_STATE`; `ROUTE110/119_STATE` and `PETALBURG_WOODS_STATE` also feed `emerald_plot_gets.h`). `ALTERING_CAVE_WILD_SET` is a table index, and `BRINEY_LOCATION` holds a location, so neither is two-state.

## Keep: minigame and facility (59, own content; each gets a hardcoded range)

Rule (design_story.md, "Story state storage"): content with a lot of non-script code (wanted in nearly every run, hard to fork), or many steps doable out of order. Location-bound content that Emerald.md counts as a county chain var (fan clubs, Glass Workshop, fossils, Mirage Tower, the weekly TM) is classed here instead, because each appears at most once per run and keeps its own code; this only lowers the per-county need. Block order follows the groups below, so a group can be packed or removed as a unit.

| Group | Vars | Merge/reuse idea |
| --- | --- | --- |
| Battle Frontier (9) | `FRONTIER_MANIAC_FACILITY 402F`, `FRONTIER_GAMBLER_CHALLENGE/SET_CHALLENGE/AMOUNT_BET/STATE 4030-4033`, `FRONTIER_BATTLE_MODE 40CE` (94 data refs), `FRONTIER_FACILITY 40CF`, `TRAINER_HILL_IS_ACTIVE 40D6`, `BRAVO_TRAINER_BATTLE_TOWER_ON 40BC` | late-game special location; most are scratch parameters that could be special vars |
| Trick House (12) | `LEVEL 4044`, `BEING_WATCHED_STATE 40A5`, `FOUND_TRICK_MASTER 40A6`, `ENTRANCE_STATE 40A7`, `PUZZLE_1-8_STATE 40AB-40B2` | largest merge target: puzzle states are small and could pack 4 per var |
| Contests (4) | `CONTEST_HALL_STATE 4086`, `CONTEST_TYPE 4088`, `CONTEST_PRIZE_PICKUP 408A`, `LILYCOVE_CONTEST_LOBBY_STATE 4099` | scratch-like; may be packed |
| Lottery corner (4) | `POKELOT_PRIZE_ITEM 4045`, `POKELOT_RND1/RND2 404B/404C` (adjacent pair), `POKELOT_PRIZE_PLACE 404D` | pack to 2 vars |
| Records (2 + 4) | `SEEDOT_SIZE_RECORD 4047`, `LOTAD_SIZE_RECORD 404F`; `CYCLING_ROAD_RECORD_COLLISIONS 4027`, `TIME_L/TIME_H 4028/4029` (32-bit pair), `CYCLING_CHALLENGE_STATE 40A9` | size records: one var if each fits 8 bits; cycling: pack time and collisions into 2 vars |
| Fan clubs (4) | `FANCLUB_FAN_COUNTER/LOSE_FAN_TIMER 4041/4042` (cast to a 2-byte struct in `field_specials.c`, so adjacent), `LILYCOVE_FAN_CLUB_STATE 4095`, `SLATEPORT_FAN_CLUB_STATE 40B7` | |
| Cruise and harbor (2) | `CRUISE_STEP_COUNT 404A`, `SS_TIDAL_STATE 40B4` | tied to Emerald harbors |
| Legendary-island puzzles (7) | `DEOXYS_ROCK_STEP_COUNT/LEVEL 4034/4035`, `FARAWAY_ISLAND_STEP_COUNTER 403A`, `REGICE_STEPS_1-3 403B-403D`, `ALTERING_CAVE_WILD_SET 403E` | legendary-county content; revisit with Milestone 10 |
| Mirage Island and Tower (3) | `MIRAGE_RND_H/L 4024/4025` (32-bit seed pair), `MIRAGE_TOWER_STATE 40CB` | seed could be derived from the identity seed |
| Ash and Glass (2) | `ASH_GATHER_COUNT 4048`, `GLASS_WORKSHOP_STATE 40BE` | tied to Emerald routes |
| Fossils and TM (2) | `FOSSIL_MANIAC_STATE 40CC`, `PACIFIDLOG_TM_RECEIVED_DAY 40C2` | the fossil revival state and choice are retired into Rustboro's slot 1 and `FLAG_FOSSIL_IS_ANORITH`; TM day may become a per-county var |
| Shops and stations (3) | `DEPT_STORE_FLOOR 4043`, `SAFARI_ZONE_STATE 40A4`, `CABLE_CAR_STATION_STATE 40A3`, `WEATHER_INSTITUTE_STATE 40B3` | location-bound; treat like city chapters (below) |

## Keep: engine and system (37, fixed meaning; merge candidates noted)

| Vars | Use | Merge/reuse idea |
| --- | --- | --- |
| `REPEL_STEP_COUNT 4021`, `ICE_STEP_COUNT 4022`, `FRIENDSHIP_STEP_COUNTER 402A`, `POISON_STEP_COUNTER 402B`, `WONDER_NEWS_STEP_COUNTER 402E` | per-step counters (repel, ice tiles, friendship, poison, news) | pack the small ones as bitfields (friendship wraps at 128, poison at 5) |
| `DAYS 4040` | RTC day count | |
| `NATIONAL_DEX 4046` | Dex unlock magic; its offset in the ROM header is computed from the constant (`rom_header_gf.c:118`) | position is free |
| `STARTER_MON 4023` | 0-2, read by ~47 scripts | could be derived later |
| `DAILY_SLOTS/WILDS/BLENDER/PLANTED_BERRIES/PICKED_BERRIES/ROULETTE 40E6-40EB`, `DAILY_BP 40F1` | once-a-day limits, cleared on a new day | 7 small counters: pack as nibbles into 2 vars |
| `SECRET_BASE_MAP 4026`, `CURRENT_SECRET_BASE 4054`, `SECRET_BASE_INITIALIZED 4089`, `INIT_SECRET_BASE 4097`, `STEP_COUNTER 40EC`, `LAST_ITEM_USED 40ED`, `LOW/HIGH_TV_FLAGS 40EE/40EF` | secret bases and their TV shows | `INITIALIZED` and `INIT` are two-state but guard `map_script_2` on every secret-base map, so they stay vars; TV flags are 32 bits across two vars (keep adjacent) |
| `PC_BOX_TO_SEND_MON 4036` | scratch for the PC transfer script | could be a special var (`0x8000`+) |
| `ABNORMAL_WEATHER_LOCATION/STEP_COUNTER/SHOULD_END 4037-4039` | abnormal weather on several routes | tied to Emerald routes; reworked with biomes |
| `WALLY/SCOTT_FORTREE/ROXANNE/SCOTT_BF/RIVAL_RAYQUAZA_CALL_STEP_COUNTER 40F2-40F6` | Pokenav match-call step timers | one shared counter with per-call thresholds in flags |
| `GIFT_PICHU_SLOT 40DD` | mystery gift Pichu | |
| `DEX_UPGRADE_JOHTO_STARTER_STATE 40D3`, `REGISTER_BIRCH_STATE 40DA`, `CABLE_CLUB_STATE 4087`, `CABLE_CLUB_TUTORIAL_STATE 40CD` | post-game gifts, Pokenav registration, link features | scratch-like; may be packed |

## Retire: main arc (39, replaced by the main-story-index at Milestone 3)

Rival, Birch, Aqua/Magma, Devon, Wally, Elite Four and the other cross-county storylines: `BIRCH_STATE 4049`, `BIRCH_LAB_STATE 4084`, `LITTLEROOT_TOWN_STATE 4050`, `LITTLEROOT_HOUSES_STATE_MAY/BRENDAN 4082/408C`, `LITTLEROOT_RIVAL_STATE 408D`, `OLDALE_TOWN_STATE 4051`, `OLDALE_RIVAL_STATE 40C7`, `ROUTE101_STATE 4060`, `PETALBURG_GYM_STATE 4085` (also the implicit badge counter), `PETALBURG_CITY_STATE 4057`, `RUSTBORO_CITY_STATE 405A`, `SLATEPORT_CITY_STATE 4058`, `MOSSDEEP_CITY_STATE 405D`, `SOOTOPOLIS_CITY_STATE 405E` (67 refs, the busiest), `SOOTOPOLIS_WALLACE_STATE 40D8`, `MT_PYRE_STATE 40B9`, `DEVON_CORP_3F_STATE 408F`, `BRINEY_LOCATION 4096`, `BOARD_BRINEY_BOAT_STATE 408E`, `STEVENS_HOUSE_STATE 40C6`, `SEAFLOOR_CAVERN_STATE 40A2`, `SKY_PILLAR_STATE/RAYQUAZA_CRY_DONE 40CA/40D7`, `ELITE_4_STATE 409C`, `MAUVILLE_GYM_STATE 4093`, `LAVARIDGE_TOWN_STATE 4053`, `SCOTT_STATE 40D1`, `SCOTT_PETALBURG_ENCOUNTER 40C9`, `ROUTE104/110/116/118/119/121_STATE 4063/4069/406F/4071/4072/4074` (`ROUTE128_STATE 407B` is already a flag).

## Retire as ported: city and special-location chapters (12 left, Milestones 1a and 2)

`SLATEPORT_HARBOR_STATE 40A0`, `SLATEPORT_MUSEUM_1F_STATE 40AA`, `SLATEPORT_OUTSIDE_MUSEUM_STATE 40D2`, `PETALBURG_WOODS_STATE 4098`, `RUSTURF_TUNNEL_STATE 409A`, `METEOR_FALLS_STATE 40BF`, `JAGGED_PASS_STATE 40C8`, `VICTORY_ROAD_1F_STATE 40C3`, `NEW_MAUVILLE_STATE 40BA`, `SOOTOPOLIS_MYSTERY_EVENTS_STATE 40C0`, `MOSSDEEP_SPACE_CENTER_STATE/STAIR_GUARD_STATE 409F/409E`. The location-bound minigames above stay as minigame-block vars instead of being retired.

## Slice size

- Supply: 224 persistent vars - 96 keep (59 minigame + 37 engine) = 128 for county slices and the story vars (the main-story-index plus about 4 optional-story vars, so 5).
- Demand per county, from `Emerald.md` ("Per-county storage needs"): one completion mask plus at most 2 chain vars (Slateport and Lilycove; the Trick House's 5 moved to the minigame block), so 3. One whole var is left for authored chapters (not yet counted).
- Result: 4 vars per county x 24 counties = 96, leaving 32 reserve (the 5 story vars, 27 spare). 5 per county (120) now fits: 224 - 96 - 5 = 123 >= 120, with 3 spare.
- The 43 legacy vars (the retire vars above) occupy the last 12 county vars (slices 21-23) and the first 31 reserve vars, so only the first 21 county slices (0-20) are free to use; `test/vars.c` asserts at least 21. `VAR_MAIN_STORY_INDEX` takes the reserve's last var.
- Packing the keep vars (daily, Trick House, step counters, records, secret base, Frontier scratch: about 30 vars) would free more; not planned.
