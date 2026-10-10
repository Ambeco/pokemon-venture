# Remaining Work

- Collect the overall plan for the mod from the user and record it in `design.md`.
- Rewrite `README.md` as the fork's own README (currently still upstream's).
- Pin down world generation details: terrain/biome/population/special-location types, story-order algorithm, key/lock rules, capital selection, and how generated data maps onto pokeemerald maps, warps, and encounters.
- Verify the real world-map tile grid size and set county size (~5x5 tiles) and the 6x4 county grid to fit it.
- Clarify route rules: behavior when population is below the city threshold (no city: which location do routes attach to?), whether the "highest story index" fallback is evaluated among adjacent counties only, and tie-breaking/duplicate-route handling.
