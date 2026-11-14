# City planning and NPC hint design

How a county lays out a city and fills it with hinter and generic NPCs, and how hint texts, main-beat hints, route blockers and Pokémon Center news are chosen.

## City planning

A city is laid out by its county (layer 5/6), in this order. Steps 5-8 are what the subplot descriptor's hint and actor fields serve. The hard part will be building and path placement (steps 2-3), not the story hooks.

1. Districts (V2 feature; V1 always uses one district, and locks sit only at doors and route ends): usually one district covers the whole city. The only reason for the split is to let a subplot put a lock mid-city. A subplot may ask for two, separated by obstacles and one narrow choke point closed by a lock (usually an NPC, or an HM or item). The planner must know which routes the player can enter yet (from the story indices), and every route entrance must be in the first district. The second district either touches no route, or guards routes the player cannot reach yet: the inner-county routes when the city is entered from an inter-county route, or the way on to the next county when leaving.
2. Paths connect the routes. Large cities get rectangular city blocks, possibly several tiles wide; small towns get meandering paths one tile wide.
3. Buildings (including city special locations) are placed against paths without covering one, and the path must reach their door. Buildings may shift paths slightly if connectivity is kept, unless that costs too much time. Placing a building also decides how many generic NPCs it holds.
4. Decoration fills the space between buildings: paving, dirt, grass, flowers, rocks. Path surface follows settlement type: paved in cities, dirt in towns, grass in villages (to be checked against what Emerald and FRLG do).
5. Main-plot hinters: for each main-story beat hosted in the city, place one hinter NPC for the next main-story beat (looked up through the planner), so a player who finishes a county's subplots always finds at least one NPC there who says where to go next, even when that is the same city.
6. Subplot hinters: a county has a few subplots, each a list of beats like the main story, and the player may interleave them in any order. A subplot asks the county for hinter NPCs, each bound to one beat index (`numHinters` per stage) and each knowing whether that beat is in the NPC's own city (close) or not (far). A subplot outside the city may want none, and one inside may want hinters for a few of its beats.
7. More generic NPCs fill out the population.
8. Each subplot hosted by this city may place actor NPCs, which the owning subplot controls entirely.

Generic NPCs get context about where they are and say a random matching line ("This is my brother!", "The flowers in this city smell wonderful!").

Hint texts: a subplot lists hint ranges (`struct SubplotHint`), each a first and last stage and two text lists by distance (far: the NPC is in another city or county; close: the NPC is at the place). A range's lists are what NPCs say while the subplot's stage is in the range; ends may be the "before" (not started) and "after" (completed) pseudo-stages. Ranges must not overlap and must cover every real stage, with both lists non-empty there (a test checks the ported subplots). An NPC picks `list[selector % count]`, so it always says the same line while the range holds, and one range holds both the lines about the present ("What are those guys doing there?") and the ones recalling or foreshadowing a neighbouring stage ("You saved us!"). A pseudo-stage with no range, or an empty list, means generic text.

Beat placement: a beat's trigger is almost always an immobile NPC, an immobile item, or a chokepoint (route-to-route transition, route-to-location transition, or a door). The descriptor therefore records a specific anchor per stage (kind plus the route, location or building role involved), not just a role, and hint texts can say where it is: "near route x", "near the <building type>", or the city. Close or far then follows from whether the hinter stands in the anchor's city (`struct BeatAnchor`, `GetBeatDistance` and `WriteBeatAnchorReference` in `subplot.h`; every anchor of a subplot belongs to its host county, so the host county's city is the anchor's city, and a route anchor is "near the road to X").

Planning (`PlanCountyHinters`, `city_hinters.h`) is derived on demand and stores nothing. Subplot hinters stand in the host city, so they are always close; far records exist only for main-plot hinters, one per county-hosted main beat, bound to the next beat that has an item or a blocker unlock. Hinters use the placeholder NPC budget of "Building roles and NPC spawning" and fall back to any building with room. The main-beat hint writer and the subplot hint writers line-break their output to the field message box (`BreakStringForFieldMessage`: 208 px, two lines, `\l` scroll beyond that), since the main-beat phrases have no breaks and a subplot template may have none; text that already has manual breaks is left alone. Hint texts embed `{HINT_ANCHOR}` (charmap `FD 0F`), which `ExpandHintTemplate` replaces with the anchor reference, `{HINT_CITY}` (`FD 10`), replaced with the host city's name so a moved subplot never names a fixed city, and `{HINT_PLACE}` (`FD 11`), replaced with the anchor's building role or location kind name ("the {HINT_PLACE} in {HINT_CITY}"; "spot" for a route or city anchor), before placeholders expand. A direction placeholder ("north of X") waits for a consumer: only far hinters need it and none exist for subplots, and the expander would need the hinter's county; `ShowCurrentSubplotHint` puts the result in `gStringVar4`. An NPC script binds itself with the `subplot_hinter slot, beat, selector` macro (`data/scripts/subplot_hinter.inc`; values from its `HinterPlacement`), which shows the hint and leaves VAR_RESULT TRUE, else the script falls through to its generic text. Emerald's static maps bind by hand (Lilycove: Man3 and RichBoy, pinned by a test); generated maps will bind from the plan at map load.

Main-beat hints are generated, not authored: `WriteMainBeatHint` (`city_hinters.h`) opens with "I heard" (far) or "Word around town is" (close), then a phrase picked by the plot kind of the beat's first item (a step-3 beat is a blocker), the host city's anchor reference, and a tail. A gym item adds a sentence naming its leader, its type and the HM it gives (the Elite Four names the Champion; an HM displaced from its gym's slot names just the HM); other kinds add nothing. The `ShowMainBeatHint` special (VAR_0x8004 = the hinter's own index, 0x8005 = its target) speaks only while the main-story-index is past the hinter's own beat and not past the target; else VAR_RESULT is FALSE and the NPC says generic text.

Route blockers: `data/scripts/route_blocker.inc` holds one generic script per push-back direction; it uses `IsMainStoryIndexPast` (VAR_0x8004 = s) and lets the player through once the main-story-index is past s.

A hinter bound to beat b says a line from the range covering the subplot's current stage; b only picks the stage whose anchor and count the text names. A hint asked about a pseudo-stage with no bound beat uses the nearest real stage's anchor and count (`ResolveHintStage`: before = stage 0, after = the last stage), and actor ranges may end at a pseudo-stage. The query takes the distance from the placement, never from the stage.

Reward and keys: a subplot's `givesReward` bit says whether finishing it gives a reward: the McGuffin in `producedKey` (the next subplot's or lock's key) or, when none is needed, a rare item chosen at generation; clear means story advancement only (a test requires the bit when `producedKey` is set). Its intro McGuffins are its `requiredKeys`: every consumed key must be produced before the subplot can start, which is how a chain entry waits for its predecessor.

News: each Pokémon Center has a newspaper, TV or reading NPC giving "the news" (`WritePokemonCenterNews`, special `ShowPokemonCenterNews`, `city_hinters.h`). It is about the first list at or after the main-story-index that has a host (an empty list is passed at once). If the player is not in the host county, it says that beat's far main-beat hint. In the host county it names the blocking subplot: the first blocking get still undone in the list (`GetFirstUndonePlotSlot`), matched to a host-county subplot that declares it produces that planner item (`SubplotDesc.producesMainGet` and `mainGet`, tested by `DoesSubplotProduce`), and says that subplot's current close hint, so the next step is easy to find without nagging. With no producing subplot (a gym, a rival beat, a blocker: the blocker is a location rather than a subplot) it says the close main-beat hint instead. A step-3 unlock that is no blocker (which `GetMainStoryIndex` can rest on, such as the Letter) has no beat but still has block x's county: away from it the news says "There's a story about something unfolding near <place>", and in it a vague "right here" line until the county is given details to hint at. The place is the county's city, else its first landmark or detour (or Enclave, for an Enclave-only county), and a beat hosted in a cityless county gets the same vague news. In the postgame, or when the county has none of those (only Emerald's empty county), it says one generic line from a small pool of copied Emerald TV lines, picked by county and player seed. Beats get no separate news strings: the far main-beat text is generated, and a subplot's current entries are required by the descriptor, so the news cannot lack one.

Where an entry is missing or an empty list, the NPC falls back to generic NPC text. An NPC says the same thing every time while the state holds. The far lists let NPCs in different cities give different hints.

## NPC dialogue

- Each named building has a short list of NPC talking points.
- Each gym type has a short list of NPC talking points, parameterized by city name and gym leader name.

## Alternatives considered

### A separate required news string per main beat

Each beat would carry its own authored news line for the Pokémon Center, instead of reusing the hint entries.

Pros:
- The news could be worded as news rather than as an NPC's hearsay.

Cons:
- Unneeded: the far main-beat text is generated, so it exists for every beat, and every stage of a subplot already has non-empty hint lists.
- One more string per beat and per subplot stage, to author and keep consistent with the hints.
