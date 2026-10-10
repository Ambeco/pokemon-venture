# Remaining Work

- Collect the overall plan for the mod from the user and record it in `design.md`.
- Rewrite `README.md` as the fork's own README (currently still upstream's).
- Pin down world generation details: terrain/biome/population/special-location types, story-order algorithm, key/lock rules, capital selection, and how generated data maps onto pokeemerald maps, warps, and encounters.
- Verify the real world-map tile grid size and set county size (~5x5 tiles) and the 6x4 county grid to fit it.
- Clarify route rules: tie-breaking and duplicate-route handling when several adjacency rules apply to the same county pair.
- Define the special-location catalog (inside-city vs outside-city lists, how a location is chosen for a county, relation to terrain/biome/story order). Note: examples given from Red/Blue use Kanto names; the first list's "Dark Tunnel"/"Whirlpool Islands" were recorded as Diglett's Cave/Seafoam Islands equivalents — confirm.
