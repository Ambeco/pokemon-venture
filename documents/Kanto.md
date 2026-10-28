# Kanto reference

Kanto (FireRed/LeafGreen) data for the generic pools (design.md, "Development approach"). Hoenn's is in `Emerald.md`. The goal is that a generated region can draw Kanto's city names, themes, NPC and sign texts and building roles from the same pools as Hoenn's, so randomization is less likely to run out of names, to force a theme onto a county that fails its constraints, or to run out of in-city locations.

## What the ROM can carry

The Emerald ROM cannot reach the FR/LG maps (`tools/mapjson` drops maps whose `region` differs from the build), and the 362 FR/LG layouts plus tilesets are large, so Kanto's maps and tiles stay out until generated maps free space. Text is a different size class: the whole FR/LG script text is about 9,000 strings and 330 KB of source, but only the generic part is wanted (city and island signs, building-role talk, names), which is a few KB. A build tool should extract it from `data/maps/*_Frlg/scripts.inc` into its own data file, outside the map scripts, as the text pools of Milestone 1.

## City themes

The slogans are the city sign texts in the repo (`<City>_Text_CitySign` in `data/maps/*_Frlg/scripts.inc`). Tags are a starting vocabulary like Hoenn's.

| City | Slogan | Tags |
| --- | --- | --- |
| Pallet Town | Shades of your journey await! | start |
| Viridian City | The Eternally Green Paradise | forest |
| Pewter City | A Stone Gray City | mountain |
| Cerulean City | A Mysterious, Blue Aura Surrounds It | water |
| Vermilion City | The Port of Exquisite Sunsets | coast, port |
| Lavender Town | The Noble Purple Town | ancient |
| Celadon City | The City of Rainbow Dreams | entertainment |
| Fuchsia City | Behold! It's Passion Pink! | park |
| Saffron City | Shining, Golden Land of Commerce | industry, capital |
| Cinnabar Island | The Fiery Town of Burning Desire | island, volcano |
| One Island | Friends Gather at Knot Island | island |
| Two Island | Boon Island for Two | island |
| Three Island | Kin Island of Family Bonding | island |
| Four Island | The Warm, Blue, Floe Island | island |
| Five Island | Chrono Island: Where Time Goes | island |
| Six Island | Fortune Island of Aged Wisdom | island |
| Seven Island | Quest Island of Infinity | island |

## Gyms

| Ordinal | City | Type | Leader |
| --- | --- | --- | --- |
| 1 | Pewter | Rock | Brock |
| 2 | Cerulean | Water | Misty |
| 3 | Vermilion | Electric | Lt. Surge |
| 4 | Celadon | Grass | Erika |
| 5 | Fuchsia | Poison | Koga |
| 6 | Saffron | Psychic | Sabrina |
| 7 | Cinnabar | Fire | Blaine |
| 8 | Viridian | Ground | Giovanni |

## Special-location kinds (draft, from memory; check against Bulbapedia)

Using the two enums from `Emerald.md`.

- `RouteLocation`: Viridian Forest (forest), Mt. Moon (cave), Cerulean Cave (cave), Rock Tunnel (tunnel), Diglett's Cave (tunnel), Seafoam Islands (cave), Victory Road (tunnel), Power Plant (facility), Safari Zone (park), Bill's House (roadside building), S.S. Anne (wreck, or a new `ship` kind if it is not abandoned).
- `TownLocation`: Pewter Museum (museum), Celadon Game Corner (entertainment), Celadon Dept. Store (store), Silph Co. (institution), Pokémon Tower (tower), Pokémon Mansion (mansion), the Vermilion harbor (harbor).

This adds three town kinds to Emerald's list (tower, mansion, and possibly a `ship` route kind), and no in-city cave.

Open: whether Kanto's special locations get a pass for building roles too (Pokémon Fan Club, Name Rater, Day Care).
