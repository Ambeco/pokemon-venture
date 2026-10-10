# Remaining Work

- Collect the overall plan for the mod from the user and record it in `design.md`.
- Rewrite `README.md` as the fork's own README (currently still upstream's).
- Pin down world generation details: terrain/biome/population/special-location types, story-order algorithm, key/lock rules, capital selection, and how generated data maps onto pokeemerald maps, warps, and encounters.
- Verify the real world-map tile grid size and set county size (~5x5 tiles) and the 6x4 county grid to fit it.
- Clarify route rules: tie-breaking and duplicate-route handling when several adjacency rules apply to the same county pair.
- Define the special-location catalog (inside-city vs outside-city lists, how a location is chosen for a county, relation to terrain/biome/story order).
- Decide how to integrate the underground tunnels around Saffron (Underground Path) into the world model; keep Red/Blue map reproducibility as a design check.
- Define the 5x5 biome grid (axis values, the 25 biomes, which 16 each 4x4 subset yields), the population-level thresholds for the 2-bit encoding, and whether the world is regenerated on demand vs cached in RAM (cost of regeneration; player ID is the seed, so check it is stable across the whole game, e.g. vs. trainer ID rerolls in the debug menu).
