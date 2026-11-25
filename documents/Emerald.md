# Pokémon Emerald reference

Emerald-specific data for the hardcoded stage (design.md, "Development approach"): one entry per county (city, gym, theme, places, planner steps, storage), the vocabularies they use, the planner inputs and exceptions, and the rival arc, story arc and chapters the first version reuses. The per-Part walkthrough analysis (McGuffin chain, buckets, levels) is in `Emerald_walkthrough.md`. Generic design lives in the `design*.md` files; Emerald examples used to explain it stay there.

## Counties

Hoenn does not fit the 6x4 county grid cleanly, so places are fudged into counties, slightly stretched apart; most content is hardcoded at this stage, so the stretching does not matter, and once the region is generated there is no fakery left. Each county holds at most one city, with the caves and other special locations spread around it. Each entry below is one county, numbered by county-story-index (walkthrough order, unique, 0-23; `src/data/region/emerald_region.h`), with its cell `(x,y)` in the 6x4 grid, terrain and biome, city, places, planner steps and storage. Place and tile data are in `emerald_region.h`, `emerald_cities.h` and `emerald_tiles.h` (positions are placeholders; the county and the place names are what this file fixes; a place with no MAPSEC of its own uses its route's or city's, see design_data.md "Region tiles").

Terrain and biome. Hoenn is mostly rainforest, so it uses the warm, wet biome subset (humidity 1-4, the three hottest temperature columns). Temperature falls from the top row (hottest column) downward; the desert north of Mauville is faked with Thorn forest, and the smaller one at (0,0) with Savanna. Ocean counties get a biome too, for surf, dive and fishing encounters: the maximum-humidity row, from Tropical rainforest in the top row down to Temperate giant rainforest in the bottom two rows. Dewford's island is too small for its own biome, so (0,3) is Ocean.

Whole-region facts: capital Lilycove; ports Dewford, Slateport and Lilycove (design_region.md "Ports"); E = 20 is the Elite Four county, R = 3 reserved legendary counties (21, 22 and the empty county 23); jumps (non-adjacent consecutive indices) are 3 to 4, 4 to 5, 10 to 11, 17 to 18 and 19 to 20, numbered 1-5 as key/lock ids. Ports and special-location kinds are best fits, not final.

Planner steps. Main-story-index `4x + s` (design_story.md "Main plot planner"): step 0 and 2 list gets, step 1 is the optional callback (host named), step 3 unlocks county x+1 via the named McGuffin; empty steps are omitted. Gets within a list are unordered for the planner; the county picks the order, so "G4 then Surf" inside a list means the county's script orders them. `G1`-`G8` are gyms (each with its HM), `RB1`-`RB5` rival battles, `Magma`/`Aqua` beats the arc beats. Route-story-index is 4 times the county number. "Done when" is the predicate the read-only adapter uses (see "Step predicates for the read-only adapter"; code `src/data/region/emerald_plot_gets.h`). Callbacks: 6 in about 21 blocks (indices 5, 45, 57, 61, 77, 81), four of them after Fly; no block has two different callback hosts.

Storage. "Gets" counts main-arc gets by host county (callback gets count for the host, not the block; a gym and its HM are two gets; a reserved county has none), "max list" is the largest single list the county hosts (what a completion mask must hold), and "chain vars" and "bools" are the county's own optional chapter state, estimated from the flags and vars Emerald's scripts set in the county's maps, classified by name rather than by reading each script. Strictly ordered chapters in one county merge into one chain var; one-shot gifts and independent visits are one bool each. Fan-club gifts, Shoal Cave salt and shells, and the Lilycove paintings count as bools but could become counters.

### 0. Littleroot — (1,3), Land, Temperate fresh-water swamp forest

- City: Littleroot, theme START ("A town that can't be shaded any hue."; tag start). Roles: home healing, bedroom PC (no Pokémon Center, no Mart).
- Places: Birch's Lab (Littleroot; inside; institution).
- Steps: 0 Starter (done when `FLAG_SYS_POKEMON_GET`, Route101:221). 3 unlock Oldale via Starter.
- Storage: gets 4 (Starter; callback: Pokédex, Poké Balls, Running Shoes), max list 3, chain vars 0, bools 0.

### 1. Oldale — (1,2), Land, Temperate shrublands

- City: Oldale, theme SCARCE ("Where things start off scarce."; tag village). Roles: Center, Mart only.
- Places: Altering Cave (Route 103; outside; cave; requires Elite Four).
- Steps: 4 RB1 (Route 103); sends the player back to Birch (`FLAG_DEFEATED_RIVAL_ROUTE103`, Route103:143). 5 callback to Littleroot (0): Pokédex, Poké Balls, Running Shoes (`FLAG_RECEIVED_POKEDEX_FROM_BIRCH`, Lab:548; `FLAG_RECEIVED_RUNNING_SHOES`, Littleroot:900; the balls have no flag). 6 none yet (generated as normal). 7 unlock Petalburg via Pokédex (the sketch artist also reads it as `FLAG_ADVENTURE_STARTED`).
- Storage: gets 1, max list 1, chain vars 0, bools 1.

### 2. Petalburg — (0,2), Land, Temperate woodland

- City: Petalburg, theme NATURE ("Where people mingle with nature."; tag nature). Roles: gym. Gym 5: Normal, Norman.
- Places: Petalburg Woods (Route 104; outside; forest), Pretty Petal Flower Shop (Route 104; outside; roadside building).
- Steps: 8 Wally has Ralts (`VAR_PETALBURG_CITY_STATE >= 3`, Petalburg:47; also the lock on Route 104's gym boy); Aqua beat (Woods grunt; `VAR_PETALBURG_WOODS_STATE >= 1`, PetalburgWoods:111). 10 Wailmer Pail (`FLAG_RECEIVED_WAILMER_PAIL`). 11 unlock Rustboro via Wailmer Pail.
- Storage: gets 5 (+ callback G5 and Surf), max list 2, chain vars 0, bools 6 (Woods, Miracle Seed, flower shop, 3 Route 104 gifts).

### 3. Rustboro — (0,1), Land, Temperate woodland

- City: Rustboro, theme INDUSTRY ("The city probing the integration of nature and science."; tag industry). Roles: gym, school. Gym 1: Rock, Roxanne.
- Places: Devon Corporation (Rustboro; inside; institution).
- Steps: 12 G1 + Cut (`FLAG_BADGE01_GET`, `FLAG_RECEIVED_HM_CUT`); Aqua beat (Stop Thief; `VAR_RUSTBORO_CITY_STATE >= 2`, RustboroCity:302). 14 Devon Goods recovered (Peeko, Route 116; `FLAG_RECOVERED_DEVON_GOODS`); PokéNav (`FLAG_RECEIVED_POKENAV`); Letter (`FLAG_RETURNED_DEVON_GOODS`, the return gives the Letter); RB2 (optional: `FLAG_DEFEATED_RIVAL_RUSTBORO` or `FLAG_DEFEATED_RIVAL_ROUTE_104`). 15 unlock Dewford via Letter (Briney's boat, port from Petalburg).
- Storage: gets 7, max list 4, chain vars 1 (fossil revival, merged with Mirage Tower's pick), bools 5.

### 4. Dewford — (0,3), Ocean, Temperate giant rainforest

- City: Dewford (port), theme TINY_ISLAND ("A tiny island in the blue sea."; tag island). Roles: gym, hall (no Mart). Gym 2: Fighting, Brawly.
- Places: Granite Cave (Route 106; outside; cave; requires Flash), Island Cave (Route 105; outside; cave; requires Surf).
- Steps: 16 G2 + Flash (`FLAG_BADGE02_GET`, `FLAG_RECEIVED_HM_FLASH`); Old Rod (`FLAG_RECEIVED_OLD_ROD`). 18 Letter delivered to Steven (Granite Cave; `FLAG_DELIVERED_STEVEN_LETTER`). 19 unlock Slateport via Letter delivered (Briney's boat).
- Storage: gets 4, max list 3, chain vars 0, bools 2.

### 5. Slateport — (2,3), Land, Temperate grassland

- City: Slateport (port), theme PORT ("The port where people and Pokémon cross paths with nature."; tags coast, port). Roles: market, fan club, service.
- Places: Oceanic Museum (Slateport; inside; museum), Stern's Shipyard (Slateport; inside; harbor; requires Devon Goods).
- Steps: 20 Shipyard visit, Stern (`FLAG_DOCK_REJECTED_DEVON_GOODS` or `FLAG_DELIVERED_DEVON_GOODS`). 22 Museum: Devon Goods delivered and the Aqua beat (two grunts, Archie), one scene (`FLAG_DELIVERED_DEVON_GOODS`); Harbor Mail (optional). 23 unlock the Cycling Road county via Devon Goods delivered.
- Storage: gets 5 (+ callback Aqua beat), max list 3, chain vars 2 (harbor and Stern; fan club), bools 10 (6 fan-club gifts).

### 6. Cycling Road — (2,2), Land, Temperate shrublands

- No city.
- Places: Cycling Road (Route 110; outside; route feature), Trick House (Route 110; outside; roadside building), Pokémon Daycare (Route 117; outside; roadside building).
- Steps: 24 RB3 (Route 110) and Itemfinder (two stored chapters in this county's slots 0 and 1, run back to back from Route 110's rival script; the script also sets `VAR_ROUTE110_STATE` for its coord events). 26 none (Trick House puzzle 1 is optional). 27 unlock Mauville via RB3.
- Storage: gets 2, max list 2, chain vars 5 (Trick House, 17 vars packed), bools 2 (Daycare egg pending, one other).

### 7. Mauville — (2,1), Land, Mangrove swamp

- City: Mauville, theme ENTERTAINMENT ("The bright and shiny city of fun!"; tag entertainment). Roles: gym. Gym 3: Electric, Wattson.
- Places: Mauville Game Corner (Mauville; inside; entertainment), New Mauville (below Mauville, from Route 110; outside; facility; requires Surf), Trainer Hill (Route 111 north of Mauville; outside; roadside building; requires Elite Four).
- Steps: 28 Wally battle 1 (optional: `FLAG_DEFEATED_WALLY_MAUVILLE`). 30 G3 + Rock Smash (`FLAG_BADGE03_GET`, `FLAG_RECEIVED_HM_ROCK_SMASH`); Coin Case from Harbor Mail (optional: `FLAG_RECEIVED_COIN_CASE`); Mach Bike or Acro Bike (`FLAG_RECEIVED_BIKE`). 31 unlock Verdanturf via Rock Smash + Dynamo Badge (Rusturf debris).
- Storage: gets 5, max list 4, chain vars 1 (New Mauville: key, generator, report), bools 4.

### 8. Verdanturf — (1,1), Mountain, Savanna

- City: Verdanturf, theme HIGHLAND ("The windswept highlands with the sweet fragrance of grass."; tag highland). Roles: Center, Mart only.
- Places: Rusturf Tunnel (between Route 116 and Verdanturf; outside; tunnel; requires Cut).
- Steps: 32 Strength from Rusturf Tunnel (`FLAG_RECEIVED_HM_STRENGTH`, RusturfTunnel:60; Emerald-only exception 3). 35 unlock Fallarbor via Fiery Path entered (implicit: `FLAG_LANDMARK_FIERY_PATH`; Magma blocks Mt. Chimney, so the way is Fiery Path to Route 112 north and Route 113).
- Storage: gets 1, max list 1, chain vars 0, bools 3.

### 9. Fallarbor — (1,0), Mountain, Savanna

- City: Fallarbor, theme FARMLAND ("A farm community with small gardens."; tag farmland). Roles: service.
- Places: Glass Workshop (Route 113; outside; roadside building).
- Steps: 39 unlock Meteor Falls via a blocker. No gets.
- Storage: gets 0, max list 0, chain vars 1 (Glass Workshop 17 values plus Fossil Maniac 3), bools 5.

### 10. Meteor Falls — (0,0), Mountain, Savanna

- No city.
- Places: Meteor Falls (between Route 114 and 115; outside; cave; requires Waterfall), Desert Underpass (Route 114 near Fallarbor; outside; cave; requires Elite Four).
- Steps: 40 Magma beat: Meteorite stolen, Archie appears (`FLAG_MET_ARCHIE_METEOR_FALLS`). 43 unlock Lavaridge via Meteorite stolen (cable car).
- Storage: gets 1, max list 1, chain vars 0, bools 2.

### 11. Lavaridge — (2,0), Mountain, Thorn forest

- City: Lavaridge, theme HOT_SPRINGS ("An excellent place for relaxing!"; tags volcano, hot springs). Roles: gym, herb shop, hot springs. Gym 4: Fire, Flannery.
- Places: Mt. Chimney (Route 112 near Lavaridge; outside; volcano), Fiery Path (Route 112; outside; tunnel), Jagged Pass (Route 112; outside; mountain; requires Mt. Pyre, 14), Magma Hideout (Jagged Pass; outside; hideout; requires Mt. Pyre, 14), Mirage Tower (Route 111; outside; tower), Desert Ruins (Route 111; outside; ruins).
- Steps: 44 Magma beat (Mt. Chimney, Meteorite recovered; `FLAG_DEFEATED_EVIL_TEAM_MT_CHIMNEY`); G4 (Strength already held; `FLAG_BADGE04_GET`). 45 callback to Petalburg (2): G5 + Surf (Norman, Wally's father; `FLAG_BADGE05_GET`, `FLAG_RECEIVED_HM_SURF`). 46 Go-Goggles (rival meeting outside the gym; `FLAG_RECEIVED_GO_GOGGLES`). 47 unlock the Weather Institute county via Surf (Route 118 river).
- Storage: gets 4 (+ callback Magma beat), max list 2, chain vars 1 (Mirage Tower and fossil pick), bools 4.

### 12. Weather Institute — (3,0), Land, Tropical rainforest

- No city. Fortree's county is (3,1), the reverse of the real map, so the order Weather Institute, then Fortree keeps every consecutive pair of counties adjacent (exception 17).
- Places: Weather Institute (Route 119 west of Fortree; outside; facility).
- Steps: 48 Aqua beat (Shelly; Castform; `FLAG_RECEIVED_CASTFORM`, Route119_WeatherInstitute_2F:123). 50 RB4 (Route 119 north; `VAR_ROUTE119_STATE >= 1`, Route119:145); Good Rod (Route 118 river fisherman; `FLAG_RECEIVED_GOOD_ROD`, Route118:36). 51 unlock Fortree via RB4.
- Storage: gets 3, max list 2, chain vars 0, bools 1.

### 13. Fortree — (3,1), Land, Mangrove swamp

- City: Fortree, theme TREETOP ("The treetop city that frolics with nature."; tag forest). Roles: gym, service. Gym 6: Flying, Winona.
- Places: Ancient Tomb (Route 120; outside; ruins; requires Flash), Berry Master's house (Route 123; outside; roadside building).
- Steps: 52 Devon Scope (Route 120; `FLAG_RECEIVED_DEVON_SCOPE`). 54 G6 + Fly (`FLAG_BADGE06_GET`, `FLAG_RECEIVED_HM_FLY`, set in Route119 with RB4, so true early). 55 unlock Mt. Pyre via G6 (Aqua rushes toward Mt. Pyre).
- Storage: gets 3, max list 2, chain vars 0, bools 4 (Wingull errand pair shared with 17).

### 14. Mt. Pyre — (4,1), Land, Tropical fresh-water swamp forest

- No city. Forced before Lilycove (exception 10).
- Places: Mt. Pyre (Route 122; outside; mountain).
- Steps: 56 Aqua beat (Red Orb stolen; Magma Emblem; `FLAG_RECEIVED_RED_OR_BLUE_ORB`, MtPyre_Summit:61). 57 callback to Lavaridge (11): Magma Hideout (Emblem + Strength), Magma beat (Groudon awakened; `FLAG_GROUDON_AWAKENED_MAGMA_HIDEOUT`). 59 unlock Lilycove via Groudon awakened.
- Storage: gets 1, max list 1, chain vars 0, bools 2.

### 15. Lilycove — (4,0), Land, Tropical seasonal rainforest

- City: Lilycove (capital, port), theme CAPITAL_COAST ("Where the land ends and the sea begins."; tags coast, port, capital). Roles: motel, fan club, service.
- Places: Scorched Slab (east of Fortree; outside; cave), Ancient Ruins (listed without a kind), Safari Zone (north of Route 121; outside; park), Aqua Hideout (Lilycove, by the harbor; outside; hideout), Lilycove Department Store (Lilycove; inside; store), Contest Hall (Lilycove; inside; entertainment; the other towns' Contest Halls are small buildings in their towns, not special locations).
- Steps: 60 RB5 (optional: `FLAG_MET_RIVAL_LILYCOVE`); Pokéblock Case (`FLAG_RECEIVED_POKEBLOCK_CASE`). 61 callback to Slateport (5): Aqua beat, Archie takes Stern's submarine (`VAR_SLATEPORT_HARBOR_STATE >= 2`, Harbor:64). 62 Aqua beat (Aqua Hideout: Matt, Master Ball, blockade gone; `FLAG_TEAM_AQUA_ESCAPED_IN_SUBMARINE`). 63 unlock the Shoal Cave county via blockade gone.
- Storage: gets 3, max list 2, chain vars 2 (museum; fan club), bools 11 (5 paintings).

### 16. Shoal Cave — (5,0), Ocean, Tropical rainforest

- No city.
- Places: Shoal Cave (Route 125; outside; cave; requires Surf), Treasure Hunter's House (Route 125; outside; roadside building).
- Steps: 67 unlock Mossdeep via a blocker. No gets.
- Storage: gets 0, max list 0, chain vars 0, bools 10 (4 salt, 4 shell).

### 17. Mossdeep — (5,1), Ocean, Tropical fresh-water swamp forest

- City: Mossdeep, theme CHERISH ("Our slogan: Cherish Pokémon!"; tag island). Roles: gym. Gym 7: Psychic, Tate and Liza (one double-battle leader entry; the stored trainer id is Tate and Liza's).
- Places: Mossdeep Game Corner (Mossdeep; inside; entertainment), Space Center (Mossdeep; inside; institution).
- Steps: 68 G7 + Dive (`FLAG_BADGE07_GET`, `FLAG_RECEIVED_HM_DIVE`); Magma beat (Space Center, Steven; `VAR_MOSSDEEP_SPACE_CENTER_STATE >= 3`, since `FLAG_DEFEATED_MAGMA_SPACE_CENTER` is cleared again later); Super Rod (`FLAG_RECEIVED_SUPER_ROD`). 70 Aqua beat (Seafloor Cavern: Shelly, Archie; Kyogre awakened; `FLAG_KYOGRE_ESCAPED_SEAFLOOR_CAVERN`; the cavern's tile sits in Ever Grande's cell, exception 9). 71 unlock Sootopolis via Dive + Kyogre awakened.
- Storage: gets 5, max list 4, chain vars 0, bools 4.

### 18. Sootopolis — (4,2), Ocean, Temperate giant rainforest

- City: Sootopolis, theme ANCIENT ("The mystical city where history slumbers."; tags island, ancient). Roles: gym. Gym 8: Water, Juan.
- Places: Cave of Origin (Sootopolis; inside; sacred cave), Victory Road (by Ever Grande; outside; tunnel; hosted from this county, exception 9).
- Steps: 72 Cave of Origin: Groudon/Kyogre climax, Wallace heads to Sky Pillar (`FLAG_WALLACE_GOES_TO_SKY_PILLAR`). 75 unlock Pacifidlog via Cave of Origin done (Surf held). Callback gets hosted here: Rayquaza ends the fight and G8 + Waterfall (block 19, step 1), Victory Road and Wally battle 2 (block 20, step 1).
- Storage: gets 6 (+ callbacks Rayquaza ends fight, G8, Waterfall, Victory Road, Wally 2), max list 3, chain vars 1 (Mystery Events house, 4 values), bools 2.

### 19. Pacifidlog — (4,3), Ocean, Temperate giant rainforest

- City: Pacifidlog, theme OCEAN_SUNRISE ("Where the morning sun smiles upon the waters."; tag ocean). Roles: service (no Mart).
- Places: Sky Pillar (north of Route 131; outside; tower).
- Steps: 76 Sky Pillar: Rayquaza awakened (`VAR_SKY_PILLAR_STATE >= 1`, SkyPillar_Top:127). 77 callback to Sootopolis (18): Rayquaza ends the fight (`FLAG_SOOTOPOLIS_ARCHIE_MAXIE_LEAVE`); G8 + Waterfall (`FLAG_BADGE08_GET`, `FLAG_RECEIVED_HM_WATERFALL`). 79 unlock Ever Grande via Waterfall + G8 (Rain Badge).
- Storage: gets 1, max list 1, chain vars 1 (weekly TM day), bools 3.

### 20. Ever Grande — (5,2), Land (island), Temperate giant rainforest — Elite Four

- City: Ever Grande, theme LEAGUE ("The paradise of flowers, the sea, and Pokémon."; tags league, coast). Roles: league (not a gym). Gym ordinal 9: Elite Four Sidney, Phoebe, Glacia, Drake; Champion Wallace (the county stores the Champion's trainer id and no gym type).
- Places: Seafloor Cavern (Route 128; outside; underwater).
- Steps: 80 none (the end game is open). 81 callback to Sootopolis (18): Victory Road, Wally battle 2 (`FLAG_DEFEATED_WALLY_VICTORY_ROAD`). 82 League lobby (8 badges), Elite Four, Champion (`FLAG_ENTERED_ELITE_FOUR`, then `FLAG_SYS_GAME_CLEAR`, set in C at the Hall of Fame). 83 unlock the reserved counties via the Hall of Fame (open). Two gates: Waterfall + Rain Badge into Ever Grande, 8 badges into the League lobby (exception 13).
- Storage: gets 3, max list 3, chain vars 0, bools 1.

### 21. Mirage Island — (5,3), Ocean, Temperate giant rainforest — reserved

- No city. Steps 84-95 are post-game and open.
- Places: Mirage Island (Route 130; Enclave, script; island), Battle Frontier (south of Route 130; Enclave, boat; island; requires Elite Four), Artisan Cave (inside Battle Frontier; Enclave; cave; requires Elite Four). Southern Island and the Battle Frontier are mapped Enclaves, Mirage Island and Artisan Cave hidden (design_region.md "Enclaves").

### 22. Sealed Chamber — (3,3), Ocean, Temperate giant rainforest — reserved

- No city. Post-game and open (exception 15).
- Places: Sealed Chamber (Route 134; outside; underwater), Southern Island (Southern Hoenn; Enclave, boat/Eon Ticket; island; requires Eon Ticket).

### 23. Empty county — (3,2), Land, Temperate fresh-water swamp forest — reserved

- No city, special location or route tile: Emerald has nothing reasonable to put there, accepted for the hardcoded stage. It takes the last reserved index and holds no legendary location (exception 14).

### Special location counts

Counted from the entries above, classifying each place by where it is in Emerald (gyms, the Elite Four, rival battles and HM pickups excluded; Glass Workshop, Pretty Petal Flower Shop, Trainer Hill, Trick House, Daycare and Berry Master are on routes):

- Outside cities: 38 over 23 non-empty counties, about 1.65 per county (busiest: Lavaridge's county with 6).
- Inside cities: 10 over 16 cities, about 0.6 per city (Devon Corp, Mauville Game Corner, Lilycove Dept. Store and Contest Hall, Mossdeep Game Corner and Space Center, Cave of Origin, Birch's Lab, Shipyard, Museum). Counting the 8 gyms and the Elite Four gives 19, about 1.2 per city.
- The entries list only notable places, so these are lower bounds.

Themes and gyms. A theme is the city's Bulbapedia slogan (its `motto`, the text NPCs use via `$city.theme.motto`, design_chapters.md "Text references") plus requirement tags that explain why it fits; in Emerald each of the 16 cities has its own, and generated regions will need more authored themes. The tags are a starting vocabulary, not final. Gym types do not follow from the slogans (Dewford is Fighting), so in the hardcoded stage gym type is its own per-city field, not a theme property; design_region.md's "theme affects gym type" stays a goal for the generator. Every Pokémon of gyms 1-8 was verified once offline against `trainers.party` to have the gym's type (the test runner has no real trainer data). Building roles per city are best fits, not verified per building.

## Special-location kinds and building roles

Source: each place's Bulbapedia infobox (`slogan` for towns and cities, `location` and `type` for other places). A place whose location is a city is inside it; one on, between or near routes is outside. Bulbapedia lists Aqua Hideout and Victory Road under their cities, but both are reached outside the city proper, so they count as outside. The Magma Hideout is inside Jagged Pass on the map; the stage treats it as its own tile.

### Special-location kinds

Two enums, since a kind only matters for eligibility (which hosts may take it, what an arc beat needs) and NPC text, and inside and outside never share a kind.

- `RouteLocation` (outside a city): cave, tunnel (links two routes or a route and a town), volcano, mountain, forest, ruins, tower, hideout, island, underwater, wreck, park, facility (power plant, weather institute), roadside building (a small standalone building on a route), route feature (Cycling Road).
- `TownLocation` (inside a city): museum, entertainment (Game Corner, Contest Hall), store (Department Store), harbor (shipyard), institution (company, lab, agency), sacred cave.

Ordinary caves are outside only; the Cave of Origin, Hoenn's single in-city cave, is the `sacred cave` town kind (a cave entered from a city, tied to a legendary or ancient site).

### Building roles

A building role is what an NPC can refer to ("the museum", "the Pokémon Center"). In-city special locations supply their own role. Roles seen across the 16 cities:

- Everywhere but the start: Pokémon Center. Almost everywhere: Mart (absent in Littleroot, Dewford and Pacifidlog). Gym where there is one.
- Start town only: home with a mother who heals the party, a bedroom PC.
- Community and shopping: house (generic), market (Slateport Market), department store, motel (Cove Lily Motel), herb shop, hot springs, hall (Dewford Hall), fan club (several cities), trainers' school, secret base guild, trade house, and service (name rater, friendship rater, move tutor, move reminder, move deleter, merged: the move reminder's and deleter's houses use the generic `LAYOUT_HOUSE2`, so there is no unique tile to map, and tutors are NPCs).
- Story-bound (inside-city special locations): lab, company, museum, shipyard, Game Corner, Contest Hall, space center, cave entrance.

Placeholder NPC objects per host map (design_chapters.md, "Chapter abstraction", Placement): 2 for each building role, 3 for a gym, 1 for a service building, 4 for a city's outdoor map; routes get none.

## Rival arc

- You scare off a Pokémon, the professor gives you two Pokémon, then May/Brendan battles you.
- On a route just before the first gym, the rival shows up and battles again, then again on a route between the second and third gyms, on a route between the fourth and fifth gyms, and in some side area between the sixth and seventh gyms.
- A secondary rival, Wally, challenges you just before the third gym and as the Victory Road champion.

## Story arc

Two eco-terrorist factions, Magma and Aqua, argue industrial progress versus environmental preservation. They awaken the Ground and Sea legendary Pokémon and lose control of them. Then a Flying legendary arrives from a tower landmark and pacifies them, and Magma and Aqua learn the error of their ways.

## Chapters

- A man in sunglasses teaches you Cut.
- Mr. Briney's Wingull is stolen.
- You can't enter Devon Corp until you reclaim the Devon Goods from a tunnel.
- Answering questions earns the Old Rod.
- Granite Cave requires Flash.
- Delivering the Devon Goods to Captain Stern.

## Planner inputs

What the main plot planner (design_story.md, "Main plot planner") reads for Emerald. Two layers, because the Steps lines above cannot come out of the hash formulas (step bits, callback hashes and the callback chance are random): the vanilla slots (each item's `{block, step}` and each callback block's host, exactly the Steps lines above; data in `emerald_plot.h`) override the hash outcome while the planner's randomization bit is off (design_story.md, "Vanilla stage overrides"), and the generic constants below are what the randomized path uses. Every vanilla slot is checked against the generic ranges and masks; the ones that fall outside are the numbered exceptions below.

Constants: E = 20 (Elite Four index), R = 3, Surf-lock block n = 11, Fly's block 13. Utility items placed: PokéNav, Old Rod, Itemfinder, Bicycle, Coin Case (optional chain), Pokéblock Case, Super Rod, and Good Rod (the Route 118 river fisherman; the lakes make it mandatory); Harbor Mail (optional chain) is not placed. Tutorial lists (blocks 0 and 1) are the hardcoded ones in design_story.md, "Opening".

### Host masks (bit i = county-story-index i)

- Gym and gym HM (Cut, Flash, Rock Smash, Strength): 1-19 (`0x000FFFFE`); county 0 only as a callback.
- Water HM (Surf, Dive, Waterfall): blocks where county x or x+1 is Ocean terrain: 3, 4, 15-19 (`0x000F8018`). The vanilla Surf block 11 is outside it (Route 118's river between two land counties; exception 2).
- Rival beats and utility items: 1-19 (`0x000FFFFE`).
- Arc beats: each beat's own host only (the location kind it needs exists once); the mask is the beat's block.
- A callback host y(x) must pass the kind's mask and, for a gym, have a settlement.

### Callback chance

- Randomized path: p = 51/256 (about 20%). Vanilla has 5 callbacks in the 19 blocks after the tutorial (26%), forced by the list above, so p never applies to the vanilla stage.

### Placeholder ranges (randomized path, for the user to check)

Windows are blocks (inclusive). Gyms 1-4 split blocks 1..n-1 into four windows with cut points ceil(k(n-1)/4); gyms 6-8 split n+1..E-1 into three with cut points n + ceil(k(E-1-n)/3). With n = 11, E = 20 these evaluate as shown; "vanilla" is the vanilla block.

| Item | Range (n=11, E=20) | Vanilla | In range |
| --- | --- | --- | --- |
| G1 / G2 / G3 / G4 | 1-3 / 4-5 / 6-8 / 9-10 | 3 / 4 / 7 / 11 | G4 no (exception 1) |
| G5 (Surf gym) | n only: 11 | 11 | yes |
| G6 (Fly, never a callback) / G7 / G8 | 12-14 / 15-17 / 18-19 | 13 / 17 / 19 | yes |
| HMs | the gym's slot (Strength included in the randomized path) | | Strength no (exception 3) |
| RB1 | 1 (tutorial) | 1 | yes |
| RB2 (before G1's window ends) | 2 to cut 1 = 2-3 | 3 | yes |
| RB3 (gyms 2-3) | 4-8 | 6 | yes |
| RB4 (gyms 4-5) | 9-11 | 12 | no (exception 16) |
| RB5 (gyms 6-7) | 12-17 | 15 | yes |
| Wally 1 / Wally 2 | G3's window 6-8 / E only: 20 | 7 / 20 | yes |
| Arc beats (authored per arc) | nominal block +-2, clipped to 2..E-1; ordered by arc sequence, several per block allowed | the blocks above | yes by construction |
| Bicycle, Itemfinder | 1 to floor(E/2) = 1-10 | 7, 6 | yes |
| Old Rod, PokéNav | 1 to ceil(E/3) = 1-7 | 4, 3 | yes |
| Good Rod | ceil(E/3)+1 to floor(2E/3) = 8-13 | 12 | yes |
| Super Rod | floor(2E/3)+1 to E-1 = 14-19 | 17 | yes |
| Other utility (Coin Case, Pokéblock Case, ...) | 1 to E-1 | 7, 15 | yes |

Arc beats cannot use disjoint windows (design_story.md says ranges of a kind are disjoint) because the Emerald arc has two beats in blocks 14, 15, 17 and 19. They are ordered instead: beats resolve in arc order, each beat's block is at least the previous beat's, and two beats never share a (block, step). Quotas: 8 gyms + League, 8 HMs, 5 rival beats + 2 Wally beats, 15 arc beats + 3 Devon Goods beats (Emerald arc), 8 placed utility items.

### Step predicates for the read-only adapter

A list is done when every blocking get in it is true; the current index is the first list, scanning from 0, that is not done (Emerald lets some gets happen early: Fly with RB4, Strength at Rusturf, Go-Goggles, so the highest true predicate is wrong). A step-3 unlock is done when its named key's predicate holds; the 2 blockers (39, 67) are done when the previous list is, and the implicit unlocks (35 Fiery Path entered, 47 the Route 119 rival) read their stand-in event, reported-only. Gets are blocking or reported-only. Blocking means vanilla forces the get (a scene or gate) or needs it by the end game; reported-only gets are reported in the done mask but never hold a list, because a skippable blocking get (the Good Rod's fisherman has decline and bag-full branches that never set its flag, and nothing later reads it) would freeze the index for good. When in doubt a get is reported-only: a forced one then costs only a hint that moves on early. So the planner's "Good Rod is mandatory" is a rule for the randomized path, not something vanilla enforces. Reported-only: RB2, RB5, Wally 1, the Museum delivery, the Shipyard visit, Shelly, Meteor Falls and Mt. Chimney beats, every utility and special item, Cut, Flash, Rock Smash, Strength and Fly, Running Shoes and the Poké Balls (no flag; `STORY_PRED_ALWAYS`). Setter sites were found with `setflag`/`setvar` in `data/` (non-Frlg), and every predicate was checked to be monotone (no `clearflag`, no lower `setvar` later). Code: `include/story/plot_stage.h`, `src/story/plot_stage.c`, data `src/data/region/emerald_plot_gets.h`, test `test/region_plot_stage.c`; the table below is the reading, the data file is the classification.

Resolved readings: the Woods grunt is `VAR_PETALBURG_WOODS_STATE >= 1`, set after the battle on both paths; Stop Thief is `VAR_RUSTBORO_CITY_STATE >= 2` (the var only climbs 1 to 8; `FLAG_DEVON_GOODS_STOLEN` is cleared later); the Shipyard visit is `FLAG_DOCK_REJECTED_DEVON_GOODS` or `FLAG_DELIVERED_DEVON_GOODS` (the dock checks the latter first); RB3 is `VAR_ROUTE110_STATE >= 1` and the Itemfinder, which has no flag, is given in the same scene; RB4 is `VAR_ROUTE119_STATE >= 1`; the submarine is `VAR_SLATEPORT_HARBOR_STATE >= 2` (MagmaHideout_4F sets 1); Sky Pillar is `VAR_SKY_PILLAR_STATE >= 1`; the Good Rod's flag is set at Route118:36. The extra Route 104 rival (`FLAG_DEFEATED_RIVAL_ROUTE_104`) is RB2 at a second venue, not a sixth battle: both venues use the `TRAINER_MAY_RUSTBORO_*`/`BRENDAN` parties and set `VAR_RUSTBORO_CITY_STATE` 8 and `VAR_ROUTE104_STATE` 2, which disables the other trigger, so RB2 is `FLAG_DEFEATED_RIVAL_RUSTBORO` or `FLAG_DEFEATED_RIVAL_ROUTE_104`. `FLAG_DEFEATED_MAGMA_SPACE_CENTER` is cleared again by the Rayquaza call (MossdeepCity_SpaceCenter_2F:328), so the Space Center beat reads `VAR_MOSSDEEP_SPACE_CENTER_STATE >= 3` instead.

### Traversal McGuffins (route unlocks only)

HMs other than Fly, each needing the HM and its gym's badge (`needs:` lines in `Emerald_walkthrough.md` list every lock; the Steps above give the unlocks they trigger: Rock Smash at Rusturf opens Verdanturf, Surf at Route 118 opens the Weather Institute county, Dive at Sootopolis and Waterfall at Ever Grande unlock those counties). Flash: see design_story.md, "Dark maps".

- Special items (as designed, fixed in this stage): Go-Goggles (Route 111 desert), Devon Scope (Route 120 bridge and Fortree gym entrance), Wailmer Pail (the planner's placeholder lock for Rustboro, index 11; vanilla Emerald has no lock there).
- Not traversal: the Bicycle (a utility item; it gates the Cycling Road and Jagged Pass optionally), the Basement Key, Storage Key and Magma Emblem (chapter and arc keys), Fly (a convenience, never a lock).

### Blocker McGuffins

A person standing in the way until a boolean. Emerald has these (chain entry, who, boolean):

| Entry | Blocker | Until |
| --- | --- | --- |
| 6 | Sketch artist, Oldale west exit | `FLAG_ADVENTURE_STARTED` (Oldale:8, set in Birch's Lab:519) |
| 14 | Gym boy, Petalburg west exit | `VAR_PETALBURG_CITY_STATE >= 3` (Petalburg:8 moves him, Petalburg:47 frees the path) |
| 133 | Ace Trainers, League lobby | 8 badges (popcount of `FLAG_BADGE01_GET`-`08`) |
| 41 | Cycling Road gate | Bicycle held (`FLAG_RECEIVED_BIKE`) |
| 13/77 | Norman, Petalburg gym | 4 badges (derived stage, design_chapters.md "Interface rules from paper tests") |
| 36, 55 | Aqua grunts at the Museum, Magma grunts at the cable car | arc beats owned by the arc (main arc bucket); not generic blockers |

The two planner blockers (indices 39, 67: unlocks into Meteor Falls and Mossdeep) have no Emerald counterpart yet. Until the pool grows (design_story.md, "Blocker McGuffins"), chapters author their own blocker scripts; the first generic blockers are those two, written as one parameterized "NPC blocks the route until the index passes step s" script.

## Per-county storage needs

The per-county figures are in "Counties" above.

Totals: 64 gets, 20 unlocks (blocks 0-19), 2 of them blockers (indices 39, 67) and 2 implicit (35, 47); about 15 chain vars and about 82 bools. At most 11 bools in any county, and about 380 flags are unused today, so a fixed 12-flag pool per county would fit (288) without packing. Not counted: post-game content (Battle Frontier, Southern Island, Regi trio, Altering Cave), the 7 Trick House and Mossdeep gym switch flags (scratch), per-city visited and landmark flags, Pokénav match-call flags, and engine vars.

- Only one list is current at a time, and the largest is 4 (Rustboro step 2, Mauville step 2, Mossdeep step 0), so one shared 4-bit completion mask would do. Per-county masks are only needed if a county's completed gets must survive past its list, which level-triggered completion makes unnecessary.
- McGuffin flags: the 9 event keys in step-3 unlocks (Letter delivered, Devon Goods delivered, RB3, Meteorite stolen, Shelly defeated, Groudon awakened, blockade gone, Kyogre awakened, Cave of Origin done) all have an Emerald flag or var (see "Planner inputs", step predicates), except that RB3 has only a var. Once the main-story-index is maintained they are derivable (a key's index is below its lock's), so they need no stored flag; the 3 blockers likewise.

## Exceptions to the normal rules

Numbered so they can be struck off as the rules or the table change.

1. G4 (Lavaridge, step 0) and G5 (Petalburg, the step-1 callback) share the Surf block 11, so G4 lies outside its window 9-10; encoded in `GetGymSlot`. Lavaridge's gym precedes Surf, which opens the Weather Institute county from Lavaridge's block, and a callback host cannot be a county that is not yet unlocked.
2. Surf's unlock (index 47) opens a land county (the Weather Institute, across Route 118's river), not an ocean county, and the Surf lock is block 11 rather than "the ocean county's dead end".
3. Strength is given at Rusturf Tunnel (index 32), long before G4 (index 44), instead of with its gym. Emerald-only; the algorithm keeps an HM at its gym's time.
4. Ports (design_region.md, "Ports"): the algorithm puts the port county first and the ocean county right after it, but here Dewford (ocean, 4) precedes Slateport (port, 5), and Briney's first trip starts at Petalburg's Route 104, not at a port city. Swapping them would put Slateport's Devon Goods delivery before Dewford's Letter. Also the third port, Lilycove (15), is later than the rule would place it (about index 10-13). Keep as an exception unless the story order changes.
5. G5's callback to Petalburg is 4 county steps from Lavaridge, beyond the pre-Fly bound of 3. Hardcoded exception.
6. Devon Goods: Peeko and the Take-Down are hosted on Route 116 (inter-county route 3-8), not inside the Rusturf Tunnel in county 8; Dewford to Slateport is keyed on "Letter delivered" (block 4's own get) instead of Devon Goods; the delivery is a block-5 get that relies on index order, not a key check.
7. Fiery Path, Route 111 north and Route 112 north are walked on the way to Fallarbor before county 11 unlocks, so they move into county 8's cell. Verdanturf's cell is otherwise nearly empty.
8. Rival Battle 3 moves from the Mauville cell to the Cycling Road cell (Route 110).
9. Seafloor Cavern moves from the Ever Grande cell to Mossdeep's. Victory Road lives in Sootopolis's cell, so Wally battle 2 is a callback.
10. Mt. Pyre (14) is forced before Lilycove (15); Emerald lets the player visit Lilycove first.
11. Blocks 9 and 16 have no mandatory gets, so their unlocks need blocker McGuffins that vanilla Emerald lacks (or a tiny chapter each). Block 8 (Fallarbor) has no get to name either, so its unlock is implicit: Fiery Path entered.
12. Verdanturf's unlock uses Rock Smash + Dynamo Badge, which also gates Route 117; vanilla leaves it open.
13. The Elite Four county has two gates: Waterfall + Rain Badge into Ever Grande, and 8 badges into the League lobby.
14. The empty county (3,2) must take a reserved index (23) and holds no legendary location.
15. Sealed Chamber and the Regi trio are mid-game in Emerald but post-Elite Four here (reserved counties).
16. RB4 (block 12, step 2) lies outside its window 9-11: the Route 119 rival follows Shelly, and Surf's block 11 precedes the Weather Institute county.
17. Fortree's city sits in cell (3,1) and the Weather Institute in (3,0), the reverse of the real map, so the story order Weather Institute, then Fortree keeps every consecutive pair of counties adjacent.

## Optional content with inter-county story

These do not use planner slots. Each is gated by keys from any earlier index (design_story.md, "Main plot planner" rules) and never blocks the main story. They need an optional-story index (tentatively a var each, see `remaining_work.md`) so NPCs in other counties can hint at them.

Item and key chains:

| Content | Host county | Gated by (source block) | Chain |
| --- | --- | --- | --- |
| Fossil revival | 11 (Mirage Tower), 3 | Mach Bike (7), Rock Smash (7) | Mirage Tower fossil, revive at Devon Corporation |
| Abandoned Ship | 0 | Surf (11), Dive (17) | Storage Key, Scanner, Captain Stern (5), DeepSeaTooth/Scale |
| Harbor Mail | 5, 7 | Devon Goods delivered (5) | Mail, Coin Case, Mauville Game Corner |
| Secret Power | 8, 5 | none | TM Secret Power (Route 111 north), Slateport Market bargains |
| Meteorite TM | 9 | Meteorite recovered (11) | Professor Cozmo's TM |
| New Mauville | 7, 6 | Surf (11), Basement Key (7) | Generator, Thunderbolt |
| Trick House | 6 | badges and field moves | Inner-county indexing is enough |

Regi trio (unlocked in the Sealed Chamber, county 22):

| Step | Host | Gated by (source block) |
| --- | --- | --- |
| Dive to the Sealed Chamber | 22 | Dive (17) |
| Braille wall | 22 | TM Dig (Route 114, 9) |
| Back wall | 22 | Wailord first, Relicanth last in party (Dive, rods) |
| Regirock (Desert Ruins) | 11 | Sealed Chamber opened, Rock Smash (7), Go-Goggles (11) |
| Regice (Island Cave) | 4 | Sealed Chamber opened, Surf (11) |
| Registeel (Ancient Tomb) | 13 | Sealed Chamber opened, Flash (4) |

Post-Elite Four unlocks, all listed "requires Elite Four" in the county table:

| Content | Host county |
| --- | --- |
| Altering Cave | 1 |
| Trainer Hill | 7 |
| Desert Underpass | 10 |
| Battle Frontier, Artisan Cave, Mirage Island | 21 |
| Southern Island (Eon Ticket) | 22 |
| Rayquaza rematch (Sky Pillar, Mach Bike) | 19 |
