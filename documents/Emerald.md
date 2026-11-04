# Pokémon Emerald reference

Emerald-specific data for the hardcoded stage (design.md, "Development approach"): the Hoenn county layout, and the Emerald rival arc, story arc and subplots the first version reuses. Generic design lives in `design.md`; Emerald examples used to explain it stay there.

## County assignment

Hoenn does not fit the 6x4 county grid cleanly, so for the hardcoded stage places are fudged into counties, with everything very slightly stretched apart. Since most content is hardcoded at this stage, the stretching does not matter, and once the region is fully generated there is no fakery left.

Each county holds at most one city, with the caves and other special locations spread around it. Each cell starts with the order in which the player first visits it in Emerald, in steps of 4 (leaving room for routes between), then its city, special locations, gym (with its badge order), rival battles and HM/McGuffin acquisitions. "Requires" names what unlocks a location. County (3,2) is deliberately empty: Emerald has nothing reasonable to put there, which is accepted for the hardcoded stage.

| Row | x=0 | x=1 | x=2 | x=3 | x=4 | x=5 |
| --- | --- | --- | --- | --- | --- | --- |
| y=0 | 40. Meteor Falls (requires Waterfall); Desert Underpass (requires Elite Four) | 36. Fallarbor; Glass Workshop | 42. Lavaridge + Gym 4; Mt. Chimney; Fiery Path; Jagged Pass (requires 52/Mt. Pyre); Magma Hideout (requires 52/Mt. Pyre); Mirage Tower; Desert Ruins | 44. Fortree + Gym 6; Rival Battle 4; HM Fly; Ancient Tomb (requires Flash) | 56. Lilycove; Scorched Slab; Ancient Ruins; Safari Zone; Dept. Store; Contest Hall; Rival Battle 5; Aqua Hideout | 60. Shoal Cave (requires Surf); Treasure Hunter's House |
| y=1 | 12. Rustboro + Gym 1; Devon Corporation; Rival Battle 2; HM Cut | 32. Verdanturf; Rusturf Tunnel (requires Cut); HM Strength | 28. Mauville + Gym 3; Rival Battle 3; Game Corner; HM Rock Smash; New Mauville (requires Surf); Trainer Hill (requires Elite Four) | 48. Berry Master; Weather Institute | 52. Mt. Pyre | 64. Mossdeep + Gym 7; Game Corner; Space Center; HM Dive |
| y=2 | 8. Petalburg + Gym 5; Petalburg Woods; Pretty Petal Flower Shop; HM Surf (requires Gym 5) | 4. Oldale; Rival Battle 1; Altering Cave (requires Elite Four) | 24. Cycling Road; Trick House; Pokémon Daycare | (empty) | 72. Sootopolis + Gym 8; HM Waterfall; Cave of Origin | 68. Ever Grande + Elite Four; Seafloor Cavern |
| y=3 | 16. Dewford + Gym 2; HM Flash; Granite Cave (requires Flash); Island Cave (requires Surf) | 0. Littleroot; Birch's Lab; Abandoned Ship (requires Surf and Dive) | 20. Slateport; Shipyard (requires Devon Goods); Museum | 84. Sealed Chamber; Southern Island (requires Eon Ticket) | 80. Pacifidlog; Sky Pillar | 76. Mirage Island; Battle Frontier (requires Elite Four); Artisan Cave (requires Elite Four) |

Lavaridge is 42 (between Meteor Falls and Fortree) to keep the existing numbering.

Terrain and biome per county. Hoenn is mostly rainforest, so it uses the warm, wet biome subset (humidity 1-4, the three hottest temperature columns). Temperature falls from the top row (hottest column) downward; the desert north of Mauville is faked with Thorn forest, and the smaller one at (0,0) with Savanna. Ocean counties get a biome too, for surf, dive and fishing encounters: the maximum-humidity row, from Tropical rainforest in the top row down to Temperate giant rainforest in the bottom two rows. Dewford's island is too small for its own biome, so (0,3) is Ocean.

| Row | x=0 | x=1 | x=2 | x=3 | x=4 | x=5 |
| --- | --- | --- | --- | --- | --- | --- |
| y=0 | Mountain, Savanna | Mountain, Savanna | Mountain, Thorn forest | Land, Tropical rainforest | Land, Tropical seasonal rainforest | Ocean, Tropical rainforest |
| y=1 | Land, Temperate woodland | Mountain, Savanna | Land, Mangrove swamp | Land, Mangrove swamp | Land, Tropical fresh-water swamp forest | Ocean, Tropical fresh-water swamp forest |
| y=2 | Land, Temperate woodland | Land, Temperate shrublands | Land, Temperate shrublands | Land, Temperate fresh-water swamp forest | Ocean, Temperate giant rainforest | Land (island), Temperate giant rainforest |
| y=3 | Ocean, Temperate giant rainforest | Land, Temperate fresh-water swamp forest | Land, Temperate grassland | Ocean, Temperate giant rainforest | Ocean, Temperate giant rainforest | Ocean, Temperate giant rainforest |

### County-story-indices used in the code

`src/data/region/emerald_region.h` uses the indices of "Main-story-index table" below (walkthrough order, unique, 0-23): Lavaridge (42 above) is 11; Sootopolis is 18 and Pacifidlog 19, so Ever Grande is index 20, the Elite Four for R = 3, followed by the reserved legendary counties Mirage Island (21), Sealed Chamber (22) and the empty county (3,2) (23). Jumps (non-adjacent consecutive indices) are 3 to 4, 4 to 5, 10 to 11, 17 to 18 and 19 to 20, numbered 1-5 as key/lock ids. Ports (Dewford, Slateport, Lilycove) and special-location kinds are best fits, not final.

### Tile placement

`src/data/region/emerald_tiles.h` puts each county's city, special locations and route stubs on county-local tiles. Positions are placeholders; the county and the place names are what Emerald.md fixes. Places with no MAPSEC of their own use their route's or city's MAPSEC (design.md, "Region tiles").

### Special location counts

Counted from the county table, classifying each place by where it is in Emerald (gyms, the Elite Four, rival battles and HM pickups excluded; Glass Workshop, Pretty Petal Flower Shop, Trainer Hill, Trick House, Daycare and Berry Master are on routes):

- Outside cities: 38 over 23 non-empty counties, about 1.65 per county (busiest: Lavaridge's county with 6).
- Inside cities: 10 over 16 cities, about 0.6 per city (Devon Corp, Mauville Game Corner, Lilycove Dept. Store and Contest Hall, Mossdeep Game Corner and Space Center, Cave of Origin, Birch's Lab, Shipyard, Museum). Counting the 8 gyms and the Elite Four gives 19, about 1.2 per city.
- The table lists only notable places, so these are lower bounds.

## Catalogs (draft, from Bulbapedia)

Source: each place's Bulbapedia infobox (`slogan` for towns and cities, `location` and `type` for other places). A place whose location is a city is inside it; one on, between or near routes is outside.

### City themes

A theme is the city's Bulbapedia slogan (its `motto`, the text NPCs use via `$city.theme.motto`, design.md "Text references") plus requirement tags that explain why it fits. In Emerald each of the 16 cities has its own theme; generated regions will need more authored themes. The tags are a starting vocabulary, not final.

| City | Slogan | Tags |
| --- | --- | --- |
| Littleroot | A town that can't be shaded any hue. | start |
| Oldale | Where things start off scarce. | village |
| Petalburg | Where people mingle with nature. | nature |
| Rustboro | The city probing the integration of nature and science. | industry |
| Dewford | A tiny island in the blue sea. | island |
| Slateport | The port where people and Pokémon cross paths with nature. | coast, port |
| Mauville | The bright and shiny city of fun! | entertainment |
| Verdanturf | The windswept highlands with the sweet fragrance of grass. | highland |
| Fallarbor | A farm community with small gardens. | farmland |
| Lavaridge | An excellent place for relaxing! | volcano, hot springs |
| Fortree | The treetop city that frolics with nature. | forest |
| Lilycove | Where the land ends and the sea begins. | coast, port, capital |
| Mossdeep | Our slogan: Cherish Pokémon! | island |
| Sootopolis | The mystical city where history slumbers. | island, ancient |
| Pacifidlog | Where the morning sun smiles upon the waters. | ocean |
| Ever Grande | The paradise of flowers, the sea, and Pokémon. | league, coast |

Gym types do not follow from these slogans (Dewford is Fighting), so in the hardcoded stage gym type is its own per-city field, not a theme property; design.md's "theme affects gym type" stays a goal for the generator.

### Gyms

| Ordinal | City | Type | Leader |
| --- | --- | --- | --- |
| 1 | Rustboro | Rock | Roxanne |
| 2 | Dewford | Fighting | Brawly |
| 3 | Mauville | Electric | Wattson |
| 4 | Lavaridge | Fire | Flannery |
| 5 | Petalburg | Normal | Norman |
| 6 | Fortree | Flying | Winona |
| 7 | Mossdeep | Psychic | Tate and Liza |
| 8 | Sootopolis | Water | Juan |
| 9 | Ever Grande | (Elite Four) | Sidney, Phoebe, Glacia, Drake; Champion Wallace |

Mossdeep's two leaders count as one gym leader entry (a double battle); the trainer id stored is Tate and Liza's. The Elite Four county stores the Champion's trainer id and no gym type. The code (`src/data/region/emerald_cities.h`) also holds each gym's planner slot; the Surf exception there (gyms 5 and 6 both in block 12, Petalburg as the step-1 callback) supersedes the Lavaridge-block placement in the main-story-index table and exceptions 1, 2 and 5 below until they are reconciled. Verified once offline against `trainers.party`: every Pokémon of gyms 1-8 has the gym's type (the test runner has no real trainer data).

### City data in code

`src/data/region/emerald_cities.h`: theme enum and building roles per city (roles are best fits, not verified per building; the starting town has the home-healing and bedroom-PC roles instead of a Pokémon Center).

| City | Theme enum | Roles besides Pokémon Center and Mart |
| --- | --- | --- |
| Littleroot | START | home healing, bedroom PC (no Center or Mart) |
| Oldale | SCARCE | |
| Petalburg | NATURE | gym |
| Rustboro | INDUSTRY | gym, school |
| Dewford | TINY_ISLAND | gym, hall (no Mart) |
| Slateport | PORT | market, fan club, service |
| Mauville | ENTERTAINMENT | gym |
| Verdanturf | HIGHLAND | |
| Fallarbor | FARMLAND | service |
| Lavaridge | HOT_SPRINGS | gym, herb shop, hot springs |
| Fortree | TREETOP | gym, service |
| Lilycove | CAPITAL_COAST | motel, fan club, service |
| Mossdeep | CHERISH | gym |
| Sootopolis | ANCIENT | gym |
| Pacifidlog | OCEAN_SUNRISE | service (no Mart) |
| Ever Grande | LEAGUE | league |

### Special-location kinds

Two enums, since a kind only matters for eligibility (which hosts may take it, what an arc beat needs) and NPC text, and inside and outside never share a kind.

- `RouteLocation` (outside a city): cave, tunnel (links two routes or a route and a town), volcano, mountain, forest, ruins, tower, hideout, island, underwater, wreck, park, facility (power plant, weather institute), roadside building (a small standalone building on a route), route feature (Cycling Road).
- `TownLocation` (inside a city): museum, entertainment (Game Corner, Contest Hall), store (Department Store), harbor (shipyard), institution (company, lab, agency), sacred cave.

Ordinary caves are outside only; the Cave of Origin, Hoenn's single in-city cave, is the `sacred cave` town kind (a cave entered from a city, tied to a legendary or ancient site).

| Place | Bulbapedia location | Placement | Kind |
| --- | --- | --- | --- |
| Meteor Falls | Between Route 114 and 115 | outside | cave |
| Desert Underpass | Route 114 near Fallarbor | outside | cave |
| Glass Workshop | Route 113 | outside | roadside building |
| Mt. Chimney | Route 112 near Lavaridge | outside | volcano |
| Fiery Path | Route 112 | outside | tunnel |
| Jagged Pass | Route 112 | outside | mountain |
| Magma Hideout | Jagged Pass | outside | hideout |
| Mirage Tower | Route 111 | outside | tower |
| Desert Ruins | Route 111 | outside | ruins |
| Ancient Tomb | Route 120 | outside | ruins |
| Scorched Slab | East of Fortree | outside | cave |
| Island Cave | Route 105 | outside | cave |
| Safari Zone | North of Route 121 | outside | park |
| Aqua Hideout | Lilycove (Emerald: by the harbor) | outside | hideout |
| Shoal Cave | Route 125 | outside | cave |
| Treasure Hunter's House | Route 125 | outside | roadside building |
| Rusturf Tunnel | Between Route 116 and Verdanturf | outside | tunnel |
| New Mauville | Below Mauville, from Route 110 | outside | facility |
| Trainer Hill | Route 111 north of Mauville | outside | roadside building |
| Weather Institute | Route 119 west of Fortree | outside | facility |
| Berry Master's house | Route 123 | outside | roadside building |
| Mt. Pyre | Route 122 | outside | mountain |
| Petalburg Woods | Route 104 | outside | forest |
| Pretty Petal Flower Shop | Route 104 | outside | roadside building |
| Altering Cave | Route 103 | outside | cave |
| Cycling Road | Route 110 | outside | route feature |
| Trick House | Route 110 | outside | roadside building |
| Pokémon Daycare | Route 117 | outside | roadside building |
| Seafloor Cavern | Route 128 | outside | underwater |
| Sealed Chamber | Route 134 | outside | underwater |
| Southern Island | Southern Hoenn | Enclave (boat/Eon Ticket) | island |
| Abandoned Ship | Route 108 | outside | wreck |
| Granite Cave | Route 106 | outside | cave |
| Sky Pillar | North of Route 131 | outside | tower |
| Mirage Island | Route 130 | Enclave (script) | island |
| Battle Frontier | South of Route 130 | Enclave (boat) | island |
| Artisan Cave | Battle Frontier | Enclave (inside Battle Frontier) | cave |
| Victory Road | By Ever Grande | outside | tunnel |
| Birch's Lab | Littleroot | inside | institution |
| Devon Corporation | Rustboro | inside | institution |
| Oceanic Museum | Slateport | inside | museum |
| Stern's Shipyard | Slateport | inside | harbor |
| Contest Hall | Slateport, Verdanturf, Fallarbor, Lilycove | inside | entertainment |
| Mauville Game Corner | Mauville | inside | entertainment |
| Mossdeep Game Corner | Mossdeep | inside | entertainment |
| Space Center | Mossdeep | inside | institution |
| Lilycove Department Store | Lilycove | inside | store |
| Cave of Origin | Sootopolis | inside | sacred cave |

Notes: Bulbapedia lists Aqua Hideout and Victory Road under their cities, but both are reached outside the city proper, so they count as outside. The Magma Hideout is inside Jagged Pass on the map; the stage treats it as its own tile. The Contest Halls other than Lilycove's are small buildings inside their towns and are not special locations in the table above; only Lilycove's stays one.

### Building roles

A building role is what an NPC can refer to ("the museum", "the Pokémon Center"). In-city special locations supply their own role. Roles seen across the 16 cities:

- Everywhere but the start: Pokémon Center. Almost everywhere: Mart (absent in Littleroot, Dewford and Pacifidlog). Gym where there is one.
- Start town only: home with a mother who heals the party, a bedroom PC.
- Community and shopping: house (generic), market (Slateport Market), department store, motel (Cove Lily Motel), herb shop, hot springs, hall (Dewford Hall), fan club (several cities), trainers' school, secret base guild, trade house, and service (name rater, friendship rater, move tutor, move reminder, move deleter, merged: the move reminder's and deleter's houses use the generic `LAYOUT_HOUSE2`, so there is no unique tile to map, and tutors are NPCs).
- Story-bound (inside-city special locations): lab, company, museum, shipyard, Game Corner, Contest Hall, space center, cave entrance.

Placeholder NPC objects per host map (design.md, "Subplot abstraction", Placement): 2 for each building role, 3 for a gym, 1 for a service building, 4 for a city's outdoor map; routes get none.

## Rival arc

- You scare off a Pokémon, the professor gives you two Pokémon, then May/Brendan battles you.
- On a route just before the first gym, the rival shows up and battles again, then again on a route between the second and third gyms, on a route between the fourth and fifth gyms, and in some side area between the sixth and seventh gyms.
- A secondary rival, Wally, challenges you just before the third gym and as the Victory Road champion.

## Story arc

Two eco-terrorist factions, Magma and Aqua, argue industrial progress versus environmental preservation. They awaken the Ground and Sea legendary Pokémon and lose control of them. Then a Flying legendary arrives from a tower landmark and pacifies them, and Magma and Aqua learn the error of their ways.

## Subplots

- A man in sunglasses teaches you Cut.
- Mr. Briney's Wingull is stolen.
- You can't enter Devon Corp until you reclaim the Devon Goods from a tunnel.
- Answering questions earns the Old Rod.
- Granite Cave requires Flash.
- Delivering the Devon Goods to Captain Stern.

## McGuffin chain (from the Bulbapedia walkthrough)

Source: Bulbapedia "Walkthrough:Pokémon Emerald", in tutorial order (Parts 1-19), so a lock usually appears before its key. Each entry is `Thing — needs: <McGuffin> — gives: <McGuffin>`. A `needs?:` is a guess: the tutorial states no requirement, so it is the previous Thing's McGuffin, or the well-known game requirement where the tutorial hints at one (Surf, Cut). Only gates and plot items are listed; ordinary rewards (Potions, Berries, TMs, Poké Balls) are not McGuffins. Field use of an HM needs both the HM and its gym badge. When a key and its lock are in different Parts, both entries say so: the key lists `unlocks: <lock> (Pn #m)` and the lock lists `key: <key> (Pn #m)`; a key not yet reached is `key: <name> (later Part)`. Entries are grouped under their Part heading; numbering continues across Parts.

### Part 1

1. Start in Littleroot — needs: nothing — gives: Littleroot access
2. Birch's House, May/Brendan leaves — needs?: Littleroot access — gives: May/Brendan out helping Birch
3. Route 101, rescue Birch (pick starter) — needs?: May/Brendan out helping Birch — gives: Starter Pokémon
4. Birch's Lab, thanks — needs: Starter Pokémon — gives: Lab free to leave; go find May/Brendan on Route 103
5. Route 101 to Oldale Town — needs?: Lab free to leave — gives: Oldale access
6. Oldale west exit, sketch artist blocks Route 102 — needs?: Pokédex — gives: Route 102 access
7. Route 103, east half (seawater divides it; also reached from Route 110) — needs?: Surf (key: P8 #78) — gives: Route 103 east half access
8. Route 103 west half, Rival Battle 1 — needs?: Oldale access — gives: Rival Battle 1 done
9. Birch's Lab, Pokédex and 5 Poké Balls — needs: Rival Battle 1 done — gives: Pokédex, Poké Balls (shops stock balls)
10. Littleroot, Mom gives Running Shoes — needs: Pokédex — gives: Running Shoes
11. Route 102 to Petalburg — needs?: Route 102 access — gives: Petalburg access
12. Petalburg Gym, Norman and Wally catch Ralts — needs?: Petalburg access — gives: Wally has Ralts
13. Petalburg Gym challenge (Norman) — needs: 4 Badges — gives: Gym 5 (Petalburg) (key: Badges P2 #21, P3 #30, P5 #50, P7 #70; fight resolved in P8 #77)
14. Petalburg west exit, man in sunglasses blocks Route 104 — needs?: Wally has Ralts — gives: Route 104 access

### Part 2

15. Petalburg Woods, small trees restrict travel to the west side — needs?: Cut (HM01 + Stone Badge) — gives: Petalburg Woods full access
16. Petalburg Woods, Devon researcher and Aqua grunt — needs?: Route 104 access — gives: Aqua grunt defeated
17. Route 104 north, Flower Shop middle sister — needs?: Aqua grunt defeated — gives: Wailmer Pail
18. Route 104 north to Rustboro — needs?: Wailmer Pail — gives: Rustboro access
19. Rustboro, Cutter's House — needs?: Rustboro access — gives: HM01 Cut
20. Devon Corporation, first floor only — needs?: Devon Goods — gives: Devon HQ upper floors access
21. Rustboro Gym (Roxanne) — needs?: Devon HQ upper floors access — gives: Stone Badge (Gym 1, enables Cut in the field)
22. Rustboro, Stop Thief (Aqua grunt flees Devon) — needs: Stone Badge — gives: Route 116 access
23. Route 116 at Rusturf Tunnel entrance, Peeko kidnapped — needs?: Route 116 access — gives: Peeko kidnapped
24. Rusturf Tunnel, debris blocks the through-passage — needs: HM06 Rock Smash (key: P5 #48) + Dynamo Badge (key: P5 #50) — gives: HM04 Strength (from Wanda's boyfriend), tunnel open — unlocks: Fiery Path side tunnels (P6 #57), Trick House puzzle 4 (P9 #87), Magma Hideout (P12 #102), Seafloor Cavern (P15 #117), Victory Road (P18 #132)
25. Rusturf Tunnel, the Take-Down — needs?: Peeko kidnapped — gives: Devon Goods, Peeko rescued

### Part 3

26. Rustboro, Devon Corporation return — needs: Devon Goods — gives: PokéNav (Map, Condition, Match Call), Letter for Steven
27. Rival Battle 2 (optional, outside the Poké Mart) — needs?: PokéNav Match Call — gives: Rival Battle 2 done
28. Briney sails to Dewford (Routes 105-106) — needs: Letter — gives: Dewford access
29. Dewford, Fisherman quiz — needs?: Dewford access — gives: Old Rod
30. Dewford Gym (Brawly) — needs?: Old Rod — gives: Knuckle Badge (Gym 2, enables Flash in the field)
31. Routes 105-108 sea, full exploration — needs: Surf + Balance Badge (fifth) (key: P8 #78) — gives: Sea routes full access
32. Granite Cave 1F, Hiker near entrance — needs?: Knuckle Badge — gives: HM05 Flash — unlocks: Granite Cave underground (P3 #33), Ancient Tomb (P11 #95), Victory Road (P18 #132)
33. Granite Cave underground (dark) — needs?: Flash — gives: Granite Cave underground access
34. Granite Cave 1F side room (a warp off lit 1F, no Flash needed), Steven — needs: Letter — gives: Letter delivered
35. Dewford pier, Briney to Slateport (Route 109 beach) — needs: Devon Goods — gives: Slateport access

### Part 4

36. Slateport, Oceanic Museum blocked by Aqua grunts — needs: Shipyard visit — gives: Museum access
37. Slateport, Stern's Shipyard (Dock) — needs?: Slateport access — gives: Shipyard visit
38. Oceanic Museum, deliver to Stern, fight two grunts, meet Archie — needs: Museum access (and $50, or enter free if broke) — gives: Devon Goods delivered, Scott registered
39. Slateport Harbor, new ferry unfinished — needs?: Devon Goods delivered — gives: Ferry access (later)
40. Slateport, buy Harbor Mail — needs?: Devon Goods delivered — gives: Harbor Mail — unlocks: Coin Case (P5 #45); the Market's extra bargains need TM Secret Power (key: P6 #58)
41. Route 110, pedestrians limited to the lower path; Seaside Cycling Road — needs: Bike (key: P5 #47) — gives: Seaside Cycling Road access
42. Route 110, Trick House (Trick Master puzzle 1; a new puzzle opens with each Badge, most mazes also need field moves) — needs?: Slateport access — gives: Trick House puzzle 1 done — unlocks: puzzle 2 with the Dynamo Badge (P5 #51)
43. Route 110, Rival Battle 3 — needs?: Trick House puzzle 1 done — gives: Itemfinder, Rival Battle 3 done
44. Route 110 to Mauville — needs?: Rival Battle 3 done — gives: Mauville access

### Part 5

45. Mauville, woman next to the Poké Mart — needs: Harbor Mail (key: P4 #40) — gives: Coin Case
46. Mauville Game Corner — needs: Coin Case — gives: Game Corner access
47. Mauville, Rydel's Cycles (checks your dirty Running Shoes) — needs: Running Shoes — gives: Mach Bike or Acro Bike (Mach Bike climbs muddy slopes, e.g. Granite Cave; Acro Bike scales Jagged Pass) — unlocks: Seaside Cycling Road (P4 #41), Jagged Pass northward (P7 #69), Mirage Tower (P8 #75), Sky Pillar return (P18 #129)
48. Mauville, Scientist in the southeast house — needs?: Mauville access — gives: HM06 Rock Smash — unlocks: Rusturf debris (P2 #24), Granite Cave B2F rocks (P5 #51), Mirage Tower (P8 #75), Trick House puzzle 3 (P7 #72), Seafloor Cavern (P15 #117), Desert Ruins (P8 #74)
49. Mauville Gym, Wally's Challenge outside — needs?: Mauville access — gives: Wally Battle done, Wally in PokéNav
50. Mauville Gym (Wattson) — needs?: Wally Battle done — gives: Dynamo Badge (Gym 3, enables Rock Smash in the field) — unlocks: Rusturf debris (P2 #24), Trick House puzzle 2 (P5 #51)
51. Granite Cave B2F cracked rocks (lock first mentioned in P3 #33) and Trick House puzzle 2 (lock in P4 #42) (optional) — needs: Rock Smash (key: P5 #48) / Dynamo Badge (key: P5 #50) — gives: Nosepass encounters; puzzle 2
52. Route 118, only the west side explorable — needs?: Surf (key: P8 #78) — gives: Route 118 full access (resolved in P9 #86)

### Part 6

53. Route 111 south — needs?: Mauville access — gives: Route 111 south access
54. Route 111, the central desert's sandstorm forces a detour — needs: Go-Goggles (key: P7 #71) — gives: Route 111 desert access
55. Route 112 south, Team Magma Grunts guard the cable car and elevated terrain blocks the west — needs: Meteorite theft seen (key: P7 #65) — gives: Mt. Chimney cable car access
56. Fiery Path — needs?: Route 112 south access — gives: Route 112 north access
57. Fiery Path side tunnels, boulders — needs: Strength (HM04 key: P5 #24, Heat Badge key: P7 #70) — gives: Fiery Path side tunnel access
58. Route 111 north, boy at the Secret Base tree — needs?: Route 112 north access — gives: TM Secret Power — unlocks: Slateport Market bargains (P4 #40)
59. Route 113, Glass Workshop owner — needs?: Route 111 north access — gives: Soot Sack
60. Fallarbor Town — needs?: Route 113 access — gives: Fallarbor access
61. Fallarbor, Professor Cozmo's house — needs: Meteorite (key: P7 #67) — gives: TM (Normal)
62. Route 114, Fossil Maniac's little brother — needs?: Fallarbor access — gives: TM Dig — unlocks: Sealed Chamber Braille wall (P17 #124)
63. Route 114 — needs?: Fallarbor access — gives: Route 114 access

### Part 7

64. Meteor Falls, rapids prevent full exploration — needs?: Surf (key: P8 #78) + Waterfall (key: P17 #127) — gives: Meteor Falls full access
65. Meteor Falls 1F, Magma Grunts steal the Meteorite and Archie appears — needs?: Route 114 access — gives: Meteorite stolen — unlocks: Route 112 cable car (P6 #55)
66. Mt. Chimney summit by cable car — needs: Cable car access (P6 #55) — gives: Mt. Chimney access
67. Mt. Chimney, Tabitha then Maxie — needs?: Mt. Chimney access — gives: Meteorite recovered (unlocks Cozmo's TM, P6 #61), Jagged Pass southward access
68. Jagged Pass southward to Lavaridge — needs?: Jagged Pass southward access — gives: Lavaridge access
69. Jagged Pass northward (rough terrain) — needs: Acro Bike (key: P5 #47) — gives: Jagged Pass northward access
70. Lavaridge Gym (Flannery) — needs?: Lavaridge access — gives: Heat Badge (Gym 4, enables Strength in the field), TM Overheat — unlocks: Go-Goggles (P7 #71), Strength in the field (P6 #57, P9 #87, P12 #102, P15 #117), Gym 5 fight (P8 #77)
71. Lavaridge, May/Brendan outside the Gym — needs: Heat Badge — gives: Go-Goggles — unlocks: Route 111 desert (P6 #54)
72. Trick House puzzle 3 (optional) — needs: Rock Smash — gives: Trick House puzzle 3 done

### Part 8

73. Route 111 desert, first visit — needs: Route 111 desert access (P6 #54) — gives: Desert access, Mirage Tower and Desert Ruins found
74. Desert Ruins, sealed — needs: Sealed Chamber opened (key: P17 #125) + Rock Smash — gives: Regirock
75. Mirage Tower, climb needs the Mach Bike and Rock Smash — needs: Mach Bike (key: P5 #47) + Rock Smash (key: P5 #48) — gives: Root Fossil or Claw Fossil (pick one; the other falls away)
76. Rustboro, Devon Corporation second-floor scientist — needs: Fossil — gives: Lileep or Anorith
77. Petalburg Gym, Norman (minimum 3 Gym Trainers on the way) — needs: 4 Badges (Stone P2 #21, Knuckle P3 #30, Dynamo P5 #50, Heat P7 #70) — gives: Balance Badge (Gym 5, enables Surf in the field), TM Facade; resolves the Gym 5 lock (P1 #13)
78. Petalburg, Wally's father's house — needs: Norman defeated — gives: HM03 Surf — unlocks: Route 103 east (P1 #7), Routes 105-108 (P3 #31), Route 118 (P5 #52), Abandoned Ship (P9 #81), New Mauville (P9 #84), Mt. Pyre route (P12 #99), Route 115 (P8 #79)
79. Route 115, full exploration — needs: Surf (key: P8 #78) — gives: Route 115 full access
80. Island Cave (Route 105), sealed — needs: Sealed Chamber opened (key: P17 #125) + a lap of the chamber — gives: Regice

### Part 9

81. Abandoned Ship, partly flooded — needs: Surf (key: P8 #78); Dive for the flooded rooms (key: P14 #113) — gives: Abandoned Ship access
82. Abandoned Ship, Captain's Office — needs?: Abandoned Ship access — gives: Storage Key — unlocks: northeast cabin (TM Ice Beam)
83. Mauville, Wattson asks you to shut down New Mauville's generator — needs: Norman defeated (P8 #77; his defeat moves Wattson from the gym to the city) — gives: Basement Key
84. New Mauville (entrance on Route 110 by Surf) — needs: Basement Key + Surf (key: P8 #78) — gives: Generator shut down
85. Mauville, report back to Wattson — needs: Generator shut down — gives: TM Thunderbolt
86. Route 118 east side and the river — needs: Surf (key: P8 #78) — gives: Route 118 full access (resolves P5 #52), Good Rod (Fisherman)
87. Trick House puzzle 4 (optional) — needs: Strength — gives: Trick House puzzle 4 done

### Part 10

88. Route 119 (Acro Bike for every item) — needs?: Route 118 full access — gives: Route 119 access
89. Weather Institute, Aqua Admin Shelly — needs?: Route 119 access — gives: Shelly defeated, Castform
90. Route 119 north, Rival Battle 4 near Fortree — needs?: Shelly defeated — gives: Rival Battle 4 done, HM Fly (field use needs Feather Badge, P11 #96)
91. Fortree City — needs?: Rival Battle 4 done — gives: Fortree access
92. Fortree Gym, entrance obstructed by something invisible — needs: Devon Scope (key: P11 #94) — gives: Fortree Gym access

### Part 11

93. Route 120 — needs?: Fortree access — gives: Route 120 access
94. Route 120, northern bridge blocked by an invisible Kecleon (Steven) — needs?: Route 120 access — gives: Devon Scope — unlocks: Fortree Gym entrance (P10 #92)
95. Ancient Tomb (Route 120), sealed — needs: Sealed Chamber opened (key: P17 #125) + Flash — gives: Registeel
96. Fortree Gym (Winona) — needs: Devon Scope (key: P11 #94) — gives: Feather Badge (Gym 6, enables Fly in the field), TM Aerial Ace
97. Route 121 to Lilycove, Aqua Grunts rush toward Mt. Pyre — needs?: Fortree Gym beaten — gives: Route 121 and Lilycove access
98. Trick House puzzle 5 (optional) — needs?: Feather Badge — gives: Trick House puzzle 5 done

### Part 12

99. Route 122 sea route to Mt. Pyre — needs?: Surf (key: P8 #78) — gives: Mt. Pyre access
100. Mt. Pyre summit, Archie steals the Red Orb and the elderly caretaker gives the Magma Emblem — needs?: Mt. Pyre access — gives: Magma Emblem, Red Orb stolen
101. Jagged Pass midpoint, cliffside opens (arrive by Fly to Lavaridge, then Acro Bike or the cable car) — needs: Magma Emblem (key: P12 #100) — gives: Magma Hideout access
102. Magma Hideout (boulders at the entrance; Tabitha then Maxie) — needs: Magma Hideout access + Strength — gives: Groudon awakened and escapes, Maxie defeated

### Part 13

103. Slateport Harbor, Archie takes Captain Stern's submarine — needs?: Magma Hideout resolved — gives: Aqua Hideout location revealed
104. Lilycove City, full visit — needs?: Slateport sub scene — gives: Lilycove access
105. Lilycove Contest Hall, left receptionist — needs?: Lilycove access — gives: Pokéblock Case — unlocks: Safari Zone entry
106. Lilycove, Rival Battle 5 blocks the Department Store (optional) — needs?: Lilycove access — gives: Rival Battle 5 done, Department Store access
107. Aqua Hideout (warp panels; Archie's office; Admin Matt) — needs?: Slateport sub scene — gives: Master Ball, Matt defeated, Archie escapes by submarine, the blockade off Lilycove's beach gone (eastern sea open)

### Part 14

108. Route 124 east of Lilycove — needs: Surf (key: P8 #78) + blockade gone (P13 #107) — gives: Route 124 and Mossdeep access
109. Mossdeep City — needs?: Route 124 access — gives: Mossdeep access
110. Mossdeep, Fisherman east of the Gym — needs?: Mossdeep access — gives: Super Rod
111. Mossdeep Gym (Tate & Liza, double battle; warp panels) — needs?: Mossdeep access — gives: Mind Badge (Gym 7, enables Dive in the field), TM Calm Mind
112. Mossdeep Space Center, Maxie and Tabitha multi battle with Steven — needs: Mind Badge — gives: Magma rocket-fuel plot ended
113. Mossdeep, Steven's house — needs: Space Center battle won — gives: HM Dive — unlocks: Abandoned Ship hidden floor (P15 #115), Seafloor Cavern (P15 #117), Sootopolis entrance (P16 #119), Sealed Chamber (P17 #123)
114. Trick House puzzle 6 (optional) — needs?: Mind Badge — gives: Trick House puzzle 6 done

### Part 15

115. Abandoned Ship hidden floor (Key to Room 1, then Room 4, Room 6, Room 2) — needs: Surf + Dive (key: P14 #113) — gives: Scanner
116. Slateport Harbor, Captain Stern — needs: Scanner — gives: DeepSeaTooth or DeepSeaScale (via the Scientist in the Captain's Office)
117. Route 128, Seafloor Cavern entrance underwater, then Rock Smash and Strength boulders and Surf currents inside — needs: Dive (key: P14 #113) + Rock Smash + Strength + Surf — gives: Seafloor Cavern access
118. Seafloor Cavern, Shelly then Archie — needs: Seafloor Cavern access — gives: Kyogre awakened, Hoenn weather crisis, Steven points to Sootopolis

### Part 16

119. Route 126, Sootopolis hidden entrance beneath the waves in the south — needs: Dive (key: P14 #113) — gives: Sootopolis access
120. Sootopolis City, Groudon and Kyogre fight and the Gym is closed — needs?: Sootopolis access — gives: Cave of Origin access (Steven leads you)
121. Cave of Origin, Wallace — needs?: Cave of Origin access — gives: Wallace heads to Sky Pillar, Sky Pillar quest
122. Routes 129-131 by Surf to Sky Pillar — needs?: Cave of Origin done + Surf (key: P8 #78) — gives: Sky Pillar access

### Part 17

123. Routes 132-134, Sealed Chamber Dive spot — needs: Dive (key: P14 #113) — gives: Sealed Chamber access
124. Sealed Chamber, Braille wall 'Dig here' — needs: TM Dig (key: P6 #62) — gives: Sealed Chamber back room access
125. Sealed Chamber back wall 'First comes Wailord. Last comes Relicanth.' — needs: Wailord first and Relicanth last in the party (Relicanth by Dive, Wailmer by Good/Super Rod) — gives: Sealed Chamber opened — unlocks: Regirock (P8 #74), Regice (P8 #80), Registeel (P11 #95)
126. Sky Pillar climb (cracked tiles on 4F) — needs?: Sky Pillar access — gives: Rayquaza awakened
127. Sootopolis, Rayquaza ends the fight — needs: Rayquaza awakened — gives: HM Waterfall (from Wallace; field use needs Rain Badge), Gym open — unlocks: Meteor Falls (P7 #64), Ever Grande (P18 #131)
128. Sootopolis Gym (Juan) — needs?: Rayquaza ended the fight — gives: Rain Badge (Gym 8, all Pokémon obey, enables Waterfall in the field), TM Water Pulse

### Part 18

129. Sky Pillar, return visit (unstable floors) — needs: Mach Bike (key: P5 #47) — gives: Rayquaza battle (L70, optional)
130. Trick House puzzle 7 (optional) — needs?: Rain Badge — gives: Trick House puzzle 7 done
131. Ever Grande City, waterfall blocks the city — needs: Waterfall + Rain Badge (key: P17 #127) — gives: Ever Grande access
132. Victory Road (Flash in the dark, Strength, Rock Smash, Surf, Waterfall inside) — needs?: Ever Grande access — gives: Victory Road passage, Wally Battle 2

### Part 19

133. Pokémon League lobby, two Ace Trainers check your Badges — needs: 8 Badges (Stone, Knuckle, Dynamo, Heat, Balance, Feather, Mind, Rain) — gives: Elite Four access
134. Elite Four (Sidney, Phoebe, Glacia, Drake) back to back, no outside help — needs: League lobby cleared — gives: Champion room access
135. Champion Wallace — needs: Elite Four beaten — gives: Hall of Fame

## Lock/key buckets

Every McGuffin chain entry (by number) in exactly one bucket, per design.md "Subplot abstraction / Scope".

- Plain access (no real lock; just the planner's path): 1, 5, 11, 44, 53, 56, 60, 63, 68, 88, 91, 93, 104, 109.
- Main arc (stays on Emerald vars behind the adapter until Milestone 3):
  - Start and Birch: 2, 3, 4, 9, 10.
  - Rival: 8, 27, 43, 71 (Go-Goggles comes from the rival), 90 (Fly), 106.
  - Wally: 12, 49.
  - Devon Goods (Rustboro, Rusturf, Dewford, Slateport): 20, 22, 23, 25, 26, 28, 34, 35, 36, 37, 38, 39.
  - Aqua/Magma, Meteorite, legendaries, League: 55, 61, 65, 66, 67, 89, 97, 100, 101, 102, 103, 107, 112, 113 (Dive), 118, 120, 121, 126, 127 (Waterfall), 134, 135.
- Gym (fixed per-city beat for now): 13, 21, 30, 50, 70, 77, 96, 111, 128, plus 78 (Surf, Norman's reward after gym 5).
- Gate (an HM, key item or blocker NPC on a route): 6, 7, 14, 15, 24, 31, 41, 52, 54, 57, 64, 69, 79, 81, 86, 92, 99, 108, 117, 119, 122, 129, 131, 132, 133.
- City subplot (convert): Rustboro Cutter's House 19, Dewford Old Rod quiz 29, Slateport Harbor Mail 40, Mauville Coin Case 45, Game Corner 46, Rydel's Cycles 47, Rock Smash scientist 48, Rustboro fossil revival 76, Lilycove Pokéblock Case 105, Mossdeep Super Rod 110.
- Special-location subplot (convert), grouped by location:
  - Petalburg Woods and Route 104 flower shop: 16, 17, 18.
  - Granite Cave: 32, 33, 51.
  - Route 111 north Secret Base tree: 58. Route 113 Glass Workshop: 59. Route 114 Fossil Maniac's brother: 62.
  - Desert and Mirage Tower: 73, 75. Desert Ruins: 74. Island Cave: 80. Ancient Tomb: 95. Sealed Chamber: 123, 124, 125.
  - Abandoned Ship: 82, 115, 116. New Mauville: 83, 84, 85. Route 120 Kecleon bridge: 94.
  - Trick House (one subplot, badge-counted puzzles): 42, 72, 87, 98, 114, 130.

Pairs where a key and its lock sit in different hosts. Per design.md these are planner unlock steps, not subplot-relative outputs:

- 40 (Slateport) to 45 (Mauville), 47/48 (Mauville bike and Rock Smash) to many locks, 75 (Mirage Tower) to 76 (Rustboro), 58 to 40's Market bargains, 62 to 124, 125 to 74/80/95.
- 115 ends in Slateport (116) and 83-85 starts in Mauville; both are mostly location-local and probably keep the location as host, with the town beat as a start or finish hook.

The subplot "Briney's Wingull is stolen" appears in the Subplots list but not in the chain (no key or lock), so it needs no bucket.

## Encounter and trainer levels (from the Bulbapedia walkthrough)

Levels the walkthrough lists, per place in tutorial order. `wild` is the land, cave and special-encounter level range by encounter type; `N trainers` is the count with their level range and the largest party; bosses list each Pokémon with its level. Rival parties depend on your starter (species differ, levels do not). Repeated across the whole game: Old Rod 5-10, Good Rod 10-30, Super Rod 20-45 (25-45 in some places), Surf encounters 5-35 (most commonly 10-30), Dive 20-35.

#### Part 1

- Save the Professor! — wild Special L2
- Route 101 — wild Grass L2-3
- Route 103 — wild Grass L2-4
- Rival Battle 1 — Brendan / May: Torchic 5
- Route 102 — wild Grass L3-4; 4 trainers L3-5 (party <=2)

#### Part 2

- Route 104 (south) — wild Grass L3-5; 3 trainers L5-9 (party <=2)
- Petalburg Woods — wild Grass L5-6; 3 trainers L3-9 (party <=4)
- Route 104 (north) — 4 trainers L5-7 (party <=3)
- Rustboro Gym — 3 trainers L8-10 (party <=2); Roxanne (Leader): Geodude 12, Geodude 12, Nosepass 15
- Route 116 — wild Grass L6-8, Swarm L8; 10 trainers L8-9 (party <=2)
- Rusturf Tunnel — wild Cave L5-8; 1 trainer L11 (party <=1)

#### Part 3

- Rival Battle 2 — Brendan: Lotad 13, Torchic 15; May: Lotad 13, Torchic 15
- Dewford Gym — Brawly (Leader): Machop 16, Meditite 16, Makuhita 19
- Route 106 — 1 trainer L7-10 (party <=3)
- Granite Cave — wild Cave L6-12
- Route 109 — 9 trainers L11-13 (party <=3)

#### Part 4

- Oceanic Museum — 2 trainers L14-15 (party <=2)
- Route 110 — wild Grass L12-13; 8 trainers L11-15 (party <=4)
- Rival Battle 3 — Brendan: Wingull 18, Lombre 18, Combusken 20; May: Wingull 18, Lombre 18, Combusken 20
- Route 103 — 6 trainers L5-15 (party <=3)

#### Part 5

- Wally's Challenge — Wally: Ralts 16
- Mauville Gym — Wattson (Leader): Voltorb 20, Electrike 20, Magneton 22, Manectric 24
- Route 110 — 6 trainers L6-16 (party <=3)
- Route 117 — wild Grass L13-14, Swarm L13; 9 trainers L11-17 (party <=6)
- Rusturf Tunnel — wild Cave L5-8; 1 trainer L16 (party <=3)
- Route 118 — 4 trainers L14-16 (party <=3)

#### Part 6

- Route 111 (south) — wild Rock Smash L5-20; 7 trainers L17-19 (party <=2)
- Winstrate House — Victor (Winstrate): Taillow 16, Zigzagoon 16; Victoria (Winstrate): Roselia 17; Vivi (Winstrate): Marill 15, Shroomish 15, Numel 15; Vicky (Winstrate): Meditite 18
- Route 112 (south) — wild Grass L14-16; 4 trainers L16-18 (party <=3)
- Fiery Path — wild Cave L14-16
- Route 112 (north) — 2 trainers L18 (party <=2)
- Route 111 (north) — 3 trainers L17-19 (party <=3)
- Route 113 — wild Grass L14-16; 10 trainers L17-19 (party <=3)
- Route 114 — wild Grass L15-18, Rock Smash L5-20; 12 trainers L16-20 (party <=3)

#### Part 7

- Meteor Falls — wild Cave L14-20
- Mt. Chimney — 2 trainers L20 (party <=1)
- VS Tabitha — Tabitha (Magma Admin): Numel 18, Poochyena 20, Numel 22, Zubat 22
- VS Maxie — Maxie (Magma Leader): Mightyena 24, Zubat 24, Camerupt 25
- Jagged Pass (first visit) — wild Grass L20-22; 6 trainers L19-22 (party <=3)
- Lavaridge Town — wild Egg L5
- Lavaridge Gym — Flannery (Leader): Numel 24, Slugma 24, Camerupt 26, Torkoal 29
- Mt. Chimney — 5 trainers L21 (party <=2)

#### Part 8

- Route 111 (desert, optional) — wild Sand L19-22; 8 trainers L21-23 (party <=3)
- Mirage Tower — wild 1F-4F L20-24
- Back to Petalburg City — wild Root L20, Claw L20
- Petalburg Gym — 7 trainers L26 (party <=1); Norman (Leader): Spinda 27, Vigoroth 27, Linoone 29, Slaking 31
- Route 103 — 2 trainers L15 (party <=1)
- Route 115 — wild Grass L23-26
- Route 105 — 1 trainer L26 (party <=1)
- Route 106 — 2 trainers L24-26 (party <=2)
- Route 107 — 1 trainer L25 (party <=2)
- Route 108 — 6 trainers L24-26 (party <=3)
- Route 109 — 7 trainers L24-27 (party <=3)

#### Part 9

- Abandoned Ship — 7 trainers L25-26 (party <=2)
- New Mauville — wild Entrance L22-26, Basement L22-26, Fake L25
- Route 118 — wild Grass L24-27; 4 trainers L25-27 (party <=2)
- Route 123 — 4 trainers L26-27 (party <=2)

#### Part 10

- Route 119 — wild Long grass L24-27; 17 trainers L20-29 (party <=4)
- Weather Institute — wild Gift L25; 5 trainers L26-28 (party <=2)
- VS Shelly — Shelly (Aqua Admin): Carvanha 28, Mightyena 28
- Rival Battle 4 — Brendan: Pelipper 29, Lombre 29, Combusken 31; May: Pelipper 29, Lombre 29, Combusken 31

#### Part 11

- Route 120 — wild Long grass L25-27, Special L30, Swarm L25; 14 trainers L27-30 (party <=3)
- Fortree Gym — Winona (Leader): Swablu 29, Tropius 29, Pelipper 30, Skarmory 31, Altaria 33
- Route 121 — wild Grass L25-28; 10 trainers L25-32 (party <=6)

#### Part 12

- Mt. Pyre — wild 1F-3F L22-29, 4F-6F L22-29, Grass L24-30; 11 trainer L26-32 (party <=6)
- Route 123 — wild Grass L25-28; 11 trainer L28-31 (party <=5)
- Magma Hideout — wild Cave L27-33; 16 trainers L28-29 (party <=2)
- VS Tabitha — Tabitha (Magma Admin): Numel 26, Mightyena 28, Zubat 30, Camerupt 33
- VS Maxie — Maxie (Magma Leader): Mightyena 37, Crobat 38, Camerupt 39

#### Part 13

- Rival Battle 5 — Brendan: Tropius 31, Pelipper 32, Ludicolo 32, Combusken 34; May: Tropius 31, Pelipper 32, Ludicolo 32, Combusken 34
- Aqua Hideout — wild Fake L30; 8 trainers L31-32 (party <=2)
- Submarine room — Matt (Aqua Admin): Mightyena 34, Golbat 34

#### Part 14

- Route 124 — 8 trainers L33-34 (party <=2)
- Mossdeep City — wild Gift L5
- Mossdeep Gym — Tate & Liza (Leader): Claydol 41, Xatu 41, Lunatone 42, Solrock 42
- Space Center — 7 trainers L26-32 (party <=3)
- VS Maxie, Tabitha — Steven: Metang 42, Skarmory 43, Aggron 44; Maxie / Tabitha (Magma Leader / Magma Admin): Mightyena 42, Crobat 43, Camerupt 44, Camerupt 36, Mightyena 38, Golbat 40
- Route 125 — 8 trainers L32-35 (party <=2)

#### Part 15

- Shoal Cave — wild Cave L26-32
- Now that you can use Dive — wild Dive L20-35
- Route 127 — 8 trainers L15-35 (party <=3)
- Route 128 — 7 trainers L31-36 (party <=3)
- Seafloor Cavern — wild Cave L28-36; 5 trainers L35-36 (party <=2)
- VS Shelly — Shelly (Aqua Admin): Sharpedo 37, Mightyena 37
- VS Archie — Archie (Aqua Leader): Mightyena 41, Crobat 41, Sharpedo 43

#### Part 16

- Route 126 — 8 trainers L31-34 (party <=3)
- Cave of Origin — wild Cave L28-36
- Route 129 — 5 trainers L26-34 (party <=2)
- Route 130 — wild Grass L5-50; 3 trainers L33-34 (party <=2)
- Route 131 — 7 trainers L33-35 (party <=2)

#### Part 17

- Routes 132, 133, and 134 — 24 trainers L19-35 (party <=6)
- Sky Pillar — wild 1F L33-38, 3F L33-38, 5F L33-39
- Sootopolis Gym — Juan (Leader): Luvdisc 41, Whiscash 41, Sealeo 43, Crawdaunt 43, Kingdra 46

#### Part 18

- Sky Pillar — wild 1F L33-38, 3F L33-38, 5F L33-39, Special L70
- Meteor Falls — wild Cave L14-40; 2 trainers L37-39 (party <=2)
- Victory Road — wild Cave L36-44, Rock Smash L30-40; 16 trainers L42-45 (party <=4)
- VS Wally — Wally: Altaria 44, Delcatty 43, Roselia 44, Magneton 41, Gardevoir 45

#### Part 19

- Battle 1: Sidney — Sidney (Elite Four): Mightyena 46, Shiftry 48, Cacturne 46, Crawdaunt 48, Absol 49
- Battle 2: Phoebe — Phoebe (Elite Four): Dusclops 48, Banette 49, Sableye 50, Banette 49, Dusclops 51
- Battle 3: Glacia — Glacia (Elite Four): Sealeo 50, Glalie 50, Sealeo 52, Glalie 52, Walrein 53
- Battle 4: Drake — Drake (Elite Four): Shelgon 52, Altaria 54, Kingdra 53, Flygon 53, Salamence 55
- Champion — Wallace (Champion): Wailord 57, Tentacruel 55, Ludicolo 56, Whiscash 56, Gyarados 56, Milotic 58

## Main-story-index table (proposed)

Emerald fitted to the main plot planner (design.md, "Main plot planner"): one row per main-story-index `4x + s`. Step 0 and 2 list gets, step 1 is the optional callback (host named), step 3 unlocks county x+1 via the named McGuffin. Gets within a list are unordered for the planner; the county picks the order, so "G4 then Surf" inside a list means the county's script orders them. `G1`-`G8` are gyms (each with its HM), `RB1`-`RB5` rival battles, `Magma`/`Aqua` beats are the Rocket-equivalent arc beats. Verdanturf, Fallarbor and the Shoal Cave county have no mandatory gets.

County numbers below are the county-story-indices the code uses, in walkthrough order and unique. They differ from the county table above for the counties after Lavaridge: that table's Lavaridge (42) becomes 11, Fortree 11 becomes 12, Weather Institute 12 becomes 13, Mt. Pyre 13 becomes 14, Lilycove 14 becomes 15, Shoal Cave 15 becomes 16, Mossdeep 16 becomes 17, Ever Grande 17 becomes 20, Pacifidlog 20 becomes 19, Mirage Island 19 becomes 21 and the Sealed Chamber 21 becomes 22. Route-story-index is 4 times the county number. The empty county (3,2) takes 23.

| Index | County | Step | Contents |
| --- | --- | --- | --- |
| 0 | 0 Littleroot | 0 | Starter |
| 1 | | 1 | (no callback) |
| 2 | | 2 | (none) |
| 3 | | 3 | Unlock Oldale via Starter |
| 4 | 1 Oldale | 0 | RB1 (Route 103); sends the player back to Birch |
| 5 | | 1 | Callback to Littleroot (0): Pokédex, Poké Balls, Running Shoes |
| 6 | | 2 | (none yet; generated as normal) |
| 7 | | 3 | Unlock Petalburg via Pokédex |
| 8 | 2 Petalburg | 0 | Wally has Ralts; Aqua beat (Petalburg Woods grunt) |
| 9 | | 1 | (no callback) |
| 10 | | 2 | Wailmer Pail |
| 11 | | 3 | Unlock Rustboro via Wailmer Pail |
| 12 | 3 Rustboro | 0 | G1 + Cut; Aqua beat (Stop Thief) |
| 13 | | 1 | (no callback) |
| 14 | | 2 | Devon Goods recovered (Peeko, Route 116); PokéNav; Letter; RB2 |
| 15 | | 3 | Unlock Dewford via Letter (Briney's boat, port from Petalburg) |
| 16 | 4 Dewford | 0 | G2 + Flash; Old Rod |
| 17 | | 1 | (no callback) |
| 18 | | 2 | Letter delivered to Steven (Granite Cave) |
| 19 | | 3 | Unlock Slateport via Letter delivered (Briney's boat) |
| 20 | 5 Slateport | 0 | Shipyard visit (Stern) |
| 21 | | 1 | (no callback) |
| 22 | | 2 | Museum: Devon Goods delivered; Aqua beat (two grunts, Archie); Harbor Mail |
| 23 | | 3 | Unlock Cycling Road county via Devon Goods delivered |
| 24 | 6 Cycling Road | 0 | RB3 (Route 110); Itemfinder |
| 25 | | 1 | (no callback) |
| 26 | | 2 | (none; Trick House puzzle 1 is optional) |
| 27 | | 3 | Unlock Mauville via RB3 |
| 28 | 7 Mauville | 0 | Wally battle 1 |
| 29 | | 1 | (no callback) |
| 30 | | 2 | G3 + Rock Smash; Coin Case (from Harbor Mail); Mach Bike or Acro Bike |
| 31 | | 3 | Unlock Verdanturf via Rock Smash + Dynamo Badge (Rusturf debris) |
| 32 | 8 Verdanturf | 0 | Strength (Rusturf Tunnel; Emerald-only exception 3) |
| 33-34 | | 1-2 | (none) |
| 35 | | 3 | Unlock Fallarbor via a blocker |
| 36-38 | 9 Fallarbor | 0-2 | (none) |
| 39 | | 3 | Unlock Meteor Falls via a blocker |
| 40 | 10 Meteor Falls | 0 | Magma beat (Meteorite stolen, Archie appears) |
| 41-42 | | 1-2 | (none) |
| 43 | | 3 | Unlock Lavaridge via Meteorite stolen (cable car) |
| 44 | 11 Lavaridge | 0 | Magma beat (Mt. Chimney, Meteorite recovered); G4 (Strength already held) |
| 45 | | 1 | (no callback) |
| 46 | | 2 | Go-Goggles (rival meeting outside the gym) |
| 47 | | 3 | Unlock Fortree (open; no lock) |
| 48 | 12 Fortree | 0 | RB4 (Route 119 north); Devon Scope (Route 120) |
| 49 | | 1 | Callback to Petalburg (2): G5 + Surf (Norman, Wally's father) |
| 50 | | 2 | G6 + Fly |
| 51 | | 3 | Unlock Weather Institute county via Surf (Route 118 river) |
| 52 | 13 Weather Institute | 0 | Aqua beat (Shelly; Castform) |
| 53 | | 1 | (no callback) |
| 54 | | 2 | Good Rod (Route 118 river fisherman) |
| 55 | | 3 | Unlock Mt. Pyre via Shelly defeated |
| 56 | 14 Mt. Pyre | 0 | Aqua beat (Red Orb stolen; Magma Emblem) |
| 57 | | 1 | Callback to Lavaridge (11): Magma Hideout (Emblem + Strength), Magma beat (Groudon awakened) |
| 58 | | 2 | (none) |
| 59 | | 3 | Unlock Lilycove via Groudon awakened |
| 60 | 15 Lilycove | 0 | RB5; Pokéblock Case |
| 61 | | 1 | Callback to Slateport (5): Aqua beat (Archie takes Stern's submarine) |
| 62 | | 2 | Aqua beat (Aqua Hideout: Matt, Master Ball, blockade gone) |
| 63 | | 3 | Unlock Shoal Cave county via blockade gone |
| 64-66 | 16 Shoal Cave | 0-2 | (none) |
| 67 | | 3 | Unlock Mossdeep via a blocker |
| 68 | 17 Mossdeep | 0 | G7 + Dive; Magma beat (Space Center, Steven); Super Rod |
| 69 | | 1 | (no callback) |
| 70 | | 2 | Aqua beat (Seafloor Cavern: Shelly, Archie; Kyogre awakened) |
| 71 | | 3 | Unlock Sootopolis via Dive + Kyogre awakened |
| 72 | 18 Sootopolis | 0 | Cave of Origin: Groudon/Kyogre climax, Wallace heads to Sky Pillar |
| 73-74 | | 1-2 | (none) |
| 75 | | 3 | Unlock Pacifidlog via Cave of Origin done (Surf held) |
| 76 | 19 Pacifidlog | 0 | Sky Pillar: Rayquaza awakened |
| 77 | | 1 | Callback to Sootopolis (18): Rayquaza ends the fight; G8 + Waterfall |
| 78 | | 2 | (none) |
| 79 | | 3 | Unlock Ever Grande via Waterfall + G8 (Rain Badge) |
| 80 | 20 Ever Grande (Elite Four) | 0 | (none; end game is open) |
| 81 | | 1 | Callback to Sootopolis (18): Victory Road, Wally battle 2 |
| 82 | | 2 | League lobby (8 badges), Elite Four, Champion |
| 83 | | 3 | Unlock the reserved counties via the Hall of Fame (open) |
| 84-95 | 21-23 reserved | 0-3 | Post-game; open (Mirage Island, Sealed Chamber and the empty county) |

Callbacks: 6 in about 21 blocks (indices 5, 49, 57, 61, 77, 81), four of them after Fly. No block has two different callback hosts.

### Planner inputs

What the main plot planner (design.md, "Main plot planner") reads for Emerald. Two layers, because the table above cannot come out of the hash formulas (step bits, callback hashes and the callback chance are random): the vanilla slots below override the hash outcome while the planner's randomization bit is off (design.md, "Vanilla stage overrides"), and the generic constants further down are what the randomized path uses. Every vanilla slot below is checked against the generic ranges and masks; the ones that fall outside are the numbered exceptions above.

Constants: E = 20 (Elite Four index), R = 3, Surf-lock block n = 12, Fly's block 12. County-story-indices are in `emerald_region.h` (table above).

#### Vanilla slots (block/step, from the table above)

- Gyms (`sEmeraldGymSlots`, host in brackets when it is not the block's county): G1 3/0, G2 4/0, G3 7/2, G4 11/0, G5 12/1 (Petalburg, 2), G6 12/2, G7 17/0, G8 19/1 (Sootopolis, 18), League lobby 20/2.
- HMs (each takes its gym's slot, except Strength): Cut 3/0, Flash 4/0, Rock Smash 7/2, Strength 8/0 (exception 3), Surf 12/1 (Petalburg), Fly 12/2, Dive 17/0, Waterfall 19/1 (Sootopolis).
- Rival beats: RB1 1/0, RB2 3/2, RB3 6/0, RB4 12/0, RB5 15/0; Wally 1 7/0, Wally 2 20/1 (Sootopolis, Victory Road).
- Arc beats, in arc order (Aqua unless noted): Petalburg Woods grunt 2/0, Stop Thief 3/0, Museum 5/2, Magma Meteor Falls 10/0, Magma Mt. Chimney 11/0, Shelly 13/0, Red Orb 14/0, Magma Hideout 14/1 (Lavaridge, 11), submarine 15/1 (Slateport, 5), Aqua Hideout 15/2, Magma Space Center 17/0, Seafloor Cavern 17/2, Cave of Origin 18/0, Sky Pillar 19/0, Rayquaza ends the fight 19/1 (Sootopolis). Devon Goods (recovered 3/2, Letter delivered 4/2, delivered 5/2) is part of the arc.
- Special-item McGuffins (fixed): Wailmer Pail 2/2, Go-Goggles 11/2, Devon Scope 12/0.
- Utility items: PokéNav 3/2, Old Rod 4/0, Itemfinder 6/0, Bicycle 7/2, Coin Case 7/2 (optional chain), Pokéblock Case 15/0, Super Rod 17/0. Good Rod 13/2 (the Route 118 river fisherman; the lakes make it mandatory). Not placed: Harbor Mail (optional chain, 5/2).
- Tutorial lists (blocks 0 and 1) are the hardcoded ones in design.md, "Opening".
- Callback blocks and hosts: block 1 to county 0, 12 to 2, 14 to 11, 15 to 5, 19 to 18, 20 to 18. All other blocks have no callback.

#### Host masks (bit i = county-story-index i)

- Gym and gym HM (Cut, Flash, Rock Smash, Strength): 1-19 (`0x000FFFFE`); county 0 only as a callback.
- Water HM (Surf, Dive, Waterfall): blocks where county x or x+1 is Ocean terrain: 3, 4, 15-19 (`0x000F8018`). The vanilla Surf block 12 is outside it (Route 118's river between two land counties; exception 2).
- Rival beats and utility items: 1-19 (`0x000FFFFE`).
- Arc beats: each beat's own host only (the location kind it needs exists once); the mask is the beat's block.
- A callback host y(x) must pass the kind's mask and, for a gym, have a settlement.

#### Callback chance

- Randomized path: p = 51/256 (about 20%). Vanilla has 5 callbacks in the 19 blocks after the tutorial (26%), forced by the list above, so p never applies to the vanilla stage.

#### Placeholder ranges (randomized path, for the user to check)

Windows are blocks (inclusive). Gyms 1-4 split blocks 1..n-1 into four windows with cut points ceil(k(n-1)/4); gyms 6-8 split n+1..E-1 into three with cut points n + ceil(k(E-1-n)/3). With n = 12, E = 20 these evaluate as shown; "vanilla" is the vanilla block.

| Item | Range (n=12, E=20) | Vanilla | In range |
| --- | --- | --- | --- |
| G1 / G2 / G3 / G4 | 1-3 / 4-6 / 7-9 / 10-11 | 3 / 4 / 7 / 11 | yes |
| G5 (Surf gym) | n only: 12 | 12 | yes |
| G6 (Fly, never a callback) / G7 / G8 | 13-15 / 16-17 / 18-19 | 12 / 17 / 19 | G6 no (exception 1) |
| HMs | the gym's slot (Strength included in the randomized path) | | Strength no (exception 3) |
| RB1 | 1 (tutorial) | 1 | yes |
| RB2 (before G1's window ends) | 2 to cut 1 = 2-3 | 3 | yes |
| RB3 (gyms 2-3) | 4-9 | 6 | yes |
| RB4 (gyms 4-5) | 10-12 | 12 | yes |
| RB5 (gyms 6-7) | 13-17 | 15 | yes |
| Wally 1 / Wally 2 | G3's window 7-9 / E only: 20 | 7 / 20 | yes |
| Arc beats (authored per arc) | nominal block +-2, clipped to 2..E-1; ordered by arc sequence, several per block allowed | the blocks above | yes by construction |
| Bicycle, Itemfinder | 1 to floor(E/2) = 1-10 | 7, 6 | yes |
| Old Rod, PokéNav | 1 to ceil(E/3) = 1-7 | 4, 3 | yes |
| Good Rod | ceil(E/3)+1 to floor(2E/3) = 8-13 | 13 | yes |
| Super Rod | floor(2E/3)+1 to E-1 = 14-19 | 17 | yes |
| Other utility (Coin Case, Pokéblock Case, ...) | 1 to E-1 | 7, 15 | yes |

Arc beats cannot use disjoint windows (design.md says ranges of a kind are disjoint) because the Emerald arc has two beats in blocks 14, 15, 17 and 19. They are ordered instead: beats resolve in arc order, each beat's block is at least the previous beat's, and two beats never share a (block, step). Quotas: 8 gyms + League, 8 HMs, 5 rival beats + 2 Wally beats, 15 arc beats + 3 Devon Goods beats (Emerald arc), 8 placed utility items.

#### Step predicates for the read-only adapter

A list is done when every blocking get in it is true; the current index is the first list, scanning from 0, that is not done (Emerald lets some gets happen early: Fly with RB4, Strength at Rusturf, Go-Goggles, so the highest true predicate is wrong). A step-3 unlock is done when its named key's predicate holds; the 3 blockers (35, 39, 67) and Fortree's open unlock (47) are done when the previous list is. Gets are blocking or reported-only. Blocking means vanilla forces the get (a scene or gate) or needs it by the end game; reported-only gets are reported in the done mask but never hold a list, because a skippable blocking get (the Good Rod's fisherman has decline and bag-full branches that never set its flag, and nothing later reads it) would freeze the index for good. When in doubt a get is reported-only: a forced one then costs only a hint that moves on early. So the planner's "Good Rod is mandatory" is a rule for the randomized path, not something vanilla enforces. Reported-only: RB2, RB5, Wally 1, the Museum delivery, the Shipyard visit, Shelly, Meteor Falls and Mt. Chimney beats, every utility and special item, Cut, Flash, Rock Smash, Strength and Fly, Running Shoes and the Poké Balls (no flag; `STORY_PRED_ALWAYS`). Setter sites were found with `setflag`/`setvar` in `data/` (non-Frlg), and every predicate was checked to be monotone (no `clearflag`, no lower `setvar` later). Code: `include/util/plot_stage.h`, `src/util/plot_stage.c`, data `src/data/region/emerald_plot_gets.h`, test `test/region_plot_stage.c`; the table below is the reading, the data file is the classification.

Resolved readings: the Woods grunt is `VAR_PETALBURG_WOODS_STATE >= 1`, set after the battle on both paths; Stop Thief is `VAR_RUSTBORO_CITY_STATE >= 2` (the var only climbs 1 to 8; `FLAG_DEVON_GOODS_STOLEN` is cleared later); the Shipyard visit is `FLAG_DOCK_REJECTED_DEVON_GOODS` or `FLAG_DELIVERED_DEVON_GOODS` (the dock checks the latter first); RB3 is `VAR_ROUTE110_STATE >= 1` and the Itemfinder, which has no flag, is given in the same scene; RB4 is `VAR_ROUTE119_STATE >= 1`; the submarine is `VAR_SLATEPORT_HARBOR_STATE >= 2` (MagmaHideout_4F sets 1); Sky Pillar is `VAR_SKY_PILLAR_STATE >= 1`; the Good Rod's flag is set at Route118:36. The extra Route 104 rival (`FLAG_DEFEATED_RIVAL_ROUTE_104`) is RB2 at a second venue, not a sixth battle: both venues use the `TRAINER_MAY_RUSTBORO_*`/`BRENDAN` parties and set `VAR_RUSTBORO_CITY_STATE` 8 and `VAR_ROUTE104_STATE` 2, which disables the other trigger, so RB2 is `FLAG_DEFEATED_RIVAL_RUSTBORO` or `FLAG_DEFEATED_RIVAL_ROUTE_104`. `FLAG_DEFEATED_MAGMA_SPACE_CENTER` is cleared again by the Rayquaza call (MossdeepCity_SpaceCenter_2F:328), so the Space Center beat reads `VAR_MOSSDEEP_SPACE_CENTER_STATE >= 3` instead.

| Index | Gets | Done when |
| --- | --- | --- |
| 0 | Starter | `FLAG_SYS_POKEMON_GET` (Route101:221) |
| 4 | RB1 | `FLAG_DEFEATED_RIVAL_ROUTE103` (Route103:143) |
| 5 | Pokédex, Poké Balls, Running Shoes | `FLAG_RECEIVED_POKEDEX_FROM_BIRCH` (Lab:548), `FLAG_RECEIVED_RUNNING_SHOES` (Littleroot:900); balls have no flag |
| 8 | Wally has Ralts; Woods grunt | `VAR_PETALBURG_CITY_STATE >= 3` (Petalburg:47); `VAR_PETALBURG_WOODS_STATE >= 1` (PetalburgWoods:111) |
| 10 | Wailmer Pail | `FLAG_RECEIVED_WAILMER_PAIL` |
| 12 | G1 + Cut; Stop Thief | `FLAG_BADGE01_GET`, `FLAG_RECEIVED_HM_CUT`; `VAR_RUSTBORO_CITY_STATE >= 2` (RustboroCity:302) |
| 14 | Devon Goods recovered; PokéNav; Letter; RB2 | `FLAG_RECOVERED_DEVON_GOODS`; `FLAG_RECEIVED_POKENAV`; `FLAG_RETURNED_DEVON_GOODS` (the return gives the Letter); RB2 optional (`FLAG_DEFEATED_RIVAL_RUSTBORO` or `FLAG_DEFEATED_RIVAL_ROUTE_104`) |
| 16 | G2 + Flash; Old Rod | `FLAG_BADGE02_GET`, `FLAG_RECEIVED_HM_FLASH`; `FLAG_RECEIVED_OLD_ROD` |
| 18 | Letter delivered | `FLAG_DELIVERED_STEVEN_LETTER` |
| 20 | Shipyard visit | `FLAG_DOCK_REJECTED_DEVON_GOODS` or `FLAG_DELIVERED_DEVON_GOODS` |
| 22 | Devon Goods delivered; Museum Aqua beat; Harbor Mail | `FLAG_DELIVERED_DEVON_GOODS` (both beats are one scene); Harbor Mail optional |
| 24 | RB3; Itemfinder | `VAR_ROUTE110_STATE >= 1` (Route110:478; also the Itemfinder, which has no flag) |
| 28 | Wally 1 | optional: `FLAG_DEFEATED_WALLY_MAUVILLE` |
| 30 | G3 + Rock Smash; Coin Case; Bike | `FLAG_BADGE03_GET`, `FLAG_RECEIVED_HM_ROCK_SMASH`; `FLAG_RECEIVED_BIKE`; Coin Case optional (`FLAG_RECEIVED_COIN_CASE`) |
| 32 | Strength | `FLAG_RECEIVED_HM_STRENGTH` (RusturfTunnel:60) |
| 40 | Meteorite stolen | `FLAG_MET_ARCHIE_METEOR_FALLS` |
| 44 | Mt. Chimney Magma beat; G4 | `FLAG_DEFEATED_EVIL_TEAM_MT_CHIMNEY`; `FLAG_BADGE04_GET` |
| 46 | Go-Goggles | `FLAG_RECEIVED_GO_GOGGLES` |
| 48 | RB4; Devon Scope | `VAR_ROUTE119_STATE >= 1` (Route119:145); `FLAG_RECEIVED_DEVON_SCOPE` |
| 49 | G5 + Surf | `FLAG_BADGE05_GET`, `FLAG_RECEIVED_HM_SURF` |
| 50 | G6 + Fly | `FLAG_BADGE06_GET`, `FLAG_RECEIVED_HM_FLY` (set in Route119 with RB4, so true early) |
| 52 | Shelly | `FLAG_RECEIVED_CASTFORM` (Route119_WeatherInstitute_2F:123) |
| 54 | Good Rod | `FLAG_RECEIVED_GOOD_ROD` (Route118:36) |
| 56 | Red Orb | `FLAG_RECEIVED_RED_OR_BLUE_ORB` (MtPyre_Summit:61) |
| 57 | Magma Hideout | `FLAG_GROUDON_AWAKENED_MAGMA_HIDEOUT` |
| 60 | RB5; Pokéblock Case | optional (`FLAG_MET_RIVAL_LILYCOVE`); `FLAG_RECEIVED_POKEBLOCK_CASE` |
| 61 | submarine | `VAR_SLATEPORT_HARBOR_STATE >= 2` (Harbor:64) |
| 62 | Aqua Hideout | `FLAG_TEAM_AQUA_ESCAPED_IN_SUBMARINE` |
| 68 | G7 + Dive; Space Center; Super Rod | `FLAG_BADGE07_GET`, `FLAG_RECEIVED_HM_DIVE`; `VAR_MOSSDEEP_SPACE_CENTER_STATE >= 3` (the flag is cleared again later); `FLAG_RECEIVED_SUPER_ROD` |
| 70 | Seafloor Cavern | `FLAG_KYOGRE_ESCAPED_SEAFLOOR_CAVERN` |
| 72 | Cave of Origin | `FLAG_WALLACE_GOES_TO_SKY_PILLAR` |
| 76 | Sky Pillar | `VAR_SKY_PILLAR_STATE >= 1` (SkyPillar_Top:127) |
| 77 | Rayquaza ends the fight; G8 + Waterfall | `FLAG_SOOTOPOLIS_ARCHIE_MAXIE_LEAVE`; `FLAG_BADGE08_GET`, `FLAG_RECEIVED_HM_WATERFALL` |
| 81 | Victory Road, Wally 2 | `FLAG_DEFEATED_WALLY_VICTORY_ROAD` |
| 82 | League lobby, Elite Four, Champion | `FLAG_ENTERED_ELITE_FOUR`, then `FLAG_SYS_GAME_CLEAR` (set in C at the Hall of Fame) |

Unlock keys that are not a get's own predicate: Pokédex lock at Oldale (index 7) is also read by the sketch artist as `FLAG_ADVENTURE_STARTED`; "Wally has Ralts" is the lock on Route 104's gym boy (`VAR_PETALBURG_CITY_STATE >= 3`).

#### Traversal McGuffins (route unlocks only)

HMs other than Fly, each with the Emerald locks it opens (chain entry numbers); every use needs the HM and its gym's badge:

- Cut: Petalburg Woods trees (15). Flash: Granite Cave B1F-B2F (33), Ancient Tomb (95), Victory Road (132); see design.md, "Dark maps". Rock Smash: Rusturf debris (24, unlocks Verdanturf, index 31), Granite Cave B2F rocks (51), Desert Ruins (74), Mirage Tower (75), Seafloor Cavern (117). Strength: Fiery Path tunnels (57), Magma Hideout (102), Seafloor Cavern (117), Victory Road (132). Surf: Route 103 east (7), seas (31), Route 118 (52, unlocks Weather Institute, index 51), Route 115 (79), Abandoned Ship (81), New Mauville (84), Mt. Pyre route (99), Route 124 (108), Sky Pillar route (122). Dive: Abandoned Ship hidden floor (115), Seafloor Cavern (117), Sootopolis entrance (119, unlocks index 71), Sealed Chamber (123). Waterfall: Meteor Falls (64), Ever Grande (131, unlocks index 79).
- Special items (as designed, fixed in this stage): Go-Goggles (Route 111 desert, 54), Devon Scope (Route 120 bridge and Fortree gym entrance, 92 and 94), Wailmer Pail (the planner's placeholder lock for Rustboro, index 11; vanilla Emerald has no lock there).
- Not traversal: the Bicycle (a utility item; it gates the Cycling Road and Jagged Pass optionally), the Basement Key, Storage Key and Magma Emblem (subplot and arc keys), Fly (a convenience, never a lock).

#### Blocker McGuffins

A person standing in the way until a boolean. Emerald has these (chain entry, who, boolean):

| Entry | Blocker | Until |
| --- | --- | --- |
| 6 | Sketch artist, Oldale west exit | `FLAG_ADVENTURE_STARTED` (Oldale:8, set in Birch's Lab:519) |
| 14 | Gym boy, Petalburg west exit | `VAR_PETALBURG_CITY_STATE >= 3` (Petalburg:8 moves him, Petalburg:47 frees the path) |
| 133 | Ace Trainers, League lobby | 8 badges (popcount of `FLAG_BADGE01_GET`-`08`) |
| 41 | Cycling Road gate | Bicycle held (`FLAG_RECEIVED_BIKE`) |
| 13/77 | Norman, Petalburg gym | 4 badges (derived stage, design.md "Interface rules") |
| 36, 55 | Aqua grunts at the Museum, Magma grunts at the cable car | arc beats owned by the arc (main arc bucket); not generic blockers |

The three planner blockers (indices 35, 39, 67: unlocks into Fallarbor, Meteor Falls and Mossdeep) have no Emerald counterpart yet. Until the pool grows (design.md, "Blocker McGuffins"), subplots author their own blocker scripts; the first generic blockers are those three, written as one parameterized "NPC blocks the route until the index passes step s" script.

### Per-county storage needs

Counted from the table above, by host county (callback gets count for the host, not the block). A gym and its HM count as two gets, a reserved county as zero. "Max list" is the largest single list the county hosts, which is what a completion mask must hold. "Chain vars" and "Bools" are the county's own optional subplot state (not main-arc gets), estimated from the flags and vars Emerald's scripts set in the county's maps, classified by name rather than by reading each script. Strictly ordered subplots in one county are merged into one chain var; one-shot gifts and independent visits are one bool each. Fan-club gifts, Shoal Cave salt and shells, and the Lilycove paintings are counted as bools but could become counters.

| County | Gets hosted | Max list | Chain vars | Bools |
| --- | --- | --- | --- | --- |
| 0 Littleroot | 4 (Starter; callback: Pokédex, Poké Balls, Running Shoes) | 3 | 0 | 0 |
| 1 Oldale | 1 | 1 | 0 | 1 |
| 2 Petalburg | 5 (+ callback G5, Surf) | 2 | 0 | 6 (Woods, Miracle Seed, flower shop, 3 Route 104 gifts) |
| 3 Rustboro | 7 | 4 | 1 (fossil revival, merged with Mirage Tower's pick) | 5 |
| 4 Dewford | 4 | 3 | 0 | 2 |
| 5 Slateport | 5 (+ callback Aqua beat) | 3 | 2 (harbor and Stern; fan club) | 10 (6 fan-club gifts) |
| 6 Cycling Road | 2 | 2 | 5 (Trick House, 17 vars packed) | 2 (Daycare egg pending, one other) |
| 7 Mauville | 5 | 4 | 1 (New Mauville: key, generator, report) | 4 |
| 8 Verdanturf | 1 | 1 | 0 | 3 |
| 9 Fallarbor | 0 | 0 | 1 (Glass Workshop 17 values plus Fossil Maniac 3) | 5 |
| 10 Meteor Falls | 1 | 1 | 0 | 2 |
| 11 Lavaridge | 4 (+ callback Magma beat) | 2 | 1 (Mirage Tower and fossil pick) | 4 |
| 12 Fortree | 4 | 2 | 0 | 4 (Wingull errand pair shared with 17) |
| 13 Weather Institute | 2 | 1 | 0 | 1 |
| 14 Mt. Pyre | 1 | 1 | 0 | 2 |
| 15 Lilycove | 3 | 2 | 2 (museum; fan club) | 11 (5 paintings) |
| 16 Shoal Cave | 0 | 0 | 0 | 10 (4 salt, 4 shell) |
| 17 Mossdeep | 5 | 4 | 0 | 4 |
| 18 Sootopolis | 6 (+ callbacks Rayquaza ends fight, G8, Waterfall, Victory Road, Wally 2) | 3 | 1 (Mystery Events house, 4 values) | 2 |
| 19 Pacifidlog | 1 | 1 | 1 (weekly TM day) | 3 |
| 20 Ever Grande | 3 | 3 | 0 | 1 |

Totals: 64 gets, 20 unlocks (blocks 0-19), 3 of them blockers (indices 35, 39, 67); about 15 chain vars and about 82 bools. At most 11 bools in any county, and about 380 flags are unused today, so a fixed 12-flag pool per county would fit (288) without packing. Not counted: post-game content (Battle Frontier, Southern Island, Regi trio, Altering Cave), the 7 Trick House and Mossdeep gym switch flags (scratch), per-city visited and landmark flags, Pokénav match-call flags, and engine vars.

- Only one list is current at a time, and the largest is 4 (Rustboro step 2, Mauville step 2, Mossdeep step 0), so one shared 4-bit completion mask would do. Per-county masks are only needed if a county's completed gets must survive past its list, which level-triggered completion makes unnecessary.
- McGuffin flags: the 9 event keys in step-3 unlocks (Letter delivered, Devon Goods delivered, RB3, Meteorite stolen, Shelly defeated, Groudon awakened, blockade gone, Kyogre awakened, Cave of Origin done) all have an Emerald flag or var (see "Planner inputs", step predicates), except that RB3 has only a var. Once the main-story-index is maintained they are derivable (a key's index is below its lock's), so they need no stored flag; the 3 blockers likewise.

### Exceptions to the normal rules

Numbered so they can be struck off as the rules or the table change.

1. G5 (Petalburg, the step-1 callback) and G6 (Fortree, step 2) share block 12, so the Surf gym and Fly's gym are adjacent in the story; encoded in `GetGymSlot`. Kept because gyms 5 and 6 in adjacent blocks would feel odd in a generated region.
2. Surf's unlock (index 51) opens a land county (Weather Institute, across Route 118's river), not an ocean county, and the Surf lock is block 12 rather than "the ocean county's dead end".
3. Strength is given at Rusturf Tunnel (index 32), long before G4 (index 44), instead of with its gym. Emerald-only; the algorithm keeps an HM at its gym's time.
4. Ports (design.md, "Ports"): the algorithm puts the port county first and the ocean county right after it, but here Dewford (ocean, 4) precedes Slateport (port, 5), and Briney's first trip starts at Petalburg's Route 104, not at a port city. Swapping them would put Slateport's Devon Goods delivery before Dewford's Letter. Also the third port, Lilycove (15), is later than the rule would place it (about index 10-13). Keep as an exception unless the story order changes.
5. G5's callback to Petalburg is 5 county steps from Fortree, beyond the pre-Fly bound of 3. Hardcoded exception.
6. Devon Goods: Peeko and the Take-Down are hosted on Route 116 (inter-county route 3-8), not inside the Rusturf Tunnel in county 8; Dewford to Slateport is keyed on "Letter delivered" (block 4's own get) instead of Devon Goods; the delivery is a block-5 get that relies on index order, not a key check.
7. Fiery Path, Route 111 north and Route 112 north are walked on the way to Fallarbor before county 11 unlocks, so they move into county 8's cell. Verdanturf's cell is otherwise nearly empty.
8. Rival Battle 3 moves from the Mauville cell to the Cycling Road cell (Route 110).
9. Seafloor Cavern moves from the Ever Grande cell to Mossdeep's. Victory Road lives in Sootopolis's cell, so Wally battle 2 is a callback.
10. Mt. Pyre (14) is forced before Lilycove (15); Emerald lets the player visit Lilycove first.
11. Blocks 8, 9 and 16 have no mandatory gets, so their unlocks need blocker McGuffins that vanilla Emerald lacks (or a tiny subplot each). Block 11's unlock into Fortree (index 47) has no lock at all: the step is open.
12. Verdanturf's unlock uses Rock Smash + Dynamo Badge, which also gates Route 117; vanilla leaves it open.
13. The Elite Four county has two gates: Waterfall + Rain Badge into Ever Grande, and 8 badges into the League lobby.
14. The empty county (3,2) must take a reserved index (23) and holds no legendary location.
15. Sealed Chamber and the Regi trio are mid-game in Emerald but post-Elite Four here (reserved counties).

### Optional content with inter-county story

These do not use planner slots. Each is gated by keys from any earlier index (design.md, "Main plot planner" rules) and never blocks the main story. They need an optional-story index (tentatively a var each, see `remaining_work.md`) so NPCs in other counties can hint at them.

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
