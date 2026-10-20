# Pokémon Emerald reference

Emerald-specific data for the hardcoded stage (design.md, "Development approach"): the Hoenn county layout, and the Emerald rival arc, story arc and subplots the first version reuses. Generic design lives in `design.md`; Emerald examples used to explain it stay there.

## County assignment

Hoenn does not fit the 6x4 county grid cleanly, so for the hardcoded stage places are fudged into counties, with everything very slightly stretched apart. Since most content is hardcoded at this stage, the stretching does not matter, and once the world is fully generated there is no fakery left.

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

### Special location counts

Counted from the county table, classifying each place by where it is in Emerald (gyms, the Elite Four, rival battles and HM pickups excluded; Glass Workshop, Pretty Petal Flower Shop, Trainer Hill, Trick House, Daycare and Berry Master are on routes):

- Outside cities: 38 over 23 non-empty counties, about 1.65 per county (busiest: Lavaridge's county with 6).
- Inside cities: 10 over 16 cities, about 0.6 per city (Devon Corp, Mauville Game Corner, Lilycove Dept. Store and Contest Hall, Mossdeep Game Corner and Space Center, Cave of Origin, Birch's Lab, Shipyard, Museum). Counting the 8 gyms and the Elite Four gives 19, about 1.2 per city.
- The table lists only notable places, so these are lower bounds.

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
34. Granite Cave back chamber, Steven — needs: Letter — gives: Letter delivered
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
83. Mauville, Wattson asks you to shut down New Mauville's generator — needs?: Mauville access — gives: Basement Key
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
