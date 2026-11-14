# Pokémon Venture — Design

Current state of the design for the mod. Organized by topic; describes what the design *is*, not how it evolved (history lives in `notes.md`, open items in `remaining_work.md`).

## Overview

The mod procedurally generates a Pokémon region rather than using a fixed map. (The user describes the idea as "overly ambitious"; the full concept is still being collected.)

The goal of the region generation below is a game region map with cities, routes, and special locations that mirrors what a normal mainline Pokémon game looks like.

## Layered generation

Generation is a stack of layers, each computed only from the layers above it, never from its own neighbors or from later layers:

1. Region map: terrain types and biomes per county, from the PRNG seeded from the region seed (today the identity seed itself; see "Regions and multiple regions" and the seeds paragraph in "Data representation and determinism", both in design_data.md).
2. Region map: county-story-indices, from layer 1 plus a couple of McGuffin boundaries.
3. Region map: county populations, city locations and names, and special location IDs and positions, from layers 1-2.
4. Region map: which counties connect to which, from layers 1-3 (using only county-story-index adjacency and the capital, never planner data).

    The main plot planner (see "Main plot planner" (design_story.md)) needs no routes: its items come from layers 1-3, and layer 4's route rules already connect each county to the next county-story-index. Layer 3 runs in this order: (a) the settlement quota: counties 0 and 1 and the Elite Four county always have a settlement, plus a random 8 to 13 more; (b) the gym hosts (the Surf gym's host is the Surf-lock county or its callback county), each picked from its hardcoded range, which gives the gym counties their settlements first; (c) the remaining settlements, distributed among the other counties, then populations and special locations (with arc-beat reservations); (d) the other items' hosts from their masks.
5. Each county: the details of its own region tiles (including exact route placement) and its route-story-indices, from region layers 1-4 only. No county layer depends on any other county's data, including an adjacent county's route-story-indices. Counties may have sub-layers like the region map.
6. Field maps: generated from region layers 1-4 and the county they are inside, never from another county's data or from adjacent field maps. The exception is inter-county routes, which may use the data of the two counties they connect.
7. Further layers continue the same pattern.

This forces single-pass generation with no recursion or transitive operations, and keeps memory use minimal. Any new rule must be checked against it: a rule that needs data from a later layer, or from a sibling's same-layer output, breaks the design.

## Documents

Detail lives in topic files; this file keeps the principles, the development order and the cross-cutting decisions. A quoted section name elsewhere (`"Main plot planner"`) is a heading in the file listed here. Every file ends with its own "Alternatives considered".

| File | Covers |
|---|---|
| `design_region.md` | The 6x4 county grid: county layout, reserved legendary locations, Enclaves, settlement types, city themes, gym type order, special locations (landmark, detour, pitstop), county-story-index assignment (with Ports and the Kanto reference pattern), route generation, route story indices, geography, landforms, biomes. |
| `design_data.md` | How region data is stored and read: determinism and seeds, county headers, region tiles, city data, the region dump, multiple regions. |
| `design_pokemon.md` | Everything about Pokémon and battle: biome type affinity, the species-to-biome mapping, species metadata, route encounter selection, trainer teams and levels, experience, catch-rate floor, legendaries and starters, move pools, shops and trading, type-recolored sprites, type icons, battle damage estimates, the team analyzer, gym guide advice. |
| `design_story.md` | The main story: arcs, arc progression, the main plot planner (steps, items, unlock gating, Surf, McGuffin kinds, the soft-lock proof's inputs), story state storage, McGuffins, the story graph and the soft-lock argument. |
| `design_subplots.md` | Subplots as data: scope and buckets, placement and requirements, the descriptor/accessor/adapter interfaces, text references, rules from paper tests, building roles and NPC spawning, county planning, plot kinds, the county chain. |
| `design_scenes.md` | Actor placement and budgets, and the scene runner: triggers, the step vocabulary, the worked Route 101 example. |
| `design_city_npcs.md` | City planning steps, hinter NPCs, hint texts and beat anchors, main-beat hints, route blockers, Pokémon Center news, generic NPC dialogue. |
| `design_dev.md` | The base codebase, config choices that shape the design, story mode, and the end-to-end test tier. |

Other files: `Emerald.md` (the hardcoded Emerald stage's data and walkthrough analysis), `Kanto.md`, `vars.md` (var inventory), `remaining_work.md` (open items), `notes.md` (history and gotchas).

## Resource budget

- ROM space is not expected to be a problem. The extra per-species and per-move metadata costs tens of thousands of bytes, but the hand-authored learnset/move tables, most maps, and encounter tables are dropped.
- RAM is scarce (hence the bitpacking and regeneration from the player-ID seed).
- CPU time is the bigger hurdle: on-demand regeneration of the region, species mapping, encounter filtering, and learn-level move picks must be cheap enough for a GBA.

## Development approach: hardcode first, then randomize

The region starts as a hardcoded Pokémon Emerald region and is progressively abstracted and randomized, rather than building the generator first. Hardcoded content exercises the same metadata the generator will later produce, and keeps the story testable throughout.

Why this order: if NPCs don't talk about subplots and story arcs, testers can't tell how to progress, which would cripple testing; and subplot babble requires city/route/location metadata anyway. NPC talk exists to support subplot shuffling, so its interfaces (descriptor, stage accessor, text references; see "Subplot abstraction" (design_subplots.md)) are defined first, for the end state, and NPC talk is built on them before any subplot is ported.

Stages, each leaving the game playable:

1. Divide the Emerald region map(s) into the 6x4 counties with hardcoded terrain type (mountainous, ocean, flat), biome, population, special location, story order, McGuffin keys and locks, and capital (Lilycove City). A key/lock is basically anywhere story advancement moves from one county to a non-adjacent county. The story-arc id is also hardcoded.
2. Hardcode the region tiles within each county: which are cities, routes, and special locations.
3. For each city and special location, hardcode its named buildings, subplot McGuffins, and city "theme"; hardcode which cities have gyms, the leader's name, and the gym type.
4. Define the subplot interfaces (see "Subplot abstraction" (design_subplots.md)), then implement the "random" NPC talking on them: some NPCs give story-arc state text, some give city/location subplot McGuffins, some talk about named buildings, the gym type, gym leader, or the city's theme. Remaining NPCs talk about their immediate vicinity or random text.
5. Abstract the subplots per city/location, add more, and randomize them; likewise abstract story-arc advancement (less visible, since region tiles keep hardcoded story indices).
6. With subplots no longer fixed, building locations are no longer fixed: randomize city layouts within a region tile (except special buildings).
7. Abstract McGuffins and make them randomizable; until then, shuffling is limited by McGuffins and encounter tables.
8. Implement randomized encounter tables by the route's biome.
9. Shuffle city locations within a county, and which special locations are in each county.
10. Shuffle the counties' story indices.
11. Randomize terrain types and biomes (the last randomizable map element).

Random Pokémon types and move sets can be implemented at virtually any point in this order.

The work for each step is broken into milestones 0-12 in `remaining_work.md`. Milestone 0 covers the hardcoded foundation (stages 1-3); Milestone 1a defines the subplot interfaces and ports the first subplot; the milestones then follow the order: shuffled NPC texts, shuffled subplots, non-fixed arc cutscenes, random types, encounters, moves, trainers, procedural buildings and terrain, shuffled McGuffins, randomized city placement, procedural story indices, procedural terrain and biomes.

### Randomization switches

Each randomization sits behind one `RANDOMIZE_*` bit (`include/constants/randomization.h`), read with `IsRandomized()` (`src/util/randomization.c`); with the bit off, that path uses vanilla Emerald data. The mask is a `u16` in `SaveBlock2` (the former `padding2`), so it survives save and load and can later become a New Game setting; a new game copies `RANDOMIZATION_NEW_GAME_DEFAULT` (`include/config/randomization.h`). A randomization is built with its bit off, so existing tests keep passing; one commit then turns it on in the default and updates the tests' expected data, with the vanilla path kept.

Tests: the runner zeroes the save blocks before each test, so every bit is off and the player identity is fixed (OT ID 0, all-zero name), which makes the randomized path reproducible. A test opts in with `SetRandomized(bit, TRUE)`. Seed sweeps (`include/test/seed_sweep.h`) loop `FOR_EACH_SWEEP_SEED(i, seed)`, which writes a fixed, per-index OT ID and EOS-padded name into the save; a failing sweep test prints its index, and `SetSweepIdentity(i)` reproduces it.

### Emerald county assignment (stage 1)

Hoenn is fudged into the 6x4 county grid for the hardcoded stage; the county, terrain and biome tables are in `Emerald.md`.

## Alternatives considered

### Generator-first development

Build the procedural region generator first and hand-author content to fit it.

Pros:
- Matches the end goal directly; no throwaway hardcoded layer.
- Finds generator performance problems early.

Cons:
- Story progression can't be tested without NPC hints, which need subplot/city metadata the generator would have to produce first.
- Everything must work before anything is playable.
- Random layouts make bugs hard to reproduce.
