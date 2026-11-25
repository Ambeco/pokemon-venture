# Actor placement and scene design

How a stage places actors and runs cutscenes: the object budget, anchors and triggers, the step vocabulary and a worked example.

### Actor placement

City planning asks, for the actors and triggers to place in the city: the chain's current stage, each optional chapter's current stage, the "after" pseudo-stage of every completed chain chapter, and the "before" pseudo-stage of every not-started one. Cutscene actors are spawned and removed at runtime and count against the same budget.

Object budget: the engine holds 16 object events live and 64 object templates per map, and the player and any follower use some. So the limit is 16 - (player and follower overhead) - actors within any screen-sized area, and 64 - (player and follower overhead) - actors over the whole map. Generic NPCs fill in after actors, spread apart from each other and from actors to stay under both limits. The exact spawn margin around the screen is to be checked in the engine.

A stage that has passed places nothing of its predecessor (Route 101: the professor, the Zigzagoon and the bag exist only in the tutorial's stage).

An actor is placed relative to its trigger or anchor (left of a door, on the door, on a chair, at every exit), so mapgen can check for a walkable route when it places the actor, which guarantees a route from the actor to the player at the trigger. A placement may also need a feature of the building: "chair", "in front of the TV", "center-stage" (the spot for a gym leader, boss or professor in a specific map) or "anywhere". Features are rare, and a host building without the feature fails placement. An actor is either a prop (a graphic with no interaction, such as the moving truck) or an NPC. Chapters mark short-lived actors "transient"; a test requires every non-transient actor to be placed in every scene, so the mom or professor cannot vanish by accident (a scene that must omit one places it in a debug map).

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

Triggers belong to the anchor, so a stage cannot ask for a trigger at a place it has no anchor for. A trigger runs a cutscene and advances the stage when it ends, which keeps no extra state. Defeating an NPC is a trigger only for immobile story actors, which confines it to anchor locations. An anchor may also have a proximity radius (about 4 tiles): coming within it starts a cutscene in which an actor approaches from behind, or appears at the anchor (a door) and approaches the player. A check run whenever the player walks or teleports, not a ROM `coord_event` (a generated map has none), fires them (see "Scene runner"). The trigger list so far (incomplete):

- Enter a map or route at this stage; stand in front of a building door; leave a building; reach a route end or finish a route; come within N tiles of an anchor; talk to an anchor NPC; defeat an immobile story actor; interact with an anchor item; cross a chokepoint (route to location, blocker); chain from the previous scene's end; use a field move or key item at an anchor (Silph Scope, Cut); a battle outcome (win, catch a given Pokémon, flee); the main-story-index crossing a value, seen at the next map load (level-triggered); time of day or weather (not needed yet).
- An actor enters by approaching from behind, emerging from a door or approaching from the anchor; it leaves by running back or forward, walking to the anchor and vanishing, going through a door, or teleporting.

### Scene runner

A scene is a `struct Scene` (`include/story/scene.h`): a list of actor local ids (slot 0 is always the player) and a step list. A fixed driver script (`data/scripts/scene.inc`) loops on `special SceneNextStep`; the special decodes one step, acts on the live objects and returns what the driver must do next: nothing, wait for movement, or show the text in `gStringVar4`. Running one step at a time matters because a `walk` target is relative to where the previous step left everyone, so paths are computed when the step runs, never ahead of time. Messages, locking and waiting reuse the script commands (`msgbox`, `lockall`, `waitmovementall`).

- A `walk` finds a path with a breadth-first search over a window of tiles around the actor (the screen plus a margin), asking `GetCollisionAtCoords` whether each step is open (so other objects, elevation and impassable metatiles all count), and queues the steps as a movement script in an EWRAM buffer, one per actor slot, which `ScriptMovement` reads until the walk ends. A `parallel_walk` fills every actor's buffer and then waits for all. A walk's goal tile is fixed when its step starts; a goal that is blocked or unreachable fails loudly (naming the scene, step and actor) rather than skipping the step.
- A target is an actor slot plus a side: the actor's own tile, or the tile in front of, behind, left or right of it (relative to its facing). The resolver is a pure function of positions and facing, so it is tested without a map.
- Steps implemented: `lock_player`, `unlock_player`, `face`, `walk` (a walk may `slide`, keeping its facing), `parallel_walk`, `say`, `hide`, `show`, `jump` (in place), `hop` (a one-tile jump with the ledge sound), `door_open`, `door_close`, `place` (spawns the actor first if it is not on the map, ignoring its hide flag), `set_flag`, `clear_flag`, `warp` (silent or not), `pause` (in 8-frame units), `chase_loop`, `music` (a song, or none to restore the map's), `advance`, `ask_yes_no`, `ask_choice`, `give`, `battle` and `custom`.
- Branching: a scene runs from a stack of step lists (at most `SCENE_MAX_DEPTH`, 4). `ask_yes_no` and `ask_choice` show the question, read the answer with `SceneTakeAnswer` (the next step would overwrite `VAR_RESULT`), push the chosen branch's list and carry on after it when the branch ends, so branches may ask again. `ask_choice` uses the dynamic multichoice stack and ignores B. `give` calls `Std_ObtainItem` (the driver reloads the item with `SceneLoadGive` first, since `switch` clobbers VAR_0x8000); when the bag is full the scene ends there (after the optional bag-full branch), never advancing, so an item cannot be lost to a later `advance`. `battle` starts a no-intro trainer battle (its defeat text in the step) or a wild battle and waits; the scene continues after a win. `custom` calls an event script as a subroutine; its script must be a `SceneCustom_` label in `data/scripts/scene_custom.inc` and may not move or place objects or name a local id, so scenes stay layout-independent (`tools/misc/check_scene_custom.py`, run first by `make check`, fails naming the line). A scene outside the scene table runs through `SceneBeginScene` and `Scene_EventScript_RunPrepared` (tests).
- A scene starts either from a script (`setvar VAR_0x8004, SCENE_ID` and `call Scene_EventScript_Run`) or from a trigger in `sSceneTriggers` (`src/data/scenes.h`): a map, a stage var value, a door (a warp index) and a kind (`SCENE_TRIGGER_PROXIMITY` within a Manhattan radius of the door, `ON_DOOR`, `IN_FRONT_OF_DOOR`, `ENTER_MAP`: no door, so no anchor, and it fires only on the check after a map load, or `EXIT`: also no anchor, firing `radius` tiles inside a named map edge where the edge tile straight ahead is walkable, so it sits at the gap, not along the whole edge). `SceneTryStartTrigger` runs from `ProcessPlayerFieldInput` right after the on-frame map script: when the player took a step, and once after each map load (so teleporting counts). It is not a coord event, so generated maps need none. A scene that does not `advance` fires again on the next step while its trigger holds. `advance` sets the scene's stage var.
- The door that fired a trigger is the scene's anchor, a pseudo-actor (`SCENE_ACTOR_ANCHOR`) usable in any target: `SIDE_SELF` is the door tile, `SIDE_FRONT` the tile below it (doors face south until generated maps supply a facing). `door_open`, `door_close` and `warp` act on it, and `warp` goes to that door's `warp_def` destination, so one scene serves both genders' houses. Walks may step onto the anchor door tile though the map grid blocks it. A scene started from a script has no anchor, and using one fails loudly.
- Route 103's rival challenge is the first scene with `battle` and `ask_choice`: after the rival faces the player, the scene asks "Let's battle!" or "Not yet" (the player may heal first) and the first branch fights `SCENE_BATTLE_RIVAL(encounter)`, whose trainer `GetRivalTrainer` (`src/story/rival.c`, also a script special via `ScriptHasRivalBeenDefeated`) picks from the player's starter and the rival's gender. Only the rival's texts differ, so there are two scenes (May, Brendan). The rival's exit stays in `Route103_EventScript_Rival`, which reads the trainer flag after the scene to tell a win from "Not yet". It is covered by `test/e2e/tests/route103_rival.json`.
- Route 110's rival and Route 119's rival are scenes started from their coord-event scripts after the hand-written approach (`SCENE_ROUTE110_RIVAL_*`, `SCENE_ROUTE110_ITEMFINDER_*`, `SCENE_ROUTE119_RIVAL_*`, `SCENE_ROUTE119_FLY_*`; two scenes per gender since the texts differ). The fight and the gift are separate scenes (Route 110) or a retry scene (Route 119), so a full bag leaves only the gift undone. The approach, the bike exit and Scott's arrival stay in the map scripts (no step spawns an off-screen actor yet); covered by `test/e2e/tests/route110_rival.json` and `route119_rival.json`.
- Littleroot's twin is converted to two `EXIT` triggers at the north gap (depth 1): the guard (stage `SAVE_BIRCH_TWIN_GUARDS`, `SCENE_LITTLEROOT_NEED_POKEMON`) and the ask (`SAVE_BIRCH_TWIN_ASKS`, `SCENE_LITTLEROOT_TWIN_ASKS`, which `advance`s). `face_original` returns an actor to its map-load facing; `SceneCustom_SetBigGuyGirlString` fills `{STR_VAR_1}` for the ask's text.
- Littleroot's arrival (stage 0 of the intro) is converted: the truck drop-off fires it by proximity, `SCENE_LITTLEROOT_ARRIVE_MALE`/`_FEMALE` differ only in the truck's hide flag. It differs from the original script in that the mom and player take turns walking onto the door tile and there are no fixed pauses.
- The Route 101 rescue (Birch chased by a Zigzagoon) cannot use a hardcoded path on a generated route. Its `chase_loop` step takes a leader, a chaser, a target to stay away from, a lap count and a lag, finds a closed ring of 8-10 walkable tiles in the on-screen window (grass preferred; vanilla's circle is a 10-tile ring), and runs both actors around it, the chaser a few tiles behind. Route generation guarantees the scene can run, like any actor placement: grass is visible from the route entrance and an open ring of that size lies near it. The step only picks among rings that satisfy this, and fails loudly naming the scene and step if none exists. The ring finder (`SceneFindRing`: 3x3, 4x3 and 3x4 rings, scored by grass, kept away from the target) is tested on fake grids; `SCENE_ROUTE101_RESCUE` uses it on vanilla Route 101 (an `ENTER_MAP` trigger at Save Birch's `SAVE_BIRCH_BIRCH_IN_GRASS` stage, Birch and the Zigzagoon placed at their template tiles), and an in-ROM test runs it.
- Rescue layout: the player enters near the bag with grass off to the left. Birch and the Zigzagoon appear in the grass, run about halfway toward the bag and circle there; the player then goes to the bag, picks a starter and has the first battle. Generation therefore keeps the ring clear of the bag and off the shortest path from the entrance to the bag, so reaching the bag never means entering the grass or the chase. No wild encounter or spawned wild Pokémon may happen during the stage (the player has no Pokémon, and a spawn could wander onto the bag or the ring): the stage sets a flag that turns encounters off and the bag script clears it. `DisableWildEncounters` is not enough: overworld spawns (`wild_encounter_ow.c`) ignore it and obey the config flags `WE_FLAG_NO_ENCOUNTER` and `WE_OWE_FLAG_DISABLED` (both are `FLAG_SCENE_NO_WILD` now; the rescue scene sets it and Birch's bag script clears it); setting either despawns spawns already out.
- Needs, not global rules: grass half a screen from the route end and the open ring are requirements the rescue stage declares (like a building feature), checked when the generator assigns the scene to a route. A route that cannot satisfy them cannot host it, so ordinary routes (desert, water, cave) stay unconstrained.
- Actors on Emerald's hand-made maps are bound by local id in the scene's actor list, as hinters are bound by hand; spawning transient actors is a later change.

### Door and off-screen approach rules

- A door has two trigger positions, "on the door" and "in front of the door", so a scene can let the player follow an NPC through: the NPC leaves from the door tile while the player waits in front. A door on the top or a side edge of a map has no "on door" tile, so a scene that needs one may not use such a door (checked when the scene is placed, since mapgen knows each door's position).
- An actor that approaches from off-screen, from behind or in front of the player, is placed by a search outward from its end tile: a breadth-first search over walkable tiles finds the nearest tile beyond the screen edge on the required side, and the actor is placed there and walks in. If the search needs more than about 10 steps, it restarts and lets the path cross other NPCs and objects (HM locks, other NPCs). If that also fails, the actor teleports next to the player with a teleport animation.

### Interleaving subplots

- Order comes only from a subplot: a chapter waits for the entry before it in its subplot (that entry at its last stage) and for the main story to reach its county, and never for a chapter of another subplot. Chapters that are independent errands (Mauville's Rock Smash, Coin Case and Rydel's Cycles) are separate subplots, so the player may do them in any order, or leave them for later.
- A key is still the way to wait on something else: the Coin Case waits for Slateport's Harbor Mail through a produced key, not through order, so it also works across counties.
- The county chain holds only the required entries of the county's subplots, merged by main-story slot and, within a slot, in subplot declaration order. Optional entries run beside the chain on their own chapter-slot var, so an unfinished optional chapter never holds a main-story slot. A chapter keeps its chapter slot whether or not it is on the chain, so no stored state moves.
- Scenes of different subplots may be triggerable at the same moment (two chapters anchored to one building). The scene runner runs at most one scene per trigger; when two qualify, the chain entry wins over an off-chain one, then the lower chapter slot, and the other fires on the next trigger.

### Open questions

- A generic mechanism for triggering chapter stage-advancement cutscenes has no plan yet: scenes today fire from the hand-written trigger table.

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

### Runtime-spawned NPCs for every chapter beat

The generic cutscene runner creates every chapter NPC and event at runtime, instead of placeholder object events in each building.

Pros:
- A chapter is independent of the host city's buildings.

Cons:
- Chapters tied to a building (a gym, a shop, a house) lose their interiors.
- Large rewrite of every chapter up front.
