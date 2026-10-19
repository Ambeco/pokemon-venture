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
