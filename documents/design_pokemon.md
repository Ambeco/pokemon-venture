# Pokémon and battle design

Procedural species types, wild encounters, trainers, moves and experience, plus the battle and PC aids that make random types readable (sprite recoloring, type icons, damage grades, the team analyzer).

## Procedural species types and wild encounters

Species types and move pools are procedurally generated, and wild Pokémon on a route should match the route's biome, like a real game.

### Biome type affinity

Each biome has a hardcoded table of how often each of the 18 Pokémon types occurs there. Possible encoding: 3 bits per type on a log scale, 54 bits (7 bytes) per biome.

### Species <-> biome mapping

A species' biome must be findable quickly without walking the generated map data, and so must the species list of a biome. This uses a two-way coprime-stride (modular multiplicative) mapping between species and biomes:

- The mapping operates on the ~541 evolutionary lines (via each species' base evolution), and associates every line with one of the region's 12 biomes.
- A stride coprime to 12 (not divisible by 2 or 3), and its modular inverse, are hardcoded.
- Species to biome: `(baseSpeciesIndex * stride + playerID) mod 12`, indexing the region's biome subset.
- The species' two types are then chosen by random selection seeded from species + player ID, weighted by that biome's type affinity.
- Biome to species: compute the first species of the biome via the inverse, then the rest follow at a fixed index distance from each other (derived from the inverse).
- Each biome ends up with ~45 evolutionary lines.
- A Pokémon whose trainer ID differs from the player's maps through that trainer ID, so it may land in a different biome and get different types.

### Species metadata

Per-species fields used for encounters and trading (field widths are tentative):

- Minimum requirements: flags for a required habitat. Fish-looking species require water (surfing or fishing) and are almost certainly Water type; land-looking species such as Pikachu (a mouse) require land. These two flags may be the only ones.
- Habitat bias: flags biasing a species toward or away from habitats. They are based on the species' image, never its type: types are randomized, each biome should have a general mix of types, and a habitat such as volcano should be neither only fire nor the only home of fire Pokémon. Example: Pikachu looks like a mouse, so it prefers tunnel, cave, valley, forest and ruins, and avoids beach, clouds, reef, ocean, snow and tundra.
- Domestication (2 bits, 0-3): 0 is fully wild (Pinsir, Kangaskhan, Tauros, Dratini, legendaries), biased toward the map edges and Safari Zones; 3 is fully domestic (Hitmonlee, Hitmonchan, Meowth, Porygon).
- Base evolution: a reference to the species' base form, so that all evolutions share the base's type generation.
- Normal level range: a 3-bit enum for the low end (2, 7, 10, 17, 25, 34, 43, 55); the high end comes from the evolution data.
- Commonness (2 bits): 0 = legendary (never wild; NPCs never have or trade them); 1 = baby, trade-evolved, item-evolved and fossil Pokémon (never wild; NPCs rarely have them, but trade normally); 2 = starters and other super-rare Pokémon such as Chansey and Tauros (super rare in the wild; NPCs rarely have them, but trade regularly); 3 = normal (encounter ratios based on catch rate; NPCs regularly have and trade them).

### Route encounter selection

The species-to-biome stride mapping only determines types. A route's encounters are built separately from the evolutionary lines correlated with the route's biome (~45 per biome). Many routes share a biome but sit at very different levels, so the species are filtered, then weighted:

- by commonness: legendaries, babies, fossils, and trade/item-evolved Pokémon are never wild, and starters and other super-rare Pokémon are very rare,
- by domestication, relative to how close the route is to the map edge or a Safari Zone,
- by habitat: minimum requirements (e.g. water for fish) and image-based habitat biases,
- by level: species whose evolve-to/evolve-from levels make them unlikely at the route's level are dropped,
- by time of day and weather,
- by a random filter that drops some of the rest.

Most of the ~45 lines are expected to be filtered out, leaving roughly 10-15.

Super-common species: a hardcoded list of very common species (catch rate >= 235; Pidgey, Rattata, Zubat, Magikarp, etc.) that, as in the mainline games, can show up on routes all over the region. When a route's table has too few species after filtering, these are also considered, bypassing the biome filter but not the others (habitat still applies: Magikarp needs water, Pidgey avoids caves, Zubat only spawns in caves and ruins). Still under consideration. The survivors' encounter ratios are proportional to their catch rates.

All of this, including the randomness, is derived from the trainer ID, so each play-through is fully deterministic. Tables are deterministic, not fixed: they are recalculated when the time of day or weather changes.

A species whose habitat strongly opposes the biome may not appear at all in a play-through. This is intended, like Sandshrew, Vulpix, Meowth, Bellsprout, Magmar, and Pinsir never appearing in Pokémon Red.

Table size and ratios:

- Tables are built on the fly and never stored in ROM, so Emerald's 12-slot land cap (probably there to keep ROM tables small) can be dropped, as can its hardcoded slot ratios, if the encounter code allows dynamic sizes and ratios.
- If a table still needs to be capped, species are dropped randomly until it fits.

Encounter methods:

- Each encounter method gets its own table (PokeAPI's `encounter_methods.csv` lists many: walk, surf, old/good/super rod, rock smash, dark grass, headbutt, spots, roaming, and others).
- Most are not needed. Walk, surf, grass, dark grass, and other common methods are calculated eagerly; item and object interactions (rods, headbutt, berry trees, "spots") are calculated lazily.

Evolution levels:

- Level-based evolutions define a line's level range, which normally keeps multiple stages of one line off the same route. Occasional overlap is fine, as in mainline games.
- Non-level evolutions use hardcoded estimated levels for encounter-table purposes only (e.g. stone evolutions at ~29, since Red's Celadon sells stones and its gym leader's ace is level 29). The Pokémon still evolves only by its real method.

### Trainer teams and levels

Route-story-indices determine wild Pokémon levels, trainer levels, hidden item choices, and similar progression-scaled content. Trainer species are chosen as follows:

- Trainers in counties with story index 1-2 generally use only species passing that county's encounter filters and biome.
- Trainers in counties with story index 3+ loosen the filters: the species may match the biome or any adjacent biome (orthogonal neighbors in the 5x5 grid, so up to 4; edge biomes have fewer), must fit the level, and must be within 1 domestication step (of the 0-3 domestication value).
- Gym members, the rival, the Elite 4, and other "boss" trainers are not limited by domestication, habitat, or biome.
- Low-level "bad guy" (Team Rocket-style) members are also not limited by domestication, habitat, or biome, but only use species whose base evolution has a catch rate of 255 (the starting cutoff, to be tuned by feel) (Rattata, Zubat, Caterpie, Pidgey, Spearow, Ekans, Oddish, Diglett, Meowth, Poliwag, Bellsprout, Geodude, Magikarp, etc.).

### Rival teams

The rival's team is stored (six vars, one species each) and updated as the player enters routes. Each route is folded into the team once, on its first entry, by applying the swap rules below to that route's encounter table, which is already generated because the route is on-screen. A rival battle then reads the stored team and generates nothing: replaying every earlier county and route at battle time would need all of them in RAM (design.md, "Resource budget", locality rule). The team therefore reflects the routes the player actually entered, in the order entered, so two players of one seed may differ; the starter and the swap rules are functions of the seed. The update runs during the route's entry transition.

- **Team size:** the stored team grows to six whatever the battle; the size of the next battle's team (from rival battles already won, global state) is applied only when the battle party is built, by taking the first members in slot order.
- **One pass, no seen-route flag:** a route is folded in one pass on every entry, and the result need not be stable or reproducible, so there is no per-route flag; walking back into an old route may change the team a little.
- **Two rivals:** Wally is the primary rival and follows the rules as written, except that he has no traditional starter (his team starts empty and nothing is protected). The May/Brendan rival is the secondary: she keeps her starter, and takes Wally's team as a second filter. She never takes a family Wally holds (a family is its line's base species), and when Wally takes a family she has, her member of it counts as an easier catch than any catch rate, so any candidate with no conflicting type replaces it. Each has six vars, twelve in all.
- **Engine:** `src/story/rival_team.c` is pure (candidates carry the final form's types and catch rate and the line's base species; the starter, if any, is protected). Candidates of one route are taken in order: those adding a missing fire/grass/water type, then lowest catch rate.

- **Starter:** the rival takes the starter whose type beats the player's starter, if one is offered; otherwise the stride pick from "Randomized starters". The starter is always slot 1 (the secondary rival's; Wally has none).
- **Candidates:** each species that spawns on the entered route is judged by its line's final form: that form's (randomized) types and catch rate. Legendaries, babies and starters (commonness 0-2) are never candidates, since they never spawn wild. When the rival fields a candidate, it uses the stage of the line that fits the battle's level (level evolutions by level, other evolutions by their estimated levels), so early teams are unevolved.
- **Type coverage:** the team's covered types are the union of its members' final-form types.
- **Swap rules, in priority order:**
  1. The rival never loses fire, grass or water coverage: a swap that removes the team's only fire, grass or water type is refused, even if it would add a missing one (a fire/flying rival missing grass skips a grass/flying candidate, and waits for a different grass one).
  2. A candidate that adds a missing fire, grass or water type (without breaking rule 1) is always taken, dropping the member(s) whose types overlap it (a candidate overlapping the starter is refused; with no overlap and a full team, the highest-catch-rate member that rule 1 allows is replaced).
  3. Otherwise a candidate replaces a member only if its final form has a lower catch rate than that member's and its types overlap no other member; a swap never drops a fire, grass or water member whose type is not also covered elsewhere (rule 1).
  4. A free slot (the team grows with the battle number, up to 6) takes the lowest-catch-rate candidate with no overlap, preferring fire/grass/water if missing.
- **Result:** with many hundreds of species, a non-conflicting fire, grass and water Pokémon almost always turns up, so late teams are a fire, a grass and a water Pokémon plus three distinctly typed ones, all with low catch rates. This is likely, not guaranteed: if a type never turns up, the rival simply lacks it; no fallback is invented.
- **Tests:** seed sweeps (entering routes in story order) assert rule 1 is never violated between consecutive battles, coverage of fire, grass and water by the last rival battle for almost all seeds (log the failures), and no duplicate types outside the forced adds.
- **Cost:** one pass over the entered route's encounter table per first entry; measure against the CPU budget (design.md, "Resource budget").

### Experience

- The whole party gains experience after every battle, as in Sword/Shield (`I_EXP_SHARE_FLAG` in `include/config/item.h`, set permanently rather than toggled by a key item).
- Level-based multipliers keep the party's levels together:
  - Underleveled bonus: a Pokémon below the current route's lowest wild level gets significantly more experience.
  - Overleveled penalty: a Pokémon above the next boss's estimated highest level (gym leader, villain leader, champion, etc.) gets significantly less.
- The next boss's highest level is estimated from the route-story-index alone (about `route-story-index + 6`, with boss levels about `route-story-index + 3`), not read from the actual boss team.
- Invariant: before every boss, at least one route with encounters lets the player train up to the boss's level + 1 without penalty. This holds by construction, so it needs no seed-sweep test: every boss is in a city or special location, each of which is reached by an actual route (which always has wild encounters) with a route-story-index 0-2 lower. Whenever the boss level formula changes, retune the penalty threshold against that 0-2 gap.

### Catch-rate floor by progression

Wild encounters are limited by a minimum catch rate that falls as the story progresses, so players can catch what they meet with the balls they can buy:

- The first route's encounters are generally limited to catch rate 250+, and each subsequent route lowers the threshold.
- Most Pokémon before the second gym should have catch rate 152+, because Great Balls are not sold until after the second badge. (At catch rate 192, a Pokémon at 50% health has a 50% chance of breaking out of a basic Poké Ball.)
- Ultra Balls are sold after the fifth badge, so from then on wild Pokémon should generally be above ~85. (At catch rate 85, a Pokémon at 30% health has a 60% chance of breaking out of a Great Ball.)
- The minimum falls to ~0 around the time of the seventh gym.
- Babies, starters, and legendaries (and possibly another excluded category) never occur in encounter tables.

### Legendaries and starters

- Around the seventh to eighth gym, McGuffins unlock a pair or trio of reserved special locations, each holding a pair or trio of legendaries from a single game generation.
- After the Elite Four, another reserved special location unlocks, holding a solo legendary.
- Ideally every legendary and starter is obtainable, but the mechanism is undecided. Starters might come as gift Pokémon after a (possibly weekly repeatable) chapter, as in Pokémon Yellow.
- The post-Elite Four solo legendary location might spawn a new legendary each week: 50% a legendary the player does not own, 50% fully random.

### Randomized starters

Later work, after the base randomization. Starters come from four hardcoded tables: "grass", "fire" and "water" (about 9 each, one per generation's starter line) and "bonus".

- **Rolling the offer:** coprime strides over the grass, fire and water tables pick one of each ("at random", from the seed). The final stride is saved, because the rival (and sometimes a second rival) take their starters from it.
- **Randomized types:** the table names are only the vanilla types. When types are randomized, every non-Splash starter in all four tables (and Pikachu and Eevee) is checked against its randomized type, and the first triplets are rolled by the same strides from the species that are now grass, fire and water, so the opening choices still cover the three types. Once those run out, the remaining starters are offered in random order, in whatever types they have. The rival takes the next pick to the right of the player's in the offered order, so may have no type advantage; this is expected, not a problem.
- **Professor's bag:** the bag graphic shows the edges of more balls than the three on offer. Interacting with it has the professor shout for the player to just grab one of the three. A second interaction repeats the shout. A third makes the three offered balls roll off screen and the next grass/fire/water triplet roll in, and every later interaction rolls out the next triplet, so the player can browse. A player who does not care takes one of the first three, which is the intended default.
- **Past the triplets:** after all grass/fire/water triplets, only two balls roll out: Pikachu and Eevee. Then the bag rolls triplets from the bonus table.
- **Bonus table (hardcoded list):** lines whose first species has a base stat total of 307-320, whose first evolution comes at level 14-18 and second at 30-36, and whose final species has a total of 525-535; the normal starters are excluded.
- **Last triplet:** Magikarp, Feebas and Cosmog, each knowing only Splash, always come last whatever the randomization (and are never part of the type scan above). The bag is then empty, so the player is stuck with one of them (or, realistically, restarts).

### Pokédex habitat view

As in Sword/Shield, the Pokédex entry shows where the species can be encountered. Selecting a region tile shows the species that spawn there: caught ones in color, the rest as black silhouettes. Both read the tile's encounter table, regenerated on demand like any other (see "Route encounter selection").

### Move pools

Moves are generated procedurally per species, from per-species anatomy and per-move requirements.

- Each species has a bitfield of appendages (e.g. mouth). Each move has a field of required appendages and an associated stat. "Roar" needs a mouth; "Tackle" needs nothing.
- The ~1000 moves are roughly sorted by power, with ~20 moves associated with each level.
- Learn levels are generated per species (a procedurally generated list of levels at which it learns a move). This guarantees no Pokémon goes long without learning something, which a per-level chance would not.
- At a learn level, the candidate range is wider than one level's ~20 moves, and exactly one move is guaranteed to be picked.
- Candidates are filtered out if the species lacks the required appendages. Moves whose type does not match the species' types are dropped with high probability, as are moves whose associated stat does not match the species' stat ratios.
- Levels 1 and 6 are always learn levels (at least one move each).
- Balance is deliberately loose: moves arrive in general level ranges, and as long as Pokémon get reasonably typed moves at roughly normal frequency and power level, each is roughly balanced. Players can catch 6+ Pokémon before the first gym, so type advantage and leveling cover the rest.
- TMs and tutors use the same validity filter as level-up, except instead of randomly picking one valid move, they only check whether a given move is valid for the species at all.
- Egg moves (tentative, pending research into how breeding works): pick a move the child species could learn by TM that a different-species parent learns naturally, chosen randomly.

## Shops and trading

- Poké Marts: villages and possibly towns keep a general Mart; cities instead have a separate store per bag category (balls, healing items, stat items, etc.).
- Towns and cities have the Pokémon Center trade room as a separate building (not the second floor).
- Each trade building has 0-3 random NPC traders per day; higher population makes more traders likely.
- For each trader, a route or special location is randomly chosen, then a random Pokémon that spawns there; that is the Pokémon the trader asks for, which guarantees the player can obtain it.
- The Pokémon the trader offers is a random species with the same commonness and a similar catch rate, whose evolution level range overlaps the chosen route's level. It may be the same species as the requested one.
- Traded Pokémon carry a different trainer ID, so their types and moves differ from the player's own (see the other-trainer-ID biome subset above).

## Type-recolored sprites

A species' sprite is recolored to suggest its randomized types: the sprite colors that stand for its original types are swapped for its new types' colors.

Type colors: 16 hues evenly spaced 22.5° apart, plus 3 achromatic types.

| Hue | Type | RGB | | Hue | Type | RGB |
| --- | --- | --- | --- | --- | --- | --- |
| 0° | Fire | #FF5555 | | 22.5° | Ground | #AA3F00 |
| 45° | Electric | #FFD455 | | 67.5° | Rock | #94AA00 |
| 90° | Bug | #AAFF55 | | 112.5° | Dragon | #15AA00 |
| 135° | Grass | #55FF7F | | 157.5° | Stellar | #00AA6A |
| 180° | Ice | #55FFFF | | 202.5° | Flying | #006AAA |
| 225° | Water | #557FFF | | 247.5° | Poison | #1500AA |
| 270° | Psychic | #AA55FF | | 292.5° | Ghost | #9400AA |
| 315° | Fairy | #FF55D4 | | 337.5° | Fighting | #AA003F |

Normal is white (#FFFFFF), Dark black (#000000), Steel gray (#7F7F7F). The bright set has S=0.67, V=1; the dark set S=1, V=0.67. The hues follow the mainline games' associations where possible (fire red, electric yellow, water blue, ghost dark purple, fairy pink, ground brown, etc.). Stellar is not a real type, so its color need not be distinct.

Color buckets define "close": a color with low saturation falls in an achromatic bucket by V (Normal, Steel, Dark); otherwise its bucket is its hue rounded to the nearest 22.5°, i.e. one of the 16 type hues. S and V never choose between hued buckets. Starting cutoffs, to be tuned by visual tests: saturation below 1/3 is achromatic (likely tuned down); achromatic V below 1/3 is Dark, above 2/3 Normal, between them Steel (the middle may need more room). A whole shading ramp of one color shares a hue, so it lands in one bucket and recolors together.

Algorithm:
1. For each original type, pick the sprite palette colors in that type's bucket.
2. If the bucket is empty, pick the most-used color that is not near black or white and (with two types) not near the other type's pick.
3. Recolor every color in a picked bucket by rotating its hue to the new type's hue (keeping its offset within the bucket) and scaling its saturation toward the new type's, while keeping each color's own V. Keeping V preserves the sprite's shading, so only the type colors' hues matter, not their brightness.
4. Achromatic types: recoloring to Normal, Dark or Steel desaturates instead of shifting hue; recoloring from them gives the picked gray a hue.

Shinies:
- A shiny's palette is its vanilla shiny palette, recolored at the same indices. Two-type shinies swap the two types' colors (a Fairy/Fire is pink+red, its shiny red+pink).
- Shimmer: when a shiny enters battle, at the start of its turn, and every ~10 s while idle, its recolored hues rotate 360° over 1 s, an iridescent ripple. This is the main shiny tell, since single-type shinies have no swap. It is a per-frame palette update of a few colors, like the existing palette fades.

Cost: GBA sprites use 16-color palettes, so recoloring touches 16 colors per Pokémon, not pixels. Steps 1-2 depend only on the original sprite, so a build tool precomputes the picked palette indices per species; only step 3 runs at runtime, when the palette loads.

## Type icons in names

Players strongly associate Pokémon with their types, so random types are a mental hurdle. To ease it, whenever a Pokémon's name is displayed it is prefixed with icons for its types, e.g. "You encountered a wild 🐉❄️Rattata!".

## Battle damage estimates

Random types and moves make matchups hard to remember, so battle menus show a damage grade.

- **Where:** the move selection menu, and the in-battle Pokémon > summary > moves page, show a grade per move. Status moves show no grade.
- **Multiple opponents:** a move's grade is the highest over all opponents; the target selection screen shows each target's own grade.
- **Level-agnostic:** grades compare stats and multipliers, not levels or HP, so a grade means the same thing anywhere in the game.
- **What the player knows decides the estimate's accuracy,** keyed on the opponent species' Pokédex state:
  - Not seen: defense base stats are assumed to be 100 (average; species base stats are mostly 50-150), and type effectiveness is skipped (treated as 1x).
  - Seen, not owned: real defense base stats, rounded to the nearest 25 (about a quarter of the 100 average, deliberately coarse); type effectiveness applied.
  - Owned: real defense base stats.
  - Temporary battle stat stages (both sides) are applied.
  - Hidden information (ability, held item, crits, the random roll) is never used.
- **Grade scale:** a letter from Z to A, with a red-to-green color gradient.
  - Z is reserved for a damaging move that the type matchup makes deal nothing (e.g. Normal vs Ghost).
  - Y is the worst possible case: move power 10, own attacking base stat 20 with 0 IVs and EVs, opposing defense base stat 170, no STAB, and 4x resisted.
  - A is the best possible case: the opposite of every Y assumption (attacking base stat 170 with max IVs and EVs, defense base stat 20, STAB, 4x super effective, move power 150).
  - The 20-170 range covers non-Mega, non-joke species; values outside it clamp to Y or A.
  - Power 150 sits just above the strongest downside-free move (140). Power 10 sits well below the weakest common single-hit moves (40, e.g. Tackle) so those don't grade as terrible early on; multi-hit moves grade by their listed per-hit power, which is no more misleading than the power the game shows.
  - X-B are evenly spaced between Y and A on a log scale, since the factors multiply, so each letter is the same damage multiplier apart.
  - Being level-agnostic, the Y-A thresholds are compile-time constants: a 24-entry threshold table searched in at most 5 comparisons.
- **Switch hint:** in the in-battle Pokémon menu, each party member gets a grade too: its best move's estimated damage times that move's accuracy squared ("if it isn't 100%, it's 0%": 50% accuracy weighs 0.25x), scaled by its defensive type matchup against the opponent's types, against whichever opponent it hits best (the same Pokédex rules apply to what the player knows about those types).
- **Implementation:** reuse the AI's damage calculation (`src/battle/ai/battle_ai_util.c`) where it fits, with the estimated defenses substituted, rather than a new formula.

## Team analyzer

A PC menu option that suggests one party/box swap, using the battle damage grades.

1. Hypothetical opponents: for each of the 18 types, one Pokémon with physical/special defense base stats 130/70, and one with 70/130 (36 total). Milder than the grade extremes, to resemble real species.
2. Each party Pokémon is graded against each opponent with the switch-hint grade (best move, accuracy squared, defensive matchup).
3. Top grade is decided by the raw damage estimate, not the letter, so ties are rare (if the raw estimate is hard to expose, tied Pokémon all count as top). Team score of a Pokémon = (opponents where it has the top grade) + 1 - avg(second place / its grade, over those opponents). It rewards covering many opponents, and covering them by a wide margin.
4. With a full party, the lowest team score is the "replaceable" Pokémon.
5. Each box Pokémon is scored as if it took the replaceable one's place; the best is the "replacement".
6. If the replacement scores strictly higher than the replaceable Pokémon, the analyzer suggests the swap; ties keep the player's existing choice.
7. With fewer than 6 party Pokémon, it instead suggests at least 1 box Pokémon to add (ideally the top 2-4, letting the player pick one).
8. After the player accepts a swap or addition, it offers to reanalyze or quit, so repeated runs iterate the team toward better coverage.

Known and accepted bias: scoring favors types whose same-type attacks are super effective against many types. Steel/Flying (e.g. Corviknight) gets STAB super effectiveness against 6 types, pure Normal against none, so the analyzer will often pick the former and rarely pure Normal types.

The player chooses to run it and waits, so it may take several seconds, behind a progress bar; run it in chunks across frames.

Future extension: a hardcoded list of 4-8 "best moves" per species (covering types and physical/special), scored the same way, to suggest move replacements. Combining it with the box scan (which box Pokémon would be best with moves they can learn later) is a combinatorial explosion (70 four-move sets per Pokémon from 8 candidates), so it would need a greedy or hill-climbing approximation; not designed yet.

## Gym guide advice

The existing tips man at each gym entrance also gives party-specific advice from the gym's type:
- Which party Pokémon have attacking moves strong against the gym type.
- Which party Pokémon have types weak defensively against the gym type.
- Species found in the current county, at an equal or lower route-story-index, with a type advantage over the gym type.

## Alternatives considered

### Two biomes per species

Associate each species with two biomes: only the first determines types, but the species can spawn in either.

Pros:
- Players could encounter ~470 of the 541 lines, so far fewer species are locked out of a playthrough.

Cons:
- 470 exceeds the 420 box slots.
- Needs a second mapping and more bookkeeping for the species-to-biome lookup.

Not planned; may be revisited depending on player feedback.

### 4x4 biome subsets with a mod-25 mapping

Each region uses 16 biomes (4 humidity x 4 temperature levels), and species map over all 25 biomes, so lines landing outside the subset are not wild.

Pros:
- Fewer wild lines (~346), closer to the box capacity.
- More biome variety within one region.

Cons:
- 4 temperature levels made a region's hot and cold ends too far apart (Hoenn fits in 3).
- Locks ~195 lines out of each run's wild encounters.

### Damage grades in the overworld party menu inside gyms

Showing the switch-hint grades in the field party menu while inside a gym, graded against the gym type.
- Pros: helps plan a gym's battles before they start.
- Cons: a rare case for extra UI; the gym guide's advice covers the same need.

### Rival team replayed over every earlier route at battle time

Store only the starter; at a rival battle, replay the swap rules over every route below the battle's route-story-index, in story order.
- Pros: nothing stored; the team is a pure function of the seed, so it is reproducible and independent of the player's route order.
- Cons: needs every earlier county and route generated in memory during the fight, which RAM does not allow (design.md, "Resource budget").

### Bitpacked rival team

Pack the six species ids at 11 bits each (about 1,570 species; the starter slot fewer) into about 60 bits.
- Pros: 4 vars instead of 6.
- Cons: the saving is two vars; the packing and unpacking code adds complexity for little gain.

### Species-to-family bi-mapping

A table mapping each species to its family and back (Pichu, Pikachu and Raichu are one family), to store families instead of species.
- Pros: none that matter here; a team still needs a species per slot.
- Cons: about 541 families do not fit in 9 bits, so the table costs ROM and gains nothing.
