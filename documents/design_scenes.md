# Actor placement and scene design

How a stage places actors and runs cutscenes: the object budget, anchors and triggers, the step vocabulary and a worked example.

### Actor placement

City planning asks, for the actors and triggers to place in the city: the chain's current stage, each optional subplot's current stage, the "after" pseudo-stage of every completed chain subplot, and the "before" pseudo-stage of every not-started one. Cutscene actors are spawned and removed at runtime and count against the same budget.

Object budget: the engine holds 16 object events live and 64 object templates per map, and the player and any follower use some. So the limit is 16 - (player and follower overhead) - actors within any screen-sized area, and 64 - (player and follower overhead) - actors over the whole map. Generic NPCs fill in after actors, spread apart from each other and from actors to stay under both limits. The exact spawn margin around the screen is to be checked in the engine.

A stage that has passed places nothing of its predecessor (Route 101: the professor, the Zigzagoon and the bag exist only in the tutorial's stage).

An actor is placed relative to its trigger or anchor (left of a door, on the door, on a chair, at every exit), so mapgen can check for a walkable route when it places the actor, which guarantees a route from the actor to the player at the trigger. A placement may also need a feature of the building: "chair", "in front of the TV", "center-stage" (the spot for a gym leader, boss or professor in a specific map) or "anywhere". Features are rare, and a host building without the feature fails placement. An actor is either a prop (a graphic with no interaction, such as the moving truck) or an NPC. Subplots mark short-lived actors "transient"; a test requires every non-transient actor to be placed in every scene, so the mom or professor cannot vanish by accident (a scene that must omit one places it in a debug map).

### Scenes

A stage lists, per map (by building role, or the host town map), its actors and its scenes. A scene is a trigger plus a step list. A map may have several scenes per stage, and their names only organize the data. A scene does not advance the stage unless it ends in `advance`, so a scene such as a blocking child can repeat any number of times.

Triggers: within N tiles of an anchor, talk, enter a door, enter the map, cross a border that is an exit, and interact with an item. Borders split into entrances and exits: an exit leads to a route the player cannot navigate yet, an entrance to one they can, decided from the route locks and the main-story-index when the map loads. Triggers fire on walking or teleporting.

Steps run in order by default; `parallel_walk{...}` runs several walks at once (the player's included), replacing an explicit wait. Walking is called `walk`, never `move`, which is ambiguous with teleporting. Pathfinding is limited to what is on screen, which is cheap because actors are placed with a checked route. The vocabulary, checked against the Route 101 tutorial, the Wally tutorial and walk to his house (Petalburg), and the Old Rod fisherman (Dewford):

- Movement and staging: `walk`, `parallel_walk`, `face`, `hide`/`show` an actor, `lock_player`/`unlock_player`, `door_open`/`door_close` on the anchor door, `warp` to a map by role and door.
- Dialogue: `say`, `ask_yes_no{yes, no}`, `ask_choice{...}` with a step list per answer.
- Effects: `give` (the stage's reward or an item, with the bag-full case handled), `battle`, `music` set and restore, `advance`.
- Escape hatch: `custom` runs a hand-written special for what no step covers (the tutorial battle swaps in a borrowed party and restores it).

Targets are relative: the player, an actor, an anchor, or a side or the front of one.

Worked example, the Route 101 tutorial (stages 0 and 5):

- Stage 0, host town map. Actors: a prop truck left of the mom's-house door; the mom at that door. Scene "arrive": trigger within 5 tiles of the door; steps: the mom walks to the player and says her line, the mom walks into the door, the player walks in front of the door, the mom is hidden, the player walks in, warp to the mom's house front door, `advance`.
- Stage 5, the mom's-house downstairs. Actor: the mom on a chair (needs a chair). Scene: talk, `say`.
- Stage 5, host town map. Actors: a little girl at every exit. Scene "need a Pokémon": trigger cross an exit; steps: `lock_player`, the girl walks in front of the player, `say`, the girl and player each take a step (`parallel_walk`), `unlock_player`. It never advances.
- Stage 5, the rival's house. Actor: the rival's mother on a chair. Scene: trigger enter the door; steps: she walks to the player's side, `say`, `advance` to stage 6.

The Wally tutorial adds a `parallel_walk` of the player and Wally, a scripted battle, a warp to a map at a fixed door and a music restore. The Old Rod fisherman adds a stage-dependent talk (one scene per stage), `ask_yes_no`, `give` with bag-full handling and `ask_choice` for flavor answers.

Triggers belong to the anchor, so a stage cannot ask for a trigger at a place it has no anchor for. A trigger runs a cutscene and advances the stage when it ends, which keeps no extra state. Defeating an NPC is a trigger only for immobile story actors, which confines it to anchor locations. An anchor may also have a proximity radius (about 4 tiles): coming within it starts a cutscene in which an actor approaches from behind, or appears at the anchor (a door) and approaches the player. A check run whenever the player walks or teleports, not a ROM `coord_event` (a generated map has none), fires them. The trigger list so far (incomplete):

- Enter a map or route at this stage; stand in front of a building door; leave a building; reach a route end or finish a route; come within N tiles of an anchor; talk to an anchor NPC; defeat an immobile story actor; interact with an anchor item; cross a chokepoint (route to location, blocker); chain from the previous scene's end; use a field move or key item at an anchor (Silph Scope, Cut); a battle outcome (win, catch a given Pokémon, flee); the main-story-index crossing a value, seen at the next map load (level-triggered); time of day or weather (not needed yet).
- An actor enters by approaching from behind, emerging from a door or approaching from the anchor; it leaves by running back or forward, walking to the anchor and vanishing, going through a door, or teleporting.

### Scene runner

A scene is a `struct Scene` (`include/field/scene.h`): a list of actor local ids (slot 0 is always the player) and a step list. A fixed driver script (`data/scripts/scene.inc`) loops on `special SceneNextStep`; the special decodes one step, acts on the live objects and returns what the driver must do next: nothing, wait for movement, or show the text in `gStringVar4`. Running one step at a time matters because a `walk` target is relative to where the previous step left everyone, so paths are computed when the step runs, never ahead of time. Messages, locking and waiting reuse the script commands (`msgbox`, `lockall`, `waitmovementall`).

- A `walk` finds a path with a breadth-first search over a window of tiles around the actor (the screen plus a margin), asking `GetCollisionAtCoords` whether each step is open (so other objects, elevation and impassable metatiles all count), and queues the steps as a movement script in an EWRAM buffer, one per actor slot, which `ScriptMovement` reads until the walk ends. A `parallel_walk` fills every actor's buffer and then waits for all. A walk's goal tile is fixed when its step starts; a goal that is blocked or unreachable fails loudly (naming the scene, step and actor) rather than skipping the step.
- A target is an actor slot plus a side: the actor's own tile, or the tile in front of, behind, left or right of it (relative to its facing). The resolver is a pure function of positions and facing, so it is tested without a map.
- Steps implemented: `lock_player`, `unlock_player`, `face`, `walk` (a walk may be `backward`, keeping its facing), `parallel_walk`, `say`, `hide`, `show`, `jump` (in place), `advance`. Every other step kind in the vocabulary fails loudly on use, naming the kind.
- A scene is started by a script (`setvar VAR_0x8004, SCENE_ID` and `call Scene_EventScript_Run`) for now; the per-step trigger check and the entrance/exit classification come later. `advance` sets the scene's stage var.
- Two things the Route 101 rescue does are not in the vocabulary: placing an actor at a tile (`setobjectxy`) and a fixed choreography (the Zigzagoon circling). Both go through `custom` until a `place` step and a literal `path` step are needed by a second scene.
- Actors on Emerald's hand-made maps are bound by local id in the scene's actor list, as hinters are bound by hand; spawning transient actors is a later change.

## Alternatives considered

### Compile a whole scene to one movement/script bytecode buffer

The runner turns the step list into script bytecode up front and runs it once.

Pros:
- No per-step driver loop; the script engine does all waiting.

Cons:
- Relative targets depend on where earlier steps leave the actors, so compiling needs a full simulation of every object's position.
- Every step then needs a bytecode form, including paths found around objects that have moved by the time they run.

### Hand-written bytecode script per stage

Each stage points at a hand-written event script in the host map's `scripts.inc`.

Pros:
- Easiest to author, and covers any oddity.

Cons:
- Movement lists and local object ids are map-specific, so they break on a generated layout.
- Only testable by running the script in the ROM.
- Converting Kanto's scripts means converting each by hand.

### Runtime-spawned NPCs for every subplot beat

The generic cutscene runner creates every subplot NPC and event at runtime, instead of placeholder object events in each building.

Pros:
- A subplot is independent of the host city's buildings.

Cons:
- Subplots tied to a building (a gym, a shop, a house) lose their interiors.
- Large rewrite of every subplot up front.
